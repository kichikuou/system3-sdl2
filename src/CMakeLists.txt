cmake_minimum_required(VERSION 3.13)

project(system3 LANGUAGES CXX)

add_executable(system3)
target_compile_definitions(system3 PRIVATE _SYSTEM3 $<$<CONFIG:Debug>:_DEBUG_CONSOLE>)
target_include_directories(system3 PRIVATE . sys)

target_sources(system3 PRIVATE
  sdlmain.cpp
  fileio.cpp
  sys/ags.cpp
  sys/ags_bmp.cpp
  sys/ags_cursor.cpp
  sys/ags_draw.cpp
  sys/ags_pms.cpp
  sys/ags_text.cpp
  sys/ags_vsp.cpp
  sys/ags_window.cpp
  sys/dri.cpp
  sys/nact.cpp
  sys/nact_crc32.cpp
  sys/nact_input.cpp
  sys/nact_sys3.cpp
  )

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  target_sources(system3 PRIVATE
	linux/nact_linux.cpp
	emscripten/mako.cpp
	)
else()
  target_sources(system3 PRIVATE
	sys/mako.cpp
	sys/mako_midi.cpp
	)
  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_sources(system3 PRIVATE
	  win/nact_win.cpp
	  win/mako_win.cpp
	  res3/Script1.rc
	  )
  else()
	target_sources(system3 PRIVATE
	  linux/nact_linux.cpp
	  linux/mako_linux.cpp
	  )
  endif()
endif()

if (EMSCRIPTEN)

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

else()

  include(FindPkgConfig)
  pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
  pkg_check_modules(SDL2TTF REQUIRED IMPORTED_TARGET sdl2_ttf)

  target_link_libraries(system3 PRIVATE PkgConfig::SDL2 PkgConfig::SDL2TTF)

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(system3 PRIVATE winmm)
  endif()

  install(TARGETS system3 RUNTIME DESTINATION bin)

endif()
