cmake_minimum_required(VERSION 3.14)

project(GAME)
set(PROJECT_ROOT_DIR ../../..)

# Compilation of SDL and companion libraries
include(FetchContent)
FetchContent_Declare(
  SDL
  URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.2/SDL2-2.30.2.tar.gz
  URL_HASH SHA1=37a0557fc39460dc2a4fbbf162d95f115d2c9be2
)
FetchContent_Declare(
  SDL_ttf
  URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.20.1/SDL2_ttf-2.20.1.tar.gz
  URL_HASH SHA1=f774ff407915ae4afa3d03e5a9395283d9090f28
)
FetchContent_Declare(
  SDL_mixer
  URL https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.6.2/SDL2_mixer-2.6.2.tar.gz
  URL_HASH SHA1=036fc7839a6b8dc1af3dbfed4e2531224bbacdd7
)

set(SDL2TTF_SAMPLES OFF CACHE BOOL "Build the SDL2_ttf sample program(s)" FORCE)
set(SDL2TTF_INSTALL OFF CACHE BOOL "Enable SDL2_ttf install target" FORCE)
set(SDL2TTF_VENDORED ON CACHE BOOL "Use vendored third-party libraries" FORCE)

set(SDL2MIXER_OPUS OFF CACHE BOOL "Enable Opus music" FORCE)
set(SDL2MIXER_FLAC OFF CACHE BOOL "Enable FLAC music" FORCE)
set(SDL2MIXER_MOD  OFF CACHE BOOL "Support loading MOD music" FORCE)
set(SDL2MIXER_MIDI OFF CACHE BOOL "Enable MIDI music" FORCE)
set(SDL2MIXER_SAMPLES OFF CACHE BOOL "Build the SDL2_mixer sample program(s)" FORCE)
set(SDL2MIXER_INSTALL OFF CACHE BOOL "Enable SDL2_mixer install target" FORCE)

FetchContent_MakeAvailable(SDL)
FetchContent_MakeAvailable(SDL_ttf)
FetchContent_MakeAvailable(SDL_mixer)

# The main CMakeLists.txt of system3
add_subdirectory(${PROJECT_ROOT_DIR} src)

# Copy asset files
set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/assets)
file(MAKE_DIRECTORY ${ASSETS_DIR}/licenses)
file(COPY_FILE ${PROJECT_ROOT_DIR}/COPYING.txt ${ASSETS_DIR}/licenses/system3)
file(COPY_FILE ${PROJECT_ROOT_DIR}/resources/fonts/MTLc3m.ttf.license ${ASSETS_DIR}/licenses/MTLc3m)
file(COPY_FILE ${PROJECT_ROOT_DIR}/deps/ymfm/LICENSE ${ASSETS_DIR}/licenses/ymfm)
file(COPY_FILE ${sdl_SOURCE_DIR}/LICENSE.txt ${ASSETS_DIR}/licenses/SDL)
file(COPY_FILE ${sdl_ttf_SOURCE_DIR}/LICENSE.txt ${ASSETS_DIR}/licenses/SDL_ttf)
file(COPY_FILE ${sdl_ttf_SOURCE_DIR}/external/freetype/docs/GPLv2.TXT ${ASSETS_DIR}/licenses/freetype)
file(COPY_FILE ${sdl_ttf_SOURCE_DIR}/external/harfbuzz/COPYING ${ASSETS_DIR}/licenses/harfbuzz)
file(COPY_FILE ${sdl_mixer_SOURCE_DIR}/LICENSE.txt ${ASSETS_DIR}/licenses/SDL_mixer)
