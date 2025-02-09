/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - window ]
*/

#include "ags.h"
#include <string.h>

void AGS::Window::clear_saved()
{
	if (screen) {
		SDL_DestroySurface(screen);
		screen = nullptr;
	}
	if (window) {
		SDL_DestroySurface(window);
		window = nullptr;
	}
}

void AGS::init_windows()
{
	for (int i = 0; i < 10; i++) {
		// ウィンドウの初期位置はシステムによって異なる
		switch (game_id.game) {
		case GameId::BUNKASAI:
			text_w[i] = {24, 304, 616, 384, false};
			menu_w[i] = {440, 18, 620, 178, true};
			break;
		case GameId::CRESCENT:
			text_w[i] = {24, 288, 616, 378, false};
			// 本来は横メニュー
			menu_w[i] = {464, 50, 623, 240, true};
			break;
		case GameId::RANCE2:
		case GameId::RANCE2_HINT:
			text_w[i] = {8, 285, 502, 396, false};
			menu_w[i] = {431, 19, 624, 181, false};
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
			text_w[i] = {48, 288, 594, 393, false};
			//menu_w[i] = {48, 288, 584, 393, false};
			menu_w[i] = {48, 288, 594, 393, false};
			break;
		case GameId::FUKEI:
			text_w[i] = {44, 282, 593, 396, false};
			menu_w[i] = {460, 14, 635, 214, false};
			break;
		case GameId::INTRUDER:
			text_w[i] = {8, 280, 629, 393, false};
			menu_w[i] = {448, 136, 623, 340, true};
			break;
		case GameId::TENGU:
			text_w[i] = {44, 282, 593, 396, false};
			menu_w[i] = {452, 14, 627, 214, false};
			break;
		case GameId::TOUSHIN_HINT:
			text_w[i] = {8, 311, 623, 391, false};
			menu_w[i] = {452, 14, 627, 214, true};
			break;
		case GameId::LITTLE_VAMPIRE:
			text_w[i] = {8, 255, 615, 383, false};
			menu_w[i] = {448, 11, 615, game_id.language == ENGLISH ? 234 : 224, false};
			break;
		case GameId::YAKATA:
			text_w[i] = {48, 288, 594, 393, false};
			menu_w[i] = {452, 14, 627, 214, false};
			break;
		case GameId::DALK_HINT:
			text_w[i] = {24, 308, 376, 386, false};
			menu_w[i] = {404, 28, 604, 244, true};
			break;
		case GameId::RANCE3_HINT:
			text_w[i] = {104, 304, 615, 383, false};
			menu_w[i] = {464, 24, 623, 200, true};
			break;
		case GameId::YAKATA2:
			text_w[i] = {104, 304, 620, 382, false};
			menu_w[i] = {420, 28, 620, 244, true};
			break;
		case GameId::GAKUEN:
			text_w[i] = {8, 260, 505, 384, false};
			if (i == 1) {
				menu_w[i] = {128, 32, 337, 178, true};
			} else {
				menu_w[i] = {288, 30, 433, 210, true};
			}
			break;
		default:
			text_w[i] = {8, 311, 623, 391, true};
			menu_w[i] = {464, 80, 623, 240, true};
			break;
		}
		text_w[i].save = false;
		menu_w[i].save = true;
	}
}

void AGS::clear_text_window(int index, bool erase)
{
	Window& w = text_w[index - 1];

	if(erase) {
		if (w.frame && fade_level) {
			draw_window(w.sx - 8, w.sy - 8, w.ex + 8, w.ey + 8, true, text.frame_color, text.back_color);
		} else {
			box_fill(0, w.sx, w.sy, w.ex, w.ey, text.back_color);
		}
	}

	text.reset_pos(w.sx, w.sy + 2);
}

bool AGS::return_text_line(int index)
{
	text.newline();
	// Return true if the next line exceeds the bottom of the window
	return text.pos.y + text.font_size > text_w[index - 1].ey;
}

void AGS::draw_push(int index)
{
	const uint8_t push_bitmap[32] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
		0x90, 0x00, 0x99, 0x74, 0xe9, 0x44, 0x89, 0x77,
		0x89, 0x15, 0x87, 0x75, 0x00, 0x00, 0xff, 0xff
	};

	Window& w = text_w[index - 1];
	int x = w.ex - 16;
	int y = w.ey - 16;
	draw_gaiji(0, x, y, push_bitmap, 16, text.font_color);
	draw_screen(x, y, 16, 16);
}

void AGS::open_text_window(int index, bool erase)
{
	Window& w = text_w[index - 1];
	int sx = w.sx - (w.frame ? 8 : 0);
	int sy = w.sy - (w.frame ? 8 : 0);
	int ex = w.ex + (w.frame ? 8 : 0);
	int ey = w.ey + (w.frame ? 8 : 0);
	int width = ex - sx + 1;
	int height = ey - sy + 1;

	if (game_id.is(GameId::PROG_CD) && !erase) {
		// prostudent G オープニング画面化け対策
		w.clear_saved();
	}

	// 画面退避
	if (w.save) {
		if (w.screen) {
			SDL_DestroySurface(w.screen);
		}

		w.screen = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
		w.screen_x = sx;
		w.screen_y = sy;

		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(hBmpScreen[0], &rect, w.screen, NULL);
	}

	if(erase) {
		// 窓初期化
		draw_window(sx, sy, ex, ey, w.frame, text.frame_color, text.back_color);
	} else if (w.save && w.window) {
		// 窓復帰
		sx = w.window_x;
		sy = w.window_y;
		width = w.window->w;
		height = w.window->h;

		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(w.window, NULL, hBmpScreen[0], &rect);
		draw_screen(sx, sy, width, height);
	}

	text.reset_pos(w.sx, w.sy + text.line_space);
}

void AGS::close_text_window(int index, bool update)
{
	Window& w = text_w[index - 1];
	int sx = w.sx - (w.frame ? 8 : 0);
	int sy = w.sy - (w.frame ? 8 : 0);
	int ex = w.ex + (w.frame ? 8 : 0);
	int ey = w.ey + (w.frame ? 8 : 0);
	int width = ex - sx + 1;
	int height = ey - sy + 1;

	// 窓退避
	if (w.save) {
		if (w.window) {
			SDL_DestroySurface(w.window);
		}

		w.window = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
		w.window_x = sx;
		w.window_y = sy;

		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(hBmpScreen[0], &rect, w.window, NULL);
	}

	// 画面復帰
	if (w.save && w.screen) {
		sx = w.screen_x;
		sy = w.screen_y;
		width = w.screen->w;
		height = w.screen->h;

		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(w.screen, NULL, hBmpScreen[0], &rect);
		draw_screen(sx, sy, width, height);
	}

	if(update) {
		text.reset_pos(w.sx, w.sy + text.line_space);
	}
}

void AGS::set_text_window(int index, int sx, int sy, int ex, int ey, bool save)
{
	Window& w = text_w[index - 1];
	w.clear_saved();
	w.sx = sx;
	w.sy = sy;
	w.ex = ex;
	w.ey = ey;
	w.save = save;
}

void AGS::clear_menu_window()
{
	SDL_Rect rect = {0, 0, 640, 480};
	SDL_FillSurfaceRect(hBmpScreen[2], &rect, menu.back_color);
	menu.reset_pos(2, 2);
}

void AGS::open_menu_window(int index)
{
	Window &w = menu_w[index - 1];
	int sx = w.sx;
	int sy = w.sy;
	int ex = w.ex;
	int ey = menu_fix ? w.ey : sy + menu.pos.y - 3;
	int width = ex - sx + 1;
	int height = ey - sy + 1;
	int wsx = sx - (w.frame ? 8 : 0);
	int wsy = sy - (w.frame ? 8 : 0);
	int wex = ex + (w.frame ? 8 : 0);
	int wey = ey + (w.frame ? 8 : 0);
	int wwidth = wex - wsx + 1;
	int wheight = wey - wsy + 1;

	// 画面退避
	if (w.save) {
		if (w.screen) {
			SDL_DestroySurface(w.screen);
		}

		w.screen = SDL_CreateSurface(wwidth, wheight, SDL_PIXELFORMAT_INDEX8);
		w.screen_x = wsx;
		w.screen_y = wsy;

		SDL_Rect rect = {wsx, wsy, wwidth, wheight};
		SDL_BlitSurface(hBmpScreen[0], &rect, w.screen, NULL);
	}

	// メニュー表示
	draw_window(wsx, wsy, wex, wey, w.frame, menu.frame_color, menu.back_color);
	SDL_Rect src_rect = {0, 0, width, height};
	SDL_Rect dst_rect = {sx, sy, width, height};
	SDL_BlitSurface(hBmpScreen[2], &src_rect, hBmpScreen[0], &dst_rect);
	box_line(0, sx, sy, ex, sy + menu.font_size + 3, menu.frame_color);
	draw_screen(wsx, wsy, wwidth, wheight);
}

void AGS::redraw_menu_window(int index, int selected)
{
	Window &w = menu_w[index - 1];
	int sx = w.sx;
	int sy = w.sy;
	int ex = w.ex;
	int ey = menu_fix ? w.ey : sy + menu.pos.y - 3;
	int width = ex - sx + 1;
	int height = ey - sy + 1;

	SDL_Rect src_rect = {0, 0, width, height};
	SDL_Rect dst_rect = {sx, sy, width, height};
	SDL_BlitSurface(hBmpScreen[2], &src_rect, hBmpScreen[0], &dst_rect);
	box_line(0, sx, sy + (menu.font_size + 4) * selected, ex, sy + (menu.font_size + 4) * (selected + 1) - 1, menu.frame_color);
	draw_screen(sx, sy, width, height);
}

void AGS::close_menu_window(int index)
{
	Window &w = menu_w[index - 1];
	// 画面復帰
	if (w.save && w.screen) {
		SDL_Rect rect = {w.screen_x, w.screen_y, w.screen->w, w.screen->h};
		SDL_BlitSurface(w.screen, NULL, hBmpScreen[0], &rect);
		draw_screen(w.screen_x, w.screen_y, w.screen->w, w.screen->h);
	}
}

void AGS::get_menu_window_rect(int index, int* sx, int* sy, int* ex, int* ey)
{
	Window &w = menu_w[index - 1];
	if (sx) *sx = w.sx;
	if (sy) *sy = w.sy;
	if (ex) *ex = w.ex;
	if (ey) *ey = w.ey;
}

void AGS::set_menu_window(int index, int sx, int sy, int ex, int ey, bool save)
{
	Window &w = menu_w[index - 1];
	w.clear_saved();
	w.sx = sx;
	w.sy = sy;
	w.ex = ex;
	w.ey = ey;
	w.save = save;
}

void AGS::draw_window(int sx, int sy, int ex, int ey, bool frame, uint8 frame_color, uint8 back_color)
{
	SDL_Rect rect = {sx, sy, ex - sx + 1, ey - sy + 1};
	SDL_FillSurfaceRect(hBmpScreen[0], &rect, back_color);

	if(frame) {
		SDL_Rect rects[] = {
			{sx + 1, sy + 1, ex - sx - 1, 2},
			{sx + 1, ey - 2, ex - sx - 1, 2},
			{sx + 1, sy + 1, 2, ey - sy - 1},
			{ex - 2, sy + 1, 2, ey - sy - 1},
		};
		SDL_FillSurfaceRects(hBmpScreen[0], rects, 4, frame_color);
		box_line(0, sx + 4, sy + 4, ex - 4, ey - 4, frame_color);
	}
	draw_screen(sx, sy, ex - sx + 1, ey - sy + 1);
}

