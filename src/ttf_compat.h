#ifndef SYSTEM3_TTF_COMPAT_H_
#define SYSTEM3_TTF_COMPAT_H_

#include "sdl_compat.h"

#if SYSTEM3_SDL_VERSION == 2
#include <SDL_ttf.h>
#elif SYSTEM3_SDL_VERSION == 3
#include <SDL3_ttf/SDL_ttf.h>
#else
#error "Unsupported SYSTEM3_SDL_VERSION"
#endif

static_assert(SDL_TTF_MAJOR_VERSION == SYSTEM3_SDL_VERSION,
			  "SDL_ttf and SDL must use the same major version");

namespace ttf {

inline TTF_Font* OpenFontIO(sdl::IOStream* stream, int point_size)
{
#if SYSTEM3_SDL_VERSION == 2
	return TTF_OpenFontRW(stream, 0, point_size);
#else
	return TTF_OpenFontIO(stream, false, point_size);
#endif
}

inline int GetFontAscent(TTF_Font* font)
{
#if SYSTEM3_SDL_VERSION == 2
	return TTF_FontAscent(font);
#else
	return TTF_GetFontAscent(font);
#endif
}

inline int GetFontDescent(TTF_Font* font)
{
#if SYSTEM3_SDL_VERSION == 2
	return TTF_FontDescent(font);
#else
	return TTF_GetFontDescent(font);
#endif
}

inline bool GetGlyphMetrics(TTF_Font* font, Uint32 code, int* minx, int* maxx,
		int* miny, int* maxy, int* advance)
{
#if SYSTEM3_SDL_VERSION == 2
	return TTF_GlyphMetrics(font, code, minx, maxx, miny, maxy, advance) == 0;
#else
	return TTF_GetGlyphMetrics(font, code, minx, maxx, miny, maxy, advance);
#endif
}

} // namespace ttf

#endif // SYSTEM3_TTF_COMPAT_H_
