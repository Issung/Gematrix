# GBA Jam 2024

My first and hopefully not last Game Boy Advance game! Written in C++ with Butano for [GBA Jam 2024](https://itch.io/jam/gbajam24).

## Credits
* [Butano](https://github.com/GValiente/butano) by [GValiente](https://github.com/GValiente/butano).
* Font by [2Pblog1](https://x.com/2pblog1).
* Menu music: [pms_are1.it by Powerack](https://modarchive.org/index.php?request=view_by_moduleid&query=162054)
* Game music #1: [4_RNDD!.xm by Drozerix](https://modarchive.org/index.php?request=view_by_moduleid&query=172898).
* Game music #2: [biotech.rth by Kokesz](https://modarchive.org/index.php?request=view_by_moduleid&query=174348).
* Game music #3: [Cirno.it by Maris](https://modarchive.org/index.php?request=view_by_moduleid&query=185072).
* Game music #4: [Fckdarules.xm by JAM](https://modarchive.org/index.php?request=view_by_moduleid&query=169181).
* Game music #5: [L3V3L_33 by Drozerix](https://modarchive.org/index.php?request=view_by_moduleid&query=172183).
* SFX made with [Bfxr](https://www.bfxr.net/).

## Development

### Requirements

* This project has the same requirements as [Butano](https://github.com/GValiente/butano), follow getting started instructions [here](https://gvaliente.github.io/butano/getting_started.html).
    * GBA Emulator (using [mGBA](https://mgba.io/) for this project).
    * [devkitARM](https://devkitpro.org/wiki/Getting_Started).
    * Python (use [pyenv](https://github.com/pyenv-win/pyenv-win)).
    * [Butano](https://github.com/GValiente/butano) (imported as a submodule).

### Extra Tools Used
* [Usenti](https://www.coranac.com/projects/usenti/) for sprite editing in format supported by the GBA.
* [VS Code](https://code.visualstudio.com/) used as IDE, this repo contains configuration to use it.

#### Guides Used 
* [Butano Getting Started](https://gvaliente.github.io/butano/getting_started.html).
* [GDB Debugging with mGBA](https://felixjones.co.uk/mgba_gdb/vscode.html).
* Help from the lovely folks in the gbadev Discord.

### Build / Debug
1. Clone the repo and `cd` into it.
2. `git submodule update --init` to pull Butano submodule.
3. `cd` into `src` this is where the actual GBA project is stored.
3. `make -j$(nproc)` to build using the makefile.
4. Open the output `.gba` file with your emulator to play.

#### Debugging:
* The `.elf` file contains all the debug symbols, run this to debug.
    * This is what is launched by VSCode debugging, specified in launch.json.
* If you want a better debugging experience you can play in the Makefile.
    * Set USERFLAGS to `-Og` to disable most optimisations.