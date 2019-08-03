cmake_minimum_required(VERSION 3.13)

project(system3 LANGUAGES CXX)

set(SRCS
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
  list(APPEND SRCS
	linux/nact_linux.cpp
	emscripten/mako.cpp
	)
else()
  list(APPEND SRCS
	sys/mako.cpp
	sys/mako_midi.cpp
	)
  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	list(APPEND SRCS
	  win/nact_win.cpp
	  win/mako_win.cpp
	  res3/Script1.rc
	  )
  else()
	list(APPEND SRCS
	  linux/nact_linux.cpp
	  linux/mako_linux.cpp
	  )
  endif()
endif()

add_library(objLib OBJECT ${SRCS})
add_executable(system3 $<TARGET_OBJECTS:objLib>)
target_compile_definitions(objLib PRIVATE _SYSTEM3 $<$<CONFIG:Debug>:_DEBUG_CONSOLE>)
target_include_directories(objLib PRIVATE . sys)

if (EMSCRIPTEN)

  set(LIBS "SHELL:-s USE_SDL=2" "SHELL:-s USE_SDL_TTF=2")
  target_compile_options(objLib PRIVATE ${LIBS})
  target_link_options(system3 PRIVATE ${LIBS})

  # Without optimizations, Asyncify generates very large code.
  list(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG "-O1")

  set(LINK_OPTS
	"SHELL:-s ENVIRONMENT=web"
	"SHELL:-s ASYNCIFY=1 -s ASYNCIFY_IGNORE_INDIRECT=1"
	"SHELL:-s ALLOW_MEMORY_GROWTH=1"
	"SHELL:-s NO_EXIT_RUNTIME=1"
	"SHELL:-s EXPORTED_FUNCTIONS=\"['_main','_ags_setAntialiasedStringMode']\""
	"SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=\"['getValue','getMemory','addRunDependency','removeRunDependency']\"")

  target_link_options(system3 PRIVATE ${LINK_OPTS})

else()

  include(FindPkgConfig)
  pkg_check_modules(SDL2 REQUIRED sdl2)
  pkg_check_modules(SDL2TTF REQUIRED sdl2_ttf)

  target_include_directories(objLib PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
  target_link_libraries(system3 PRIVATE ${SDL2TTF_LIBRARIES} ${SDL2_LIBRARIES})
  target_link_directories(system3 PRIVATE ${SDL2_LIBRARY_DIRS} ${SDL2TTF_LIBRARY_DIRS})

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(system3 PRIVATE winmm)
  endif()

  install(TARGETS system3 RUNTIME DESTINATION bin)

endif()
