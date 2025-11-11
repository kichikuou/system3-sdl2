/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS ]
*/

#ifndef _AGS_H_
#define _AGS_H_

#include <algorithm>
#include <unordered_set>
#include <vector>
#include <memory>
#include <optional>
#include <stdio.h>
#include "common.h"
#include "game_id.h"
#include "dri.h"
#include <SDL_ttf.h>

inline uint8_t* surface_line(SDL_Surface* surface, int y)
{
	return static_cast<uint8*>(surface->pixels) + surface->pitch * y;
}

struct Config;
class FILEIO;

struct SurfaceDeleter {
	void operator()(SDL_Surface* s) const noexcept {
		if (s) SDL_FreeSurface(s);
	}
};

struct CG {
	std::unique_ptr<SDL_Surface, SurfaceDeleter> surface_;
	int x;
	int y;

	explicit CG(SDL_Surface* surface = nullptr, int x = 0, int y = 0)
		: surface_(surface), x(x), y(y) {}
	explicit operator bool() const noexcept { return static_cast<bool>(surface_); }

	SDL_Surface* surface() const { return surface_.get(); }
	int width() const { return surface_->w; }
	int height() const { return surface_->h; }
	SDL_Palette* palette() const { return surface_->format->palette; }
};

class AGS
{
protected:
	const GameId& game_id;
private:
	SDL_Texture* sdlTexture;
	SDL_Texture* scanline_texture;

	// Surface
	SDL_Surface* hBmpScreen[3]; // 8bpp * 3 (表, 裏, メニュー)
	uint8_t (*vram[3])[640];  // convenience pointer to hBmpScreen[i]->pixels

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
	CG load_gm3(const std::vector<uint8_t>& data, int transparent); // Intruder -桜屋敷の探索-
	CG load_vsp2l(const std::vector<uint8_t>& data, int transparent); // Little Vampire
	CG load_gl3(const std::vector<uint8_t>& data, bool set_palette, int transparent);
	CG load_pms(int page, const std::vector<uint8_t>& data, bool set_palette, int transparent);
	CG load_vsp(const std::vector<uint8_t>& data, bool set_palette, int transparent);

	void draw_char(int dest, int dest_x, int dest_y, uint16 code, TTF_Font* font, uint8 color);
	void draw_char_antialias(int dest, int dest_x, int dest_y, uint16 code, TTF_Font* font, uint8 color, uint8 cache[]);
	void draw_gaiji(int dest, int dest_x, int dest_y, const uint8_t bitmap[32], int size, uint8 color);

	void draw_screen(int sx, int sy, int width, int heignt);

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

	void load(FILEIO* fio);
	void save(FILEIO* fio);

	void update_screen();

	void load_cg(int page, int transparent);
	void set_cg_file(const char *file_name);

	void load_censor_list(const char* fname);

	void set_palette(int index, uint8_t r, uint8_t g, uint8_t b);
	std::vector<uint32_t> get_screen_palette() const;
	uint8 get_pixel(int dest, int x, int y) const { return vram[dest][y][x]; }
	void set_pixel(int dest, int x, int y, uint8 color) { vram[dest][y][x] = color; }

	void fade_out(int duration_ms, bool white);
	void fade_in(int duration_ms);

	void copy(int sx, int sy, int ex, int ey, int dx, int dy);
	void gcopy(int gsc, int gde, int glx, int gly, int gsw);
	void paint(int x, int y, uint8 color);
	void draw_box(int index);
	void draw_mesh(int sx, int sy, int width, int height);
	void box_fill(int dest, int sx, int sy, int ex, int ey, uint8 color);
	void box_line(int dest, int sx, int sy, int ex, int ey, uint8 color);
	void draw_window(int sx, int sy, int ex, int ey, bool frame, uint8 frame_color, uint8 back_color);

	void draw_text(const char* string, bool text_wait = false);

	void clear_text_window(int index, bool erase);
	bool return_text_line(int index);
	void draw_push(int index);
	void open_text_window(int index, bool erase);
	void close_text_window(int index, bool update);
	void set_text_window(int index, int sx, int sy, int ex, int ey, bool save) {
		text_w[index - 1].reset(sx, sy, ex, ey, text_w[index - 1].frame, save);
	}
	void set_text_window_frame(int index, bool frame) {
		text_w[index - 1].frame = frame;
	}

	void clear_menu_window();
	void open_menu_window(int index);
	void redraw_menu_window(int index, int selected);
	void close_menu_window(int index);
	void get_menu_window_rect(int index, int* sx, int* sy, int* ex, int* ey);
	void set_menu_window(int index, int sx, int sy, int ex, int ey, bool save) {
		menu_w[index - 1].reset(sx, sy, ex, ey, menu_w[index - 1].frame, save);
	}
	void set_menu_window_frame(int index, bool frame) {
		menu_w[index - 1].frame = frame;
	}

	void set_box(int index, uint8 color, int sx, int sy, int ex, int ey) {
		box[index - 1] = {color, sx, sy, ex, ey};
	}

	void load_cursor(int page);
	void select_cursor();
	void translate_mouse_coords(int* x, int* y);

	bool get_scanline_mode() const { return scanline_texture; }
	void set_scanline_mode(bool enable);
	bool save_screenshot(const char* path);
	int calculate_menu_max(int window);

	SDL_Rect dirty_rect = {};

	// 画面選択
	int src_screen;
	int dest_screen;

	int scroll = 0;
	int window_width, window_height;
	int screen_width, screen_height;

	struct TextContext {
		SDL_Point pos;
		int origin_x;
		int line_space;
		int font_size;
		uint8_t font_color;
		uint8_t frame_color;
		uint8_t back_color;
		int current_line_height;

		void reset_pos(int x, int y) {
			pos.x = origin_x = x;
			pos.y = y;
			current_line_height = 0;
		}
		void newline() {
			pos.x = origin_x;
			pos.y += std::max(font_size, current_line_height) + line_space;
			current_line_height = 0;
		}
	};
	// メッセージ表示
	TextContext text;

	// メニュー表示
	TextContext menu;
	bool menu_fix;

	bool draw_hankaku;
	bool draw_menu;

	// CG表示
	std::optional<SDL_Point> cg_dest;
	bool get_palette;
	bool extract_palette;
	bool extract_cg;
	std::unordered_set<int> censor_list;
	std::unordered_set<int> ignore_palette;
	int palette_bank;

	// マウスカーソル
	uint8 cursor_color;
	int cursor_index;

private:
	// Window (B command)
	struct Window {
		int sx;
		int sy;
		int ex;
		int ey;
		bool frame;
		bool save;
		CG screen;
		CG window;

		void reset(int sx, int sy, int ex, int ey, bool frame, bool save);
	};
	Window menu_w[10];
	Window text_w[10];
	void init_windows();

	// Box (E and Y7 commands)
	struct Box {
		uint8 color = 0;
		int sx = 0;
		int sy = 0;
		int ex = 639;
		int ey = 399;
	};
	Box box[20];

	Dri acg;
	const char* bmp_prefix = NULL;
};

extern "C" void ags_setAntialiasedStringMode(int on);

#endif
