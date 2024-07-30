cmake_minimum_required(VERSION 3.14)

# Enable CMAKE_MSVC_RUNTIME_LIBRARY, requires CMake >=3.15
if (POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif ()

project(System3 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)

# In msys, `pkg-config --libs` returns `-lmingw32` but find_library() cannot
# find it in default search paths.
if (MSYS)
  list(APPEND CMAKE_LIBRARY_PATH $ENV{MINGW_PREFIX}/$ENV{MINGW_CHOST}/lib)
endif ()

if (MSVC)
  # Use statically-linked runtime library.
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

add_library(ymfm
  deps/ymfm/src/ymfm_opn.cpp
  deps/ymfm/src/ymfm_adpcm.cpp
  deps/ymfm/src/ymfm_ssg.cpp)
target_include_directories(ymfm PUBLIC deps/ymfm/src)


if (ANDROID)
  add_library(system3 SHARED)

  target_sources(system3 PRIVATE
    src/android/nact_android.cpp
    src/android/mako.cpp
    src/sys/mako_midi.cpp
    )
  target_link_libraries(system3 PRIVATE SDL2 SDL2_ttf SDL2_mixer ymfm)

elseif (EMSCRIPTEN)
  add_executable(system3)

  target_sources(system3 PRIVATE
    src/emscripten/nact_emscripten.cpp
    src/emscripten/mako.cpp
    )
  set(LIBS -sUSE_SDL=2 -sUSE_SDL_TTF=2)
  target_compile_options(system3 PRIVATE ${LIBS})
  target_link_options(system3 PRIVATE ${LIBS})
  target_link_libraries(system3 PRIVATE idbfs.js ymfm)

  # Without optimizations, Asyncify generates very large code.
  list(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG "-O1")

  target_link_options(system3 PRIVATE
    -sENVIRONMENT=web
    -sMODULARIZE=1
    -sEXPORT_ES6=1
    -sASYNCIFY=1
    -sASYNCIFY_IGNORE_INDIRECT=1
    -sASYNCIFY_REMOVE=SDL_Delay
    -sALLOW_MEMORY_GROWTH=1
    -sNO_EXIT_RUNTIME=1
    "-sASYNCIFY_ADD=NACT::execute()"
    -sEXPORTED_FUNCTIONS=_main,_malloc
    -sEXPORTED_RUNTIME_METHODS=FS,IDBFS,getValue,addRunDependency,removeRunDependency
    --emit-tsd=system3.d.ts
    )

else()  # NOT (ANDROID OR EMSCRIPTEN)
  add_executable(system3)
  target_link_libraries(system3 PRIVATE ymfm)

  if (MSVC)
    include(FetchContent)
    FetchContent_Declare(sdl2
      URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.2/SDL2-devel-2.30.2-VC.zip
      URL_HASH SHA1=f354c841e99a9894ff0f5ba85ec306184316e7b6)
    FetchContent_MakeAvailable(sdl2)
    find_package(SDL2 REQUIRED CONFIG PATHS ${sdl2_SOURCE_DIR}/cmake NO_DEFAULT_PATH)
    get_target_property(SDL2_DLL SDL2::SDL2 IMPORTED_LOCATION)

    FetchContent_Declare(sdl2_ttf
      URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.20.2/SDL2_ttf-devel-2.20.2-VC.zip
      URL_HASH SHA1=dee48e9c5184c139aa8bcab34a937d1b3df4f503)
    FetchContent_MakeAvailable(sdl2_ttf)
    find_package(SDL2_ttf REQUIRED CONFIG PATHS ${sdl2_ttf_SOURCE_DIR}/cmake NO_DEFAULT_PATH)
    get_target_property(SDL2TTF_DLL SDL2_ttf::SDL2_ttf IMPORTED_LOCATION)
    get_filename_component(SDL2TTF_LIBDIR ${SDL2TTF_DLL} DIRECTORY)

    target_compile_definitions(system3 PRIVATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(system3 PRIVATE /utf-8 /wd4244)
    target_link_libraries(system3 PRIVATE SDL2::SDL2main SDL2::SDL2 SDL2_ttf::SDL2_ttf)
    set_target_properties(system3 PROPERTIES
      WIN32_EXECUTABLE TRUE
      VS_DEBUGGER_ENVIRONMENT "PATH=${SDL2_LIBDIR}$<SEMICOLON>${SDL2TTF_LIBDIR}$<SEMICOLON>$ENV{PATH}")
    set_directory_properties(PROPERTIES VS_STARTUP_PROJECT system3)

    install(FILES ${SDL2_DLL} ${SDL2TTF_DLL} DESTINATION .)
    install(TARGETS system3 RUNTIME DESTINATION .)
  else()
    include(FindPkgConfig)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
    pkg_check_modules(SDL2TTF REQUIRED IMPORTED_TARGET SDL2_ttf)
    target_link_libraries(system3 PRIVATE PkgConfig::SDL2 PkgConfig::SDL2TTF)
  endif()

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(system3 PRIVATE
      src/win/nact_win.cpp
      src/win/mako.cpp
      src/win/resource.rc
      src/sys/mako_midi.cpp
      )
    target_link_libraries(system3 PRIVATE winmm)
    if (MSYS)
      set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
    endif()
  else()
    pkg_check_modules(SDL2MIXER REQUIRED IMPORTED_TARGET SDL2_mixer)
    target_sources(system3 PRIVATE
      src/linux/nact_linux.cpp
      src/linux/mako.cpp
      src/sys/mako_midi.cpp
      )
    target_link_libraries(system3 PRIVATE PkgConfig::SDL2MIXER)
    set(RESOURCE_PATH ${CMAKE_INSTALL_PREFIX}/share/system3/)
    install(TARGETS system3 RUNTIME DESTINATION bin)
    install(DIRECTORY resources/
      DESTINATION share/system3
      FILES_MATCHING PATTERN *.MDA PATTERN *.ttf)
  endif()
endif()

target_sources(system3 PRIVATE
  src/sdlmain.cpp
  src/config.cpp
  src/fileio.cpp
  src/resource.cpp
  src/encoding.cpp
  src/texthook.cpp
  src/fm/makofm.cpp
  src/fm/mako_ymfm.cpp
  src/sys/ags.cpp
  src/sys/ags_bmp.cpp
  src/sys/ags_cursor.cpp
  src/sys/ags_draw.cpp
  src/sys/ags_gl3.cpp
  src/sys/ags_gm3.cpp
  src/sys/ags_pms.cpp
  src/sys/ags_text.cpp
  src/sys/ags_vsp.cpp
  src/sys/ags_vsp2l.cpp
  src/sys/ags_window.cpp
  src/sys/dri.cpp
  src/sys/msgskip.cpp
  src/sys/nact.cpp
  src/sys/nact_crc32.cpp
  src/sys/nact_input.cpp
  src/sys/nact_sys1.cpp
  src/sys/nact_sys2.cpp
  src/sys/nact_sys3.cpp
  )

if(NINTENDO_SWITCH)
  set(RESOURCE_PATH romfs:/)

  nx_generate_nacp(system3.nacp
    NAME "system3"
    AUTHOR "kichikuou"
    VERSION "0.8.0"
  )

  nx_create_nro(system3
    NACP system3.nacp
    ICON "switch/icon.jpg"
    ROMFS "resources"
  )
endif()

target_compile_definitions(system3 PRIVATE
  $<$<CONFIG:Debug>:_DEBUG_CONSOLE>
  RESOURCE_PATH="${RESOURCE_PATH}")
target_include_directories(system3 PRIVATE src src/sys)
