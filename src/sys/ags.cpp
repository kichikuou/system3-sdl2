/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS ]
*/

#include "ags.h"
#include <string.h>
#include "nact.h"
#include "game_id.h"
#include "config.h"
#include "fileio.h"

extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;

namespace {

const uint32 SCANLINE_ALPHA = 0x38;  // 0-255

SDL_Texture* create_scanline_texture(SDL_Renderer* renderer, int width, int height)
{
	SDL_Surface* sf = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
	for (int y = 0; y < height; y++) {
		uint32* p = reinterpret_cast<uint32*>(surface_line(sf, y));
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

AGS::AGS(const Config& config, const GameId& game_id) : game_id(game_id)
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

	SDL_SetWindowSize(g_window, window_width, window_height);
	SDL_RenderSetLogicalSize(g_renderer, window_width, window_height);
	sdlTexture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
	scanline_texture = NULL;

	// DIBSection 8bpp * 3 (表, 裏, メニュー)
	for(int i = 0; i < 3; i++) {
		hBmpScreen[i] = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 8, SDL_PIXELFORMAT_INDEX8);
		vram[i] = reinterpret_cast<uint8_t(*)[640]>(hBmpScreen[i]->pixels);
	}

	// All surfaces share the same palette.
	screen_palette = hBmpScreen[0]->format->palette;
	SDL_SetSurfacePalette(hBmpScreen[1], screen_palette);
	SDL_SetSurfacePalette(hBmpScreen[2], screen_palette);

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
	program_palette = SDL_AllocPalette(256);
	program_palette->colors[0x00] = {0x00, 0x00, 0x00, 0xff};
	program_palette->colors[0x01] = {0x00, 0x00, 0xaa, 0xff};
	program_palette->colors[0x02] = {0xaa, 0x00, 0x00, 0xff};
	program_palette->colors[0x03] = {0xaa, 0x00, 0xaa, 0xff};
	program_palette->colors[0x04] = {0x00, 0x00, 0x00, 0xff};
	program_palette->colors[0x05] = {0x00, 0xaa, 0xaa, 0xff};
	program_palette->colors[0x06] = {0xaa, 0xaa, 0x00, 0xff};
	program_palette->colors[0x07] = {0xdd, 0xdd, 0xdd, 0xff};
	program_palette->colors[0x08] = {0x77, 0x77, 0x77, 0xff};
	program_palette->colors[0x09] = {0x00, 0x00, 0xff, 0xff};
	program_palette->colors[0x0a] = {0xff, 0x00, 0x00, 0xff};
	program_palette->colors[0x0b] = {0xff, 0x00, 0xff, 0xff};
	program_palette->colors[0x0c] = {0x00, 0xff, 0x00, 0xff};
	program_palette->colors[0x0d] = {0x00, 0xff, 0xff, 0xff};
	program_palette->colors[0x0e] = {0xff, 0xff, 0x00, 0xff};
	program_palette->colors[0x0f] = {0xff, 0xff, 0xff, 0xff};
	if (game_id.sys_ver == 1) {
		program_palette->colors[0x10] = {0x00, 0x00, 0x00, 0xff};
		program_palette->colors[0x11] = {0x00, 0x00, 0xff, 0xff};
		program_palette->colors[0x12] = {0xff, 0x00, 0x00, 0xff};
		program_palette->colors[0x13] = {0xff, 0x00, 0xff, 0xff};
		program_palette->colors[0x14] = {0x00, 0xff, 0x00, 0xff};
		program_palette->colors[0x15] = {0x00, 0xff, 0xff, 0xff};
		program_palette->colors[0x16] = {0xff, 0xff, 0x00, 0xff};
		program_palette->colors[0x17] = {0xff, 0xff, 0xff, 0xff};
		program_palette->colors[0x18] = {0x00, 0x00, 0x00, 0xff};
		program_palette->colors[0x19] = {0x00, 0x00, 0xff, 0xff};
		program_palette->colors[0x1a] = {0xff, 0x00, 0x00, 0xff};
		program_palette->colors[0x1b] = {0xff, 0x00, 0xff, 0xff};
		program_palette->colors[0x1c] = {0x00, 0xff, 0x00, 0xff};
		program_palette->colors[0x1d] = {0x00, 0xff, 0xff, 0xff};
		program_palette->colors[0x1e] = {0xff, 0xff, 0x00, 0xff};
	}
	program_palette->colors[0x1f] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0x2f] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0x3f] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0x4f] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0x5f] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0x6f] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0x7f] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0x8f] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0x9f] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0xaf] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0xbf] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0xcf] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0xdf] = {0xff, 0xff, 0xff, 0xff};
	program_palette->colors[0xef] = {0xff, 0xff, 0xff, 0xff};

	// To improve the quality of text antialiasing, preset the antialiasing
	// colors for text drawn in the primary colors (red, green, blue, yellow,
	// magenta, cyan, and white) on a black background.
	for (int i = 1; i <= 7; i++) {
		for (int j = 0; j <= 7; j++) {
			int n = 255 * j / 7;
			uint8_t r = (i & 4) ? n : 0;
			uint8_t g = (i & 2) ? n : 0;
			uint8_t b = (i & 1) ? n : 0;
			program_palette->colors[0xc0 + i * 8 + j] = {r, g, b, 255};
		}
	}

	SDL_SetPaletteColors(screen_palette, program_palette->colors, 0, 256);

	init_windows();

	// 画面選択
	src_screen = dest_screen = 0;

	// メッセージ表示
	text.reset_pos(text_w[0].sx, text_w[0].sy + 2);
	text.line_space = 2;
	text.font_size = 16;
	if (game_id.sys_ver == 1) {
		text.font_color = 15 + 16;
		text.frame_color = 15 + 16;
		text.back_color = 0 + 16;
	} else {
		text.font_color = 15;
		text.frame_color = 15;
		text.back_color = 0;
	}

	// メニュー表示
	menu.reset_pos(2, 2);
	menu.line_space = 4;
	menu.font_size = 16;
	if (game_id.sys_ver == 1) {
		menu.font_color = 15 + 16;
		menu.frame_color = 15 + 16;
		menu.back_color = 0 + 16;
	} else {
		menu.font_color = 15;
		menu.frame_color = 15;
		menu.back_color = 0;
	}
	menu_fix = false;

	draw_hankaku = false;
	draw_menu = false;

	// CG表示
	set_cg_dest = false;
	cg_dest_x = cg_dest_y = 0;
	get_palette = extract_palette = extract_cg = true;
	palette_bank = -1;

	// マウスカーソル
	cursor_color = 15;
	cursor_index = 0;

	set_scanline_mode(config.scanline);
}

AGS::~AGS()
{
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

	SDL_FreePalette(program_palette);

	for(int i = 0; i < 3; i++) {
		SDL_FreeSurface(hBmpScreen[i]);
	}

	SDL_DestroyTexture(sdlTexture);
}

void AGS::load(FILEIO* fio)
{
	menu.font_size = fio->getw();
	text.font_size = fio->getw();
	palette_bank = fio->getw();
	if (!palette_bank) {
		palette_bank = -1;
	}
	text.font_color = fio->getw();
	menu.font_color = fio->getw();
	menu.frame_color = fio->getw();
	menu.back_color = fio->getw();
	text.frame_color = fio->getw();
	text.back_color = fio->getw();
	for (int i = 0; i < 10; i++) {
		menu_w[i].clear_saved();
		menu_w[i].sx = fio->getw();
		menu_w[i].sy = fio->getw();
		menu_w[i].ex = fio->getw();
		menu_w[i].ey = fio->getw();
		menu_w[i].save = fio->getw() ? true : false;
		menu_w[i].frame = fio->getw() ? true : false;
		fio->getw();
		fio->getw();
	}
	for (int i = 0; i < 10; i++) {
		text_w[i].clear_saved();
		text_w[i].sx = fio->getw();
		text_w[i].sy = fio->getw();
		text_w[i].ex = fio->getw();
		text_w[i].ey = fio->getw();
		text_w[i].save = fio->getw() ? true : false;
		text_w[i].frame = fio->getw() ? true : false;
		fio->getw();
		fio->getw();
	}
}

void AGS::save(FILEIO* fio)
{
	fio->putw(menu.font_size);
	fio->putw(text.font_size);
	fio->putw(palette_bank == -1 ? 0 : palette_bank);
	fio->putw(text.font_color);
	fio->putw(menu.font_color);
	fio->putw(menu.frame_color);
	fio->putw(menu.back_color);
	fio->putw(text.frame_color);
	fio->putw(text.back_color);
	for (int i = 0; i < 10; i++) {
		fio->putw(menu_w[i].sx);
		fio->putw(menu_w[i].sy);
		fio->putw(menu_w[i].ex);
		fio->putw(menu_w[i].ey);
		fio->putw(menu_w[i].save ? 1 : 0);
		fio->putw(menu_w[i].frame ? 1 : 0);
		fio->putw(0);
		fio->putw(0);
	}
	for (int i = 0; i < 10; i++) {
		fio->putw(text_w[i].sx);
		fio->putw(text_w[i].sy);
		fio->putw(text_w[i].ex);
		fio->putw(text_w[i].ey);
		fio->putw(text_w[i].save ? 1 : 0);
		fio->putw(text_w[i].frame ? 1 : 0);
		fio->putw(0);
		fio->putw(0);
	}
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

void AGS::set_palette(int index, uint8_t r, uint8_t g, uint8_t b)
{
	SDL_Color color = {r, g, b, 255};
	if (index < 16) {
		color.r = (color.r & 0xf) * 0x11;
		color.g = (color.g & 0xf) * 0x11;
		color.b = (color.b & 0xf) * 0x11;
	}
	SDL_SetPaletteColors(screen_palette, &color, index, 1);
	SDL_SetPaletteColors(program_palette, &color, index, 1);
	dirty_rect = {0, 0, screen_width, screen_height};
}

std::vector<uint32_t> AGS::get_screen_palette() const
{
	std::vector<uint32_t> palette(256);
	for (int i = 0; i < 256; i++) {
		palette[i] = palR(i) << 16 | palG(i) << 8 | palB(i);
	}
	return palette;
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
	}
	update_screen();
}

void AGS::draw_screen(int sx, int sy, int width, int height)
{
	SDL_Rect rect = {sx, sy, width, height};
	SDL_Rect screen_rect = {0, 0, screen_width, screen_height};
	SDL_IntersectRect(&rect, &screen_rect, &rect);
	SDL_UnionRect(&dirty_rect, &rect, &dirty_rect);
}

void AGS::update_screen()
{
	if (!SDL_RectEmpty(&dirty_rect)) {
		SDL_Surface *sf;
		SDL_LockTextureToSurface(sdlTexture, &dirty_rect, &sf);
		SDL_BlitSurface(hBmpScreen[0], &dirty_rect, sf, NULL);
		SDL_UnlockTexture(sdlTexture);
		dirty_rect = {};
	}

	SDL_RenderClear(g_renderer);
	SDL_Rect src = {0, 0, screen_width, screen_height};
	SDL_Rect dest = {0, 0, screen_width, screen_height};
	if (scroll > 0) {
		src.y = scroll;
		src.h = dest.h = screen_height - scroll;
	} else if (scroll < 0) {
		dest.y = -scroll;
		src.h = dest.h = screen_height + scroll;
	}
	SDL_RenderCopy(g_renderer, sdlTexture, &src, &dest);

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
}

void AGS::set_scanline_mode(bool enable)
{
	if (enable && !scanline_texture) {
		scanline_texture = create_scanline_texture(g_renderer, screen_width, screen_height);
	} else if (!enable && scanline_texture) {
		SDL_DestroyTexture(scanline_texture);
		scanline_texture = NULL;
	}
}

bool AGS::save_screenshot(const char* path)
{
	SDL_Surface* sf = SDL_CreateRGBSurface(0, screen_width, screen_height, 32, 0, 0, 0, 0);
	SDL_BlitSurface(hBmpScreen[0], NULL, sf, NULL);

	if (scanline_texture) {
		SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(sf);
		SDL_Texture *tx = create_scanline_texture(renderer, screen_width, screen_height);
		SDL_RenderCopy(renderer, tx, NULL, NULL);
		SDL_DestroyTexture(tx);
		SDL_DestroyRenderer(renderer);
	}

	bool ok = SDL_SaveBMP(sf, path) == 0;
	if (!ok) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "system3",
								 SDL_GetError(), g_window);
		SDL_ClearError();
	}

	SDL_FreeSurface(sf);
	return ok;
}

int AGS::calculate_menu_max(int window) {
	if (game_id.is(GameId::INTRUDER))
		return 6;
	if (game_id.is(GameId::GAKUEN))
		return (menu_w[window - 1].ey - menu_w[window - 1].sy) / (menu.font_size + 4);
	return 11;
}

#ifdef __EMSCRIPTEN__
extern "C" {

bool EMSCRIPTEN_KEEPALIVE save_screenshot(const char* path) {
	return g_nact->ags->save_screenshot(path);
}

}
#endif
