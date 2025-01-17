/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS ]
*/

#ifndef _AGS_H_
#define _AGS_H_

#include <vector>
#include <memory>
#include <stdio.h>
#include "../common.h"
#include "game_id.h"
#include "dri.h"
#include <SDL_ttf.h>

#define MAX_CG 10000

inline uint32* surface_line(SDL_Surface* surface, int y)
{
	return (uint32*)((uint8*)surface->pixels + surface->pitch * y);
}

struct Config;
class FILEIO;

class AGS
{
protected:
	const GameId& game_id;
private:
	SDL_Texture* sdlTexture;
	SDL_Texture* scanline_texture;

	// Surface
	SDL_Surface* hBmpScreen[3]; // 8bpp * 3 (表, 裏, メニュー)
	SDL_Surface* hBmpDest; // DIBSection 24bpp (最終出力先)
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
	void load_gm3(uint8* data, int page, int transparent);		// Intruder -桜屋敷の探索-
	void load_vsp2l(uint8* data, int page, int transparent);	// Little Vampire
	void load_gl3(uint8* data, int page, int transparent);
	void load_pms(uint8* data, int page, int transparent);
	void load_bmp(const char* file_name);				// あゆみちゃん物語 フルカラー実写版
	void load_vsp(uint8* data, int page, int transparent);

	void draw_char(int dest, int dest_x, int dest_y, uint16 code, TTF_Font* font, uint8 color);
	void draw_char_antialias(int dest, int dest_x, int dest_y, uint16 code, TTF_Font* font, uint8 color, uint8 cache[]);
	void draw_gaiji(int dest, int dest_x, int dest_y, uint16 code, int size, uint8 color);

	void draw_screen(int sx, int sy, int width, int heignt);
	void invalidate_screen(int sx, int sy, int width, int height);

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

	void flush_screen(bool update);

	void load_cg(int page, int transparent);
	void set_cg_file(const char *file_name);

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

	void clear_menu_window();
	void open_menu_window(int index);
	void redraw_menu_window(int index, int selected);
	void close_menu_window(int index);

	void load_cursor(int page);
	void select_cursor();
	void translate_mouse_coords(int* x, int* y);

	bool get_scanline_mode() const { return scanline_texture; }
	void set_scanline_mode(bool enable);
	void save_screenshot(const char* path);
	int calculate_menu_max(int window);

	bool dirty;

	// 画面選択
	int src_screen;
	int dest_screen;

	int scroll;
	int window_width, window_height;
	int screen_width, screen_height;

	// メッセージ表示
	int text_dest_x;
	int text_dest_y;
	int text_space;
	int text_font_size;
	uint8 text_font_color;
	uint8 text_frame_color;
	uint8 text_back_color;
	int text_font_maxsize;	// その行での最大フォントサイズ

	// メニュー表示
	int menu_dest_x;
	int menu_dest_y;
	int menu_font_size;
	uint8 menu_font_color;
	uint8 menu_frame_color;
	uint8 menu_back_color;
	bool menu_fix;

	bool draw_hankaku;
	bool draw_menu;

	// ウィンドウ (Bコマンド)
	typedef struct {
		int sx;
		int sy;
		int ex;
		int ey;
		bool frame;
		bool push;
		SDL_Surface* screen;
		int screen_x;
		int screen_y;
		SDL_Surface* window;
		int window_x;
		int window_y;
	} WINDOW;
	WINDOW menu_w[10];
	WINDOW text_w[10];

	// ボックス (E, Y7コマンド)
	typedef struct {
		uint8 color;
		int sx;
		int sy;
		int ex;
		int ey;
	} BOX;
	BOX box[20];

	// CG表示
	bool set_cg_dest;
	int cg_dest_x;
	int cg_dest_y;
	bool get_palette;
	bool extract_palette;
	bool extract_cg;
	bool extract_palette_cg[MAX_CG];
	int palette_bank;

	// マウスカーソル
	uint8 cursor_color;
	int cursor_index;

private:
	Dri acg;
	const char* bmp_prefix = NULL;
};

extern "C" void ags_setAntialiasedStringMode(int on);

#endif
