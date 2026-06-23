cmake_minimum_required(VERSION 3.14)

project(GAME)
set(PROJECT_ROOT_DIR ../../..)

# SDL3 and SDL3_ttf are provided as Android Archives (.aar) and exposed to CMake
# via prefab (see app/build.gradle).
find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

# The main CMakeLists.txt of system3
add_subdirectory(${PROJECT_ROOT_DIR} src)

# Copy license files into the assets directory so they can be shown in-app.
# SDL.txt / SDL_ttf.txt are extracted from the release zips by app/build.gradle;
# freetype's and harfbuzz' licenses (statically linked into the SDL3_ttf .aar)
# are not shipped there, so they are vendored under android/licenses.
set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/assets)
set(LICENSES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../licenses)
file(MAKE_DIRECTORY ${ASSETS_DIR}/licenses)
file(COPY_FILE ${PROJECT_ROOT_DIR}/COPYING.txt ${ASSETS_DIR}/licenses/system3)
file(COPY_FILE ${PROJECT_ROOT_DIR}/resources/fonts/MTLc3m.ttf.license ${ASSETS_DIR}/licenses/MTLc3m)
file(COPY_FILE ${PROJECT_ROOT_DIR}/deps/ymfm/LICENSE ${ASSETS_DIR}/licenses/ymfm)
file(COPY_FILE ${LICENSES_DIR}/SDL.txt ${ASSETS_DIR}/licenses/SDL)
file(COPY_FILE ${LICENSES_DIR}/SDL_ttf.txt ${ASSETS_DIR}/licenses/SDL_ttf)
file(COPY_FILE ${LICENSES_DIR}/freetype.txt ${ASSETS_DIR}/licenses/freetype)
file(COPY_FILE ${LICENSES_DIR}/harfbuzz.txt ${ASSETS_DIR}/licenses/harfbuzz)
