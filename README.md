# System3 for SDL2

This is an SDL2 port of
[System3 for Win32](http://takeda-toshiya.my.coocan.jp/alice/) by Takeda
Toshiya. It supports multiple platforms, including Android and Emscripten.

## Compatibility

See the [game compatibility table](game_compatibility.md) for a list of games
that can be played with system3-sdl2.

## Installing

For Windows and Android, you can download pre-built binaries from the
[Releases](https://github.com/kichikuou/system3-sdl2/releases) page.

For other platforms, you need to build the program from source. See the
[Building](#building) section for instructions.

Note for Windows:
- The 64-bit version supports Windows 10 or later. For older versions of
  Windows, please use the 32-bit version.

## Running

- Windows: Copy `system3.exe` to the game folder and double-click it.
- Android: Install the APK file and follow the on-screen instructions.
- Other platforms: Run `system3` from within the game directory.

### Options

`system3` supports the following command line options:

#### `-noantialias`
Disables text anti-aliasing.

#### `-fontfile` _filename_
Specifies the font file used for rendering text. Both `.ttf` and `.otf` files
are supported.

#### `-playlist` _filename_
_filename_ is a text file that lists the audio files to play in lieu of CD
audio tracks, one per line. For example:

```plaintext
# This line is ignored
BGM/track02.mp3
BGM/track03.mp3
...
```
The first line is not used because track 1 on a game CD is usually a data
track.

#### `-fm`
By default, system3-sdl2 uses MIDI sound if available. This option forces FM
tone generator emulation.

#### `-mididevice` _number_
Specifies the MIDI device number to use. If not specified, the first available
device is used.

#### `-texthook` _mode_
Specifies the text hook mode. This option is used for capturing in-game text
for translation or other purposes. The available modes are:
- `none`: Disable text hooking.
- `print`: Print the captured text to the console.
- `copy`: Copy the captured text to the clipboard.

#### `-texthook_suppress` _list_
Suppress text hook on specified scenario pages. _list_ is a comma-separated list
of page numbers.

#### `-trace`
Enables trace mode, which prints debug information to the console.

#### `-game` _game_id_
As System1-3 have slight variations depending on the game, `system3` uses the
fingerprint of the scenario file (ADISK.DAT) to identify the game being played.
This option allows you to override this detection, which is useful when running
patched games.

See [`game_compatibility.md`](game_compatibility.md) for a list of game IDs
and their corresponding titles.

### Configuration File `system3.ini`

Every option that can be set via command line flags can also be configured
through the `system3.ini` file located in the game folder. Refer to
[`system3.ini.example`](system3.ini.example) for the file format and available
options. Options specified on the command line will override those in
`system3.ini`.

## Debugging

system3-sdl2 has a built-in debugger that allows you to step through the game
and examine or modify game variables. There are two ways to use the debugger:

- Through [Visual Studio Code](https://code.visualstudio.com/) (recommended):
  The [vscode-system3x](https://github.com/kichikuou/vscode-system3x) extension
  provides a graphical debugging interface for System 1-3.
- Using the CLI Debugger: Running system3 with the `-debugger cli` option will
  launch the debugger with a console interface. Type `help` to see a list of
  available commands.

## Localizing a Game

While the original System 1-3 only supported Shift_JIS (a Japanese character
encoding), system3-sdl2 supports Unicode and can run games translated into
languages other than Japanese and English.

For instructions on how to build a game with Unicode support, see the
[sys3c](https://github.com/kichikuou/sys3c) documentation.

When running a modified (translated) game, system3 cannot automatically detect
the game ID. You need to specify the `game` option in `system3.ini`.

## Building from Source

### Linux (Debian, Ubuntu)

```bash
$ git submodule update --init
$ sudo apt install g++ cmake libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev librtmidi-dev nlohmann-json3-dev
$ mkdir -p out/debug
$ cd out/debug
$ cmake -DCMAKE_BUILD_TYPE=Debug ../../
$ make
$ sudo make install
```

### MacOS

```bash
$ git submodule update --init
$ brew install cmake pkg-config sdl2 sdl2_ttf sdl2_mixer rtmidi nlohmann-json
$ mkdir -p out/debug
$ cd out/debug
$ cmake -DCMAKE_BUILD_TYPE=Debug ../../
$ make
$ sudo make install
```

### Windows (MSYS2)

```bash
$ git submodule update --init
$ pacman -S make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_ttf mingw-w64-ucrt-x86_64-rtmidi mingw-w64-ucrt-x86_64-nlohmann-json
$ mkdir -p out/debug
$ cd out/debug
$ cmake -G"MSYS Makefiles" -DCMAKE_BUILD_TYPE=Debug ../../
$ make
```

### Windows (Microsoft Visual Studio)

- Visual Studio 2022 can be used to clone this repository and will
  automatically clone submodules as well.
- Install [CMake](https://cmake.org/download/). (The CMake integration in
  Visual Studio does not work.)
- In the CMake GUI, press the "Browse Source..." button and select the root
  folder of this repository.
- Press the "Browse Build..." button. Create a new folder (e.g., `out`) under
  the top-level directory of the repository and select it.
- Press the "Configure" button. Specify the generator for your version of
  Visual Studio and click "Finish."
- Press the "Generate" button.
- A `System3.sln` file should be generated in the build folder. Open it with
  Visual Studio.

### Emscripten

```bash
$ git submodule update --init
$ mkdir -p out/wasm
$ cd out/wasm
$ emcmake cmake -DCMAKE_BUILD_TYPE=Release ../../
$ make
```

### Android

See [android/README.md](android/README.md).

### Nintendo Switch

See [switch/README.md](switch/README.md).
