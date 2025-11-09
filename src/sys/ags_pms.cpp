/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - pms loader ]
*/

#include "ags.h"
#include "game_id.h"
#include <string.h>

CG AGS::load_pms(int page, const std::vector<uint8_t>& data, bool set_palette, int transparent)
{
	// ヘッダ取得
	int sx = data[0x0] | (data[0x1] << 8);
	int sy = data[0x2] | (data[0x3] << 8);
	int ex = data[0x4] | (data[0x5] << 8);
	int ey = data[0x6] | (data[0x7] << 8);
	int width = ex - sx + 1;
	int height = ey - sy + 1;
	uint16_t mask = data[0xa] | (data[0xb] << 8);

	// Jコマンドの処理
	if(set_cg_dest) {
		sx = cg_dest_x;
		sy = cg_dest_y;
		ex = sx + width - 1;
		ey = sy + height - 1;
		set_cg_dest = false;
	}

	if(get_palette) {
		SDL_Color colors[256];
		int p = 0x20;
		for(int i = 0; i < 16; i++) {
			for(int j = 0; j < 16; j++) {
				uint8_t r = data[p++];
				uint8_t g = data[p++];
				uint8_t b = data[p++];
				colors[i * 16 + j] = {r, g, b, 255};
			}
		}
		if (game_id.is_rance4x() || game_id.is(GameId::HASHIRIONNA2)) {
			// 上下16色は取得しない
			for(int i = 1; i < 15; i++) {
				if (!(mask & 1 << i)) {
					SDL_SetPaletteColors(program_palette, colors + i * 16, i * 16, 16);
				}
			}
		} else if (game_id.is(GameId::MUGEN) && transparent != -1) {
			// Uコマンドでは上下32色は取得しない
			for(int i = 2; i < 14; i++) {
				if (!(mask & 1 << i)) {
					SDL_SetPaletteColors(program_palette, colors + i * 16, i * 16, 16);
				}
			}
		} else {
			for(int i = 0; i < 16; i++) {
				if (!(mask & 1 << i)) {
					SDL_SetPaletteColors(program_palette, colors + i * 16, i * 16, 16);
				}
			}
		}
	}

	// パレット展開
	if (set_palette) {
		if (game_id.is_rance4x() || game_id.is(GameId::HASHIRIONNA2)) {
			// 上下16色は展開しない
			for(int i = 1; i < 15; i++) {
				if (!(mask & 1 << i)) {
					SDL_SetPaletteColors(screen_palette, program_palette->colors + i * 16, i * 16, 16);
				}
			}
		} else if (game_id.is(GameId::MUGEN) && transparent != -1) {
			// Uコマンドでは上下32色は展開しない
			for(int i = 2; i < 14; i++) {
				if (!(mask & 1 << i)) {
					SDL_SetPaletteColors(screen_palette, program_palette->colors + i * 16, i * 16, 16);
				}
			}
		} else {
			for(int i = 0; i < 16; i++) {
				if (!(mask & 1 << i)) {
					SDL_SetPaletteColors(screen_palette, program_palette->colors + i * 16, i * 16, 16);
				}
			}
		}
	}

	// Extract pixel data
	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 8, SDL_PIXELFORMAT_INDEX8);
	if (transparent >= 0) {
		SDL_SetColorKey(surface, SDL_TRUE, transparent);
	}
	std::vector<uint8_t> buf[3];
	buf[0].resize(width);
	buf[1].resize(width);
	buf[2].resize(width);
	size_t p = 0x320;

	for (int y = 0; y < height; y++) {
		int x = 0;
		while (x < width) {
			// Some PMS images in Rance 4.1/4.2 English translation do not have
			// enough pixels for the image size.
			if (p >= data.size()) {
				WARNING("CG #%d: PMS data is incomplete or corrupted.", page);
				return CG(surface, sx, sy);
			}

			uint8_t d1 = data[p++];
			if (d1 == 0xff) {
				int length = data[p++] + 3;
				memcpy(buf[0].data() + x, buf[1].data() + x, length);
				x += length;
			} else if (d1 == 0xfe) {
				int length = data[p++] + 3;
				memcpy(buf[0].data() + x, buf[2].data() + x, length);
				x += length;
			} else if (d1 == 0xfd) {
				int length = data[p++] + 4;
				uint8_t d2 = data[p++];
				memset(buf[0].data() + x, d2, length);
				x += length;
			} else if (d1 == 0xfc) {
				int length = data[p++] + 3;
				uint8_t d2 = data[p++];
				uint8_t d3 = data[p++];
				for(int i = 0; i < length; i++) {
					buf[0][x++] = d2;
					buf[0][x++] = d3;
				}
			} else if (d1 == 0xfb || d1 == 0xfa || d1 == 0xf9 || d1 == 0xf8) {
				buf[0][x++] = data[p++];
			} else {
				buf[0][x++] = d1;
			}
		}

		// Transfer the row to the surface
		memcpy(surface_line(surface, y), buf[0].data(), width);
		buf[2].swap(buf[1]);
		buf[1].swap(buf[0]);
	}

	return CG(surface, sx, sy);
}
