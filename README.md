# System3 for SDL2

武田俊也さんの [System3 for Win32](http://takeda-toshiya.my.coocan.jp/alice/) をSDL2に移植して、emscriptenでコンパイルできるようにしたものです。

## ビルド方法

### Linux (Debian, Ubuntu)

    $ sudo apt install cmake libsdl2-dev libsdl-ttf2.0-0
    $ mkdir -p out/debug
    $ cd out/debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ../../src/
    $ make
    $ sudo make install

### MacOS

    $ brew install cmake pkg-config sdl2 sdl2_ttf
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

### Android

[android/README.md](android/) を参照してください。

## 実行方法

ビルドすると4つの実行可能ファイル `system1`, `system2`, `system3`, `prog_omake` が生成されます。実行ファイルとゲームとの対応は以下のとおりです。

- system1
  - クレセントムーンがぁる
  - D.P.S. - Dream Program System
  - 婦警さんＶＸ (ALICEの館3 UNITBASE/Y_SYUREN.LZH)
  - Intruder -桜屋敷の探索-
  - あぶないてんぐ伝説
  - Little Vampire
- system2
  - あゆみちゃん物語 PROTO
  - Super D.P.S
  - Prostudent -G- (FD)
- system3
  - AmbivalenZ
  - DPS全部
  - Funny Bee
  - Only You
  - Prostudent -G- (CD)
  - Rance 4.1
  - Rance 4.2
  - あゆみちゃん物語 (CD)
  - あゆみちゃん物語 実写版
  - あゆみちゃん物語 フルカラー実写版
  - アリスの館３
  - 走り女２ (Rance 4.x ヒントディスク)
  - 闘神都市２ 〜そして、それから〜
  - 乙女戦記
  - 人魚 -蘿子-
  - 夢幻泡影
- prog_omake
  - prostudent G おまけ

Emscripten版を実行するには、[鬼畜王 on Webのリポジトリ](https://github.com/kichikuou/web)をチェックアウトして、`docs`ディレクトリに `out/wasm/system3.*` をすべてコピーしてください。
