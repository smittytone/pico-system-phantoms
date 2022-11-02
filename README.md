# Phantom Slayer for the Pimoroni PicoSystem 1.1.2

A retro-style 3D arcade game written in C++.

The original Phantom Slayer was created Ken Kalish for Med Systems. It was written in 6809 machine code for the Tandy Color Computer and was compatible with the Dragon 32 micro.

![Phantom Slayer for the Pimoroni PicoSystem](images/phantoms-ps-001.jpg)

For more details on the game [see this page](https://smittytone.net/pico-phantoms/).

#### Requirements

* [Pimoroni PicoSystem](https://shop.pimoroni.com/products/picosystem).

### The Code

Build from source code, or copy `bin/phantoms.uf2` to your PicoSystem. Please check the SHA 256 first:

```
e1a01d002e47584dba17c9c448e8afa15979c7f79db5995fac63473b2e619df5
```

### Build the Code

This code requires the installation of the [Pico SDK](https://github.com/raspberrypi/pico-sdk) and the [PicoSystem SDK](https://github.com/pimoroni/picosystem).

* Use Visual Studio Code with the [CMake](https://marketplace.visualstudio.com/items?itemName=twxs.cmake) and [CMakeTools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) extensions:
    1. Click **No active kit** to select your compiler, `GCC 10.3.1 arm-none-eabi`:<br />![Select a kit in VSCode](images/kits.png)
    1. Click **Build**:<br />![The VSCode Toolbar](images/vscode.png)
* From the command line:
    1. `cd <YOUR_GIT_DIRECTORY>/pico-system-phantoms`
    1. `cmake -S . -B build/`
    1. `cmake --build build --clean-first`

### The Game

See [this blog post for full details](https://blog.smittytone.net/2021/03/26/3d-arcade-action-courtesy-of-raspberry-pi-pico/).

Read the [original game instructions](original-instructions.md).

![Phantom Slayer for the Pimoroni PicoSystem](images/phantoms-ps-002.jpg)

### To Do

* Better sound!

### Credits

This games is based on a 1982 original created by Ken Kalish of Med Systems for the Tandy Color Computer. The design is Ken’s; the code is mine, and I’ve taken only a few liberties with certain gameplay and visual details.

### Release Notes

* 1.1.2 *Unreleased*
    * Fix some issue introduced by SDK.
    * Better randomisation.
    * Refactoring.
* 1.1.1 *29 November 2021*
    * Bug fixes:
        * Phantoms' health could become huge -- fixed.
        * Dead phantoms could move -- fixed.
        * Refactor Phantom generation, movement.
* 1.1.0 *19 November 2021*
    * Initial release.