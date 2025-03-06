#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#undef ERROR
#endif
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "common.h"

SDL_IOStream* open_resource(const char* name, const char* type) {
#ifdef _WIN32
	// On Windows, read from resource.
	HINSTANCE hInst = GetModuleHandle(NULL);
	HRSRC hRes = FindResource(hInst, name, type);
	HGLOBAL hGlobal = LoadResource(hInst, hRes);
	if (!hGlobal) {
		WARNING("Cannot load resource %s (type: %s)", name, type);
		return NULL;
	}
	return SDL_IOFromConstMem(LockResource(hGlobal), SizeofResource(hInst, hRes));
#else
	// On Android, read from APK assets.
	// On other platforms, read from a file under RESOURCE_PATH.
	char path[PATH_MAX];
	snprintf(path, PATH_MAX, "%s%s/%s", RESOURCE_PATH, type, name);
	return SDL_IOFromFile(path, "rb");
#endif
}

SDL_IOStream* open_file(const char* name) {
#ifdef __ANDROID__
	// SDL_IOFromFile() does not resolve relative paths on Android.
	char path[PATH_MAX];
	if (!realpath(name, path)) {
		return NULL;
	}
	return SDL_IOFromFile(path, "rb");
#else
	return SDL_IOFromFile(name, "rb");
#endif
}
