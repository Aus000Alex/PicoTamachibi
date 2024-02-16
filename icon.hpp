#ifndef ICON_HPP
#define ICON_HPP
#include "framebuffer.hpp"
#include "asyncman.hpp"
#include "basic-coro/SingleEvent.hpp"
#include "basic-coro/AwaitableTask.hpp"

#include <etl/string_view.h>
#include <etl/vector.h>
#include <etl/variant.h>
#include <etl/hash.h>
#include <etl/bitset.h>


class Image {
    friend class Animation;

    Framebuffer image;
    bool inverted = false;
    size_t name_hash;

public:
    Image() : image(0, 0) {}
    Image(const char *filename, unsigned width = 16, unsigned height = 16, etl::string_view name = "Empty")
          : image(width, height) {
        name_hash = etl::hash<etl::string_view>()(name);
        set_framebuffer(load(filename));
    }

    bool operator ==(etl::string_view v) const {
        return etl::hash<etl::string_view>()(v) == name_hash;
    }

    const Framebuffer& get_framebuffer() const {
        return image;
    }
    void set_framebuffer(const Framebuffer::ROData& buf);

    bool is_inverted() const {
        return inverted;
    }
    void set_inverted(bool v);

    Framebuffer::ROData load(const char *filename);
};


class Icon : public Image, public AsyncObject {
    void on_tick() override;

public:
    unsigned x, y;

    Icon(AsyncMan& aman, const char *filename, unsigned x = 0, unsigned y = 0, unsigned width = 16, unsigned height = 16, etl::string_view name = "Empty")
          : Image(filename, width, height, name), AsyncObject(aman), x(x), y(y) {}
};


class Animation final : public AsyncObject {
public:
    enum AnimationSpeed {
        very_slow,
        slow,
        normal,
        fast
    };

    enum AnimationType {
        default_,
        loop,
        reverse,
        bounce
    };

private:
    enum Flags {
        done,
        pause_when_done,
        flag_count
    };

    etl::vector<Image, 16> frames;
    AnimationType type = default_;

    unsigned frame_index,
             step;
    etl::bitset<flag_count> flags;

    basiccoro::SingleEvent<void> on_done;

    void load(const char *filename, unsigned width, unsigned height);
    void update_frame_index();

    void on_tick() override;
    void on_activate() override;

public:
    AnimationSpeed speed = normal;

    unsigned short repeats = -1;
    unsigned x,
             y;

    Animation(AsyncMan& aman, const char *filename = NULL, AnimationType animation_type = default_, unsigned x = 0, unsigned y = 0, unsigned width = 16, unsigned height = 16, etl::vector<Image, 16>&& frames = {});
    Animation(const Animation&) = delete;
    Animation(Animation&&) = delete;

    AnimationType get_type() const {
        return type;
    }
    void set_type(AnimationType v) {
        type = v;
        reset();
    }

    unsigned get_frame_count() const {
        return frames.size();
    }

    bool is_done() const {
        return flags.test(done);
    }

    void reset() {
        step = 0;
        flags.reset(done);
        update_frame_index();
    }

    void set_pause_when_done(bool v) {
        flags.set(pause_when_done, v);
    }
    bool get_pause_when_done() const {
        return flags.test(pause_when_done);
    }

    const Image& get_current_image() const {
        return frames[frame_index];
    }

    basiccoro::AwaitableTask<void> wait_done();
};


using OptionallyAnimatedIconBase = etl::variant<Image, Animation>;

class OptionallyAnimatedIcon : public OptionallyAnimatedIconBase {
public:
    using OptionallyAnimatedIconBase::OptionallyAnimatedIconBase;

    bool operator ==(etl::string_view) const;
    void set_inverted(bool v);
};


class Toolbar final : public AsyncObject {
    etl::vector<OptionallyAnimatedIcon, 14> icons;
    int selection_index;

    void on_tick() override;

public:
    constexpr static unsigned spacer = 2;

    Toolbar(AsyncMan& aman, decltype(icons)&& images, unsigned initial_index = 0)
          : AsyncObject(aman), icons(std::move(images)), selection_index(initial_index) {
        this->icons[selection_index].set_inverted(true);
    }

    Toolbar(const Toolbar&) = delete;
    Toolbar(Toolbar&& o)
        : AsyncObject(o.get_async_manager()), icons(etl::move(o.icons)), selection_index(o.selection_index) {}

    bool operator ==(etl::string_view v) const {
        return icons[selection_index] == v;
    }

    etl::span<OptionallyAnimatedIcon> get_icons() {
        return icons;
    }

    void set_selection_index(unsigned new_index) {
        icons[selection_index].set_inverted(false);
        icons[new_index].set_inverted(true);

        selection_index = new_index;
    }

    void refresh_selection() {
        icons[selection_index].set_inverted(true);
    }

    void next() {
        auto index = selection_index;
        if (++index == icons.size())
            index = 0;

        set_selection_index(index);
    }
    void previous() {
        auto index = selection_index;
        if (--index < 0)
            index = icons.size()-1;

        set_selection_index(index);
    }
};
#endif // ICON_HPP
