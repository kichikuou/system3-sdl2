cmake_minimum_required(VERSION 3.14)

project(GAME)
set(PROJECT_ROOT_DIR ../../..)

# SDL3 is provided as an Android Archive (.aar) and exposed to CMake via prefab
# (see app/build.gradle).
find_package(SDL3 REQUIRED CONFIG)

include(FetchContent)
set(SDLTTF_VENDORED ON CACHE BOOL "Use vendored third-party libraries" FORCE)
set(SDLTTF_HARFBUZZ OFF CACHE BOOL "Use harfbuzz to improve text shaping" FORCE)
set(SDLTTF_PLUTOSVG OFF CACHE BOOL "Enable plutosvg for color SVG fonts" FORCE)
set(SDLTTF_SAMPLES OFF CACHE BOOL "Build the SDL3_ttf sample program(s)" FORCE)
set(SDLTTF_INSTALL OFF CACHE BOOL "Enable SDL3_ttf install target" FORCE)
FetchContent_Declare(SDL_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  GIT_TAG release-3.2.2
  GIT_SHALLOW TRUE
  GIT_SUBMODULES external/freetype)
FetchContent_MakeAvailable(SDL_ttf)

# The main CMakeLists.txt of system3
add_subdirectory(${PROJECT_ROOT_DIR} src)

# Copy license files into the assets directory so they can be shown in-app.
# SDL.txt is extracted from the release zip by app/build.gradle; SDL_ttf's and
# freetype's licenses are copied from the fetched source trees.
set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/assets)
set(LICENSES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../licenses)
file(MAKE_DIRECTORY ${ASSETS_DIR}/licenses)
file(COPY_FILE ${PROJECT_ROOT_DIR}/COPYING.txt ${ASSETS_DIR}/licenses/system3)
file(COPY_FILE ${PROJECT_ROOT_DIR}/resources/fonts/MTLc3m.ttf.license ${ASSETS_DIR}/licenses/MTLc3m)
file(COPY_FILE ${PROJECT_ROOT_DIR}/deps/ymfm/LICENSE ${ASSETS_DIR}/licenses/ymfm)
file(COPY_FILE ${LICENSES_DIR}/SDL.txt ${ASSETS_DIR}/licenses/SDL)
file(COPY_FILE ${sdl_ttf_SOURCE_DIR}/LICENSE.txt ${ASSETS_DIR}/licenses/SDL_ttf)
# FreeType is dual-licensed (FTL or GPLv2); ship the GPLv2 text to match system3.
file(COPY_FILE ${sdl_ttf_SOURCE_DIR}/external/freetype/docs/GPLv2.TXT ${ASSETS_DIR}/licenses/freetype)
