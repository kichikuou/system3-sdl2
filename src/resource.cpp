#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#undef ERROR
#endif
#include "common.h"
#include "sdl_compat.h"

sdl::IOStream* open_resource(const char* name, const char* type) {
#ifdef _WIN32
	// On Windows, read from resource.
	HINSTANCE hInst = GetModuleHandle(NULL);
	HRSRC hRes = FindResource(hInst, name, type);
	HGLOBAL hGlobal = LoadResource(hInst, hRes);
	if (!hGlobal) {
		WARNING("Cannot load resource %s (type: %s)", name, type);
		return NULL;
	}
	return sdl::IOFromConstMem(LockResource(hGlobal), SizeofResource(hInst, hRes));
#else
	// On Android, read from APK assets.
	// On other platforms, read from a file under RESOURCE_PATH.
	char path[PATH_MAX];
	snprintf(path, PATH_MAX, "%s%s/%s", RESOURCE_PATH, type, name);
	return sdl::IOFromFile(path, "rb");
#endif
}

sdl::IOStream* open_file(const char* name) {
#ifdef __ANDROID__
	// SDL_IOFromFile() treats relative paths as APK assets on Android, so use
	// an absolute path to open files relative to the current game directory.
	char path[PATH_MAX];
	if (!realpath(name, path))
		return NULL;
	return sdl::IOFromFile(path, "rb");
#else
	return sdl::IOFromFile(name, "rb");
#endif
}
