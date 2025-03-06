cmake_minimum_required(VERSION 3.14)

project(GAME)
set(PROJECT_ROOT_DIR ../../..)

# Compilation of SDL and companion libraries
include(FetchContent)
FetchContent_Declare(
  SDL
  URL https://github.com/libsdl-org/SDL/releases/download/release-3.2.8/SDL3-3.2.8.tar.gz
  URL_HASH SHA1=e5c1749aa6bd848f74e07ee3559442acdc06991a
)
FetchContent_Declare(
  SDL_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  GIT_TAG release-3.2.0
  GIT_SUBMODULES_RECURSE TRUE
)

option(SDLTTF_HARFBUZZ "Use harfbuzz to improve text shaping" OFF)
option(SDLTTF_PLUTOSVG "Use plutosvg for color emoji support" OFF)

FetchContent_MakeAvailable(SDL)
FetchContent_MakeAvailable(SDL_ttf)

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
