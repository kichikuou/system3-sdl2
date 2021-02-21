cmake_minimum_required(VERSION 3.13)

project(GAME)

# armeabi-v7a requires cpufeatures library
# include(AndroidNdkModules)
# android_ndk_import_module_cpufeatures()

include(FetchContent)
FetchContent_Declare(
  SDL
  URL https://www.libsdl.org/release/SDL2-2.0.14.tar.gz
  URL_HASH SHA1=212b17d988c417a1a905ab09c50d1845cc48ddb7
  PATCH_COMMAND patch -p1 -i ${CMAKE_CURRENT_LIST_DIR}/SDL.patch
)
# TODO: Use SDL2_ttf 2.0.16 once released
FetchContent_Declare(
  SDL_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  GIT_TAG 11722a2bb4335d489d724c08cfdbb45f97d07aa7
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
  add_subdirectory(${sdl_ttf_SOURCE_DIR} ${sdl_ttf_BINARY_DIR})
endif()

# The main CMakeLists.txt of system3
add_subdirectory(../../../src src)
