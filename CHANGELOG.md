# Changelog

## 1.6.1 - 2025-06-06
- Fixed crash in Rance 4.1 / 4.2 English translations. (#34)

## 1.6.0 - 2025-04-26
- Enabled `Y 240` command in System2 games, to support translation of these games.
- Android: "Install from ZIP" button is moved to the top-right option menu.
- Android: Improved extraction logic of track numbers from audio file names.

## 1.5.1 - 2025-03-23
- Added quit confirmation dialog
- Android: Back button to exit the game
- Android: Removed 3-finger touch menu
- Bugfixes

## 1.5.0 - 2025-01-20
- Windows: Now system3.exe is a standalone executable. You can just copy it to
  the game folder and run it.
- Windows: Changed the app icon.

## 1.4.0 - 2025-01-04
- New supported game: Rance 2 Hint Disk (#33)
- Added `trace` option that logs executed NACT commands on the console
- Improved messages for startup errors

## 1.3.0 - 2024-12-28
- Added debugger support that can be used from Visual Studio Code. See
  [vscode-system3x](https://github.com/kichikuou/vscode-system3x) for details.
- Fixed crash at startup of D.P.S. All.

## 1.2.1 - 2024-12-21
- Fixed an issue where post-load messages were not displayed in some games
- Fixed a graphic issue in the underground passage in Intruder (#31)
- Fixed various issues in 走り女II (#32)

## 1.2.0 - 2024-12-06
- Windows: `Message` -> `Auto Copy to Clipboard` menu command has been added.
  When enabled, in-game text will be copied to the clipboard automatically. This
  can be used with text hooker tools that support clipboard monitoring.

## 1.1.0 - 2024-08-17
- Real-time MIDI playback has been restored on Windows. This allows seamless
  music looping and fixes issues where games cannot detect the end of music.
  As a trade-off, MIDI is no longer supported on Android.

## 1.0.2 - 2024-04-12
- Fixed a bug in parsing boolean values in system3.ini.

## 1.0.1 - 2024-04-07
- Fixed crash bug in system3.ini parser

## 1.0.0 - 2023-04-09
- Android: Added support for sound effects
- Fixed infinite loop bug in the opening demo of Only You
- Added touch screen support for Only You
- Improved text anti-aliasing quality for some games
- Added support for the unofficial port of Gakuen Senki
- Added scanline screen effect

## 0.9.0 - 2023-03-03
- [Nintendo Switch build support](https://github.com/kichikuou/system3-sdl2/blob/v0.9.0/switch/README.md) (#24)
- Game controller support (#23)
- On a touch screen, tapping on the black bars on the left/right or top/bottom is now treated as a right click.
- Fixed an issue where the menu screen of Alice's Cottage 3 could not be operated with a touch screen.

## 0.8.0 - 2022-08-07
- Switched FM sound chip emulator to [ymfm](https://github.com/aaronsgiles/ymfm).
- Windows: Added a menu command to enable/disable automatic mouse movements.
- Fade in/out effects now behave more like the original (PC98) version.

## 0.7.0 - 2021-09-20
- First release for Windows.
- Added multi-language support. (#16)
- Upgraded SDL2 to 2.0.16.

## 0.6.0 - 2021-06-25
- Introduced `system3.ini` configuration file.
- Supported English translated games compiled with SysEng 0.5.
- Supported proportional fonts.
- Fixed many bugs (especially in System1 and System2 games).

## 0.5.0 - 2020-11-29
- System menu pops up with 3-finger touch.
- FM sound chip emulation is added. You can select FM / MIDI from the system menu.
- Added support for text input dialog.

## 0.4.0 - 2020-05-24
- Merged System3 for Win32 changes between 2016-01-16 and 2020-03-21 (#3).
- MDA files are now included in the APK.
- MIDI music fades out at the last second, so it doesn't sound weird when looping.
- SDL2 is upgraded to 2.0.12.

## 0.3.0 - 2020-02-24
- Added support for System1 / System2 games.
- Fixed an issue where installation from ZIPs downloaded from AliceSoft archives fails in Android < 7.0.

## 0.2.0 - 2020-02-16
- Added support for MIDI music (often referred to as "FM synthesizer" in game text)

## 0.1.0 - 2020-02-10
- First public release of the Android port.
