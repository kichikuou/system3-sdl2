/*
	ALICE SOFT SYSTEM 1 for Win32

	[ AGS - vsp2l loader ]
*/

#include "ags.h"
#include <string.h>

void AGS::load_vsp2l(uint8* data, int page, int transparent)
{
	// ヘッダ取得
	int sx = data[0] | (data[1] << 8);
	int sy = data[2] | (data[3] << 8);
	int ex = data[4] | (data[5] << 8);
	int ey = data[6] | (data[7] << 8);
	int width = ex - sx;
	int height = ey - sy;
	uint8 base = 0;//(data[9] & 0x0f) << 4;

	// Jコマンドの処理
	if(set_cg_dest) {
		sx = cg_dest_x >> 3;
		sy = cg_dest_y;
		ex = sx + width;
		ey = sy + height;
		set_cg_dest = false;
	}

	// Zコマンドの処理
	if(palette_bank != -1) {
		base = (palette_bank & 0x0f) << 4;
	}

	// パレット取得
	screen_palette[base + 0] = SETPALETTE16(0x0, 0x0, 0x0);
	screen_palette[base + 1] = SETPALETTE16(0x0, 0x0, 0xf);
	screen_palette[base + 2] = SETPALETTE16(0xf, 0x0, 0x0);
	screen_palette[base + 3] = SETPALETTE16(0xf, 0x0, 0xf);
	screen_palette[base + 4] = SETPALETTE16(0x0, 0xf, 0x0);
	screen_palette[base + 5] = SETPALETTE16(0x0, 0xf, 0xf);
	screen_palette[base + 6] = SETPALETTE16(0xf, 0xf, 0x0);
	screen_palette[base + 7] = SETPALETTE16(0xf, 0xf, 0xf);

	// VSP2L展開
	uint8 cgdata[3][2][200], mask = 0;
	int p = 0x1a;
	memset(cgdata, 0, sizeof(cgdata));

	for(int x = 0; x < width; x++) {
		for(int pl = 0;  pl < 3; pl++) {
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

		// VRAMに転送
		if(extract_cg) {
			for(int y = 0; y < height; y++) {
				uint8 b0, b1, b2, c[8];
				b0 = cgdata[0][1][y] = cgdata[0][0][y];
				b1 = cgdata[1][1][y] = cgdata[1][0][y];
				b2 = cgdata[2][1][y] = cgdata[2][0][y];
				c[0] = ((b0 >> 7) & 1) | ((b1 >> 6) & 2) | ((b2 >> 5) & 4);
				c[1] = ((b0 >> 6) & 1) | ((b1 >> 5) & 2) | ((b2 >> 4) & 4);
				c[2] = ((b0 >> 5) & 1) | ((b1 >> 4) & 2) | ((b2 >> 3) & 4);
				c[3] = ((b0 >> 4) & 1) | ((b1 >> 3) & 2) | ((b2 >> 2) & 4);
				c[4] = ((b0 >> 3) & 1) | ((b1 >> 2) & 2) | ((b2 >> 1) & 4);
				c[5] = ((b0 >> 2) & 1) | ((b1 >> 1) & 2) | ((b2     ) & 4);
				c[6] = ((b0 >> 1) & 1) | ((b1     ) & 2) | ((b2 << 1) & 4);
				c[7] = ((b0     ) & 1) | ((b1 << 1) & 2) | ((b2 << 2) & 4);

				uint32* dest0 = &vram[dest_screen][(y + sy) * 2 + 0][(x + sx) * 8];
				uint32* dest1 = &vram[dest_screen][(y + sy) * 2 + 1][(x + sx) * 8];
				if(transparent == -1) {
					for(int i = 0; i < 8; i++) {
						dest0[i] = dest1[i] = c[i] | base;
					}
				} else {
					for(int i = 0; i < 8; i++) {
						if(c[i] != transparent) {
							dest0[i] = dest1[i] = c[i] | base;
						}
					}
				}
			}
		}
	}

	// 画面更新
	if(dest_screen == 0 && extract_cg) {
		draw_screen(sx * 8, sy * 2, width * 8, height * 2);
	}
}

