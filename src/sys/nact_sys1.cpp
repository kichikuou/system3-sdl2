/*
	ALICE SOFT SYSTEM1 for Win32

	D.P.S. - Dream Program System
	Intruder -桜屋敷の探索-
	Little Vampire
	あぶないてんぐ伝説
	クレセントムーンがぁる
	婦警さん物語ＶＸ

	[ NACT - command ]
*/

#include "nact.h"
#include "ags.h"
#include "mako.h"
#include "crc32.h"
#include "../fileio.h"

// メニューの改ページ
#define MENU_MAX (crc32_a == CRC32_INTRUDER ? 6 : 11)

#define WAIT(tm) \
{ \
	Uint32 dwTime = SDL_GetTicks() + (tm); \
	for(;;) { \
		if(terminate) { \
			return; \
		} \
		if(dwTime <= SDL_GetTicks()) { \
			break; \
		} \
		SDL_Delay(10); \
	} \
}

#define WAIT_KEYQUIT(tm) \
{ \
	Uint32 dwTime = SDL_GetTicks() + (tm); \
	for(;;) { \
		if(terminate) { \
			return; \
		} \
		if(get_key()) { \
			for(;;) { \
				if(terminate) { \
					return; \
				} \
				if(!get_key()) { \
					break; \
				} \
				if(dwTime <= SDL_GetTicks()) { \
					break; \
				} \
				SDL_Delay(10); \
			} \
			break; \
		} \
		if(dwTime <= SDL_GetTicks()) { \
			break; \
		} \
		SDL_Delay(10); \
	} \
}

NACT_Sys1::NACT_Sys1(uint32 crc32_a, uint32 crc32_b, const char* font_file, const char* playlist)
	: NACT(1, crc32_a, crc32_b, font_file, playlist)
{
	switch (crc32_a) {
	case CRC32_DPS:
	case CRC32_DPS_SG:
	//case CRC32_DPS_SG2:
	case CRC32_DPS_SG3:
		text_refresh = false;
		strcpy_s(tvar[0], 22, "\x83\x4A\x83\x58\x83\x5E\x83\x80"); // "カスタム" in SJIS
		strcpy_s(tvar[1], 22, "\x83\x8A\x81\x5B\x83\x69\x83\x58"); // "リーナス" in SJIS
		strcpy_s(tvar[2], 22, "\x82\xA9\x82\xC2\x82\xDD"); // "かつみ" in SJIS
		strcpy_s(tvar[3], 22, "\x97\x52\x94\xFC\x8E\x71"); // "由美子" in SJIS
		strcpy_s(tvar[4], 22, "\x82\xA2\x82\xC2\x82\xDD"); // "いつみ" in SJIS
		strcpy_s(tvar[5], 22, "\x82\xD0\x82\xC6\x82\xDD"); // "ひとみ" in SJIS
		strcpy_s(tvar[6], 22, "\x90\x5E\x97\x9D\x8E\x71"); // "真理子" in SJIS
		break;
	case CRC32_INTRUDER:
		paint_x = paint_y = map_page = 0;
		break;
	}
}

void NACT_Sys1::opening()
{
	switch (crc32_a) {
	case CRC32_CRESCENT:
		mako->play_music(2);
		ags->load_cg(1, -1);
		WAIT(3000);
		ags->load_cg(81, -1);
		break;
	case CRC32_INTRUDER:
		ags->load_cg(77, -1);
		WAIT(1667);
		ags->load_cg(74, -1);
		break;
	case CRC32_VAMPIRE:
		mako->play_music(4);
		ags->load_cg(3, -1);
		WAIT(2000);
		ags->load_cg(38, -1);
		cmd_a();
		ags->draw_box(0);
		break;
	}
}

void NACT_Sys1::cmd_calc()
{
	int index = getd();
	if(0x80 <= index && index <= 0xbf) {
		index &= 0x3f;
	} else {
		index = ((index & 0x3f) << 8) | getd();
	}
	var[index] = cali();

	output_console("\n!var[%d]:%d!", index, var[index]);
}

void NACT_Sys1::cmd_branch()
{
	int condition = cali();
	int nest = 0;
	bool set_menu = false;

	if(!condition) {
		// 次の'}'命令までスキップする（ネストも考慮する）
		for(;;) {
			prev_addr = scenario_addr;
			uint8 cmd = getd();

			if(cmd == '!') {
				int index = getd();
				if(!(0x80 <= index && index <= 0xbf)) {
					getd();
				}
				cali();
			} else if(cmd == '{') {
				cali();
				nest++;
			} else if(cmd == '}') {
				if(nest) {
					nest--;
				} else {
					break;
				}
			} else if(cmd == '@') {
				getw();
			} else if(cmd == '\\') {
				getw();
			} else if(cmd == '&') {
				cali();
			} else if(cmd == '%') {
				cali();
			} else if(cmd == '$') {
				if(!set_menu) {
					getw();
					set_menu = true;
				} else {
					set_menu = false;
				}
			} else if(cmd == '[') {
				getd();
				getd();
				getw();
			} else if(cmd == ':') {
				cali();
				getd();
				getd();
				getw();
			} else if(cmd == ']') {
				
			} else if(cmd == 'A') {
				
			} else if(cmd == 'F') {
				
			} else if(cmd == 'G') {
				getd();
			} else if(cmd == 'L') {
				getd();
			} else if(cmd == 'P') {
				getd();
			} else if(cmd == 'Q') {
				getd();
			} else if(cmd == 'R') {
				
			} else if(cmd == 'S') {
				getd();
			} else if(cmd == 'U') {
				getd();
				getd();
			} else if(cmd == 'X') {
				getd();
			} else if(cmd == 'Y') {
				cali();
				cali();
			} else if(cmd == 'Z') {
				cali();
				cali();
			} else if(cmd == 0x20 || (0xa1 <= cmd && cmd <= 0xdd)) {
				// message (1 byte)
			} else if((0x81 <= cmd && cmd <= 0x9f) || 0xe0 <= cmd) {
				// message (2 bytes)
				getd();
			} else {
				if(cmd >= 0x20 && cmd < 0x7f) {
					fatal("Unknown Command: '%c' at page = %d, addr = %d", cmd, scenario_page, prev_addr);
				} else {
					fatal("Unknown Command: %02x at page = %d, addr = %d", cmd, scenario_page, prev_addr);
				}
				break;
			}
		}
	}

	output_console("\n{%d:", condition);
}

void NACT_Sys1::cmd_label_jump()
{
	int next_addr = getw();
	scenario_addr = next_addr;

	output_console("\n@%x:", next_addr);
}

void NACT_Sys1::cmd_label_call()
{
	int next_addr = getw();

	output_console("\n\\%x:", next_addr);

	if(next_addr == 0) {
		// リターン
		if(label_depth == 0) {
//			fatal_error = true;
			return;
		}
		scenario_addr = label_stack[--label_depth];
	} else {
		label_stack[label_depth++] = scenario_addr;
		scenario_addr = next_addr;
	}
}

void NACT_Sys1::cmd_page_jump()
{
	int next_page = cali();
	load_scenario(next_page);
	scenario_page = next_page;
	scenario_addr = 2;

	output_console("\n&%d:", next_page);
}

void NACT_Sys1::cmd_page_call()
{
	int next_page = cali(), next_addr;

	output_console("\n%%%d:", next_page);

	if(next_page == 0) {
		// リターン
		if(page_depth == 0) {
//			fatal_error = true;
			return;
		}
		next_page = page_stack[--page_depth];
		next_addr = addr_stack[page_depth];
	} else {
		page_stack[page_depth] = scenario_page;
		addr_stack[page_depth++] = scenario_addr;
		next_addr = 2;
	}
	load_scenario(next_page);
	scenario_page = next_page;
	scenario_addr = next_addr;
}

void NACT_Sys1::cmd_set_menu()
{
	if(ags->draw_menu) {
		ags->menu_dest_x = 2;
		ags->menu_dest_y += ags->menu_font_size + 2;
		ags->draw_menu = false;

		output_console("$");
	} else {
		if(!menu_index) {
			ags->clear_menu_window();
			ags->menu_dest_y = 0;
		}
		menu_addr[menu_index++] = getw();
		ags->menu_dest_x = 2;
		ags->menu_dest_y += 2;
		ags->draw_menu = true;

		output_console("\n$%x,", menu_addr[menu_index - 1]);
	}
}

void NACT_Sys1::cmd_open_menu()
{
	output_console("\n]");

	if(!menu_index) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
		return;
	}

	if(crc32_a == CRC32_DPS || crc32_a == CRC32_DPS_SG || crc32_a == CRC32_DPS_SG2 || crc32_a == CRC32_DPS_SG3) {
		if(!text_refresh) {
			cmd_a();
		}
	}

	// メニュー表示
	ags->open_menu_window(menu_window);

	// マウス移動
	int sx = ags->menu_w[menu_window - 1].sx;
	int sy = ags->menu_w[menu_window - 1].sy;
	int ex = ags->menu_w[menu_window - 1].ex;
	int mx = ex - 16;
	int my = sy + 10;
	int height = ags->menu_font_size + 4;
	int current_index = 0;

	set_cursor(mx, my);
	wait_after_open_menu();

	// メニュー選択
	for(bool selectable = true;;) {
		// 入力待機
		int val = 0, current_mx = mx, current_my = my;
		for(;;) {
			if(terminate) {
				return;
			}
			if((val = get_key()) != 0) {
				SDL_Delay(100);
				break;
			}
			get_cursor(&current_mx, &current_my);
			if(abs(my - current_my) > 3) {
				break;
			}
			SDL_Delay(10);
		}
		if(val) {
			for(;;) {
				if(terminate) {
					return;
				}
				if(!get_key()) {
					break;
				}
				SDL_Delay(10);
			}
		}

		if(val == 0) {
			// マウス操作
			mx = current_mx; my = current_my;
			int index = (my - sy) / height;
			if(sx <= mx && mx <= ex && 0 <= index && index < menu_index) {
				current_index = index;
				ags->redraw_menu_window(menu_window, current_index);
				selectable = true;
			} else {
				selectable = false;
			}
		} else if(val == 1 || val == 2 || val == 4 || val == 8) {
			if(val == 1) {
				current_index = current_index ? current_index - 1 : menu_index - 1;
			} else if(val == 2) {
				current_index = (current_index < menu_index - 1) ? current_index + 1 : 0;
			} else if(val == 4) {
				current_index = 0;
			} else if(val == 8) {
				current_index = menu_index - 1;
			}
			ags->redraw_menu_window(menu_window, current_index);
			selectable = true;
		} else if(val == 16 && selectable) {
			break;
		} else if(val == 32) {
			current_index = -1;
			break;
		}
	}

	// 画面更新
	ags->close_menu_window(menu_window);
	if(clear_text) {
		ags->clear_text_window(text_window, true);
	}

	if(current_index != -1) {
		scenario_addr = menu_addr[current_index];
	}
	menu_index = 0;
}

void NACT_Sys1::cmd_set_verbobj()
{
	int verb = getd();
	int obj = getd();
	int addr = getw();

	menu_addr[menu_index] = addr;
	menu_verb[menu_index] = verb;
	menu_obj[menu_index++] = obj;
	verb_obj = true;

	output_console("\n[%x,%s,%s:", addr, caption_verb[verb], caption_obj[obj]);
}

void NACT_Sys1::cmd_set_verbobj2()
{
	int condition = cali();
	int verb = getd();
	int obj = getd();
	int addr = getw();

	if(condition) {
		menu_addr[menu_index] = addr;
		menu_verb[menu_index] = verb;
		menu_obj[menu_index++] = obj;
	}
	verb_obj = true;

	output_console("\n:%d,%x,%s,%s:", condition, addr, caption_verb[verb], caption_obj[obj]);
}

void NACT_Sys1::cmd_open_verb()
{
	// 動詞メニューの表示
	output_console("\nopen verb-obj menu");
	verb_obj = false;

	// 表示する動詞のチェック
	int chk[MAX_VERB], page = 0, cnt = 0;
	
	memset(chk, 0, sizeof(chk));
	for(int i = 0; i < menu_index; i++) {
		chk[menu_verb[i]] = 1;
	}
	for(int i = 0; i < MAX_VERB; i++) {
		if(chk[i]) {
			cnt++;
		}
	}

top:
	// メニュー項目の準備
	int id[32], index = 0;

	ags->clear_menu_window();
	ags->menu_dest_y = 0;
	ags->draw_menu = true;

	if(cnt <= MENU_MAX) {
		// 1ページ内に全て表示できる
		for(int i = 0; i < MAX_VERB; i++) {
			if(chk[i]) {
				ags->menu_dest_x = 2;
				ags->menu_dest_y += 2;
				ags->draw_text(caption_verb[i]);
				id[index++] = i;
				ags->menu_dest_y += ags->menu_font_size + 2;
			}
		}
	} else {
top2:
		for(int i = page; i < MAX_VERB; i++) {
			if(chk[i]) {
				ags->menu_dest_x = 2;
				ags->menu_dest_y += 2;
				ags->draw_text(caption_verb[i]);
				id[index++] = i;
				ags->menu_dest_y += ags->menu_font_size + 2;
			}
			page = i + 1;
			if(index == MENU_MAX - 1) {
				break;
			}
		}
		if(index == 0) {
			// 最初のページに戻る
			page = 0;
			goto top2;
		}
		// 次のページを追加
		ags->menu_dest_x = 2;
		ags->menu_dest_y += 2;
		ags->draw_text(SJIS_NEXT_PAGE);
		id[index++] = -1;
		ags->menu_dest_y += ags->menu_font_size + 2;
	}
	ags->draw_menu = false;

	// メニュー表示
	ags->open_menu_window(menu_window);

	// マウス移動
	int sx = ags->menu_w[menu_window - 1].sx;
	int sy = ags->menu_w[menu_window - 1].sy;
	int ex = ags->menu_w[menu_window - 1].ex;
	int mx = ex - 16;
	int my = sy + 10;
	int height = ags->menu_font_size + 4;
	int current_index = 0;

	set_cursor(mx, my);
	wait_after_open_menu();

	// メニュー選択
	for(bool selectable = true;;) {
		// 入力待機
		int val = 0, current_mx = mx, current_my = my;
		for(;;) {
			if(terminate) {
				return;
			}
			if((val = get_key()) != 0) {
				SDL_Delay(100);
				break;
			}
			get_cursor(&current_mx, &current_my);
			if(abs(my - current_my) > 3) {
				break;
			}
			SDL_Delay(10);
		}
		if(val) {
			for(;;) {
				if(terminate) {
					return;
				}
				if(!get_key()) {
					break;
				}
				SDL_Delay(10);
			}
		}

		if(val == 0) {
			// マウス操作
			mx = current_mx; my = current_my;
			int mindex = (my - sy) / height;
			if(sx <= mx && mx <= ex && 0 <= mindex && mindex < index) {
				current_index = mindex;
				ags->redraw_menu_window(menu_window, current_index);
				selectable = true;
			} else {
				selectable = false;
			}
		} else if(val == 1 || val == 2 || val == 4 || val == 8) {
			if(val == 1) {
				current_index = current_index ? current_index - 1 : index - 1;
			} else if(val == 2) {
				current_index = (current_index < index - 1) ? current_index + 1 : 0;
			} else if(val == 4) {
				current_index = 0;
			} else if(val == 8) {
				current_index = index - 1;
			}
			ags->redraw_menu_window(menu_window, current_index);
			selectable = true;
		} else if(val == 16 && selectable) {
			break;
		} else if(val == 32) {
			current_index = -1;
			break;
		}
	}

	// 画面更新
	ags->close_menu_window(menu_window);
	if(clear_text) {
		ags->clear_text_window(text_window, true);
	}

	if(current_index == -1) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
	} else if(id[current_index] == -1) {
		goto top;
	} else {
		cmd_open_obj(id[current_index]);
	}
	menu_index = 0;
}

void NACT_Sys1::cmd_open_obj(int verb)
{
	// 目的語メニューの表示
	verb_obj = false;

	// 表示する目的語のチェック
	int chk[MAX_OBJ], addr[MAX_OBJ], page = 0, cnt = 0;
	
	memset(chk, 0, sizeof(chk));
	for(int i = 0; i < menu_index; i++) {
		if(menu_verb[i] == verb) {
			chk[menu_obj[i]] = 1;
			addr[menu_obj[i]] = menu_addr[i];
		}
	}
	for(int i = 0; i < MAX_OBJ; i++) {
		if(chk[i]) {
			cnt++;
		}
	}
	// 目的語がない場合
	if(chk[0] && cnt == 1) {
		scenario_addr = addr[0];
		return;
	}
	// 以後、obj=0は戻るとして扱う
	chk[0] = 0;
	addr[0] = scenario_data[0] | (scenario_data[1] << 8);

top:
	// メニュー項目の準備
	int id[32], index = 0;

	ags->clear_menu_window();
	ags->menu_dest_y = 0;
	ags->draw_menu = true;

	if(cnt <= MENU_MAX - 1) {
		// 1ページ内に全て表示できる
		for(int i = 0; i < MAX_OBJ; i++) {
			if(chk[i]) {
				ags->menu_dest_x = 2;
				ags->menu_dest_y += 2;
				ags->draw_text(caption_obj[i]);
				id[index++] = i;
				ags->menu_dest_y += ags->menu_font_size + 2;
			}
		}
		// 戻るを追加
		ags->menu_dest_x = 2;
		ags->menu_dest_y += 2;
		ags->draw_text(SJIS_BACK);
		id[index++] = 0;
		ags->menu_dest_y += ags->menu_font_size + 2;
	} else {
top2:
		for(int i = page; i < MAX_OBJ; i++) {
			if(chk[i]) {
				ags->menu_dest_x = 2;
				ags->menu_dest_y += 2;
				ags->draw_text(caption_obj[i]);
				id[index++] = i;
				ags->menu_dest_y += ags->menu_font_size + 2;
			}
			page = i + 1;
			if(index == MENU_MAX - 2) {
				break;
			}
		}
		if(index == 0) {
			// 最初のページに戻る
			page = 0;
			goto top2;
		}
		// 戻るを追加
		ags->menu_dest_x = 2;
		ags->menu_dest_y += 2;
		ags->draw_text(SJIS_BACK);
		id[index++] = 0;
		ags->menu_dest_y += ags->menu_font_size + 2;

		// 次のページを追加
		ags->menu_dest_x = 2;
		ags->menu_dest_y += 2;
		ags->draw_text(SJIS_NEXT_PAGE);
		id[index++] = -1;
		ags->menu_dest_y += ags->menu_font_size + 2;
	}
	ags->draw_menu = false;

	// メニュー表示
	ags->open_menu_window(menu_window);

	// マウス移動
	int sx = ags->menu_w[menu_window - 1].sx;
	int sy = ags->menu_w[menu_window - 1].sy;
	int ex = ags->menu_w[menu_window - 1].ex;
	int mx = ex - 16;
	int my = sy + 10;
	int height = ags->menu_font_size + 4;
	int current_index = 0;

	set_cursor(mx, my);
	wait_after_open_menu();

	// メニュー選択
	for(bool selectable = true;;) {
		// 入力待機
		int val = 0, current_mx = mx, current_my = my;
		for(;;) {
			if(terminate) {
				return;
			}
			if((val = get_key()) != 0) {
				SDL_Delay(100);
				break;
			}
			get_cursor(&current_mx, &current_my);
			if(abs(my - current_my) > 3) {
				break;
			}
			SDL_Delay(10);
		}
		if(val) {
			for(;;) {
				if(terminate) {
					return;
				}
				if(!get_key()) {
					break;
				}
				SDL_Delay(10);
			}
		}

		if(val == 0) {
			// マウス操作
			mx = current_mx; my = current_my;
			int mindex = (my - sy) / height;
			if(sx <= mx && mx <= ex && 0 <= mindex && mindex < index) {
				current_index = mindex;
				ags->redraw_menu_window(menu_window, current_index);
				selectable = true;
			} else {
				selectable = false;
			}
		} else if(val == 1 || val == 2 || val == 4 || val == 8) {
			if(val == 1) {
				current_index = current_index ? current_index - 1 : index - 1;
			} else if(val == 2) {
				current_index = (current_index < index - 1) ? current_index + 1 : 0;
			} else if(val == 4) {
				current_index = 0;
			} else if(val == 8) {
				current_index = index - 1;
			}
			ags->redraw_menu_window(menu_window, current_index);
			selectable = true;
		} else if(val == 16 && selectable) {
			break;
		} else if(val == 32) {
			current_index = -1;
			break;
		}
	}

	// 画面更新
	ags->close_menu_window(menu_window);
	if(clear_text) {
		ags->clear_text_window(text_window, true);
	}

	if(current_index == -1) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
	} else if(id[current_index] == -1) {
		goto top;
	} else {
		scenario_addr = addr[id[current_index]];
	}
}

void NACT_Sys1::cmd_a()
{
	output_console("A\n");

	if(!text_skip_enb) {
		// Pushマークの表示
		if(show_push) {
			ags->draw_push(text_window);
		}

		// キーが押されて離されるまで待機
		for(;;) {
			if(terminate) {
				return;
			}
			if(get_key()) {
				break;
			}
			SDL_Delay(10);
		}
		SDL_Delay(100);
		for(;;) {
			if(terminate) {
				return;
			}
			if(!(get_key() & 0x18)) {
				break;
			}
			SDL_Delay(10);
		}
	}

	// ウィンドウ更新
	ags->clear_text_window(text_window, true);

	if(crc32_a == CRC32_DPS || crc32_a == CRC32_DPS_SG || crc32_a == CRC32_DPS_SG2 || crc32_a == CRC32_DPS_SG3) {
		text_refresh = true;
	}
}

void NACT_Sys1::cmd_b()
{
	// 未使用
	fatal("Unknown Command: 'B' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_d()
{
	// 未使用
	fatal("Unknown Command: 'D' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_e()
{
	// 未使用
	fatal("Unknown Command: 'E' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_f()
{
	output_console("\nF");

	scenario_addr = 2;
}

void NACT_Sys1::cmd_g()
{
	int page = getd();

	output_console("\nG %d:", page);

	if(crc32_a == CRC32_INTRUDER) {
		page = (page == 97) ? 96 : (page == 98) ? 97 : page;
	}

	ags->load_cg(page, -1);

	if(crc32_a == CRC32_INTRUDER) {
		if(page == 94) {
			WAIT(400);
		}
		if(page == 81 || page == 82) {
			map_page = page;
		}
	}
}

void NACT_Sys1::cmd_h()
{
	// 未使用
	fatal("Unknown Command: 'H' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_i()
{
	// 未使用
	fatal("Unknown Command: 'I' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_j()
{
	// 未使用
	fatal("Unknown Command: 'J' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_k()
{
	// 未使用
	fatal("Unknown Command: 'K' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_l()
{
	int index = getd();

	output_console("\nL %d:", index);

	if(1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		char file_name[_MAX_PATH];
		sprintf_s(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + index - 1);

		FILEIO* fio = new FILEIO();
		if(fio->Fopen(file_name, FILEIO_READ_BINARY)) {
			fio->Fseek(112, FILEIO_SEEK_SET);

			int next_page = fio->Fgetw() - 1;
			fio->Fgetw();
			fio->Fgetw();	// cg no?
			fio->Fgetw();
			int next_music = fio->Fgetw();
			fio->Fgetw();
			int next_addr = fio->Fgetw();
			fio->Fgetw();
			for(int i = 0; i < 512; i++) {
				var[i] = fio->Fgetw();
			}
			ags->menu_font_size = fio->Fgetw();
			ags->text_font_size = fio->Fgetw();
			ags->palette_bank = fio->Fgetw();
			if(!ags->palette_bank) {
				ags->palette_bank = -1;
			}
			ags->text_font_color = fio->Fgetw();
			ags->menu_font_color = fio->Fgetw();
			ags->menu_frame_color = fio->Fgetw();
			ags->menu_back_color = fio->Fgetw();
			ags->text_frame_color = fio->Fgetw();
			ags->text_back_color = fio->Fgetw();
			for(int i = 0; i < 10; i++) {
				ags->menu_w[i].sx = fio->Fgetw();
				ags->menu_w[i].sy = fio->Fgetw();
				ags->menu_w[i].ex = fio->Fgetw();
				ags->menu_w[i].ey = fio->Fgetw();
				ags->menu_w[i].push = fio->Fgetw() ? true : false;
				ags->menu_w[i].frame = fio->Fgetw() ? true : false;
				fio->Fgetw();
				fio->Fgetw();

				if(ags->menu_w[i].screen) {
					free(ags->menu_w[i].screen);
				}
				ags->menu_w[i].screen = NULL;
				if(ags->menu_w[i].window) {
					free(ags->menu_w[i].window);
				}
				ags->menu_w[i].window = NULL;
			}
			for(int i = 0; i < 10; i++) {
				ags->text_w[i].sx = fio->Fgetw();
				ags->text_w[i].sy = fio->Fgetw();
				ags->text_w[i].ex = fio->Fgetw();
				ags->text_w[i].ey = fio->Fgetw();
				ags->text_w[i].push = fio->Fgetw() ? true : false;
				ags->text_w[i].frame = fio->Fgetw() ? true : false;
				fio->Fgetw();
				fio->Fgetw();

				if(ags->text_w[i].screen) {
					free(ags->text_w[i].screen);
				}
				ags->text_w[i].screen = NULL;
				if(ags->text_w[i].window) {
					free(ags->text_w[i].window);
				}
				ags->text_w[i].window = NULL;
			}
			for(int i = 0; i < 10; i++) {
				fio->Fread(tvar[i], 22, 1);
			}
			for(int i = 0; i < 30; i++) {
				for(int j = 0; j < 10; j++) {
					fio->Fread(tvar_stack[i][j], 22, 1);
				}
			}
			for(int i = 0; i < 30; i++) {
				for(int j = 0; j < 20; j++) {
					var_stack[i][j] = fio->Fgetw();
				}
			}
			fio->Fclose();

			load_scenario(next_page);
			scenario_page = next_page;
			scenario_addr = next_addr;

			mako->play_music(next_music);
		}
		delete fio;
	}
}

void NACT_Sys1::cmd_m()
{
	// 未使用
	fatal("Unknown Command: 'M' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_n()
{
	// 未使用
	fatal("Unknown Command: 'N' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_o()
{
	// 未使用
	fatal("Unknown Command: 'O' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_p()
{
	int param = getd();

	ags->text_font_color = (uint8)((param & 0x7) + 16);

	output_console("\nP %d:", param);
}

#define FWRITE(data, size) { \
	memcpy(&buffer[p], data, size); \
	p += size; \
}
#define FPUTW(data) { \
	uint16 tmp = (data); \
	buffer[p++] = tmp & 0xff; \
	buffer[p++] = (tmp >> 8) & 0xff; \
}

void NACT_Sys1::cmd_q()
{
	static char header[112] = {
		'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'a', 'v', 'e', ' ', 'd', 'a', 't',
		'a', ' ', 'f', 'o', 'r', ' ', 'S', 'y', 's', 't', 'e', 'm', '1', ' ', ' ', 'U',
		'n', 'i', 'o', 'n', ' ', '[', 'W', 'i', 'n', '9', 'x', '/', 'N', 'T', '4', '0',
		'/', '2', '0', '0', '0', '/', 'X', 'P', '/', 'C', 'E', ']', ' ', 'F', 'o', 'r',
		' ', 'N', 'A', 'C', 'T', '/', 'A', 'D', 'V', ' ', 's', 'y', 's', 't', 'e', 'm',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '(', 'C', ')', '2', '0', '0',
		'4', ' ', 'A', 'L', 'I', 'C', 'E', '-', 'S', 'O', 'F', 'T', ' ', ' ', ' ', ' '
	};

	int index = getd();

	output_console("\nQ %d:", index);

	if(1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		char file_name[_MAX_PATH];
		sprintf_s(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + index - 1);

		FILEIO* fio = new FILEIO();
		if(fio->Fopen(file_name, FILEIO_WRITE_BINARY)) {
			uint8 buffer[9510];
			int p = 0;

			FWRITE(header, 112);
			FPUTW(scenario_page + 1);
			FPUTW(0);
			FPUTW(0);	// cg no?
			FPUTW(0);
			FPUTW(mako->current_music);
			FPUTW(0);
			FPUTW(scenario_addr);
			FPUTW(0);
			for(int i = 0; i < 512; i++) {
				FPUTW(var[i]);
			}
			FPUTW(ags->menu_font_size);
			FPUTW(ags->text_font_size);
			FPUTW(ags->palette_bank == -1 ? 0 : ags->palette_bank);
			FPUTW(ags->text_font_color);
			FPUTW(ags->menu_font_color);
			FPUTW(ags->menu_frame_color);
			FPUTW(ags->menu_back_color);
			FPUTW(ags->text_frame_color);
			FPUTW(ags->text_back_color);
			for(int i = 0; i < 10; i++) {
				FPUTW(ags->menu_w[i].sx);
				FPUTW(ags->menu_w[i].sy);
				FPUTW(ags->menu_w[i].ex);
				FPUTW(ags->menu_w[i].ey);
				FPUTW(ags->menu_w[i].push ? 1 : 0);
				FPUTW(ags->menu_w[i].frame ? 1 : 0);
				FPUTW(0);
				FPUTW(0);
			}
			for(int i = 0; i < 10; i++) {
				FPUTW(ags->text_w[i].sx);
				FPUTW(ags->text_w[i].sy);
				FPUTW(ags->text_w[i].ex);
				FPUTW(ags->text_w[i].ey);
				FPUTW(ags->text_w[i].push ? 1 : 0);
				FPUTW(ags->text_w[i].frame ? 1 : 0);
				FPUTW(0);
				FPUTW(0);
			}
			for(int i = 0; i < 10; i++) {
				FWRITE(tvar[i], 22);
			}
			for(int i = 0; i < 30; i++) {
				for(int j = 0; j < 10; j++) {
					FWRITE(tvar_stack[i][j], 22);
				}
			}
			for(int i = 0; i < 30; i++) {
				for(int j = 0; j < 20; j++) {
					FPUTW(var_stack[i][j]);
				}
			}

			fio->Fwrite(buffer, 9510, 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void NACT_Sys1::cmd_r()
{
	output_console("R\n");

	// ウィンドウの表示範囲外の場合は改ページ
	if(ags->return_text_line(text_window)) {
		cmd_a();
	}
}

void NACT_Sys1::cmd_s()
{
	int page = getd();

	output_console("\nS %d:", page);

	if(page) {
		mako->play_music(page);
	} else {
		mako->stop_music();
	}
}

void NACT_Sys1::cmd_t()
{
	// 未使用
	fatal("Unknown Command: 'T' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_u()
{
	int page = getd();
	int transparent = getd();

	output_console("\nU %d,%d:", page, transparent);

	if(crc32_a == CRC32_INTRUDER) {
		ags->load_cg(page, 11);

		if(page == 5) {
			WAIT(400);
		}
	} else {
		ags->load_cg(page, transparent);
	}
}

void NACT_Sys1::cmd_v()
{
	// 未使用
	fatal("Unknown Command: 'V' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_w()
{
	// 未使用
	fatal("Unknown Command: 'W' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys1::cmd_x()
{
	int index = getd();

	output_console("\nX %d:", index);

	if(1 <= index && index <= 10) {
		ags->draw_text(tvar[index - 1]);
	}
}

void NACT_Sys1::cmd_y()
{
	int cmd = cali();
	int param = cali();

	output_console("\nY %d,%d:", cmd, param);

	if(crc32_a == CRC32_INTRUDER) {
		switch(cmd) {
			case 0:
				ags->clear_text_window(text_window, true);
				break;
			case 1:
				if(show_push) {
					ags->draw_push(text_window);
				}
				for(;;) {
					if(terminate) {
						return;
					}
					if(get_key()) {
						break;
					}
					SDL_Delay(10);
				}
				SDL_Delay(100);
				for(;;) {
					if(terminate) {
						return;
					}
					if(!(get_key() & 0x18)) {
						break;
					}
					SDL_Delay(10);
				}
				ags->clear_text_window(text_window, true);
				break;
			case 2:
				if(param == 0) {
					post_quit = true;
					fatal_error = true;
				} else {
					WAIT(100)
				}
				break;
			case 3:
				WAIT(param * 1000)
				break;
			case 255:
				post_quit = true;
				fatal_error = true;
				break;
		}
	} else {
		switch(cmd) {
			case 0:
				ags->clear_text_window(text_window, true);
				break;
			case 1:
				{
					char buf[16];
					buf[0] = '0' + (int)(param / 10000);
					param %= 10000;
					buf[1] = '0' + (int)(param / 1000);
					param %= 1000;
					buf[2] = '0' + (int)(param / 100);
					param %= 100;
					buf[3] = '0' + (int)(param / 10);
					buf[4] = '0' + (param % 10);
					buf[5] = '\0';

					int p = 0;
					for(;;) {
						if(buf[p] != '0') {
							break;
						}
						p++;
					}
					ags->draw_text(&buf[p < 4 ? p : 4]);
				}
				break;
			case 2:
				if(crc32_a == CRC32_CRESCENT) {
					for(int i = 39; i <= 48; i++) {
						var[i] = 0;
					}
				} else if(crc32_a == CRC32_DPS || crc32_a == CRC32_DPS_SG || crc32_a == CRC32_DPS_SG2 || crc32_a == CRC32_DPS_SG3) {
					for(int i = 0; i <= 20; i++) {
						var[i] = 0;
					}
				} else {
					for(int i = 0; i <= 16; i++) {
						var[i] = 0;
					}
				}
				break;
			case 3:
				WAIT(param * 1000 / 60)
				break;
			case 4:
				RND = (param == 0 || param == 1) ? 0 : random(param);
				break;
			case 7:
				if(crc32_a == CRC32_DPS || crc32_a == CRC32_DPS_SG || crc32_a == CRC32_DPS_SG2 || crc32_a == CRC32_DPS_SG3) {
					if(param == 1) {
						ags->box_fill(0, 40, 8, 598, 270, 0);
					}
				}
				break;
			case 130:
				if(crc32_a == CRC32_TENGU) {
					WAIT(2000)
					ags->load_cg(180, -1);

					WAIT(2000)
					ags->draw_box(0);
					ags->dest_screen = 1;
					ags->load_cg(173, -1);
					ags->dest_screen = 0;
					ags->gcopy(0x1ecd, 0x3a2a, 8, 60, 3);
					WAIT(667);
					ags->gcopy(0x1b65, 0x262e, 8, 60, 3);
					ags->gcopy(0x2e25, 0x38ee, 7, 9, 3);
					WAIT(667);
					ags->gcopy(0x1914, 0x1914, 8, 63, 3);
					WAIT(667);
					ags->gcopy(0x182c, 0x182c, 7, 50, 3);
					WAIT(667);
					ags->gcopy(0x0000, 0x1ecd, 32, 60, 1);
					WAIT(667);
					for(int i = 12; i <= 57; i++) {
						ags->gcopy(i + 0x2306, i + 0x2306, 1, 165, 3);
					}
					WAIT_KEYQUIT(6000)
					ags->draw_box(param);
				}
				break;
			case 253:
				post_quit = false;
				fatal_error = true;
				break;
			case 254:
				RND = 0;
				break;
			case 255:
				if(crc32_a == CRC32_DPS || crc32_a == CRC32_DPS_SG || crc32_a == CRC32_DPS_SG2 || crc32_a == CRC32_DPS_SG3) {
					post_quit = false;
				} else {
					post_quit = true;
				}
				fatal_error = true;
				break;
		}
	}
}

void NACT_Sys1::cmd_z()
{
	if (crc32_a == CRC32_CRESCENT) {
		cmd_y();
		return;
	}
	int cmd = cali();
	int param = cali();

	output_console("\nZ %d,%d:", cmd, param);

	switch (crc32_a) {
	case CRC32_INTRUDER:
		if(cmd == 1 && 1 <= param && param <= 4) {
			const char* buf[4] = {
				"\x81\x83", // "＜" in SJIS
				"\x81\xC8", // "∧" in SJIS
				"\x81\x84", // "＞" in SJIS
				"\x81\xC9", // "∨" in SJIS
			};

			// 矢印を消去する
			if(map_page) {
				ags->load_cg(map_page, -1);
				if(paint_x) {
					ags->paint(paint_x, paint_y * 2, 2 + 16);
				}
			}

			// 矢印を表示する
			int x = ags->text_dest_x;
			int y = ags->text_dest_y;
			int color = ags->text_font_color;
			ags->text_font_size = 24;
			ags->text_font_color = 16;

			ags->text_dest_x = 456;
			ags->text_dest_y = 103;
			ags->draw_text(buf[param - 1]);

			ags->text_dest_x = x;
			ags->text_dest_y = y;
			ags->text_font_size = 16;
			ags->text_font_color = color;
		} else if(cmd == 2) {
			ags->load_cg(74, -1);
			ags->load_cg(81, -1);
			map_page = 81;
		} else if(cmd == 3) {
			RND = (param == 0 || param == 1) ? 0 : random(param);
		} else if(460 <= cmd && cmd <= 625 && 20 <= param && param <= 55) {
			if(paint_x) {
				ags->paint(paint_x, paint_y * 2, 5 + 16);
			}
			ags->paint(cmd, param * 2, 2 + 16);
			paint_x = cmd;
			paint_y = param;
		} else {
			paint_x = 0;
		}
		break;

	case CRC32_TENGU:
		if(cmd == 10) {
			// nop
		} else if(cmd == 20) {
			if(param == 20) {
				ags->gcopy(8, 8, 46, 144, 2);
			} else if(param == 21) {
				ags->gcopy(8, 8, 46, 144, 3);
			}
		}
		break;
	}
}


// 下位関数

uint16 NACT_Sys1::cali()
{
	uint32 cali[256];
	int p = 1;
	bool ok = false;

	while(p > 0) {
		uint8 dat = getd();

		// 除算はサポートしない？
		if(0x80 <= dat && dat <= 0xbf) {
			cali[p++] = var[dat & 0x3f];
		} else if(0xc0 <= dat && dat <= 0xff) {
			cali[p++] = var[((dat & 0x3f) << 8) | getd()];
		} else if(0x00 <= dat && dat <= 0x3f) {
			cali[p++] = ((dat & 0x3f) << 8) | getd();
		} else if(0x40 <= dat && dat <= 0x77) {
			cali[p++] = dat & 0x3f;
		} else if(dat == 0x78) {
			cali[p - 2] = cali[p - 2] * cali[p - 1];
			if (cali[p - 2] > 65535) {
				cali[p - 2] = 65535;
			}
			p--;
		} else if(dat == 0x79) {
			cali[p - 2] = cali[p - 2] + cali[p - 1];
			if (cali[p - 2] > 65535) {
				cali[p - 2] = 65535;
			}
			p--;
		} else if(dat == 0x7a) {
			if(cali[p - 2] > cali[p - 1]) {
				cali[p - 2] = cali[p - 2] - cali[p - 1];
			} else {
				cali[p - 2] = 0;
			}
			p--;
		} else if(dat == 0x7b) {
			cali[p - 2] = (cali[p - 2] == cali[p - 1]) ? 1 : 0;
			p--;
		} else if(dat == 0x7c) {
			cali[p - 2] = (cali[p - 2] < cali[p - 1]) ? 1 : 0;
			p--;
		} else if(dat == 0x7d) {
			cali[p - 2] = (cali[p - 2] > cali[p - 1]) ? 1 : 0;
			p--;
		} else if(dat == 0x7e) {
			cali[p - 2] = (cali[p - 2] != cali[p - 1]) ? 1 : 0;
			p--;
		} else if(dat == 0x7f) {
			if(p == 2) {
				ok = true;
			}
			p = 0;
		}
	}
	if (!ok) {
		fatal("cali: invalid expression");
	}
	return (uint16)(cali[1] & 0xffff);
}

uint16 NACT_Sys1::cali2()
{
	uint16 val = 0;
	uint16 dat = getd();

	if(0x80 <= dat && dat <= 0xbf) {
		val = dat & 0x3f;
	} else if(0xc0 <= dat && dat <= 0xff) {
		val = ((dat & 0x3f) << 8) | getd();
	} else {
		fatal("cali2: invalid expression");
	}
	if(getd() != 0x7f) {
		fatal("cali2: invalid expression");
	}
	return val;
}
