cmake_minimum_required(VERSION 3.13)

project(System3 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)

# In msys, `pkg-config --libs` returns `-lmingw32` but find_library() cannot
# find it in default search paths.
if (MSYS)
  list(APPEND CMAKE_LIBRARY_PATH $ENV{MINGW_PREFIX}/$ENV{MINGW_CHOST}/lib)
endif ()

if (ANDROID)
  add_library(system3 SHARED)

  target_sources(system3 PRIVATE
	linux/nact_linux.cpp
	android/mako.cpp
	)
  target_link_libraries(system3 PRIVATE SDL2 SDL2_ttf)

elseif (EMSCRIPTEN)
  add_executable(system3)

  target_sources(system3 PRIVATE
	linux/nact_linux.cpp
	emscripten/mako.cpp
	)
  set(LIBS "SHELL:-s USE_SDL=2" "SHELL:-s USE_SDL_TTF=2")
  target_compile_options(system3 PRIVATE ${LIBS})
  target_link_options(system3 PRIVATE ${LIBS})
  target_link_libraries(system3 PRIVATE idbfs.js)

  # Without optimizations, Asyncify generates very large code.
  list(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG "-O1")

  target_link_options(system3 PRIVATE
	"SHELL:-s ENVIRONMENT=web"
	"SHELL:-s ASYNCIFY=1 -s ASYNCIFY_IGNORE_INDIRECT=1"
	"SHELL:-s ALLOW_MEMORY_GROWTH=1"
	"SHELL:-s NO_EXIT_RUNTIME=1"
	"SHELL:-s EXPORTED_FUNCTIONS=\"['_main','_ags_setAntialiasedStringMode']\""
	"SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=\"['getValue','getMemory','addRunDependency','removeRunDependency']\"")

else()  # NOT (ANDROID OR EMSCRIPTEN)
  add_executable(system3)

  include(FindPkgConfig)
  pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
  pkg_check_modules(SDL2TTF REQUIRED IMPORTED_TARGET SDL2_ttf)
  target_link_libraries(system3 PRIVATE PkgConfig::SDL2 PkgConfig::SDL2TTF)

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_sources(system3 PRIVATE win/nact_win.cpp win/mako.cpp res3/Script1.rc)
	target_link_libraries(system3 PRIVATE winmm)
	set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
  else()
	pkg_check_modules(SDL2MIXER REQUIRED IMPORTED_TARGET SDL2_mixer)
	target_sources(system3 PRIVATE linux/nact_linux.cpp linux/mako.cpp)
	target_link_libraries(system3 PRIVATE PkgConfig::SDL2MIXER)
	set(RESOURCE_PATH ${CMAKE_INSTALL_PREFIX}/share/system3/)
	install(TARGETS system3 RUNTIME DESTINATION bin)
	install(DIRECTORY ../resources/
	  DESTINATION share/system3
	  FILES_MATCHING PATTERN *.MDA)
  endif()
endif()

target_sources(system3 PRIVATE
  sdlmain.cpp
  fileio.cpp
  resource.cpp
  utfsjis.cpp
  texthook.cpp
  sys/ags.cpp
  sys/ags_bmp.cpp
  sys/ags_cursor.cpp
  sys/ags_draw.cpp
  sys/ags_gl3.cpp
  sys/ags_gm3.cpp
  sys/ags_pms.cpp
  sys/ags_text.cpp
  sys/ags_vsp.cpp
  sys/ags_vsp2l.cpp
  sys/ags_window.cpp
  sys/dri.cpp
  sys/mako_midi.cpp
  sys/nact.cpp
  sys/nact_crc32.cpp
  sys/nact_input.cpp
  sys/nact_sys1.cpp
  sys/nact_sys2.cpp
  sys/nact_sys3.cpp
  )

target_compile_definitions(system3 PRIVATE
  $<$<CONFIG:Debug>:_DEBUG_CONSOLE>
  RESOURCE_PATH="${RESOURCE_PATH}")
target_include_directories(system3 PRIVATE . sys)
