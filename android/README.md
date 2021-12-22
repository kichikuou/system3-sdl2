# System3 for Android

## Download
Prebuilt APKs are [here](https://github.com/kichikuou/system3-sdl2/releases).

## Build
Prerequisites:
- CMake >=3.20
- Android SDK >=28
- Android NDK >=r15c

### Using Android Studio
Clone this repository and its submodules:
```sh
git clone --recurse-submodules https://github.com/kichikuou/system3-sdl2.git
```

Then open `system3-sdl2/android` directory as an Android Studio project.

### Command line build
Configure environment variables and run the `gradlew` script in this folder.

Example build instructions (for Debian bullseye):
```sh
# Install necessary packages
sudo apt install git wget unzip default-jdk-headless ninja-build
sudo apt install -t bullseye-backports cmake

# Install Android SDK / NDK
export ANDROID_SDK_ROOT=$HOME/android-sdk
mkdir -p $ANDROID_SDK_ROOT/cmdline-tools
wget https://dl.google.com/android/repository/commandlinetools-linux-7583922_latest.zip
unzip commandlinetools-linux-7583922_latest.zip -d $ANDROID_SDK_ROOT/cmdline-tools
mv $ANDROID_SDK_ROOT/cmdline-tools/cmdline-tools $ANDROID_SDK_ROOT/cmdline-tools/tools
yes |$ANDROID_SDK_ROOT/cmdline-tools/tools/bin/sdkmanager --licenses
$ANDROID_SDK_ROOT/cmdline-tools/tools/bin/sdkmanager ndk-bundle
export ANDROID_NDK_HOME=$ANDROID_SDK_ROOT/ndk-bundle

# Check out and build system3-sdl2
git clone --recurse-submodules https://github.com/kichikuou/system3-sdl2.git
cd system3-sdl2/android
./gradlew build  # or ./gradlew installDebug if you have a connected device
```

## Use
### Basic Usage
1. Create a ZIP file containing all the game files and BGM files (see [below](#preparing-a-zip) for details), and transfer it to your device.
2. Open the app. A list of installed games is displayed. Since nothing has been installed yet, only the "Install from ZIP" button is displayed. Tap it.
3. Select the ZIP file you created in 1.
4. The game starts. Two-finger touch is treated as a right click.

### Preparing a ZIP
- Include all `.DAT` files.
- Music files (`.mp3`, `.ogg` or `.wav`) whose file names end with a number are recognized as BGM files. For example:
  - `Track2.mp3`
  - `15.ogg`
  - `rance41_03.wav` (This shouldn't be `rance4103.wav`, because it would be treated as the 4103rd track)

Note: This form of ZIP can be used in [Kichikuou on Web](http://kichikuou.github.io/web/) as well.

### Miscellaneous
- You can export / import saved files using the option menu of the game list.
- To uninstall a game, long-tap the title in the game list.
- System menu pops up with 3-finger touch during game play.

## Known Issues
- Installation from a ZIP containing multiple games (e.g. DPS series) does not work well.
