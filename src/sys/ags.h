/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS ]
*/

#ifndef _AGS_H_
#define _AGS_H_

#include <optional>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <stdio.h>
#include "common.h"
#include "game_id.h"
#include "cg.h"
#include "dri.h"
#include <SDL_ttf.h>

struct Config;

enum ScreenId {
	SCREEN_FRONT,
	SCREEN_BACK,
	NR_SCREENS,
};

enum CgFlags {
	CG_EXTRACT_CG = 1,
	CG_EXTRACT_PALETTE = 2,
	CG_GET_PALETTE = 4,
};

class AGS
{
protected:
	const GameId& game_id;
private:
	SDL_Texture* sdlTexture;
	SDL_Texture* scanline_texture;

	// Surface
	SDL_Surface* hBmpScreen[NR_SCREENS]; // 8bpp
	uint8_t (*vram[NR_SCREENS])[640];  // convenience pointer to hBmpScreen[i]->pixels

	SDL_Palette* program_palette;
	SDL_Palette* screen_palette;

	// フォント
	SDL_RWops* rw_font;
	TTF_Font* hFont16;
	TTF_Font* hFont24;
	TTF_Font* hFont32;
	TTF_Font* hFont48;
	TTF_Font* hFont64;

	// カーソル
	SDL_Cursor* hCursor[10];

	// AGS
	CG load_gm3(const std::vector<uint8_t>& data, int transparent, uint8_t flags); // Intruder -桜屋敷の探索-
	CG load_vsp2l(const std::vector<uint8_t>& data, int transparent, uint8_t flags); // Little Vampire
	CG load_gl3(const std::vector<uint8_t>& data, bool set_palette, int transparent, uint8_t flags);
	CG load_pms(int page, const std::vector<uint8_t>& data, bool set_palette, int transparent, uint8_t flags);
	CG load_vsp(const std::vector<uint8_t>& data, bool set_palette, int transparent, uint8_t flags);

	void draw_char(ScreenId dest, int dest_x, int dest_y, uint16 code, TTF_Font* font, uint8 color);
	void draw_char_antialias(ScreenId dest, int dest_x, int dest_y, uint16 code, TTF_Font* font, uint8 color, uint8 cache[]);

	uint8_t palR(uint8_t col) const { return screen_palette->colors[col].r; }
	uint8_t palG(uint8_t col) const { return screen_palette->colors[col].g; }
	uint8_t palB(uint8_t col) const { return screen_palette->colors[col].b; }
	int nearest_color(int r, int g, int b);

	uint8 gaiji[188][32];

	int fade_level = 0;  // 0-255
	int fade_color = 0;  // 0: black, 255: white

public:
	AGS(const Config& config, const GameId& game_id);
	~AGS();

	void update_screen();

	void load_cg(int page, int transparent, uint8_t flags);
	// Decodes a CG without drawing it, but still updates the palette.
	CG load_cg_surface(int page, int transparent, uint8_t flags);
	// Returns an ACG page bytes, without decoding it.
	std::vector<uint8_t> load_cg_data(int page) { return acg.load(page); }
	void blit_cg(ScreenId dest, CG& cg, const SDL_Rect* src, int dx, int dy);
	void blit_cg(CG& dest, CG& cg, const SDL_Rect* src, int dx, int dy);
	void set_cg_file(const char *file_name);
	CG create_offscreen(int width, int height);

	void load_censor_list(const char* fname);

	void set_palette(int index, uint8_t r, uint8_t g, uint8_t b);
	std::vector<uint32_t> get_screen_palette() const;
	uint8 get_pixel(ScreenId dest, int x, int y) const { return vram[dest][y][x]; }
	void set_pixel(ScreenId dest, int x, int y, uint8 color) { vram[dest][y][x] = color; }
	void invalidate_screen(int sx, int sy, int width, int height);
	void draw_gaiji(ScreenId dest, int dest_x, int dest_y, const uint8_t bitmap[32], int size, uint8 color);
	bool is_faded() const { return fade_level != 0; }
	void set_fade_color(bool white) { fade_color = white ? 255 : 0; }
	void set_fade_level(int level) { fade_level = level; }

	void copy(int sx, int sy, int ex, int ey, int dx, int dy) {
		copy_screen(src_screen, dest_screen, sx, sy, ex, ey, dx, dy);
	}
	void copy_screen(ScreenId src, ScreenId dest, int sx, int sy, int ex, int ey, int dx, int dy,
					 int transparent_color = -1);
	void gcopy(int gsc, int gde, int glx, int gly, int gsw);
	void paint(int x, int y, uint8 color);
	void draw_mesh(int sx, int sy, int width, int height);
	void box_fill(ScreenId dest, int sx, int sy, int ex, int ey, uint8 color);
	void box_line(ScreenId dest, int sx, int sy, int ex, int ey, uint8 color);
	void draw_window(int sx, int sy, int ex, int ey, bool frame, uint8 frame_color, uint8 back_color);

	CG save_rect(int x, int y, int width, int height);
	void restore_rect(const CG& cg);

	int draw_text(ScreenId dest, int x, int y, std::u16string_view codes, int font_size, uint8 color);

	void load_cursor(int page, uint8_t flags);
	void select_cursor();
	void translate_mouse_coords(int* x, int* y);

	bool get_scanline_mode() const { return scanline_texture; }
	void set_scanline_mode(bool enable);
	bool save_screenshot(const char* path);

	SDL_Rect dirty_rect = {};

	ScreenId src_screen = SCREEN_FRONT;
	ScreenId dest_screen = SCREEN_FRONT;

	int scroll = 0;
	int window_width, window_height;
	int screen_width, screen_height;

	// CG表示
	std::optional<SDL_Point> cg_dest;
	std::unordered_set<int> censor_list;
	std::unordered_set<int> ignore_palette;
	int palette_bank;

	// マウスカーソル
	int cursor_index;

private:
	Dri acg;
	const char* bmp_prefix = NULL;
};

extern "C" void ags_setAntialiasedStringMode(int on);

#endif
