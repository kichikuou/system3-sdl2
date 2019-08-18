# System3 for SDL2

武田俊也さんの [System3 for Win32](http://takeda-toshiya.my.coocan.jp/alice/) をSDL2に移植して、emscriptenでコンパイルできるようにしたものです。

## ビルド方法

[CMake](https://cmake.org/) が必要です。

必要なライブラリ (SDL2, SDL_ttf) をインストールして、

    $ mkdir out
    $ cd out
    $ cmake -DCMAKE_BUILD_TYPE=Debug ../src/
    $ make && make install

でコンパイル・インストールできます。

実行ファイルとゲームとの対応は以下のとおりです。

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

Emscripten版を実行するには、[鬼畜王 on Webのリポジトリ](https://github.com/kichikuou/web)をチェックアウトして、`docs`ディレクトリに `out/system3.*` をすべてコピーしてください。
