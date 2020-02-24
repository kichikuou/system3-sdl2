# System3 for Android

## Download
Prebuilt APKs are [here](https://github.com/kichikuou/system3-sdl2/releases).

## Build
Prerequisites:
- CMake >=3.13
- Android SDK >=26
- Android NDK >=r15c

### Using Android Studio
Open this directory as an Android Studio project.

### Command line build
Configure environment variables and run the `gradlew` script in this folder.

Example build instructions (for Debian Stretch):
```sh
# Install necessary packages
sudo apt install git wget unzip default-jdk-headless ninja-build
sudo apt -t stretch-backports install cmake

# Install Android SDK / NDK
export ANDROID_HOME=$HOME/android-sdk
export PATH=$PATH:$ANDROID_HOME/tools
export PATH=$PATH:$ANDROID_HOME/platform-tools
wget https://dl.google.com/android/repository/sdk-tools-linux-4333796.zip
unzip sdk-tools-linux-4333796.zip -d $ANDROID_HOME
yes |$ANDROID_HOME/tools/bin/sdkmanager --licenses
$ANDROID_HOME/tools/bin/sdkmanager ndk-bundle

# Check out and build system3
git clone https://github.com/kichikuou/system3-sdl2.git
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

## Known Issues
- Installation from a ZIP containing multiple games (e.g. DPS series) does not work well.
