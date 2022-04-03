#ifdef _WIN32
#include <windows.h>
#undef ERROR
#endif
#include "common.h"

SDL_RWops* open_resource(const char* name, const char* type) {
#ifdef _WIN32
	// On Windows, read from resource.
	HINSTANCE hInst = GetModuleHandle(NULL);
	HRSRC hRes = FindResource(hInst, name, type);
	HGLOBAL hGlobal = LoadResource(hInst, hRes);
	if (!hGlobal) {
		WARNING("Cannot load resource %s (type: %s)", name, type);
		return NULL;
	}
	return SDL_RWFromConstMem(LockResource(hGlobal), SizeofResource(hInst, hRes));
#else
	// On Android, read from APK assets.
	// On other platforms, read from a file under RESOURCE_PATH.
	char path[_MAX_PATH];
	sprintf_s(path, _MAX_PATH, "%s%s/%s", RESOURCE_PATH, type, name);
	return SDL_RWFromFile(path, "rb");
#endif
}

SDL_RWops* open_file(const char* name) {
#ifdef __ANDROID__
	// We cannot use SDL_RWFromFile() because it does not resolve relative
	// paths using the current directory on Android.
	FILE *fp = fopen(name, "rb");
	if (!fp)
		return NULL;
	return SDL_RWFromFP(fp, SDL_TRUE);
#else
	return SDL_RWFromFile(name, "rb");
#endif
}
