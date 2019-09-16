cmake_minimum_required(VERSION 3.13)

project(System3 LANGUAGES CXX)

# In msys, `pkg-config --libs` returns `-lmingw32` but find_library() cannot
# find it in default search paths.
if (MSYS)
  list(APPEND CMAKE_LIBRARY_PATH $ENV{MINGW_PREFIX}/$ENV{MINGW_CHOST}/lib)
endif ()

set(SRC_COMMON
  sdlmain.cpp
  fileio.cpp
  utfsjis.cpp
  texthook.cpp
  sys/ags.cpp
  sys/ags_cursor.cpp
  sys/ags_draw.cpp
  sys/ags_text.cpp
  sys/ags_vsp.cpp
  sys/ags_window.cpp
  sys/dri.cpp
  sys/nact.cpp
  sys/nact_crc32.cpp
  sys/nact_input.cpp
  )

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(SRC_PLATFORM
	linux/nact_linux.cpp
	emscripten/mako.cpp
	)
else()
  set(SRC_MAKO
	sys/mako.cpp
	sys/mako_midi.cpp
	)
  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(SRC_PLATFORM
	  win/nact_win.cpp
	  win/mako_win.cpp
	  )
	set(RES1 res1/Script1.rc)
	set(RES3 res3/Script1.rc)
  else()
	set(SRC_PLATFORM
	  linux/nact_linux.cpp
	  linux/mako_linux.cpp
	  )
  endif()
endif()

add_library(common INTERFACE)
target_compile_definitions(common INTERFACE $<$<CONFIG:Debug>:_DEBUG_CONSOLE>)
target_include_directories(common INTERFACE . sys)
target_sources(common INTERFACE ${SRC_COMMON} ${SRC_PLATFORM} ${SRC_MAKO})

add_executable(system3)
target_sources(system3 PRIVATE sys/nact_sys3.cpp ${RES3} sys/ags_bmp.cpp sys/ags_pms.cpp)
target_compile_definitions(system3 PRIVATE _SYSTEM3)
target_link_libraries(system3 PRIVATE common)

if (NOT EMSCRIPTEN)

  add_executable(system1)
  target_sources(system1 PRIVATE sys/nact_sys1.cpp ${RES1} sys/ags_gl3.cpp sys/ags_gm3.cpp sys/ags_vsp2l.cpp)
  target_compile_definitions(system1 PRIVATE _SYSTEM1)
  target_link_libraries(system1 PRIVATE common)

  add_executable(system2)
  target_sources(system2 PRIVATE sys/nact_sys2.cpp ${RES1} sys/ags_gl3.cpp sys/ags_pms.cpp)
  target_compile_definitions(system2 PRIVATE _SYSTEM2 _SDPS)
  target_link_libraries(system2 PRIVATE common)

  add_executable(prog_omake)
  target_sources(prog_omake PRIVATE sys/nact_sys3.cpp ${RES3} sys/ags_pms.cpp)
  target_compile_definitions(prog_omake PRIVATE _PROG_OMAKE)
  target_link_libraries(prog_omake PRIVATE common)


  include(FindPkgConfig)
  pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
  pkg_check_modules(SDL2TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

  target_link_libraries(common INTERFACE PkgConfig::SDL2 PkgConfig::SDL2TTF)

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(common INTERFACE winmm)
  endif()

  install(TARGETS system1 system2 system3 prog_omake RUNTIME DESTINATION bin)

else()  # emscripten

  set(LIBS "SHELL:-s USE_SDL=2" "SHELL:-s USE_SDL_TTF=2")
  target_compile_options(system3 PRIVATE ${LIBS})
  target_link_options(system3 PRIVATE ${LIBS})

  # Without optimizations, Asyncify generates very large code.
  list(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG "-O1")

  target_link_options(system3 PRIVATE
	"SHELL:-s ENVIRONMENT=web"
	"SHELL:-s ASYNCIFY=1 -s ASYNCIFY_IGNORE_INDIRECT=1"
	"SHELL:-s ALLOW_MEMORY_GROWTH=1"
	"SHELL:-s NO_EXIT_RUNTIME=1"
	"SHELL:-s EXPORTED_FUNCTIONS=\"['_main','_ags_setAntialiasedStringMode']\""
	"SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=\"['getValue','getMemory','addRunDependency','removeRunDependency']\"")

endif()