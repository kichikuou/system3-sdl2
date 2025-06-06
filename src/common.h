/*
	ALICE SOFT SYSTEM 3 for Win32

	[ Common Header ]
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#define SYSTEM3_VERSION "1.6.1"

#include <SDL.h>
#include <limits.h>
#include <stdio.h>

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
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

#define ERROR sys_error
#define WARNING(fmt, ...) sys_warning("%s:%d: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#define NOTICE SDL_Log

[[noreturn]] void sys_error(const char* format, ...);
void sys_warning(const char* format, ...);

#ifndef WIN32

#define _MAX_PATH PATH_MAX
#define sscanf_s sscanf

inline void strcpy_s(char* dst, size_t n, const char* src)
{
	strncpy(dst, src, n);
	dst[n - 1] = '\0';
}

#endif // !WIN32

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

extern uint32_t sdl_custom_event_type;
enum CustomEvent {
	DEBUGGER_COMMAND,
};

// resource.cpp
SDL_RWops* open_resource(const char* name, const char* type);
SDL_RWops* open_file(const char* name);

#endif
