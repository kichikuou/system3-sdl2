/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS ]
*/

#include "ags.h"
#include <string.h>
#include "nact.h"
#include "game_id.h"
#include "../config.h"
#include "../fileio.h"

extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;
static SDL_Surface* display_surface;

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

namespace {

const uint32 SCANLINE_ALPHA = 0x38;  // 0-255

SDL_Texture* create_scanline_texture(SDL_Renderer* renderer, int width, int height)
{
	SDL_Surface* sf = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
	for (int y = 0; y < height; y++) {
		uint32* p = surface_line(sf, y);
		uint32 v = y % 2 ? (SCANLINE_ALPHA << 24) : 0;
		for (int x = 0; x < width; x++) {
			p[x] = v;
		}
	}
	SDL_Texture* tx = SDL_CreateTextureFromSurface(renderer, sf);
	SDL_SetTextureBlendMode(tx, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(sf);
	return tx;
}

} // namespace

AGS::AGS(const Config& config, const GameId& game_id) : game_id(game_id), dirty(false)
{
	// 画面サイズ
	if (game_id.is(GameId::GAKUEN)) {
		window_width = 582;
		screen_width = 512;
		window_height = screen_height = 424;
	} else {
		window_width = screen_width = 640;
		window_height = screen_height = 400;
	}
	scroll = screen_height;

	SDL_SetWindowSize(g_window, window_width, window_height);
	SDL_RenderSetLogicalSize(g_renderer, window_width, window_height);
	sdlTexture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, screen_width, screen_height); // TOOD: pixelformat?
	scanline_texture = NULL;

	// DIBSection 8bpp * 3 (表, 裏, メニュー)
	for(int i = 0; i < 3; i++) {
		hBmpScreen[i] = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);

		// TODO: clear surface
		// memset(lpBmpScreen[i], 0, 640 * 480 * sizeof(DWORD));

		// 仮想VRAMへのポインタ取得
		for(int j = 0; j < 480; j++) {
			vram[i][j] = surface_line(hBmpScreen[i], j);
		}
	}

	// DIBSection 24bpp * 1 (最終出力先)
	display_surface = hBmpDest = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);
	// TODO: clear surface

	// フォント
	TTF_Init();
	if (!config.font_file.empty()) {
		rw_font = open_file(config.font_file.c_str());
		if (!rw_font)
			sys_error("Cannot open font file %s", config.font_file.c_str());
	} else {
		rw_font = open_resource(FONT_RESOURCE_NAME, "fonts");
		if (!rw_font)
			sys_error("Cannot open default font");
	}
	hFont16 = TTF_OpenFontRW(rw_font, 0, 16);
	SDL_RWseek(rw_font, 0, SEEK_SET);
	hFont24 = TTF_OpenFontRW(rw_font, 0, 24);
	SDL_RWseek(rw_font, 0, SEEK_SET);
	hFont32 = TTF_OpenFontRW(rw_font, 0, 32);
	SDL_RWseek(rw_font, 0, SEEK_SET);
	hFont48 = TTF_OpenFontRW(rw_font, 0, 48);
	SDL_RWseek(rw_font, 0, SEEK_SET);
	hFont64 = TTF_OpenFontRW(rw_font, 0, 64);
	if (!hFont16 || !hFont24 || !hFont32 || !hFont48 || !hFont64) {
		sys_error("TTF_OpenFontRW failed: %s", TTF_GetError());
	}
	if (config.no_antialias)
		ags_setAntialiasedStringMode(0);

	// カーソル初期化
	for(int i = 0; i < 10; i++) {
		hCursor[i] = NULL;
	}

	// GAIJI.DAT読み込み
	memset(gaiji, 0, sizeof(gaiji));

	auto fio = FILEIO::open("GAIJI.DAT", FILEIO_READ_BINARY);
	if (fio) {
		int row, cell;
		while ((cell = fio->getc()) != EOF) {
			row = fio->getc();
			if (0x76 <= row && row <= 0x77 && 0x21 <= cell && cell <= 0x7e) {
				int idx = (row - 0x76) * 94 + (cell - 0x21);
				fio->read(gaiji[idx], 32);
			} else {
				fio->seek(32, SEEK_CUR);
			}
		}
	}
	fio.reset();

	// SYSTEM3 初期化

	acg.open("ACG.DAT");

	// パレット
	program_palette = {0};
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
	if (game_id.sys_ver == 1) {
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
	}
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

	// To improve the quality of text antialiasing, preset the antialiasing
	// colors for text drawn in the primary colors (red, green, blue, yellow,
	// magenta, cyan, and white) on a black background.
	for (int i = 1; i <= 7; i++) {
		for (int j = 0; j <= 7; j++) {
			int n = 255 * j / 7;
			int r = (i & 4) ? n : 0;
			int g = (i & 2) ? n : 0;
			int b = (i & 1) ? n : 0;
			program_palette[0xc0 + i * 8 + j] = SETPALETTE16(r, g, b);
		}
	}

	screen_palette = program_palette;

	// Bコマンド
	for(int i = 0; i < 10; i++) {
		// ウィンドウの初期位置はシステムによって異なる
		switch (game_id.game) {
		case GameId::BUNKASAI:
			SET_TEXT(i, 24, 304, 616, 384, false);
			SET_MENU(i, 440, 18, 620, 178, true);
			break;
		case GameId::CRESCENT:
			SET_TEXT(i, 24, 288, 616, 378, false);
			// 本来は横メニュー
			SET_MENU(i, 464, 50, 623, 240, true);
			break;
		case GameId::RANCE2:
		case GameId::RANCE2_HINT:
			SET_TEXT(i, 8, 285, 502, 396, false);
			SET_MENU(i, 431, 19, 624, 181, false);
			break;
		case GameId::DPS:
		case GameId::DPS_SG_FAHREN:
		case GameId::DPS_SG_KATEI:
		case GameId::DPS_SG_NOBUNAGA:
		case GameId::DPS_SG2_ANTIQUE:
		case GameId::DPS_SG2_IKENAI:
		case GameId::DPS_SG2_AKAI:
		case GameId::DPS_SG3_RABBIT:
		case GameId::DPS_SG3_SHINKON:
		case GameId::DPS_SG3_SOTSUGYOU:
			SET_TEXT(i, 48, 288, 594, 393, false);
			//SET_MENU(i, 48, 288, 584, 393, false);
			SET_MENU(i, 48, 288, 594, 393, false);
			break;
		case GameId::FUKEI:
			SET_TEXT(i, 44, 282, 593, 396, false);
			SET_MENU(i, 460, 14, 635, 214, false);
			break;
		case GameId::INTRUDER:
			SET_TEXT(i, 8, 280, 629, 393, false);
			SET_MENU(i, 448, 136, 623, 340, true);
			break;
		case GameId::TENGU:
			SET_TEXT(i, 44, 282, 593, 396, false);
			SET_MENU(i, 452, 14, 627, 214, false);
			break;
		case GameId::TOUSHIN_HINT:
			SET_TEXT(i, 8, 311, 623, 391, false);
			SET_MENU(i, 452, 14, 627, 214, true);
			break;
		case GameId::LITTLE_VAMPIRE:
			SET_TEXT(i, 8, 255, 615, 383, false);
			SET_MENU(i, 448, 11, 615, game_id.language == ENGLISH ? 234 : 224, false);
			break;
		case GameId::YAKATA:
			SET_TEXT(i, 48, 288, 594, 393, false);
			SET_MENU(i, 452, 14, 627, 214, false);
			break;
		case GameId::DALK_HINT:
			SET_TEXT(i, 24, 308, 376, 386, false);
			SET_MENU(i, 404, 28, 604, 244, true);
			break;
		case GameId::RANCE3_HINT:
			SET_TEXT(i, 104, 304, 615, 383, false);
			SET_MENU(i, 464, 24, 623, 200, true);
			break;
		case GameId::YAKATA2:
			SET_TEXT(i, 104, 304, 620, 382, false);
			SET_MENU(i, 420, 28, 620, 244, true);
			break;
		case GameId::GAKUEN:
			SET_TEXT(i, 8, 260, 505, 384, false);
			if (i == 1) {
				SET_MENU(i, 128, 32, 337, 178, true);
			} else {
				SET_MENU(i, 288, 30, 433, 210, true);
			}
			break;
		default:
			SET_TEXT(i, 8, 311, 623, 391, true);
			SET_MENU(i, 464, 80, 623, 240, true);
			break;
		}
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
	if (game_id.sys_ver == 2) {
		if (game_id.is(GameId::SDPS_TONO) || game_id.is(GameId::SDPS_KAIZOKU)) {
			SET_BOX(0, 0, 40, 8, 598, 271);
		} else if (game_id.is(GameId::PROG_FD)) {
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
	}

	// 画面選択
	src_screen = dest_screen = 0;

	// メッセージ表示
	text_dest_x = text_w[0].sx;
	text_dest_y = text_w[0].sy + 2;
	text_font_maxsize = 0;
	text_space = 2;
	text_font_size = 16;
	if (game_id.sys_ver == 1) {
		text_font_color = 15 + 16;
		text_frame_color = 15 + 16;
		text_back_color = 0 + 16;
	} else {
		text_font_color = 15;
		text_frame_color = 15;
		text_back_color = 0;
	}

	// メニュー表示
	menu_dest_x = 2;
	menu_dest_y = 0;
	menu_font_size = 16;
	if (game_id.sys_ver == 1) {
		menu_font_color = 15 + 16;
		menu_frame_color = 15 + 16;
		menu_back_color = 0 + 16;
	} else {
		menu_font_color = 15;
		menu_frame_color = 15;
		menu_back_color = 0;
	}
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

	set_scanline_mode(config.scanline);
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
	if (rw_font) {
		TTF_CloseFont(hFont16);
		TTF_CloseFont(hFont24);
		TTF_CloseFont(hFont32);
		TTF_CloseFont(hFont48);
		TTF_CloseFont(hFont64);
		SDL_RWclose(rw_font);
	}

	// surface開放
	for(int i = 0; i < 3; i++) {
		SDL_FreeSurface(hBmpScreen[i]);
	}

	SDL_FreeSurface(hBmpDest);

	SDL_DestroyTexture(sdlTexture);
}

void AGS::set_cg_file(const char *file_name)
{
	bmp_prefix = NULL;
	if (game_id.sys_ver == 3) {
		// あゆみちゃん物語 フルカラー実写版
		if (!strcmp(file_name, "CGA000.BMP")) {
			bmp_prefix = "CGA";
			return;
		} else if (!strcmp(file_name, "CGB000.BMP")) {
			bmp_prefix = "CGB";
			return;
		}
	}
	acg.open(file_name);
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

void AGS::fade_out(int duration_ms, bool white)
{
	if (fade_level)
		return;

	fade_color = white ? 255 : 0;

	Uint32 dwStart = SDL_GetTicks();
	while (fade_level < 255) {
		g_nact->sys_sleep(16);
		int t = SDL_GetTicks() - dwStart;
		fade_level = t >= duration_ms ? 255 : t * 255 / duration_ms;
		dirty = true;
	}
	update_screen();
}

void AGS::fade_in(int duration_ms)
{
	if (fade_level == 0)
		return;

	Uint32 dwStart = SDL_GetTicks();
	while (fade_level > 0) {
		g_nact->sys_sleep(16);
		int t = SDL_GetTicks() - dwStart;
		fade_level = t >= duration_ms ? 0 : (duration_ms - t) * 255 / duration_ms;
		dirty = true;
	}
	update_screen();
}

void AGS::flush_screen(bool update)
{
	if(update) {
		for(int y = 0; y < screen_height; y++) {
			uint32* src = vram[0][y];
			uint32* dest = surface_line(hBmpDest, y);
			for(int x = 0; x < screen_width; x++) {
				if (game_id.sys_ver == 3 && src[x] & 0x80000000) {
					// あゆみちゃん物語 フルカラー実写版
					dest[x] = src[x] & 0xffffff;
				} else {
					dest[x] = screen_palette[src[x] & 0xff];
				}
			}
		}
	}
	invalidate_screen(0, 0, screen_width, screen_height);
}

void AGS::draw_screen(int sx, int sy, int width, int height)
{
	SDL_Rect rect = {sx, sy, width, height};
	SDL_Rect screen = {0, 0, screen_width, screen_height};
	SDL_Rect clip;
	SDL_IntersectRect(&rect, &screen, &clip);

	for (int y = clip.y; y < clip.y + clip.h; y++) {
		uint32* src = vram[0][y];
		uint32* dest = surface_line(hBmpDest, y);
		for(int x = clip.x; x < clip.x + clip.w; x++) {
			uint32 a=src[x];
			if (game_id.sys_ver == 3 && src[x] & 0x80000000) {
				// あゆみちゃん物語 フルカラー実写版
				dest[x] = src[x] & 0xffffff;
			} else {
				dest[x] = screen_palette[src[x] & 0xff];
			}
		}
	}
	invalidate_screen(clip.x, clip.y, clip.w, clip.h);
}

void AGS::invalidate_screen(int sx, int sy, int width, int height)
{
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
	dirty = true;
}

void AGS::update_screen()
{
	if (!dirty)
		return;
	SDL_RenderClear(g_renderer);
	SDL_RenderCopy(g_renderer, sdlTexture, NULL, NULL);
	if (fade_level) {
		SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(g_renderer, fade_color, fade_color, fade_color, fade_level);
		SDL_RenderFillRect(g_renderer, NULL);
		SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
		SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);
	}
	if (scanline_texture)
		SDL_RenderCopy(g_renderer, scanline_texture, NULL, NULL);
	SDL_RenderPresent(g_renderer);
	dirty = false;
}

void AGS::set_scanline_mode(bool enable)
{
	if (enable && !scanline_texture) {
		scanline_texture = create_scanline_texture(g_renderer, screen_width, screen_height);
		dirty = true;
		update_screen();
	} else if (!enable && scanline_texture) {
		SDL_DestroyTexture(scanline_texture);
		scanline_texture = NULL;
		dirty = true;
		update_screen();
	}
}

void AGS::save_screenshot(const char* path)
{
	SDL_Surface* sf = SDL_CreateRGBSurface(0, screen_width, screen_height, 32, 0, 0, 0, 0);
	SDL_Rect r = {0, 0, screen_width, screen_height};
	SDL_BlitSurface(hBmpDest, &r, sf, NULL);

	if (scanline_texture) {
		SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(sf);
		SDL_Texture *tx = create_scanline_texture(renderer, screen_width, screen_height);
		SDL_RenderCopy(renderer, tx, NULL, NULL);
		SDL_DestroyTexture(tx);
		SDL_DestroyRenderer(renderer);
	}

	if (SDL_SaveBMP(sf, path) != 0) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "system3",
								 SDL_GetError(), g_window);
		SDL_ClearError();
	}

	SDL_FreeSurface(sf);
}

int AGS::calculate_menu_max(int window) {
	if (game_id.is(GameId::INTRUDER))
		return 6;
	if (game_id.is(GameId::GAKUEN))
		return (menu_w[window - 1].ey - menu_w[window - 1].sy) / (menu_font_size + 4);
	return 11;
}

#ifdef __EMSCRIPTEN__
extern "C" {

void* EMSCRIPTEN_KEEPALIVE sdl_getDisplaySurface() {
	if (SDL_MUSTLOCK(display_surface))
		return NULL;
	return display_surface->pixels;
}

}
#endif
