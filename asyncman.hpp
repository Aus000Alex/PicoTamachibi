#ifndef ASYNCMAN_HPP
#define ASYNCMAN_HPP
#include <functional>
#include <etl/vector.h>
#include <etl/limits.h>


class AsyncMan {
public:
    using HandleID = uint8_t;
    using HandleCb = std::function<void()>;

    class Handle {
        friend AsyncMan;

        HandleID id = 0;
        bool active = true;

        Handle(HandleID id) : id(id) {}

    public:
        class AsyncObject *object = nullptr;

        HandleID get_id() const {
            return id;
        }

        bool is_active() const {
            return active;
        }
        void set_active(bool v);

        bool is_valid() const {
            return id != 0 && object != nullptr;
        }
    };

private:
    constexpr static HandleID max_handle_id = etl::numeric_limits<HandleID>::max();

    etl::vector<Handle, 16> handles;

    HandleID find_free_id() const;

public:
    AsyncMan() {}
    AsyncMan(const AsyncMan&) = delete;
    AsyncMan(AsyncMan&&) = delete;

    Handle& new_handle() {
        Handle fres(find_free_id());
        return handles.emplace_back(std::move(fres));
    }
    void delete_handle(HandleID id);

    Handle& get_handle(HandleID id);

    void tick();
};


class UniqueAsyncManHandle {
    AsyncMan& man;

    AsyncMan::HandleID id;

public:
    UniqueAsyncManHandle(AsyncMan& man, bool active = false, AsyncObject *object = nullptr) : man(man) {
        auto& handle = man.new_handle();
        id = handle.get_id();
        handle.object = object;
        handle.set_active(active);
    }
    ~UniqueAsyncManHandle() {
        man.delete_handle(id);
    }
    UniqueAsyncManHandle(const UniqueAsyncManHandle& o) : man(o.man) {
        auto& handle = o.man.new_handle();
        id = handle.get_id();
        handle.set_active(o->is_active());
    }
    UniqueAsyncManHandle(UniqueAsyncManHandle&& o) : man(o.man), id(o.id) {
        o.id = 0;
    }

    AsyncMan::Handle& operator *() {
        return man.get_handle(id);
    }
    AsyncMan::Handle *operator ->() {
        return &man.get_handle(id);
    }

    const AsyncMan::Handle& operator *() const {
        return man.get_handle(id);
    }
    const AsyncMan::Handle *operator ->() const {
        return &man.get_handle(id);
    }

    AsyncMan& get_async_manager() const {
        return man;
    }
    unsigned get_handle_id() const {
        return id;
    }
};


class AsyncObject {
    friend AsyncMan;

    virtual void on_tick() = 0;
    virtual void on_activate() {}
    virtual void on_deactivate() {}

public:
    UniqueAsyncManHandle handle;

    AsyncObject(AsyncMan& man, bool active = true) : handle(man, active, this) {}

    AsyncObject(const AsyncObject& o) : handle(o.handle) {
        handle->object = this;
    }
    AsyncObject(AsyncObject&&) = delete;

    virtual ~AsyncObject() {}

    AsyncMan::Handle& operator *() {
        return get_async_handle();
    }
    AsyncMan::Handle *operator ->() {
        return &get_async_handle();
    }

    const AsyncMan::Handle& operator *() const {
        return get_async_handle();
    }
    const AsyncMan::Handle *operator ->() const {
        return &get_async_handle();
    }

    AsyncMan& get_async_manager() const {
        return handle.get_async_manager();
    }
    AsyncMan::HandleID get_async_handle_id() const {
        return handle.get_handle_id();
    }
    const AsyncMan::Handle& get_async_handle() const {
        return *handle;
    }
    AsyncMan::Handle& get_async_handle() {
        return *handle;
    }
};


class AsyncSelector {
    AsyncObject *current;

public:
    AsyncSelector(AsyncObject& object)
          : current(&object) {
        (*current)->set_active(true);
    }
    ~AsyncSelector() {
        (*current)->set_active(false);
    }

    // This should be stacking and based on priorities to prevent conflicts
    auto operator =(AsyncObject& object) {
        (*current)->set_active(false);
        current = &object;
        (*current)->set_active(true);
    }

    operator AsyncObject &() const {
        return *current;
    }

    bool operator ==(const auto& o) const {
        return current == static_cast<const AsyncObject*>(&o);
    }
};

#endif // ASYNCMAN_HPP