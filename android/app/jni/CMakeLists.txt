cmake_minimum_required(VERSION 3.13)

project(GAME)
set(PROJECT_ROOT_DIR ../../..)

include(FetchContent)
FetchContent_Declare(
  SDL
  URL https://github.com/libsdl-org/SDL/releases/download/release-2.26.1/SDL2-2.26.1.tar.gz
  URL_HASH SHA1=08a8cec21ffcebf44e4633a786d594a7811100d2
)
FetchContent_Declare(
  SDL_ttf
  URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.20.1/SDL2_ttf-2.20.1.tar.gz
  URL_HASH SHA1=f774ff407915ae4afa3d03e5a9395283d9090f28
)

# Compilation of SDL and companion libraries
FetchContent_GetProperties(SDL)
if(NOT sdl_POPULATED)
  FetchContent_Populate(SDL)
  add_subdirectory(${sdl_SOURCE_DIR} ${sdl_BINARY_DIR})
endif()
FetchContent_GetProperties(SDL_ttf)
if(NOT sdl_ttf_POPULATED)
  FetchContent_Populate(SDL_ttf)
  set(SDL2TTF_SAMPLES OFF CACHE BOOL "Build the SDL2_ttf sample program(s)" FORCE)
  set(SDL2TTF_INSTALL OFF CACHE BOOL "Enable SDL2_ttf install target" FORCE)
  set(SDL2TTF_VENDORED ON CACHE BOOL "Use vendored third-party libraries" FORCE)
  add_subdirectory(${sdl_ttf_SOURCE_DIR} ${sdl_ttf_BINARY_DIR})
endif()

# The main CMakeLists.txt of system3
add_subdirectory(${PROJECT_ROOT_DIR}/src src)

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
