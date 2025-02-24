cmake_minimum_required(VERSION 3.14)

# Enable CMAKE_MSVC_RUNTIME_LIBRARY, requires CMake >=3.15
if (POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif ()

project(System3 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)

# Generates a static library from pkg_check_modules() result
function(add_static_library name pkg)
  add_library(${name} INTERFACE)
  set_target_properties(${name}
      PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${${pkg}_STATIC_INCLUDE_DIRS}"
          INTERFACE_COMPILE_OPTIONS "${${pkg}_STATIC_CFLAGS_OTHER}"
          INTERFACE_LINK_LIBRARIES "${${pkg}_STATIC_LIBRARIES}"
          INTERFACE_LINK_OPTIONS "${${pkg}_STATIC_LDFLAGS_OTHER}")
endfunction()

if (MSVC)
  # Use statically-linked runtime library.
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  option(BUILD_SHARED_LIBS "Build as shared library" OFF)
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
    src/generic/mako.cpp
    )
  find_library(ndk_log log)
  target_link_libraries(system3 PRIVATE SDL2 SDL2_ttf SDL2_mixer ymfm ${ndk_log})

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
    -sEXPORTED_RUNTIME_METHODS=FS,IDBFS,HEAPU8,HEAP16,getValue,addRunDependency,removeRunDependency,stringToUTF8OnStack
    --emit-tsd=system3.d.ts
    )

else()  # NOT (ANDROID OR EMSCRIPTEN)
  add_executable(system3)
  target_link_libraries(system3 PRIVATE ymfm)

  if (MSVC)
    include(FetchContent)
    FetchContent_Declare(sdl3
      URL https://github.com/libsdl-org/SDL/releases/download/release-3.2.24/SDL3-devel-3.2.24-VC.zip
      URL_HASH SHA1=9874438f58da720f52eb27dda5dbfed6a86cbd00)
    FetchContent_MakeAvailable(sdl3)
    find_package(SDL3 REQUIRED CONFIG PATHS ${sdl3_SOURCE_DIR}/cmake NO_DEFAULT_PATH)
    get_target_property(SDL3_DLL SDL3::SDL3 IMPORTED_LOCATION)

    FetchContent_Declare(sdl3_ttf
      URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-3.2.2/SDL3_ttf-devel-3.2.2-VC.zip
      URL_HASH SHA1=3786bc016d89ca4cf9739e4d342ca29e3e29c137)
    FetchContent_MakeAvailable(sdl3_ttf)
    find_package(SDL3_ttf REQUIRED CONFIG PATHS ${sdl3_ttf_SOURCE_DIR}/cmake NO_DEFAULT_PATH)
    get_target_property(SDL3TTF_DLL SDL3_ttf::SDL3_ttf IMPORTED_LOCATION)
    get_filename_component(SDL3TTF_LIBDIR ${SDL3TTF_DLL} DIRECTORY)

    FetchContent_Declare(rtmidi
      URL https://github.com/thestk/rtmidi/archive/refs/tags/6.0.0.zip
      URL_HASH SHA1=43287aff8c2c03d3b38cff4d44611763f5e149fa
      EXCLUDE_FROM_ALL)
    set(RTMIDI_BUILD_TESTING OFF CACHE BOOL "Build test programs" FORCE)
    FetchContent_MakeAvailable(rtmidi)
    set_property(TARGET rtmidi PROPERTY COMPILE_WARNING_AS_ERROR OFF)
    target_compile_definitions(system3 PRIVATE USE_MIDI)
    target_link_libraries(system3 PRIVATE rtmidi)
    target_sources(system3 PRIVATE src/sys/mako_midi.cpp)

    FetchContent_Declare(nlohmann_json
      URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
      URL_HASH SHA1=886dd96952c7c8ad78fdac1a26c409f9e886377b)
    FetchContent_MakeAvailable(nlohmann_json)
    set(ENABLE_DEBUGGER ON)
    target_link_libraries(system3 PRIVATE nlohmann_json::nlohmann_json)

    target_compile_definitions(system3 PRIVATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(system3 PRIVATE /utf-8 /wd4244)
    target_link_libraries(system3 PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf)
    set_target_properties(system3 PROPERTIES
      WIN32_EXECUTABLE TRUE
      VS_DEBUGGER_ENVIRONMENT "PATH=${SDL3_LIBDIR}$<SEMICOLON>${SDL3TTF_LIBDIR}$<SEMICOLON>$ENV{PATH}")
    set_directory_properties(PROPERTIES VS_STARTUP_PROJECT system3)

    install(FILES ${SDL3_DLL} ${SDL3TTF_DLL} DESTINATION .)
    install(TARGETS system3 RUNTIME DESTINATION .)
  else()
    include(FindPkgConfig)
    pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
    pkg_check_modules(SDL3TTF REQUIRED IMPORTED_TARGET SDL3-ttf)
    if (MINGW)
      add_static_library(sdl3 SDL3)
      add_static_library(sdl3_ttf SDL3TTF)
      # Workaround for linking error
      set_property(TARGET sdl3_ttf PROPERTY INTERFACE_LINK_LIBRARIES
          $<LINK_GROUP:RESCAN,${SDL3TTF_STATIC_LIBRARIES}>)
    else()
      add_library(sdl3 ALIAS PkgConfig::SDL3)
      add_library(sdl3_ttf ALIAS PkgConfig::SDL3TTF)
    endif()
    target_link_libraries(system3 PRIVATE sdl3 sdl3_ttf)

    find_package(nlohmann_json 3.2.0)
    if (nlohmann_json_FOUND)
      set(ENABLE_DEBUGGER ON)
      target_link_libraries(system3 PRIVATE nlohmann_json::nlohmann_json)
    endif()

    pkg_check_modules(RTMIDI IMPORTED_TARGET rtmidi)
    if (RTMIDI_FOUND)
      if (MINGW)
        add_static_library(RTMIDI RTMIDI)
      else()
        add_library(RTMIDI ALIAS PkgConfig::RTMIDI)
      endif()
      target_compile_definitions(system3 PRIVATE USE_MIDI)
      target_link_libraries(system3 PRIVATE RTMIDI)
      target_sources(system3 PRIVATE src/sys/mako_midi.cpp)
    endif()
  endif()

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(system3 PRIVATE
      src/win/nact_win.cpp
      src/win/mako.cpp
      src/win/resource.rc
      )
    target_compile_definitions(system3 PRIVATE NOMINMAX)
    target_link_libraries(system3 PRIVATE winmm)
    if (MINGW)
      target_link_options(system3 PRIVATE -static)
    endif()
  else()
    pkg_check_modules(SDL2MIXER REQUIRED IMPORTED_TARGET SDL2_mixer)
    target_sources(system3 PRIVATE
      src/generic/nact_generic.cpp
      src/generic/mako.cpp
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
  src/sys/game_id.cpp
  src/sys/msgskip.cpp
  src/sys/nact.cpp
  src/sys/nact_input.cpp
  src/sys/nact_sys1.cpp
  src/sys/nact_sys2.cpp
  src/sys/nact_sys3.cpp
  src/sys/scenario.cpp
  )
if (ENABLE_DEBUGGER)
  target_compile_definitions(system3 PRIVATE ENABLE_DEBUGGER)
  target_sources(system3 PRIVATE
    src/debugger/debug_info.cpp
    src/debugger/debugger.cpp
    src/debugger/cli_frontend.cpp
    src/debugger/dap_frontend.cpp
  )
endif()

if(NINTENDO_SWITCH)
  set(RESOURCE_PATH romfs:/)

  nx_generate_nacp(system3.nacp
    NAME "system3"
    AUTHOR "kichikuou"
    VERSION "1.6.1"
  )

  nx_create_nro(system3
    NACP system3.nacp
    ICON "switch/icon.jpg"
    ROMFS "resources"
  )
endif()

target_compile_definitions(system3 PRIVATE RESOURCE_PATH="${RESOURCE_PATH}")
target_include_directories(system3 PRIVATE src src/sys)
