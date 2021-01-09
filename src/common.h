/*
	ALICE SOFT SYSTEM 3 for Win32

	[ Common Header ]
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <SDL.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#define SDL_Delay emscripten_sleep
#endif

#define FONT_RESOURCE_NAME "MTLc3m.ttf"

// type definition
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

#define ERROR(fmt, ...) \
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define WARNING(fmt, ...) \
	SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define NOTICE SDL_Log

#ifndef WIN32

#define _MAX_PATH PATH_MAX
#define sscanf_s sscanf
#define sprintf_s snprintf

inline void strcpy_s(char* dst, size_t n, const char* src)
{
	strncpy(dst, src, n);
	dst[n - 1] = '\0';
}

#endif // !WIN32

static inline bool is_1byte_message(uint8_t c) {
	return c == ' ' || (0xa1 <= c && c <= 0xdd);
}

static inline bool is_2byte_message(uint8_t c) {
	return (0x81 <= c && c <= 0x9f) || 0xe0 <= c;
}

// resource.cpp
SDL_RWops* open_resource(const char* name, const char* type);

#endif
