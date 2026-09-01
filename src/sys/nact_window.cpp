/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - window ]
*/

#include "nact.h"
#include "ags.h"
#include "fileio.h"

void NACT::load_display_state(FILEIO* fio)
{
	ags->menu.font_size = fio->getw();
	ags->text.font_size = fio->getw();
	ags->palette_bank = fio->getw();
	if (!ags->palette_bank) {
		ags->palette_bank = -1;
	}
	ags->text.font_color = fio->getw();
	ags->menu.font_color = fio->getw();
	ags->menu.frame_color = fio->getw();
	ags->menu.back_color = fio->getw();
	ags->text.frame_color = fio->getw();
	ags->text.back_color = fio->getw();
	for (int i = 0; i < 10; i++) {
		int sx = fio->getw();
		int sy = fio->getw();
		int ex = fio->getw();
		int ey = fio->getw();
		bool save = fio->getw() ? true : false;
		bool frame = fio->getw() ? true : false;
		ags->menu_w[i].reset(sx, sy, ex, ey, frame, save);
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
		ags->text_w[i].reset(sx, sy, ex, ey, frame, save);
		fio->getw();
		fio->getw();
	}
}

void NACT::save_display_state(FILEIO* fio)
{
	fio->putw(ags->menu.font_size);
	fio->putw(ags->text.font_size);
	fio->putw(ags->palette_bank == -1 ? 0 : ags->palette_bank);
	fio->putw(ags->text.font_color);
	fio->putw(ags->menu.font_color);
	fio->putw(ags->menu.frame_color);
	fio->putw(ags->menu.back_color);
	fio->putw(ags->text.frame_color);
	fio->putw(ags->text.back_color);
	for (int i = 0; i < 10; i++) {
		fio->putw(ags->menu_w[i].sx);
		fio->putw(ags->menu_w[i].sy);
		fio->putw(ags->menu_w[i].ex);
		fio->putw(ags->menu_w[i].ey);
		fio->putw(ags->menu_w[i].save ? 1 : 0);
		fio->putw(ags->menu_w[i].frame ? 1 : 0);
		fio->putw(0);
		fio->putw(0);
	}
	for (int i = 0; i < 10; i++) {
		fio->putw(ags->text_w[i].sx);
		fio->putw(ags->text_w[i].sy);
		fio->putw(ags->text_w[i].ex);
		fio->putw(ags->text_w[i].ey);
		fio->putw(ags->text_w[i].save ? 1 : 0);
		fio->putw(ags->text_w[i].frame ? 1 : 0);
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
