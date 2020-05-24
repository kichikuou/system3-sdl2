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

#ifdef __ANDROID__
#include <android/log.h>
#endif

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

#ifdef __ANDROID__

#define ERROR(fmt, ...) \
	__android_log_print(ANDROID_LOG_ERROR, "System3", "(%s:%d): " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define WARNING(fmt, ...) \
	__android_log_print(ANDROID_LOG_WARN, "System3", "(%s:%d): " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define NOTICE(fmt, ...) \
	__android_log_print(ANDROID_LOG_INFO, "System3", fmt, ##__VA_ARGS__)

#else // __ANDROID__

#define ERROR(fmt, ...) \
	fprintf(stderr, "*ERROR*(%s:%d): " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#define WARNING(fmt, ...) \
	fprintf(stderr, "*WARNING*(%s:%d): " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#define NOTICE(fmt, ...) \
	fprintf(stderr, fmt "\n", ##__VA_ARGS__)

#endif // __ANDROID__

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

// resource.cpp
SDL_RWops* open_resource(const char* name, const char* type);

#endif
