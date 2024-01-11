# System3 for Switch

## Build
Prerequisites:
- CMake >=3.20
- devkitPro environment ([how to install](https://devkitpro.org/wiki/devkitPro_pacman))
- Nintendo Switch development tools package (switch-dev)
- SDL2, SDL2_Mixer and SDL2_ttf Switch ports (switch-sdl2, switch-sdl2_ttf, switch-sdl2_mixer)

```sh
git submodule update --init
sudo (dkp-)pacman -S switch-dev switch-sdl2 switch-sdl2_ttf switch-sdl2_mixer
mkdir -p out/debug
cd out/debug
/opt/devkitpro/portlibs/switch/bin/aarch64-none-elf-cmake -DCMAKE_BUILD_TYPE=Debug ../../
make
```

## Use
1. Copy the resulting `system3.nro` file to a folder inside your Nintendo Switch's SD card (eg. `/switch/[game name]/`)
2. Copy all the game files, BGM files and configuration.
3. Open system3 through the Homebrew Launcher

## Notes
- The Switch needs its own MIDI Sound Library (eg. freepats, configurable through `timiditycfg` on the `system3.ini` config) due to the lack of built-in MIDI files.
