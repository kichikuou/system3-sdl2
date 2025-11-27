/*
	ALICE SOFT SYSTEM 3 for Win32

	[ Common Header ]
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#define SYSTEM3_VERSION "1.7.0"

#include <stdint.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#define FONT_RESOURCE_NAME "MTLc3m.ttf"

// type definition
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

#define ERROR sys_error
#define WARNING(fmt, ...) sys_warning("%s:%d: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#define NOTICE SDL_Log

[[noreturn]] void sys_error(const char* format, ...);
void sys_warning(const char* format, ...);

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

extern uint32_t sdl_custom_event_type;
enum CustomEvent {
	DEBUGGER_COMMAND,
};

// resource.cpp
struct SDL_RWops;
SDL_RWops* open_resource(const char* name, const char* type);
SDL_RWops* open_file(const char* name);

#endif
