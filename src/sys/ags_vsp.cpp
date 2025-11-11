/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - vsp loader ]
*/

#include "ags.h"
#include "game_id.h"
#include <string.h>

namespace {

void trim(CG& cg, int w, int h)
{
	SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 8, SDL_PIXELFORMAT_INDEX8);
	SDL_SetSurfacePalette(sf, cg.palette());
	if (SDL_HasColorKey(cg.surface())) {
		Uint32 key;
		SDL_GetColorKey(cg.surface(), &key);
		SDL_SetColorKey(sf, SDL_TRUE, key);
	}
	SDL_Rect srcrect = { 0, 0, w, h };
	SDL_BlitSurface(cg.surface(), &srcrect, sf, NULL);
	cg.surface_.reset(sf);
}

}  // namespace

CG AGS::load_vsp(const std::vector<uint8_t>& data, bool set_palette, int transparent)
{
	// ヘッダ取得
	int sx = data[0] | (data[1] << 8);
	int sy = data[2] | (data[3] << 8);
	int ex = data[4] | (data[5] << 8);
	int ey = data[6] | (data[7] << 8);
	int width = ex - sx;
	int height = ey - sy;
	uint8 base = (data[9] & 0x0f) << 4;

	// Zコマンドの処理
	if (game_id.is(GameId::AMBIVALENZ_FD) || game_id.is(GameId::AMBIVALENZ_CD)) {
		// Z 0,numを無視する
	} else {
		if(palette_bank != -1) {
			base = (palette_bank & 0x0f) << 4;
		}
	}

	// パレット取得
	if(get_palette && transparent != 101) {
		int p = 0x0a;
		SDL_Color colors[16];
		for(int i = 0; i < 16; i++) {
			uint8_t b = (data[p++] & 0xf) * 0x11;
			uint8_t r = (data[p++] & 0xf) * 0x11;
			uint8_t g = (data[p++] & 0xf) * 0x11;
			colors[i] = {r, g, b, 255};
		}
		SDL_SetPaletteColors(program_palette, colors, base, 16);
	}

	// パレット展開
	if (set_palette) {
		SDL_SetPaletteColors(screen_palette, program_palette->colors + base, base, 16);
	}

	// VSP展開
	SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, width * 8, height, 8, SDL_PIXELFORMAT_INDEX8);
	if (transparent >= 0) {
		SDL_SetColorKey(surface, SDL_TRUE, transparent | base);
	}
	uint8 cgdata[4][2][480], mask = 0;
	int p = 0x3a;
	memset(cgdata, 0, sizeof(cgdata));

	for(int x = 0; x < width; x++) {
		for(int pl = 0;  pl < 4; pl++) {
			int y = 0;
			while(y < height) {
				uint8 d1 = data[p++];
				if(d1 == 0) {
					int length = data[p++] + 1;
					for(int i = 0; i < length; i++) {
						cgdata[pl][0][y] = cgdata[pl][1][y];
						y++;
					}
				} else if(d1 == 1) {
					int length = data[p++] + 1;
					uint8 d2 = data[p++];
					for(int i = 0; i < length; i++) {
						cgdata[pl][0][y++] = d2;
					}
				} else if(d1 == 2) {
					int length = data[p++] + 1;
					uint8 d2 = data[p++];
					uint8 d3 = data[p++];
					for(int i = 0; i < length; i++) {
						cgdata[pl][0][y++] = d2;
						cgdata[pl][0][y++] = d3;
					}
				} else if(d1 == 3) {
					int length = data[p++] + 1;
					for(int i = 0; i < length; i++) {
						cgdata[pl][0][y] = cgdata[0][0][y] ^ mask;
						y++;
					}
					mask = 0;
				} else if(d1 == 4) {
					int length = data[p++] + 1;
					for(int i = 0; i < length; i++) {
						cgdata[pl][0][y] = cgdata[1][0][y] ^ mask;
						y++;
					}
					mask = 0;
				} else if(d1 == 5) {
					int length = data[p++] + 1;
					for(int i = 0; i < length; i++) {
						cgdata[pl][0][y] = cgdata[2][0][y] ^ mask;
						y++;
					}
					mask = 0;
				} else if(d1 == 6) {
					mask = 0xff;
				} else if(d1 == 7) {
					cgdata[pl][0][y++] = data[p++];
				} else {
					cgdata[pl][0][y++] = d1;
				}
			}
		}

		// Transfer to the surface
		for (int y = 0; y < height; y++) {
			uint8 b0, b1, b2, b3;
			b0 = cgdata[0][1][y] = cgdata[0][0][y];
			b1 = cgdata[1][1][y] = cgdata[1][0][y];
			b2 = cgdata[2][1][y] = cgdata[2][0][y];
			b3 = cgdata[3][1][y] = cgdata[3][0][y];
			uint8_t* dest = surface_line(surface, y) + x * 8;
			dest[0] = ((b0 >> 7) & 1) | ((b1 >> 6) & 2) | ((b2 >> 5) & 4) | ((b3 >> 4) & 8) | base;
			dest[1] = ((b0 >> 6) & 1) | ((b1 >> 5) & 2) | ((b2 >> 4) & 4) | ((b3 >> 3) & 8) | base;
			dest[2] = ((b0 >> 5) & 1) | ((b1 >> 4) & 2) | ((b2 >> 3) & 4) | ((b3 >> 2) & 8) | base;
			dest[3] = ((b0 >> 4) & 1) | ((b1 >> 3) & 2) | ((b2 >> 2) & 4) | ((b3 >> 1) & 8) | base;
			dest[4] = ((b0 >> 3) & 1) | ((b1 >> 2) & 2) | ((b2 >> 1) & 4) | ((b3     ) & 8) | base;
			dest[5] = ((b0 >> 2) & 1) | ((b1 >> 1) & 2) | ((b2     ) & 4) | ((b3 << 1) & 8) | base;
			dest[6] = ((b0 >> 1) & 1) | ((b1     ) & 2) | ((b2 << 1) & 4) | ((b3 << 2) & 8) | base;
			dest[7] = ((b0     ) & 1) | ((b1 << 1) & 2) | ((b2 << 2) & 4) | ((b3 << 3) & 8) | base;
		}
	}

	if (game_id.is(GameId::GAKUEN)) {
		// Gakuen Senki uses exact sx values rather than 8x.
		CG cg(surface, sx, sy);
		// Gakuen Senki's images were converted to VSP for the modern port, but don't always adhere to VSP's width restrictions, which demand
		// every image width be a factor of 8. Thankfully, the exceptions are designed to fit into specific parts of the GUI, and so have
		// consistent widths for each output position.
		if (sx == 289 && sy == 26) trim(cg, 188, surface->h);
		else if (sx == 289 && sy == 92) trim(cg, 190, surface->h);
		else if (sx == 278 && sy == 208) trim(cg, 212, surface->h);
		return cg;
	}
	return CG(surface, sx * 8, sy);
}

