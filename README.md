# GBA Jam 2024

Fun little experiments messing around with C++ for [GBA Jam 2024](https://itch.io/jam/gbajam24).

## Development

### Requirements

* This project has the same requirements as [Butano](https://github.com/GValiente/butano), follow getting started instructions [here](https://gvaliente.github.io/butano/getting_started.html).
    * GBA Emulator (using [mGBA](https://mgba.io/) for this project).
    * [devkitARM](https://devkitpro.org/wiki/Getting_Started).
    * Python (use [pyenv](https://github.com/pyenv-win/pyenv-win)).
    * Butano (is used as a submodule in this repo).

### Extra Tools Used
* [Usenti](https://www.coranac.com/projects/usenti/) for sprite editing in format supported by the GBA.
* [VS Code](https://code.visualstudio.com/) used as IDE, this repo contains configuration to use it.

#### Guides Used 
* [Butano Getting Started](https://gvaliente.github.io/butano/getting_started.html).
* [GDB Debugging with mGBA](https://felixjones.co.uk/mgba_gdb/vscode.html).

## Instructions

1. Clone the repo and `cd` into it.
2. `git submodule update --init` to pull Butano submodule.
3. `cd` into `src` this is where the actual GBA project is stored.
3. `make -j$(nproc)` to build using the makefile.
4. Open the output `.gba` file with your emulator to test.