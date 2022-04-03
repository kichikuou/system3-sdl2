/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - draw ]
*/

#include "ags.h"
#include <string.h>

extern SDL_Window* g_window;

void AGS::load_cursor(int page)
{
	// カーソルCGをロードする
	bool j_set = set_cg_dest;
	int j_x = cg_dest_x;
	int j_y = cg_dest_y;
	int dest = dest_screen;

	set_cg_dest = true;
	cg_dest_x = 0;
	cg_dest_y = 0;
	dest_screen = 1;

	load_cg(page, -1);

	set_cg_dest = j_set;
	cg_dest_x = j_x;
	cg_dest_y = j_y;
	dest_screen = dest;

	// フォントの生成
	for(int i = 0; i < 10; i++) {
		// パターン読み込み
		uint8 pat[34][34];
		memset(pat, 0, sizeof(pat));

		for(int y = 0; y < 32; y++) {
			for(int x = 0; x < 32; x++) {
				pat[y + 1][x + 1] = (vram[1][y >> 1][(x >> 1) + 16 * i] & 0xf) ? 1 : 0;
			}
		}

		// パターン縁取り
		for(int y = 1; y <= 32; y++) {
			for(int x = 1; x <= 32; x++) {
				if(pat[y][x] == 0 && (pat[y - 1][x] == 1 || pat[y + 1][x] == 1 || pat[y][x - 1] == 1 || pat[y][x + 1] == 1)) {
					pat[y][x] = 2;
				}
			}
		}

		// フォント生成
		uint8 amask[128], xmask[128];
		memset(amask, 0, sizeof(amask));
		memset(xmask, 0, sizeof(xmask));

		for(int y = 0; y < 32; y++) {
			for(int x = 0; x < 32; x += 8) {
				amask[y * 4 + (x >> 3)]  = (pat[y + 1][x + 1] == 0) ? 0x80 : 0;
				amask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 2] == 0) ? 0x40 : 0;
				amask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 3] == 0) ? 0x20 : 0;
				amask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 4] == 0) ? 0x10 : 0;
				amask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 5] == 0) ? 0x08 : 0;
				amask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 6] == 0) ? 0x04 : 0;
				amask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 7] == 0) ? 0x02 : 0;
				amask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 8] == 0) ? 0x01 : 0;

				xmask[y * 4 + (x >> 3)]  = (pat[y + 1][x + 1] == 1) ? 0x80 : 0;
				xmask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 2] == 1) ? 0x40 : 0;
				xmask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 3] == 1) ? 0x20 : 0;
				xmask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 4] == 1) ? 0x10 : 0;
				xmask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 5] == 1) ? 0x08 : 0;
				xmask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 6] == 1) ? 0x04 : 0;
				xmask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 7] == 1) ? 0x02 : 0;
				xmask[y * 4 + (x >> 3)] |= (pat[y + 1][x + 8] == 1) ? 0x01 : 0;
			}
		}
		if(hCursor[i]) {
			SDL_FreeCursor(hCursor[i]);
		}
		// TODO: fix amask/xmask values
		hCursor[i] = SDL_CreateCursor(amask, xmask, 32, 32, 2, 2);
	}
}

void AGS::select_cursor()
{
	if(cursor_index == 0) {
		SDL_SetCursor(SDL_GetDefaultCursor());
	} else if(1 <= cursor_index && cursor_index <= 10 && hCursor[cursor_index - 1]) {
		SDL_SetCursor(hCursor[cursor_index - 1]);
	}
}

void AGS::translate_mouse_coords(int* x, int* y)
{
	// scale mouse x and y
	float scalex, scaley;
	SDL_RenderGetScale(sdlRenderer, &scalex, &scaley);
	*x *= scalex;
	*y *= scaley;

	// calculate window borders
	int logw, logh;
	SDL_RenderGetLogicalSize(sdlRenderer, &logw, &logh);

	float scalew, scaleh;
	scalew = logw * scalex;
	scaleh = logh * scaley;

	int winw, winh;
	SDL_GetWindowSize(g_window, &winw, &winh);

	float border_left = (winw - scalew) / 2;
	float border_top  = (winh - scaleh) / 2;

	// offset x and y by window borders
	*x += border_left;
	*y += border_top;
}
