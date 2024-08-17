# PicoTamachibi
A Tamagotchi like toy based on PRI Pico

- The code is based on a Project from [kevinmcaleer](https://www.youtube.com/@kevinmcaleer28), the [original code can be found on his Github page](https://github.com/kevinmcaleer/picotamachibi)
- Used only for educational purposes
- *Folders:*
- contains needed libraries
- pcb_tamachibi_1-2 contains the KiCad- & gerber-files needed to create the PCBs for this project (not needed inside the Picos storage to run the project)
- raw_assets contains the graphic assets for this project (not needed inside the Picos storage to run the project)


## Setup with VSCode

It is recommended to create a new vscode profile for this type of project to have a clean editor.
VSCode will recommend extensions for this project, all of them are required to run the project.

If the console says "MemoryError: memory allocation failed, allocating 81920 bytes" on upload, the Pico needs to be reset.

Please run `> MicroPico > Configure Project` command via `Ctrl+Shift+P` the first time you open the project.