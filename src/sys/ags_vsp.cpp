/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - vsp loader ]
*/

#include "ags.h"
#include "game_id.h"
#include <string.h>

void AGS::load_vsp(uint8* data, int page, int transparent)
{
	// ヘッダ取得
	int sx = data[0] | (data[1] << 8);
	int sy = data[2] | (data[3] << 8);
	int ex = data[4] | (data[5] << 8);
	int ey = data[6] | (data[7] << 8);
	int width = ex - sx;
	int height = ey - sy;
	uint8 base = (data[9] & 0x0f) << 4;

	// Jコマンドの処理
	if(set_cg_dest) {
		sx = cg_dest_x >> 3;
		sy = cg_dest_y;
		ex = sx + width;
		ey = sy + height;
		set_cg_dest = false;
	}

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
		for(int i = 0; i < 16; i++) {
			uint32 b = data[p++] & 0xf;
			uint32 r = data[p++] & 0xf;
			uint32 g = data[p++] & 0xf;
			program_palette[base + i] = SETPALETTE16(r, g, b);
		}
	}

	// パレット展開
	if (game_id.is(GameId::FUNNYBEE_FD) || game_id.is(GameId::FUNNYBEE_CD)) {
		if(extract_palette_cg[page]) {
			for(int i = 0; i < 16; i++) {
				screen_palette[base + i] = program_palette[base + i];
			}
		}
	} else {
		if(extract_palette && extract_palette_cg[page]) {
			for(int i = 0; i < 16; i++) {
				screen_palette[base + i] = program_palette[base + i];
			}
		}
	}

	// VSP展開
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

		// VRAMに転送
		if(extract_cg) {
			for(int y = 0; y < height; y++) {
				uint8 b0, b1, b2, b3, c[8];
				b0 = cgdata[0][1][y] = cgdata[0][0][y];
				b1 = cgdata[1][1][y] = cgdata[1][0][y];
				b2 = cgdata[2][1][y] = cgdata[2][0][y];
				b3 = cgdata[3][1][y] = cgdata[3][0][y];
				c[0] = ((b0 >> 7) & 1) | ((b1 >> 6) & 2) | ((b2 >> 5) & 4) | ((b3 >> 4) & 8);
				c[1] = ((b0 >> 6) & 1) | ((b1 >> 5) & 2) | ((b2 >> 4) & 4) | ((b3 >> 3) & 8);
				c[2] = ((b0 >> 5) & 1) | ((b1 >> 4) & 2) | ((b2 >> 3) & 4) | ((b3 >> 2) & 8);
				c[3] = ((b0 >> 4) & 1) | ((b1 >> 3) & 2) | ((b2 >> 2) & 4) | ((b3 >> 1) & 8);
				c[4] = ((b0 >> 3) & 1) | ((b1 >> 2) & 2) | ((b2 >> 1) & 4) | ((b3     ) & 8);
				c[5] = ((b0 >> 2) & 1) | ((b1 >> 1) & 2) | ((b2     ) & 4) | ((b3 << 1) & 8);
				c[6] = ((b0 >> 1) & 1) | ((b1     ) & 2) | ((b2 << 1) & 4) | ((b3 << 2) & 8);
				c[7] = ((b0     ) & 1) | ((b1 << 1) & 2) | ((b2 << 2) & 4) | ((b3 << 3) & 8);

				uint8_t* dest;
				// Gakuen Senki uses exact sx values rather than 8x.
				if (game_id.is(GameId::GAKUEN))
					dest = &vram[dest_screen][y + sy][(x * 8) + sx];
				else
					dest = &vram[dest_screen][y + sy][(x + sx) * 8];

				if(transparent == -1) {
					for(int i = 0; i < 8; i++) {
						dest[i] = c[i] | base;
					}
				} else {
					for(int i = 0; i < 8; i++) {
						if(c[i] != transparent) {
							dest[i] = c[i] | base;
						}
					}
				}
			}
		}
	}

	// 画面更新
	if(dest_screen == 0 && extract_cg) {
		if (game_id.is(GameId::GAKUEN)) {
			// Gakuen Senki's images were converted to VSP for the modern port, but don't always adhere to VSP's width restrictions, which demand
			// every image width be a factor of 8. Thankfully, the exceptions are designed to fit into specific parts of the GUI, and so have
			// consistent widths for each output position.
			//
			// As above, GS also uses exact sx values rather than values that are meant to be multiplied by 8.
			if (sx == 289 && sy == 26) draw_screen(sx, sy, 188, height);
			else if (sx == 289 && sy == 92) draw_screen(sx, sy, 190, height);
			else if (sx == 278 && sy == 208) draw_screen(sx, sy, 212, height);
			else draw_screen(sx, sy, width * 8, height);
		}
		else {
			draw_screen(sx * 8, sy, width * 8, height);
		}
	}
}

