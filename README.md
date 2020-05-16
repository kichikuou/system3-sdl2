# System3 for SDL2

This is a SDL2 port of [System3 for Win32](http://takeda-toshiya.my.coocan.jp/alice/) by Takeda Toshiya that supports multiple platforms, including Android and Emscripten.

## Building

### Linux (Debian, Ubuntu)

    $ sudo apt install g++ cmake libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev
    $ mkdir -p out/debug
    $ cd out/debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ../../src/
    $ make
    $ sudo make install

### MacOS

    $ brew install cmake pkg-config sdl2 sdl2_ttf sdl2_mixer
    $ mkdir -p out/debug
    $ cd out/debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ../../src/
    $ make
    $ sudo make install

### Windows (MSYS2 mingw64)

    $ pacman -S cmake mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf
    $ mkdir -p out/debug
    $ cd out/debug
    $ cmake -G"MSYS Makefiles" -DCMAKE_BUILD_TYPE=Debug ../../src/
    $ make

### Emscripten

    $ mkdir -p out/wasm
    $ cd out/wasm
    $ emcmake cmake -DCMAKE_BUILD_TYPE=MinSizeRel ../../src/
    $ make

To use the Emscripten build, check out https://github.com/kichikuou/web and copy `out/wasm/system3.*` into its `docs` directory.

### Android

See [android/README.md](android/).

## Running
Usage:
```
system3 [options]
```

### Options

#### `-antialias`
Enables text anti-aliasing.

#### `-fontfile` _filename_
Specifies a font file used to render text. `.ttf` and `.otf` files are supported.

#### `-playlist` _filename_
_filename_ is a text file that specifies audio files to be played instead of CD audio tracks, one per line. For example:

```
# This line is ignored
BGM/track02.mp3
BGM/track03.mp3
...
```
The first line is not used, because track 1 of game CD is usually a data track.

#### `-game` _game_id_
Since System1-3 behave slightly differently depending on the game, `system3` uses fingerprint of the scenario file (ADISK.DAT) to determine which game you are playing. This option allows you to override this. This is useful when running patched games.

Here's the list of available game IDs and corresponding titles:
| game_id | Title |
----------|--------
| `crescent` | クレセントムーンがぁる |
| `dps` | D.P.S - Dream Program System |
| `fukei` | 婦警さんＶＸ |
| `intruder` | Intruder -桜屋敷の探索- |
| `tengu` | あぶないてんぐ伝説 |
| `little_vampire` | Little Vampire |
| `ayumi_proto` | あゆみちゃん物語 PROTO |
| `sdps_maria` | Super D.P.S マリアとカンパン |
| `sdps_tono` | Super D.P.S 遠野の森 |
| `sdps_kaizoku` | Super D.P.S 海賊家業 |
| `prog_fd` | Prostudent G (FD) |
| `ambivalenz_fd` | AmbivalenZ −二律背反− (FD) |
| `ambivalenz_cd` | AmbivalenZ −二律背反− (CD) |
| `dps_all` | D.P.S. 全部 |
| `funnybee_cd` | 宇宙快盗ファニーBee (CD) |
| `funnybee_fd` | 宇宙快盗ファニーBee (FD) |
| `onlyyou` | Only You −世紀末のジュリエット達− |
| `onlyyou_demo` | Only You −世紀末のジュリエット達− デモ版 |
| `prog_cd` | Prostudent G (CD) |
| `prog_omake` | Prostudent G おまけ |
| `rance41` | ランス 4.1 〜お薬工場を救え！〜 |
| `rance42` | ランス 4.2 〜エンジェル組〜 |
| `ayumi_cd` | あゆみちゃん物語 (CD) |
| `ayumi_live_256` | あゆみちゃん物語 実写版 |
| `ayumi_live_full` | あゆみちゃん物語 フルカラー実写版 |
| `yakata3_cd` | アリスの館3 (CD) |
| `yakata3_fd` | アリスの館3 (FD) |
| `hashirionna2` | 走り女2 |
| `toushin2_sp` | 闘神都市2 そして、それから… |
| `otome` | 乙女戦記 |
| `ningyo` | 人魚 -蘿子- |
| `mugen` | 夢幻泡影 |
