/*
	ALICE SOFT SYSTEM 3 for Win32

	[ CG ]
*/

#ifndef _CG_H_
#define _CG_H_

#include <memory>
#include "sdl_compat.h"
#include "common.h"

inline uint8_t* surface_line(SDL_Surface* surface, int y)
{
	return static_cast<uint8*>(surface->pixels) + surface->pitch * y;
}

struct SurfaceDeleter {
	void operator()(SDL_Surface* s) const noexcept {
		if (s) sdl::DestroySurface(s);
	}
};

struct CG {
	std::unique_ptr<SDL_Surface, SurfaceDeleter> surface_;
	int x;
	int y;

	CG() = default;
	CG(int x, int y, int width, int height)
		: surface_(sdl::CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8)),
		  x(x), y(y) {}
	explicit operator bool() const noexcept { return static_cast<bool>(surface_); }

	SDL_Surface* surface() const { return surface_.get(); }
	int width() const { return surface_->w; }
	int height() const { return surface_->h; }
	SDL_Palette* palette() const { return sdl::GetSurfacePalette(surface_.get()); }
};

#endif // _CG_H_
