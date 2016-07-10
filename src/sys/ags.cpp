/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS ]
*/

#include "ags.h"
#include "crc32.h"
#include "../fileio.h"

extern _TCHAR g_root[_MAX_PATH];
extern SDL_Window* g_window;

#define SET_TEXT(n, x1, y1, x2, y2, f) { \
	text_w[n].sx = x1; \
	text_w[n].sy = y1; \
	text_w[n].ex = x2; \
	text_w[n].ey = y2; \
	text_w[n].frame = f; \
}
#define SET_MENU(n, x1, y1, x2, y2, f) { \
	menu_w[n].sx = x1; \
	menu_w[n].sy = y1; \
	menu_w[n].ex = x2; \
	menu_w[n].ey = y2; \
	menu_w[n].frame = f; \
}
#define SET_BOX(n, c, x1, y1, x2, y2) { \
	box[n].color = c; \
	box[n].sx = x1; \
	box[n].sy = y1; \
	box[n].ex = x2; \
	box[n].ey = y2; \
}

AGS::AGS(NACT* parent) : nact(parent)
{
	sdlRenderer = SDL_CreateRenderer(g_window, -1, 0);
	SDL_RenderSetLogicalSize(sdlRenderer, 640, 400);
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 640, 400); // TOOD: pixelformat?

	// DIBSection 8bpp * 3 (表, 裏, メニュー)
	for(int i = 0; i < 3; i++) {
		hBmpScreen[i] = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);
		SDL_LockSurface(hBmpScreen[i]);

		// TODO: clear surface
		// memset(lpBmpScreen[i], 0, 640 * 480 * sizeof(DWORD));

		// 仮想VRAMへのポインタ取得
		for(int j = 0; j < 480; j++) {
			vram[i][j] = surface_line(hBmpScreen[i], j);
		}
	}

	// DIBSection 24bpp * 1 (最終出力先)
	hBmpDest = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);
	SDL_LockSurface(hBmpDest);
	// TODO: clear surface

	// フォント
	TTF_Init();
	const char fontfile[] = "MTLc3m.ttf";
	hFont16 = TTF_OpenFont(fontfile, 16);
	hFont24 = TTF_OpenFont(fontfile, 24);
	hFont32 = TTF_OpenFont(fontfile, 32);
	hFont48 = TTF_OpenFont(fontfile, 48);
	hFont64 = TTF_OpenFont(fontfile, 64);

	// カーソル初期化
	for(int i = 0; i < 10; i++) {
		hCursor[i] = NULL;
	}

	// GAIJI.DAT読み込み
	memset(gaiji, 0, sizeof(gaiji));

	FILEIO* fio = new FILEIO();
	_TCHAR file_path[_MAX_PATH];
	_stprintf_s(file_path, _MAX_PATH, _T("%sGAIJI.DAT"), g_root);

	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		int d1, d2;
		while((d1 = fio->Fgetc()) != EOF) {
			d2 = fio->Fgetc();
			// jis to sjis
			d1 += (d2 & 1) ? 0x1f : 0x7d;
			d1 += (d1 > 0x7f) ? 1 : 0;
			d2 = ((d2 - 0x21) >> 1) + 0x81;
			d2 += (d2 > 0x9f) ? 0x40 : 0;
			uint16 code = (d1 & 0xff) | ((d2 & 0xff) << 8);

			if(0xeb9f <= code && code <= 0xebfc) {
				for(int i = 0; i < 32; i++) {
					gaiji[code - 0xeb9f][i] = fio->Fgetc();
				}
			} else if(0xec40 <= code && code <= 0xec9e) {
				for(int i = 0; i < 32; i++) {
					gaiji[code - 0xec40 + 94][i] = fio->Fgetc();
				}
			} else {
				fio->Fseek(32, FILEIO_SEEK_CUR);
			}
		}
		fio->Fclose();
	}
	delete fio;

	// SYSTEM3 初期化

	// ACG.DAT
	_tcscpy_s(acg, 16, _T("ACG.DAT"));

	// パレット
	memset(program_palette, 0, sizeof(program_palette));
	program_palette[0x00] = SETPALETTE16(0x0, 0x0, 0x0);
	program_palette[0x01] = SETPALETTE16(0x0, 0x0, 0xa);
	program_palette[0x02] = SETPALETTE16(0xa, 0x0, 0x0);
	program_palette[0x03] = SETPALETTE16(0xa, 0x0, 0xa);
	program_palette[0x04] = SETPALETTE16(0x0, 0x0, 0x0);
	program_palette[0x05] = SETPALETTE16(0x0, 0xa, 0xa);
	program_palette[0x06] = SETPALETTE16(0xa, 0xa, 0x0);
	program_palette[0x07] = SETPALETTE16(0xd, 0xd, 0xd);
	program_palette[0x08] = SETPALETTE16(0x7, 0x7, 0x7);
	program_palette[0x09] = SETPALETTE16(0x0, 0x0, 0xf);
	program_palette[0x0a] = SETPALETTE16(0xf, 0x0, 0x0);
	program_palette[0x0b] = SETPALETTE16(0xf, 0x0, 0xf);
	program_palette[0x0c] = SETPALETTE16(0x0, 0xf, 0x0);
	program_palette[0x0d] = SETPALETTE16(0x0, 0xf, 0xf);
	program_palette[0x0e] = SETPALETTE16(0xf, 0xf, 0x0);
	program_palette[0x0f] = SETPALETTE16(0xf, 0xf, 0xf);
#if defined(_SYSTEM1)
	program_palette[0x10] = SETPALETTE16(0x0, 0x0, 0x0);
	program_palette[0x11] = SETPALETTE16(0x0, 0x0, 0xf);
	program_palette[0x12] = SETPALETTE16(0xf, 0x0, 0x0);
	program_palette[0x13] = SETPALETTE16(0xf, 0x0, 0xf);
	program_palette[0x14] = SETPALETTE16(0x0, 0xf, 0x0);
	program_palette[0x15] = SETPALETTE16(0x0, 0xf, 0xf);
	program_palette[0x16] = SETPALETTE16(0xf, 0xf, 0x0);
	program_palette[0x17] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0x18] = SETPALETTE16(0x0, 0x0, 0x0);
	program_palette[0x19] = SETPALETTE16(0x0, 0x0, 0xf);
	program_palette[0x1a] = SETPALETTE16(0xf, 0x0, 0x0);
	program_palette[0x1b] = SETPALETTE16(0xf, 0x0, 0xf);
	program_palette[0x1c] = SETPALETTE16(0x0, 0xf, 0x0);
	program_palette[0x1d] = SETPALETTE16(0x0, 0xf, 0xf);
	program_palette[0x1e] = SETPALETTE16(0xf, 0xf, 0x0);
#endif
	program_palette[0x1f] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0x2f] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0x3f] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0x4f] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0x5f] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0x6f] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0x7f] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0x8f] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0x9f] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0xaf] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0xbf] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0xcf] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0xdf] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0xef] = SETPALETTE16(0xf, 0xf, 0xf);
	program_palette[0xff] = SETPALETTE16(0xf, 0xf, 0xf);
	memcpy(screen_palette, program_palette, sizeof(program_palette));

	// Bコマンド
	for(int i = 0; i < 10; i++) {
		// ウィンドウの初期位置はシステムによって異なる
#if defined(_BUNKASAI)
		SET_TEXT(i, 24, 304, 616, 384, false);
		SET_MENU(i, 440, 18, 620, 178, true);
#elif defined(_CRESCENT)
		SET_TEXT(i, 24, 288, 616, 378, false);
		// 本来は横メニュー
		SET_MENU(i, 464, 50, 623, 240, true);
#elif defined(_DPS)
		SET_TEXT(i, 48, 288, 594, 393, false);
		//SET_MENU(i, 48, 288, 584, 393, false);
		SET_MENU(i, 48, 288, 594, 393, false);
#elif defined(_FUKEI)
		SET_TEXT(i, 44, 282, 593, 396, false);
		SET_MENU(i, 460, 14, 635, 214, false);
#elif defined(_INTRUDER)
		SET_TEXT(i, 8, 280, 629, 393, false);
		SET_MENU(i, 448, 136, 623, 340, true);
#elif defined(_TENGU)
		SET_TEXT(i, 44, 282, 593, 396, false);
		SET_MENU(i, 452, 14, 627, 214, false);
#elif defined(_VAMPIRE)
		SET_TEXT(i, 8, 255, 615, 383, false);
		SET_MENU(i, 448, 11, 615, 224, false);
#else
		SET_TEXT(i, 8, 311, 623, 391, true);
		SET_MENU(i, 464, 80, 623, 240, true);
#endif
		text_w[i].push = false;
		text_w[i].screen = NULL;
		text_w[i].window = NULL;
		menu_w[i].push = true;
		menu_w[i].screen = NULL;
		menu_w[i].window = NULL;

	}

	// Eコマンド
	for(int i = 0; i < 20; i++) {
//		SET_BOX(i, 0, 0, 0, 631, 399);
		SET_BOX(i, 0, 0, 0, 639, 399);
	}
#if defined(_SYSTEM2)
	if(nact->crc32 == CRC32_SDPS_TONO || nact->crc32 == CRC32_SDPS_KAIZOKU) {
		SET_BOX(0, 0, 40, 8, 598, 271);
	} else if(nact->crc32 == CRC32_PROSTUDENTG_FD) {
		SET_BOX(0, 0, 64, 13, 407, 289);
		SET_BOX(1, 0, 24, 298, 111, 390);
		SET_BOX(2, 0, 0, 0, 639, 307);
		SET_BOX(3, 0, 0, 0, 319, 399);
		SET_BOX(4, 0, (16*8), 310, ((77*8)-1), 390);
		SET_BOX(5, 0, (4*8), 310, ((76*8)-1), 390);
		SET_BOX(6, 0, (2*8), 317, ((56*8)-1), 389);
		SET_BOX(7, 15, 64, 13, 407, 289);
		SET_BOX(8, 0, 0, 0, 319, 399);
		SET_BOX(9, 0, 320, 0, 639, 40);
	}
#endif

	// 画面選択
	src_screen = dest_screen = 0;

	// 画面サイズ
	screen_height = 400;
	scroll = 400;

	// メッセージ表示
	text_dest_x = text_w[0].sx;
	text_dest_y = text_w[0].sy + 2;
	text_space = 2;
	text_font_size = 16;
#if defined(_SYSTEM1)
	text_font_color = 15 + 16;
	text_frame_color = 15 + 16;
	text_back_color = 0 + 16;
#else
	text_font_color = 15;
	text_frame_color = 15;
	text_back_color = 0;
#endif

	// メニュー表示
	menu_dest_x = 2;
	menu_dest_y = 0;
	menu_font_size = 16;
#if defined(_SYSTEM1)
	menu_font_color = 15 + 16;
	menu_frame_color = 15 + 16;
	menu_back_color = 0 + 16;
#else
	menu_font_color = 15;
	menu_frame_color = 15;
	menu_back_color = 0;
#endif
	menu_fix = false;

	draw_hankaku = false;
	draw_menu = false;

	// CG表示
	set_cg_dest = false;
	cg_dest_x = cg_dest_y = 0;
	for(int i = 0; i < MAX_CG; i++) {
		extract_palette_cg[i] = true;
	}
	get_palette = extract_palette = extract_cg = true;
	palette_bank = -1;

	// マウスカーソル
	cursor_color = 15;
	cursor_index = 0;

	// フェード状態
	fader = false;
	memset(fader_screen, 0, sizeof(fader_screen));
}

AGS::~AGS()
{
	// 退避領域の開放
	for(int i = 0; i < 10; i++) {
		if(menu_w[i].screen) {
			free(menu_w[i].screen);
		}
		if(menu_w[i].window) {
			free(menu_w[i].window);
		}
		if(text_w[i].screen) {
			free(text_w[i].screen);
		}
		if(text_w[i].window) {
			free(text_w[i].window);
		}
	}

	// カーソル開放
	for(int i = 0; i < 10; i++) {
		if(hCursor[i]) {
			SDL_FreeCursor(hCursor[i]);
		}
	}

	// フォント開放
	TTF_CloseFont(hFont16);
	TTF_CloseFont(hFont24);
	TTF_CloseFont(hFont32);
	TTF_CloseFont(hFont48);
	TTF_CloseFont(hFont64);

	// surface開放
	for(int i = 0; i < 3; i++) {
		SDL_FreeSurface(hBmpScreen[i]);
	}

	SDL_FreeSurface(hBmpDest);
}

void AGS::set_palette(int index, int r, int g, int b)
{
	if(index < 16) {
		screen_palette[index] = program_palette[index] = SETPALETTE16(r, g, b);
	} else {
		screen_palette[index] = program_palette[index] = SETPALETTE256(r, g, b);
	}
}

uint8 AGS::get_pixel(int dest, int x, int y)
{
	if(vram[dest][y][x] & 0x80000000) {
		return 0;
	} else {
		return (uint8)vram[dest][y][x];
	}
}

void AGS::set_pixel(int dest, int x, int y, uint8 color)
{
	vram[dest][y][x] = color;
}

void AGS::fade_start()
{
	memcpy(fader_screen, hBmpDest->pixels, sizeof(fader_screen));
}

void AGS::fade_end()
{
	memcpy(hBmpDest->pixels, fader_screen, sizeof(fader_screen));
}

void AGS::fade_out(int depth, bool white)
{
	// 通常画面 → 白黒画面
	int fx = fade_x[depth];
	int fy = fade_y[depth];
	fader = true;

	for(int y = 0; y < 480; y += 4) {
		uint32* dest = surface_line(hBmpDest, y + fy) + fx;
		for(int x = 0; x < 640; x += 4) {
			dest[x] = white ? 0xffffff : 0;
		}
	}

	// 画面更新
	invalidate_screen(0, 0, 640, screen_height);
}

void AGS::fade_in(int depth)
{
	// 白黒画面 → 通常画面
	int fx = fade_x[depth];
	int fy = fade_y[depth];
	fader = (depth == 15) ? false : true;

	for(int y = 0; y < 480; y += 4) {
		uint32* src = &fader_screen[640 * (y + fy) + fx];
		uint32* dest = surface_line(hBmpDest, y + fy) + fx;
		for(int x = 0; x < 640; x += 4) {
			dest[x] = src[x];
		}
	}

	// 画面更新
	invalidate_screen(0, 0, 640, screen_height);
}

void AGS::flush_screen(bool update)
{
	if(update) {
		for(int y = 0; y < screen_height; y++) {
			uint32* src = vram[0][y];
			uint32* dest = surface_line(hBmpDest, y);
			for(int x = 0; x < 640; x++) {
#if defined(_SYSTEM3)
				// あゆみちゃん物語 フルカラー実写版
				if(src[x] & 0x80000000) {
					dest[x] = src[x] & 0xffffff;
				} else
#endif
				dest[x] = screen_palette[src[x] & 0xff];
			}
		}
	}
	invalidate_screen(0, 0, 640, screen_height);
}

void AGS::draw_screen(int sx, int sy, int width, int height)
{
	if(fader) {
		for(int y = sy; y < (sy + height) && y < 480; y++) {
			uint32* src = vram[0][y];
			uint32* dest = &fader_screen[640 * y];
			for(int x = sx; x < (sx + width) && x < 640; x++) {
				uint32 a=src[x];
#if defined(_SYSTEM3)
				// あゆみちゃん物語 フルカラー実写版
				if(src[x] & 0x80000000) {
					dest[x] = src[x] & 0xffffff;
				} else
#endif
				dest[x] = screen_palette[src[x] & 0xff];
			}
		}
	} else {
		for(int y = sy; y < (sy + height) && y < 480; y++) {
			uint32* src = vram[0][y];
			uint32* dest = surface_line(hBmpDest, y);
			for(int x = sx; x < (sx + width) && x < 640; x++) {
				uint32 a=src[x];
#if defined(_SYSTEM3)
				// あゆみちゃん物語 フルカラー実写版
				if(src[x] & 0x80000000) {
					dest[x] = src[x] & 0xffffff;
				} else
#endif
				dest[x] = screen_palette[src[x] & 0xff];
			}
		}
		invalidate_screen(sx, sy, width, height);
	}
}

void AGS::invalidate_screen(int sx, int sy, int width, int height)
{
	SDL_UnlockSurface(hBmpDest);
	int top = screen_height == 400 ? sy + (scroll - 400) : sy;
	if (top < 0) {
		height += top;
		top = 0;
	}
	uint32* pixels = surface_line(hBmpDest, top) + sx;
	if (sy + height > screen_height)
		height = screen_height - sy;
	if (sx + width > hBmpDest->w)
		width = hBmpDest->w - sx;
	SDL_Rect rect = {sx, sy, width, height};
	SDL_UpdateTexture(sdlTexture, &rect, pixels, hBmpDest->pitch);
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);
	SDL_LockSurface(hBmpDest);
}
