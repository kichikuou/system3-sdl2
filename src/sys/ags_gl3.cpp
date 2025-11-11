/*
	ALICE SOFT SYSTEM 1 for Win32

	[ AGS - gl3 loader ]
*/

#include "ags.h"
#include <string.h>

CG AGS::load_gl3(const std::vector<uint8_t>& data, bool set_palette, int transparent)
{
	// ヘッダ取得
	uint16 tmp = (data[0x30] | (data[0x31] << 8)) - 0x8000;
	int sx = tmp % 80;
	int sy = tmp / 80;
	int width = data[0x32] | (data[0x33] << 8);
	int height = data[0x34] | (data[0x35] << 8);
	uint8 base = 0;

	// Zコマンドの処理
	if(palette_bank != -1) {
		base = (palette_bank & 0x0f) << 4;
	}

	// パレット取得
	if(get_palette) {
		SDL_Color colors[16];
		for(int i = 0; i < 16; i++) {
			uint8_t b = (data[i * 3 + 0] & 0xf) * 0x11;
			uint8_t r = (data[i * 3 + 1] & 0xf) * 0x11;
			uint8_t g = (data[i * 3 + 2] & 0xf) * 0x11;
			colors[i] = {r, g, b, 255};
		}
		SDL_SetPaletteColors(program_palette, colors, base, 16);
	}

	// パレット展開
	if (set_palette) {
		SDL_SetPaletteColors(screen_palette, program_palette->colors + base, base, 16);
	}

	// GL3展開
	CG cg(sx * 8, sy, width * 8, height);
	if (transparent >= 0) {
		SDL_SetColorKey(cg.surface(), SDL_TRUE, transparent | base);
	}
	uint8 cgdata[4][80][3];
	int p = 0x36;
	memset(cgdata, 0, sizeof(cgdata));

	for(int y = 0; y < height; y++) {
		for(int pl = 0; pl < 4; pl++) {
			int x = 0;
			while(x < width) {
				uint8 d1 = data[p++];
				if(d1 == 0x0) {
					uint8 d2 = data[p++];
					if(d2 < 0x80) {
						uint8 d3 = data[p++];
						for(int i = 0; i < d2; i++) {
							cgdata[pl][x++][0] = d3;
						}
					} else {
						uint8 d3 = cgdata[pl][x + 1][2];
						for(int i = 0; i < d2 - 0x80; i++) {
							cgdata[pl][x++][0] = d3;
						}
					}
				} else if(d1 == 0x1) {
					cgdata[pl][x++][0] = data[p++];
				} else if(0x2 <= d1 && d1 <= 0x7) {
					uint8 d2 = data[p++];
					for(int i = 0; i < d1; i++) {
						cgdata[pl][x++][0] = d2;
					}
				} else if(0x8 <= d1 && d1 <= 0xd) {
					uint8 d2 = cgdata[pl][x + 1][2];
					for(int i = 0; i < d1 - 6; i++) {
						cgdata[pl][x++][0] = d2;
					}
				} else if(d1 == 0xe) {
					uint8 d2 = data[p++];
					for(int i = 0; i < d2; i++) {
						cgdata[pl][x][0] = cgdata[2][x][0];
						x++;
					}
				} else if(d1 == 0xf) {
					uint8 d2 = data[p++];
					if(d2 < 0x80) {
						for(int i = 0; i < d2; i++) {
							cgdata[pl][x][0] = cgdata[0][x][0];
							x++;
						}
					} else {
						for(int i = 0; i < d2 - 0x80; i++) {
							cgdata[pl][x][0] = cgdata[1][x][0];
							x++;
						}
					}
				} else {
					cgdata[pl][x++][0] = d1;
				}
			}
		}

		// Transfer to the surface
		for (int x = 0; x < width; x++) {
			for (int pl = 0; pl < 4; pl++) {
				cgdata[pl][x][2] = cgdata[pl][x][1];
				cgdata[pl][x][1] = cgdata[pl][x][0];
			}
			uint8 b0, b1, b2, b3;
			b0 = cgdata[0][x][0];
			b1 = cgdata[1][x][0];
			b2 = cgdata[2][x][0];
			b3 = cgdata[3][x][0];
			uint8_t* dest = surface_line(cg.surface(), y) + x * 8;
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

	return cg;
}

