# System3 for Android

Minimum supported Android version: 5.0

## Download
You can download prebuilt APKs
[here](https://github.com/kichikuou/system3-sdl2/releases).

## Build

### Using Android Studio
Clone this repository along with its submodules:
```sh
git clone --recurse-submodules https://github.com/kichikuou/system3-sdl2.git
```

Next, open the `system3-sdl2/android` directory as an Android Studio project.

### Command Line Build
Set environment variables and run the `gradlew` script in this directory.

Example build instructions (for Debian bookworm):
```sh
# Install necessary packages
sudo apt install git wget unzip default-jdk-headless ninja-build

# Install Android SDK / NDK
export ANDROID_SDK_ROOT=$HOME/android-sdk
mkdir -p $ANDROID_SDK_ROOT/cmdline-tools
wget https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip
unzip commandlinetools-linux-11076708_latest.zip -d $ANDROID_SDK_ROOT/cmdline-tools
mv $ANDROID_SDK_ROOT/cmdline-tools/cmdline-tools $ANDROID_SDK_ROOT/cmdline-tools/tools
yes | $ANDROID_SDK_ROOT/cmdline-tools/tools/bin/sdkmanager --licenses
$ANDROID_SDK_ROOT/cmdline-tools/tools/bin/sdkmanager ndk-bundle 'cmake;3.22.1'
export ANDROID_NDK_HOME=$ANDROID_SDK_ROOT/ndk-bundle

# Clone and build system3-sdl2
git clone --recurse-submodules https://github.com/kichikuou/system3-sdl2.git
cd system3-sdl2/android
./gradlew build  # or ./gradlew installDebug if you have a connected device
```

## Usage
### Basic Usage
1. Create a ZIP file containing all the game files and BGM files (see
   [below](#preparing-a-zip) for details), and transfer it to your device.
2. Open the app. A list of installed games will be displayed. Since no games
   have been installed yet, only the "Install from ZIP" button will be visible.
   Tap it.
3. Select the ZIP file you created in step 1.
4. The game will start. To simulate a right-click, tap the black bars on either
   the left or right, or top or bottom of the screen.

### Preparing a ZIP
- Include all `.DAT` files.
- Music files (`.mp3`, `.ogg`, or `.wav`) whose filenames end with a number are
  recognized as BGM files. For example:
  - `Track2.mp3`
  - `15.ogg`
  - `rance41_03.wav` (Note: The filename shouldn't be `rance4103.wav`, as it
    would be treated as the 4103rd track.)

Note: This ZIP format is also compatible with
[Kichikuou on Web](http://kichikuou.github.io/web/).

### Miscellaneous
- You can export or import save files via the game list's option menu.
- To uninstall a game, long-tap its title in the game list.

## Known Issues
- Installing from a ZIP file containing multiple games (e.g., the DPS series)
  may not work correctly.
