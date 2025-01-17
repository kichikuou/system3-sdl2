/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - window ]
*/

#include "ags.h"
#include <string.h>

void AGS::clear_text_window(int index, bool erase)
{
	int sx = text_w[index - 1].sx;
	int sy = text_w[index - 1].sy;
	int ex = text_w[index - 1].ex;
	int ey = text_w[index - 1].ey;

	if(erase) {
		if(text_w[index - 1].frame && fade_level) {
			draw_window(sx - 8, sy - 8, ex + 8, ey + 8, true, text_frame_color, text_back_color);
		} else {
			box_fill(0, sx, sy, ex, ey, text_back_color);
		}
	}

	text_dest_x = sx;
	text_dest_y = sy + 2;//text_space;
	text_font_maxsize = 0;
}

bool AGS::return_text_line(int index)
{
	int sx = text_w[index - 1].sx;
	int ey = text_w[index - 1].ey;

	text_dest_x = sx;
	text_dest_y += (text_font_maxsize > text_font_size) ? text_font_maxsize : text_font_size;
	text_dest_y += text_space;
	text_font_maxsize = 0;

	// 次の行が表示しきれるか？
	return (text_dest_y + text_font_size) > ey ? true : false;
}

void AGS::draw_push(int index)
{
	// Pushのパターンデータ
	static uint16 pattern[16] = {
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xe000,
		0x9000, 0x9974, 0xe944, 0x8977, 0x8915, 0x8775, 0x0000, 0xffff
	};

	int x = text_w[index - 1].ex - 16;
	int y = text_w[index - 1].ey - 16;

	for(int i = 0; i < 16; i++) {
		if(pattern[i] & 0x8000) vram[0][y + i][x +  0] = text_font_color;
		if(pattern[i] & 0x4000) vram[0][y + i][x +  1] = text_font_color;
		if(pattern[i] & 0x2000) vram[0][y + i][x +  2] = text_font_color;
		if(pattern[i] & 0x1000) vram[0][y + i][x +  3] = text_font_color;
		if(pattern[i] & 0x0800) vram[0][y + i][x +  4] = text_font_color;
		if(pattern[i] & 0x0400) vram[0][y + i][x +  5] = text_font_color;
		if(pattern[i] & 0x0200) vram[0][y + i][x +  6] = text_font_color;
		if(pattern[i] & 0x0100) vram[0][y + i][x +  7] = text_font_color;
		if(pattern[i] & 0x0080) vram[0][y + i][x +  8] = text_font_color;
		if(pattern[i] & 0x0040) vram[0][y + i][x +  9] = text_font_color;
		if(pattern[i] & 0x0020) vram[0][y + i][x + 10] = text_font_color;
		if(pattern[i] & 0x0010) vram[0][y + i][x + 11] = text_font_color;
		if(pattern[i] & 0x0008) vram[0][y + i][x + 12] = text_font_color;
		if(pattern[i] & 0x0004) vram[0][y + i][x + 13] = text_font_color;
		if(pattern[i] & 0x0002) vram[0][y + i][x + 14] = text_font_color;
		if(pattern[i] & 0x0001) vram[0][y + i][x + 15] = text_font_color;
	}
	draw_screen(x, y, 16, 16);
}

void AGS::open_text_window(int index, bool erase)
{
	int sx = text_w[index - 1].sx - (text_w[index - 1].frame ? 8 : 0);
	int sy = text_w[index - 1].sy - (text_w[index - 1].frame ? 8 : 0);
	int ex = text_w[index - 1].ex + (text_w[index - 1].frame ? 8 : 0);
	int ey = text_w[index - 1].ey + (text_w[index - 1].frame ? 8 : 0);
	int width = ex - sx + 1;
	int height = ey - sy + 1;

	// 画面退避
	if(text_w[index - 1].push) {
		if(text_w[index - 1].screen) {
			SDL_FreeSurface(text_w[index - 1].screen);
		}

		text_w[index - 1].screen = SDL_CreateRGBSurfaceWithFormat(0, width, height, 8, SDL_PIXELFORMAT_INDEX8);
		SDL_SetSurfacePalette(text_w[index - 1].screen, hBmpScreen[0]->format->palette);
		text_w[index - 1].screen_x = sx;
		text_w[index - 1].screen_y = sy;
		text_w[index - 1].screen_palette = screen_palette;

		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(hBmpScreen[0], &rect, text_w[index - 1].screen, NULL);
	}

	if(erase) {
		// 窓初期化
		draw_window(sx, sy, ex, ey, text_w[index - 1].frame, text_frame_color, text_back_color);
	} else if(text_w[index - 1].push && text_w[index - 1].window) {
		// 窓復帰
		sx = text_w[index - 1].window_x;
		sy = text_w[index - 1].window_y;
		width = text_w[index - 1].window->w;
		height = text_w[index - 1].window->h;

		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(text_w[index - 1].window, NULL, hBmpScreen[0], &rect);

		Palette palette = screen_palette;
		screen_palette = text_w[index - 1].window_palette;
		draw_screen(sx, sy, width, height);
		screen_palette = palette;
	}

	// テキスト描画位置更新
	text_dest_x = text_w[index - 1].sx;
	text_dest_y = text_w[index - 1].sy + text_space;;
	text_font_maxsize = 0;
}

void AGS::close_text_window(int index, bool update)
{
	int sx = text_w[index - 1].sx - (text_w[index - 1].frame ? 8 : 0);
	int sy = text_w[index - 1].sy - (text_w[index - 1].frame ? 8 : 0);
	int ex = text_w[index - 1].ex + (text_w[index - 1].frame ? 8 : 0);
	int ey = text_w[index - 1].ey + (text_w[index - 1].frame ? 8 : 0);
	int width = ex - sx + 1;
	int height = ey - sy + 1;

	// 窓退避
	if(text_w[index - 1].push) {
		if(text_w[index - 1].window) {
			SDL_FreeSurface(text_w[index - 1].window);
		}

		text_w[index - 1].window = SDL_CreateRGBSurfaceWithFormat(0, width, height, 8, SDL_PIXELFORMAT_INDEX8);
		SDL_SetSurfacePalette(text_w[index - 1].window, hBmpScreen[0]->format->palette);
		text_w[index - 1].window_x = sx;
		text_w[index - 1].window_y = sy;
		text_w[index - 1].window_palette = screen_palette;

		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(hBmpScreen[0], &rect, text_w[index - 1].window, NULL);
	}

	// 画面復帰
	if(text_w[index - 1].push && text_w[index - 1].screen) {
		sx = text_w[index - 1].screen_x;
		sy = text_w[index - 1].screen_y;
		width = text_w[index - 1].screen->w;
		height = text_w[index - 1].screen->h;

		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(text_w[index - 1].screen, NULL, hBmpScreen[0], &rect);

		Palette palette = screen_palette;
		screen_palette = text_w[index - 1].screen_palette;
		draw_screen(sx, sy, width, height);
		screen_palette = palette;
	}

	// テキスト描画位置更新
	if(update) {
		text_dest_x = text_w[index - 1].sx;
		text_dest_y = text_w[index - 1].sy + text_space;;
		text_font_maxsize = 0;
	}
}

void AGS::clear_menu_window()
{
	SDL_Rect rect = {0, 0, 640, 480};
	SDL_FillRect(hBmpScreen[2], &rect, menu_back_color);
}

void AGS::open_menu_window(int index)
{
	int sx = menu_w[index - 1].sx;
	int sy = menu_w[index - 1].sy;
	int ex = menu_w[index - 1].ex;
	int ey = menu_fix ? menu_w[index - 1].ey : sy + menu_dest_y - 1;
	int width = ex - sx + 1;
	int height = ey - sy + 1;
	int wsx = sx - (menu_w[index - 1].frame ? 8 : 0);
	int wsy = sy - (menu_w[index - 1].frame ? 8 : 0);
	int wex = ex + (menu_w[index - 1].frame ? 8 : 0);
	int wey = ey + (menu_w[index - 1].frame ? 8 : 0);
	int wwidth = wex - wsx + 1;
	int wheight = wey - wsy + 1;

	// 画面退避
	if(menu_w[index - 1].push) {
		if(menu_w[index - 1].screen) {
			SDL_FreeSurface(menu_w[index - 1].screen);
		}

		menu_w[index - 1].screen = SDL_CreateRGBSurfaceWithFormat(0, wwidth, wheight, 8, SDL_PIXELFORMAT_INDEX8);
		SDL_SetSurfacePalette(menu_w[index - 1].screen, hBmpScreen[0]->format->palette);
		menu_w[index - 1].screen_x = wsx;
		menu_w[index - 1].screen_y = wsy;
		menu_w[index - 1].screen_palette = screen_palette;

		SDL_Rect rect = {wsx, wsy, wwidth, wheight};
		SDL_BlitSurface(hBmpScreen[0], &rect, menu_w[index - 1].screen, NULL);
	}

	// メニュー表示
	draw_window(wsx, wsy, wex, wey, menu_w[index - 1].frame, menu_frame_color, menu_back_color);
	SDL_Rect src_rect = {0, 0, width, height};
	SDL_Rect dst_rect = {sx, sy, width, height};
	SDL_BlitSurface(hBmpScreen[2], &src_rect, hBmpScreen[0], &dst_rect);
	box_line(0, sx, sy, ex, sy + menu_font_size + 3, menu_frame_color);
	draw_screen(wsx, wsy, wwidth, wheight);
}

void AGS::redraw_menu_window(int index, int selected)
{
	int sx = menu_w[index - 1].sx;
	int sy = menu_w[index - 1].sy;
	int ex = menu_w[index - 1].ex;
	int ey = menu_fix ? menu_w[index - 1].ey : sy + menu_dest_y - 1;
	int width = ex - sx + 1;
	int height = ey - sy + 1;

	SDL_Rect src_rect = {0, 0, width, height};
	SDL_Rect dst_rect = {sx, sy, width, height};
	SDL_BlitSurface(hBmpScreen[2], &src_rect, hBmpScreen[0], &dst_rect);
	box_line(0, sx, sy + (menu_font_size + 4) * selected, ex, sy + (menu_font_size + 4) * (selected + 1) - 1, menu_frame_color);
	draw_screen(sx, sy, width, height);
}

void AGS::close_menu_window(int index)
{
	// 画面復帰
	if(menu_w[index - 1].push && menu_w[index - 1].screen) {
		int sx = menu_w[index - 1].screen_x;
		int sy = menu_w[index - 1].screen_y;
		int width = menu_w[index - 1].screen->w;
		int height = menu_w[index - 1].screen->h;

		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(menu_w[index - 1].screen, NULL, hBmpScreen[0], &rect);

		Palette palette = screen_palette;
		screen_palette = menu_w[index - 1].screen_palette;
		draw_screen(sx, sy, width, height);
		screen_palette = palette;
	}
}

void AGS::draw_window(int sx, int sy, int ex, int ey, bool frame, uint8 frame_color, uint8 back_color)
{
	SDL_Rect rect = {sx, sy, ex - sx + 1, ey - sy + 1};
	SDL_FillRect(hBmpScreen[0], &rect, back_color);

	if(frame) {
		SDL_Rect top    = {sx + 1, sy + 1, ex - sx - 1, 2};
		SDL_Rect bottom = {sx + 1, ey - 2, ex - sx - 1, 2};
		SDL_Rect left   = {sx + 1, sy + 1, 2, ey - sy - 1};
		SDL_Rect right  = {ex - 2, sy + 1, 2, ey - sy - 1};
		SDL_FillRect(hBmpScreen[0], &top, frame_color);
		SDL_FillRect(hBmpScreen[0], &bottom, frame_color);
		SDL_FillRect(hBmpScreen[0], &left, frame_color);
		SDL_FillRect(hBmpScreen[0], &right, frame_color);
		box_line(0, sx + 4, sy + 4, ex - 4, ey - 4, frame_color);
	}
	draw_screen(sx, sy, ex - sx + 1, ey - sy + 1);
}

