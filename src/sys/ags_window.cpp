/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - window ]
*/

#include "ags.h"
#include <string.h>

void AGS::Window::reset(int sx, int sy, int ex, int ey, bool frame, bool save) {
	this->sx = sx;
	this->sy = sy;
	this->ex = ex;
	this->ey = ey;
	this->frame = frame;
	this->save = save;
	screen.surface_.reset();
	window.surface_.reset();
}

void AGS::init_windows()
{
	for (int i = 0; i < 10; i++) {
		// ウィンドウの初期位置はシステムによって異なる
		switch (game_id.game) {
		case GameId::BUNKASAI:
			text_w[i].reset(24, 304, 616, 384, false, false);
			menu_w[i].reset(440, 18, 620, 178, true, true);
			break;
		case GameId::CRESCENT:
			text_w[i].reset(24, 288, 616, 378, false, false);
			// 本来は横メニュー
			menu_w[i].reset(464, 50, 623, 240, true, true);
			break;
		case GameId::RANCE2:
		case GameId::RANCE2_HINT:
			text_w[i].reset(8, 285, 502, 396, false, false);
			menu_w[i].reset(431, 19, 624, 181, false, true);
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
			text_w[i].reset(48, 288, 594, 393, false, false);
			//menu_w[i].reset(48, 288, 584, 393, false, true);
			menu_w[i].reset(48, 288, 594, 393, false, true);
			break;
		case GameId::FUKEI:
			text_w[i].reset(44, 282, 593, 396, false, false);
			menu_w[i].reset(460, 14, 635, 214, false, true);
			break;
		case GameId::INTRUDER:
			text_w[i].reset(8, 280, 629, 393, false, false);
			menu_w[i].reset(448, 136, 623, 340, true, true);
			break;
		case GameId::TENGU:
			text_w[i].reset(44, 282, 593, 396, false, false);
			menu_w[i].reset(452, 14, 627, 214, false, true);
			break;
		case GameId::TOUSHIN_HINT:
			text_w[i].reset(8, 311, 623, 391, false, false);
			menu_w[i].reset(452, 14, 627, 214, true, true);
			break;
		case GameId::LITTLE_VAMPIRE:
			text_w[i].reset(8, 255, 615, 383, false, false);
			menu_w[i].reset(448, 11, 615, game_id.language == ENGLISH ? 234 : 224, false, true);
			break;
		case GameId::YAKATA:
			text_w[i].reset(48, 288, 594, 393, false, false);
			menu_w[i].reset(452, 14, 627, 214, false, true);
			break;
		case GameId::DALK_HINT:
			text_w[i].reset(24, 308, 376, 386, false, false);
			menu_w[i].reset(404, 28, 604, 244, true, true);
			break;
		case GameId::RANCE3_HINT:
			text_w[i].reset(104, 304, 615, 383, false, false);
			menu_w[i].reset(464, 24, 623, 200, true, true);
			break;
		case GameId::YAKATA2:
			text_w[i].reset(104, 304, 620, 382, false, false);
			menu_w[i].reset(420, 28, 620, 244, true, true);
			break;
		case GameId::GAKUEN:
			text_w[i].reset(8, 260, 505, 384, false, false);
			if (i == 1) {
				menu_w[i].reset(128, 32, 337, 178, true, true);
			} else {
				menu_w[i].reset(288, 30, 433, 210, true, true);
			}
			break;
		default:
			text_w[i].reset(8, 311, 623, 391, true, false);
			menu_w[i].reset(464, 80, 623, 240, true, true);
			break;
		}
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
		w.screen = CG();
		w.window = CG();
	}

	// 画面退避
	if (w.save) {
		w.screen = CG(sx, sy, width, height);
		SDL_SetPaletteColors(w.screen.palette(), screen_palette->colors, 0, 256);

		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(hBmpScreen[0], &rect, w.screen.surface(), NULL);
	}

	if(erase) {
		// 窓初期化
		draw_window(sx, sy, ex, ey, w.frame, text.frame_color, text.back_color);
	} else if (w.save && w.window) {
		// 窓復帰
		sx = w.window.x;
		sy = w.window.y;
		width = w.window.width();
		height = w.window.height();

		SDL_SetSurfacePalette(hBmpScreen[0], w.window.palette());
		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(w.window.surface(), NULL, hBmpScreen[0], &rect);
		draw_screen(sx, sy, width, height);
		SDL_SetSurfacePalette(hBmpScreen[0], screen_palette);
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
		w.window = CG(sx, sy, width, height);
		SDL_SetPaletteColors(w.window.palette(), screen_palette->colors, 0, 256);

		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(hBmpScreen[0], &rect, w.window.surface(), NULL);
	}

	// 画面復帰
	if (w.save && w.screen) {
		sx = w.screen.x;
		sy = w.screen.y;
		width = w.screen.width();
		height = w.screen.height();

		SDL_SetSurfacePalette(hBmpScreen[0], w.screen.palette());
		SDL_Rect rect = {sx, sy, width, height};
		SDL_BlitSurface(w.screen.surface(), NULL, hBmpScreen[0], &rect);
		draw_screen(sx, sy, width, height);
		SDL_SetSurfacePalette(hBmpScreen[0], screen_palette);
	}

	if(update) {
		text.reset_pos(w.sx, w.sy + text.line_space);
	}
}

void AGS::clear_menu_window()
{
	SDL_Rect rect = {0, 0, 640, 480};
	SDL_FillRect(hBmpScreen[2], &rect, menu.back_color);
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
		w.screen = CG(wsx, wsy, wwidth, wheight);
		SDL_SetPaletteColors(w.screen.palette(), screen_palette->colors, 0, 256);

		SDL_Rect rect = {wsx, wsy, wwidth, wheight};
		SDL_BlitSurface(hBmpScreen[0], &rect, w.screen.surface(), NULL);
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
		SDL_SetSurfacePalette(hBmpScreen[0], w.screen.palette());
		SDL_Rect rect = {w.screen.x, w.screen.y, w.screen.width(), w.screen.height()};
		SDL_BlitSurface(w.screen.surface(), NULL, hBmpScreen[0], &rect);
		draw_screen(w.screen.x, w.screen.y, w.screen.width(), w.screen.height());
		SDL_SetSurfacePalette(hBmpScreen[0], screen_palette);
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

void AGS::draw_window(int sx, int sy, int ex, int ey, bool frame, uint8 frame_color, uint8 back_color)
{
	SDL_Rect rect = {sx, sy, ex - sx + 1, ey - sy + 1};
	SDL_FillRect(hBmpScreen[0], &rect, back_color);

	if(frame) {
		SDL_Rect rects[] = {
			{sx + 1, sy + 1, ex - sx - 1, 2},
			{sx + 1, ey - 2, ex - sx - 1, 2},
			{sx + 1, sy + 1, 2, ey - sy - 1},
			{ex - 2, sy + 1, 2, ey - sy - 1},
		};
		SDL_FillRects(hBmpScreen[0], rects, 4, frame_color);
		box_line(0, sx + 4, sy + 4, ex - 4, ey - 4, frame_color);
	}
	draw_screen(sx, sy, ex - sx + 1, ey - sy + 1);
}

