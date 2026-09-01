/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - window ]
*/

#include "nact.h"
#include "ags.h"
#include "fileio.h"

void NACT::Window::reset(int sx, int sy, int ex, int ey, bool frame, bool save) {
	this->sx = sx;
	this->sy = sy;
	this->ex = ex;
	this->ey = ey;
	this->frame = frame;
	this->save = save;
	screen.surface_.reset();
	window.surface_.reset();
}

void NACT::init_windows()
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
		case GameId::GAKUEN_SENKI:
			text_w[i].reset(8, 260, 505, 384, false, false);
			if (i == 1) {
				menu_w[i].reset(128, 32, 337, 178, true, true);
			} else {
				menu_w[i].reset(288, 30, 433, 210, true, true);
			}
			break;
		case GameId::GAKUEN_KING:
			switch (i) {
			default:
			case 0: text_w[i].reset(112, 310, 623, 391, false, false); break;
			case 1: text_w[i].reset(48, 310, 591, 391, false, false); break;
			case 2: text_w[i].reset(0, 0, 639, 391, false, false); break;
			case 3: text_w[i].reset(24, 310, 183, 391, false, false); break;
			case 4: text_w[i].reset(464, 310, 623, 391, false, false); break;
			}
			switch (i) {
			default:
			case 0: menu_w[i].reset(416, 24, 591, 248, true, true); break;
			case 1: menu_w[i].reset(32, 16, 191, 216, true, true); break;
			case 2: menu_w[i].reset(448, 16, 607, 216, true, true); break;
			case 3: menu_w[i].reset(32, 223, 191, 383, true, true); break;
			case 4: menu_w[i].reset(448, 223, 607, 383, true, true); break;
			}
			break;
		default:
			text_w[i].reset(8, 311, 623, 391, true, false);
			menu_w[i].reset(464, 80, 623, 240, true, true);
			break;
		}
	}
}

void NACT::clear_text_window(int index, bool erase)
{
	Window& w = text_w[index - 1];

	if (erase) {
		if (w.frame && ags->is_faded()) {
			ags->draw_window(w.sx - 8, w.sy - 8, w.ex + 8, w.ey + 8, true, text.frame_color, text.back_color);
		} else {
			ags->box_fill(0, w.sx, w.sy, w.ex, w.ey, text.back_color);
		}
	}

	text.reset_pos(w.sx, w.sy + 2);
}

bool NACT::return_text_line(int index)
{
	text.newline();
	// Return true if the next line exceeds the bottom of the window
	return text.pos.y + text.font_size > text_w[index - 1].ey;
}

void NACT::draw_push(int index)
{
	static const uint8_t builtin_bitmap[32] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
		0x90, 0x00, 0x99, 0x74, 0xe9, 0x44, 0x89, 0x77,
		0x89, 0x15, 0x87, 0x75, 0x00, 0x00, 0xff, 0xff
	};

	Window& w = text_w[index - 1];
	int x = w.ex - 16;
	int y = w.ey - 16;
	ags->draw_gaiji(0, x, y, push_bitmap ? push_bitmap : builtin_bitmap, 16, text.font_color);
	ags->invalidate_screen(x, y, 16, 16);
}

void NACT::open_text_window(int index, bool erase)
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

	if (w.save) {
		w.screen = ags->save_rect(sx, sy, width, height);
	}

	if (erase) {
		ags->draw_window(sx, sy, ex, ey, w.frame, text.frame_color, text.back_color);
	} else if (w.save && w.window) {
		ags->restore_rect(w.window);
	}

	text.reset_pos(w.sx, w.sy + text.line_space);
}

void NACT::close_text_window(int index, bool update)
{
	Window& w = text_w[index - 1];
	int sx = w.sx - (w.frame ? 8 : 0);
	int sy = w.sy - (w.frame ? 8 : 0);
	int ex = w.ex + (w.frame ? 8 : 0);
	int ey = w.ey + (w.frame ? 8 : 0);
	int width = ex - sx + 1;
	int height = ey - sy + 1;

	if (w.save) {
		w.window = ags->save_rect(sx, sy, width, height);
	}

	if (w.save && w.screen) {
		ags->restore_rect(w.screen);
	}

	if (update) {
		text.reset_pos(w.sx, w.sy + text.line_space);
	}
}

void NACT::clear_menu_window()
{
	ags->box_fill(2, 0, 0, 639, 479, menu.back_color);
	menu.reset_pos(2, 2);
}

void NACT::open_menu_window(int index)
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

	if (w.save) {
		w.screen = ags->save_rect(wsx, wsy, wwidth, wheight);
	}

	ags->draw_window(wsx, wsy, wex, wey, w.frame, menu.frame_color, menu.back_color);
	ags->copy_screen(2, 0, 0, 0, width - 1, height - 1, sx, sy);
	ags->box_line(0, sx, sy, ex, sy + menu.font_size + 3, menu.frame_color);
	ags->invalidate_screen(wsx, wsy, wwidth, wheight);
}

void NACT::redraw_menu_window(int index, int selected)
{
	Window &w = menu_w[index - 1];
	int sx = w.sx;
	int sy = w.sy;
	int ex = w.ex;
	int ey = menu_fix ? w.ey : sy + menu.pos.y - 3;
	int width = ex - sx + 1;
	int height = ey - sy + 1;

	ags->copy_screen(2, 0, 0, 0, width - 1, height - 1, sx, sy);
	ags->box_line(0, sx, sy + (menu.font_size + 4) * selected, ex, sy + (menu.font_size + 4) * (selected + 1) - 1, menu.frame_color);
	ags->invalidate_screen(sx, sy, width, height);
}

void NACT::close_menu_window(int index)
{
	Window &w = menu_w[index - 1];
	if (w.save && w.screen) {
		ags->restore_rect(w.screen);
	}
}

void NACT::get_menu_window_rect(int index, int* sx, int* sy, int* ex, int* ey)
{
	Window &w = menu_w[index - 1];
	if (sx) *sx = w.sx;
	if (sy) *sy = w.sy;
	if (ex) *ex = w.ex;
	if (ey) *ey = w.ey;
}

int NACT::calculate_menu_max(int window)
{
	if (game_id.is(GameId::INTRUDER))
		return 6;
	if (game_id.is(GameId::GAKUEN_SENKI))
		return (menu_w[window - 1].ey - menu_w[window - 1].sy) / (menu.font_size + 4);
	return 11;
}

void NACT::load_display_state(FILEIO* fio)
{
	menu.font_size = fio->getw();
	text.font_size = fio->getw();
	ags->palette_bank = fio->getw();
	if (!ags->palette_bank) {
		ags->palette_bank = -1;
	}
	text.font_color = fio->getw();
	menu.font_color = fio->getw();
	menu.frame_color = fio->getw();
	menu.back_color = fio->getw();
	text.frame_color = fio->getw();
	text.back_color = fio->getw();
	for (int i = 0; i < 10; i++) {
		int sx = fio->getw();
		int sy = fio->getw();
		int ex = fio->getw();
		int ey = fio->getw();
		bool save = fio->getw() ? true : false;
		bool frame = fio->getw() ? true : false;
		menu_w[i].reset(sx, sy, ex, ey, frame, save);
		fio->getw();
		fio->getw();
	}
	for (int i = 0; i < 10; i++) {
		int sx = fio->getw();
		int sy = fio->getw();
		int ex = fio->getw();
		int ey = fio->getw();
		bool save = fio->getw() ? true : false;
		bool frame = fio->getw() ? true : false;
		text_w[i].reset(sx, sy, ex, ey, frame, save);
		fio->getw();
		fio->getw();
	}
}

void NACT::save_display_state(FILEIO* fio)
{
	fio->putw(menu.font_size);
	fio->putw(text.font_size);
	fio->putw(ags->palette_bank == -1 ? 0 : ags->palette_bank);
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

void NACT::draw_box(int index)
{
	if (index == 0) {
		// Clear the entire screen
		ags->box_fill(ags->dest_screen, 0, 0, 639, 479, 0);
		return;
	}

	Box& b = box[index - 1];
	if (1 <= index && index <= 10) {
		ags->box_fill(ags->dest_screen, b.sx, b.sy, b.ex, b.ey, b.color);
	} else if (11 <= index && index <= 20) {
		ags->box_line(ags->dest_screen, b.sx, b.sy, b.ex, b.ey, b.color);
	}
}

void NACT::load_cg(int page, int transparent)
{
	ags->load_cg(page, transparent, cg_flags);
}
