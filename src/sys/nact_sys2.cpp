/*
	ALICE SOFT SYSTEM2 for Win32

	Super D.P.S. - Dream Program System

	[ NACT - command ]
*/

#include "nact.h"
#include "ags.h"
#include "mako.h"
#include "crc32.h"
#include "../fileio.h"

#define WAIT(tm) \
{ \
	Uint32 dwTime = SDL_GetTicks() + (tm); \
	for(;;) { \
		if(params.terminate) { \
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
		if(params.terminate) { \
			return; \
		} \
		if(get_key()) { \
			for(;;) { \
				if(params.terminate) { \
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

extern _TCHAR g_root[_MAX_PATH];
extern HWND g_hwnd;

void NACT::cmd_calc()
{
	int index = getd();
	if(0x80 <= index && index <= 0xbf) {
		index &= 0x3f;
	} else {
		index = ((index & 0x3f) << 8) | getd();
	}
	var[index] = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\n!var[%d]:%d!", index, var[index]);
	output_console(log);
#endif
}

void NACT::cmd_branch()
{
	int condition = cali();
	int nest = 0;
	bool set_menu = false;

	if(!condition) {
		for(;;) {
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
				getd();
				getd();
			} else if(cmd == '\\') {
				getd();
				getd();
			} else if(cmd == '&') {
				cali();
			} else if(cmd == '%') {
				cali();
			} else if(cmd == '$') {
				if(!set_menu) {
					getd();
					getd();
					set_menu = true;
				} else {
					set_menu = false;
				}
			} else if(cmd == '[') {
				getd();
				getd();
				getd();
				getd();
			} else if(cmd == ':') {
				cali();
				getd();
				getd();
				getd();
				getd();
			} else if(cmd == ']') {
				
			} else if(cmd == 'A') {
				
			} else if(cmd == 'B') {
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
			} else if(cmd == 'D') {
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
			} else if(cmd == 'E') {
				cali();
				cali();
				cali();
			} else if(cmd == 'F') {
				
			} else if(cmd == 'G') {
				cali();
			} else if(cmd == 'H') {
				getd();
				cali();
			} else if(cmd == 'I') {
				cali();
				cali();
				getd();
			} else if(cmd == 'J') {
				cali();
				cali();
			} else if(cmd == 'K') {
				
			} else if(cmd == 'L') {
				getd();
			} else if(cmd == 'M') {
				for(;;) {
					uint8 val = getd();
					if(val == 0x20 || (0xa1 <= val && val <= 0xdd)) {
						// message (1 byte)
					} else if((0x81 <= val && val <= 0x9f) || 0xe0 <= val) {
						// message (2 bytes)
						getd();
					} else if(val == ':') {
						break;
					}
				}
			} else if(cmd == 'N') {
				cali();
				cali();
			} else if(cmd == 'O') {
				cali();
				cali();
				cali();
			} else if(cmd == 'P') {
				getd();
			} else if(cmd == 'Q') {
				getd();
			} else if(cmd == 'R') {
				
			} else if(cmd == 'S') {
				getd();
			} else if(cmd == 'T') {
				cali();
				cali();
				cali();
			} else if(cmd == 'U') {
				getd();
				getd();
			} else if(cmd == 'V') {
				cali();
				cali();
			} else if(cmd == 'W') {
				cali();
				cali();
				cali();
				cali();
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
				fatal_error = true;
				break;
			}
		}
	}

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\n{%d:", condition);
	output_console(log);
#endif
}

void NACT::cmd_label_jump()
{
	int next_addr = getw();
	scenario_addr = next_addr;

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\n@%x:", next_addr);
	output_console(log);
#endif
}

void NACT::cmd_label_call()
{
	int next_addr = getw();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\n\\%x:", next_addr);
	output_console(log);
#endif

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

void NACT::cmd_page_jump()
{
	int next_page = cali();
	load_scenario(next_page);
	scenario_page = next_page;
	scenario_addr = 2;

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\n&%d:", next_page);
	output_console(log);
#endif
}

void NACT::cmd_page_call()
{
	int next_page = cali(), next_addr;

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\n%%%d:", next_page);
	output_console(log);
#endif

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

void NACT::cmd_set_menu()
{
	if(ags->draw_menu) {
		ags->menu_dest_x = 2;
		ags->menu_dest_y += ags->menu_font_size + 2;
		ags->draw_menu = false;

#if defined(_DEBUG_CONSOLE)
		char log[128];
		sprintf_s(log, 128, "$");
		output_console(log);
#endif
	} else {
		if(!menu_index) {
			ags->clear_menu_window();
			ags->menu_dest_y = 0;
		}
		menu_addr[menu_index++] = getw();
		ags->menu_dest_x = 2;
		ags->menu_dest_y += 2;
		ags->draw_menu = true;

#if defined(_DEBUG_CONSOLE)
		char log[128];
		sprintf_s(log, 128, "\n$%x,", menu_addr[menu_index - 1]);
		output_console(log);
#endif
	}
}

void NACT::cmd_open_menu()
{
#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\n]");
	output_console(log);
#endif

	if(!menu_index) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
		return;
	}

	// クリック中の間は待機
	for(;;) {
		if(params.terminate) {
			return;
		}
//		if(get_key() != 32) {
		if(!get_key()) {
			break;
		}
		SDL_Delay(10);
	}

	// メニュー表示
	ags->open_menu_window(menu_window);
	int current_index = 0;

	// マウス移動
	int sx = ags->menu_w[menu_window - 1].sx;
	int sy = ags->menu_w[menu_window - 1].sy;
	int ex = ags->menu_w[menu_window - 1].ex;
	int mx = ex - 16;
	int my = sy + 10;
	int height = ags->menu_font_size + 4;
	set_cursor(mx, my);

	// メニュー選択
	for(;;) {
		// 入力待機
		int val = 0, current_mx = mx, current_my = my;
		for(;;) {
			if(params.terminate) {
				return;
			}
			if(val = get_key()) {
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
				if(params.terminate) {
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
		} else if(val == 16) {
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

void NACT::cmd_set_verbobj()
{
	int verb = getd();
	int obj = getd();
	int addr = getw();

	menu_addr[menu_index] = addr;
	menu_verb[menu_index] = verb;
	menu_obj[menu_index++] = obj;
	verb_obj = true;

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\n[%x,%s,%s:", addr, caption_verb[verb], caption_obj[obj]);
	output_console(log);
#endif
}

void NACT::cmd_set_verbobj2()
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

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\n:%d,%x,%s,%s:", condition, addr, caption_verb[verb], caption_obj[obj]);
	output_console(log);
#endif
}

void NACT::cmd_open_verb()
{
	// 動詞メニューの表示
#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nopen verb-obj menu");
	output_console(log);
#endif
	verb_obj = false;

	// 表示する動詞のチェック
	int chk[MAX_VERB], page = 0;
	
	memset(chk, 0, sizeof(chk));
	for(int i = 0; i < menu_index; i++) {
		chk[menu_verb[i]] = 1;
	}

	// メニュー項目の準備
	int id[32], index = 0;

	ags->clear_menu_window();
	ags->menu_dest_y = 0;
	ags->draw_menu = true;

	for(int i = 0; i < MAX_VERB; i++) {
		if(chk[i]) {
			ags->menu_dest_x = 2;
			ags->menu_dest_y += 2;
			ags->draw_text(caption_verb[i]);
			id[index++] = i;
			ags->menu_dest_y += ags->menu_font_size + 2;
		}
	}
	ags->draw_menu = false;

	// クリック中の間は待機
	for(;;) {
		if(params.terminate) {
			return;
		}
//		if(get_key() != 32) {
		if(!get_key()) {
			break;
		}
		SDL_Delay(10);
	}

	// メニュー表示
	ags->open_menu_window(menu_window);
	int current_index = 0;

	// マウス移動
	int sx = ags->menu_w[menu_window - 1].sx;
	int sy = ags->menu_w[menu_window - 1].sy;
	int ex = ags->menu_w[menu_window - 1].ex;
	int mx = ex - 16;
	int my = sy + 10;
	int height = ags->menu_font_size + 4;
	set_cursor(mx, my);

	// メニュー選択
	for(;;) {
		// 入力待機
		int val = 0, current_mx = mx, current_my = my;
		for(;;) {
			if(params.terminate) {
				return;
			}
			if(val = get_key()) {
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
				if(params.terminate) {
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
		} else if(val == 16) {
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
	} else {
		cmd_open_obj(id[current_index]);
	}
	menu_index = 0;
}

void NACT::cmd_open_obj(int verb)
{
	// 目的語メニューの表示
	verb_obj = false;

	// 表示する目的語のチェック
	int chk[MAX_OBJ], addr[MAX_OBJ], page = 0;
	
	memset(chk, 0, sizeof(chk));
	for(int i = 0; i < menu_index; i++) {
		if(menu_verb[i] == verb) {
			chk[menu_obj[i]] = 1;
			addr[menu_obj[i]] = menu_addr[i];
		}
	}
	// 目的語がない場合
	if(chk[0]) {
		scenario_addr = addr[0];
		return;
	}
	// 以後、obj=0は戻るとして扱う
	chk[0] = 0;
	addr[0] = scenario_data[0] | (scenario_data[1] << 8);

	// メニュー項目の準備
	int id[32], index = 0;

	ags->clear_menu_window();
	ags->menu_dest_y = 0;
	ags->draw_menu = true;

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
	ags->draw_text("戻る");
	id[index++] = 0;
	ags->menu_dest_y += ags->menu_font_size + 2;
	ags->draw_menu = false;

	// クリック中の間は待機
	for(;;) {
		if(params.terminate) {
			return;
		}
//		if(get_key() != 32) {
		if(!get_key()) {
			break;
		}
		SDL_Delay(10);
	}

	// メニュー表示
	ags->open_menu_window(menu_window);
	int current_index = 0;

	// マウス移動
	int sx = ags->menu_w[menu_window - 1].sx;
	int sy = ags->menu_w[menu_window - 1].sy;
	int ex = ags->menu_w[menu_window - 1].ex;
	int mx = ex - 16;
	int my = sy + 10;
	int height = ags->menu_font_size + 4;
	set_cursor(mx, my);

	// メニュー選択
	for(;;) {
		// 入力待機
		int val = 0, current_mx = mx, current_my = my;
		for(;;) {
			if(params.terminate) {
				return;
			}
			if(val = get_key()) {
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
				if(params.terminate) {
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
		} else if(val == 16) {
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
	} else {
		scenario_addr = addr[id[current_index]];
	}
}

void NACT::cmd_a()
{
#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "A\n");
	output_console(log);
#endif

	if(!text_skip_enb) {
		// Pushマークの表示
		if(show_push) {
			ags->draw_push(text_window);
		}

		// キーが押されて離されるまで待機
		for(;;) {
			if(params.terminate) {
				return;
			}
			if(get_key()) {
				break;
			}
			SDL_Delay(10);
		}
		SDL_Delay(100);
		for(;;) {
			if(params.terminate) {
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
}

void NACT::cmd_b()
{
	int cmd = cali();
	int index = cali();
	int p1 = cali();
	int p2 = cali();
	int p3 = cali();
	int p4 = cali();
	int p5 = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nB %d,%d,%d,%d,%d,%d,%d:", cmd, index, p1, p2, p3, p4, p5);
	output_console(log);
#endif

	if(cmd == 1) {
		ags->menu_w[index - 1].sx = p1 * 8;
		ags->menu_w[index - 1].sy = p2;
		ags->menu_w[index - 1].ex = p3 * 8 - 1;
		ags->menu_w[index - 1].ey = p4;
		ags->menu_w[index - 1].push = p5 ? true : false;

		// 退避画面の破棄
		if(ags->menu_w[index - 1].screen) {
			free(ags->menu_w[index - 1].screen);
			ags->menu_w[index - 1].screen = NULL;
		}
	} else if(cmd == 2) {
//		if(crc32 == CRC32_PROSTUDENTG_FD) {
//			ags->menu_w[index - 1].frame = (index == 1 || index == 3) ? true : false;
//		} else
		ags->menu_w[index - 1].frame = p1 ? true : false;
		menu_window = index;
	} else if(cmd == 3) {
		ags->text_w[index - 1].sx = p1 * 8;
		ags->text_w[index - 1].sy = p2;
		ags->text_w[index - 1].ex = p3 * 8 - 1;
		ags->text_w[index - 1].ey = p4;
		ags->text_w[index - 1].push = p5 ? true : false;

		// 退避画面の破棄
		if(ags->text_w[index - 1].screen) {
			free(ags->text_w[index - 1].screen);
			ags->text_w[index - 1].screen = NULL;
		}
		// 退避窓の破棄
		if(ags->text_w[index - 1].window) {
			free(ags->text_w[index - 1].window);
			ags->text_w[index - 1].window = NULL;
		}
	} else if(cmd == 4) {
		if(p5 == 0) {
			// ウィンドウ退避
//			if(crc32 == CRC32_PROSTUDENTG_FD) {
//				ags->text_w[index - 1].frame = (index == 1 || index == 3) ? true : false;
//			} else
			ags->text_w[index - 1].frame = p1 ? true : false;
			ags->open_text_window(index, p4 ? false : true);
			text_window = index;
		} else {
			// ウィンドウ復帰
			ags->close_text_window(index, text_window == index ? true : false);
		}
	}
}

void NACT::cmd_d()
{
	int p1 = cali();
	int p2 = cali();
	int p3 = cali();
	int p4 = cali();
	int p5 = cali();
	int p6 = cali();
	int p7 = cali();
	int p8 = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nD %d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8);
	output_console(log);
#endif
}

void NACT::cmd_e()
{
	int p1 = cali();
	int p2 = cali();
	int p3 = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nE %d,%d,%d:", p1, p2, p3);
	output_console(log);
#endif
}

void NACT::cmd_f()
{
#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nF");
	output_console(log);
#endif

	scenario_addr = 2;
}

void NACT::cmd_g()
{
	int page = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nG %d:", page);
	output_console(log);
#endif

	if(crc32 == CRC32_SDPS_TONO || crc32 == CRC32_SDPS_KAIZOKU) {
		if(20 <= page && page <= 100) {
			page--;
		}
	}
	ags->load_cg(page, -1);
}

void NACT::cmd_h()
{
	int length = getd();
	int val = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nH %d,%d:", length, val);
	output_console(log);
#endif

	// 8桁の文字列を生成
	char tmp[8];
	tmp[0] = '0';
	tmp[1] = '0';
	tmp[2] = '0';
	for(int i = 3, ref = 10000; i < 8; i++) {
		tmp[i] = '0' + (int)(val / ref);
		val %= ref;
		ref /= 10;
	}
	for(int i = 0; i < 7; i++) {
		if(tmp[i] != '0')
			break;
		tmp[i] = ' ';
	}

	// 指定した桁数だけ出力
	char string[9];
	int p = 0;
	if(length == 0) {
		for(int i = 0; i < 8; i++) {
			if(tmp[i] != ' ') {
				string[p++] = tmp[i];
			}
		}
	} else {
		for(int i = 8 - length; i < 8; i++) {
			string[p++] = tmp[i];
		}
	}
	string[p++] = '\0';
	ags->draw_text(string);
}

void NACT::cmd_i()
{
	int p1 = cali();
	int p2 = cali();
	int p3 = getd();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nI %d,%d,%d:", p1, p2, p3);
	output_console(log);
#endif
}

void NACT::cmd_j()
{
	int p1 = cali();
	int p2 = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nJ %d,%d:", p1, p2);
	output_console(log);
#endif
}

void NACT::cmd_k()
{
	// 未使用
	cmd_r();
}

void NACT::cmd_l()
{
	int index = getd();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nL %d:", index);
	output_console(log);
#endif

	if(1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		_TCHAR path[_MAX_PATH], file_path[_MAX_PATH];
		_tcscpy_s(path, _MAX_PATH, _T("ASLEEP_A.DAT"));
		path[7] = _T('A') + index - 1;
		_stprintf_s(file_path, _MAX_PATH, _T("%s%s"), g_root, path);

		FILEIO* fio = new FILEIO();
		if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
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

//			ags->old_text_font_size = 0;
//			ags->old__menu_font_fize = 0;
			mako->play_music(next_music);
		}
		delete fio;
	}
}

void NACT::cmd_m()
{
	char string[22];
	int d, p = 0;

	while((d = getd()) != ':') {
		if((0x81 <= d && d <= 0x9f) || 0xe0 <= d) {
			// 2bytes
			string[p++] = d;
			string[p++] = getd();
		} else {
			string[p++] = d;
		}
	}
	string[p] = '\0';

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nM %s:", string);
	output_console(log);
#endif

	if(1 <= tvar_index && tvar_index <= 10) {
		memcpy(tvar[tvar_index - 1], string, 22);
	}
}

void NACT::cmd_n()
{
	int p1 = cali();
	int p2 = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nN %d,%d:", p1, p2);
	output_console(log);
#endif
}

void NACT::cmd_o()
{
	int st = cali();
	int width = cali();
	int height = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nO %d,%d,%d:", st, width, height);
	output_console(log);
#endif

	// white mesh
	int sx = (st % 80) * 8;
	int sy = (int)(st / 80);
	ags->draw_mesh(sx, sy, width, height);
}

void NACT::cmd_p()
{
	int param = getd();

	ags->text_font_color = (uint8)((param & 0x7) + 16);

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nP %d:", param);
	output_console(log);
#endif
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

void NACT::cmd_q()
{
	static char header[112] = {
		'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'a', 'v', 'e', ' ', 'd', 'a', 't',
		'a', ' ', 'f', 'o', 'r', ' ', 'S', 'y', 's', 't', 'e', 'm', '2', ' ', ' ', 'U',
		'n', 'i', 'o', 'n', ' ', '[', 'W', 'i', 'n', '9', 'x', '/', 'N', 'T', '4', '0',
		'/', '2', '0', '0', '0', '/', 'X', 'P', '/', 'C', 'E', ']', ' ', 'F', 'o', 'r',
		' ', 'N', 'A', 'C', 'T', '/', 'A', 'D', 'V', ' ', 's', 'y', 's', 't', 'e', 'm',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '(', 'C', ')', '2', '0', '0',
		'4', ' ', 'A', 'L', 'I', 'C', 'E', '-', 'S', 'O', 'F', 'T', ' ', ' ', ' ', ' '
	};

	int index = getd();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nQ %d:", index);
	output_console(log);
#endif

	if(1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		_TCHAR path[_MAX_PATH], file_path[_MAX_PATH];
		_tcscpy_s(path, _MAX_PATH, _T("ASLEEP_A.DAT"));
		path[7] = _T('A') + index - 1;
		_stprintf_s(file_path, _MAX_PATH, _T("%s%s"), g_root, path);

		FILEIO* fio = new FILEIO();
		if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
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

void NACT::cmd_r()
{
#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "R\n");
	output_console(log);
#endif

	// ウィンドウの表示範囲外の場合は改ページ
	if(ags->return_text_line(text_window)) {
		cmd_a();
	}
}

void NACT::cmd_s()
{
	int page = getd();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nS %d:", page);
	output_console(log);
#endif

	if(page) {
		mako->play_music(page);
	} else {
		mako->stop_music();
	}
}

void NACT::cmd_t()
{
	int p1 = cali();
	int p2 = cali();
	int p3 = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nT %d,%d,%d:", p1, p2, p3);
	output_console(log);
#endif
}

void NACT::cmd_u()
{
	int page = getd();
	int transparent = getd();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nU %d,%d:", page, transparent);
	output_console(log);
#endif

#if defined(_INTRUDER)
	ags->load_cg(page, 11);

	if(page == 5) {
		WAIT(400)
	}
#else
	ags->load_cg(page, transparent);
#endif
}

void NACT::cmd_v()
{
	int p1 = cali();
	int p2 = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nV %d,%d:", p1, p2);
	output_console(log);
#endif
}

void NACT::cmd_w()
{
	int p1 = cali();
	int p2 = cali();
	int p3 = cali();
	int p4 = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nW %d,%d,%d,%d:", p1, p2, p3, p4);
	output_console(log);
#endif
}

void NACT::cmd_x()
{
	int index = getd();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nX %d:", index);
	output_console(log);
#endif

	if(1 <= index && index <= 10) {
		ags->draw_text(tvar[index - 1]);
	}
}

void NACT::cmd_y()
{
	int cmd = cali();
	int param = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nY %d,%d:", cmd, param);
	output_console(log);
#endif

	switch(cmd) {
		case 1:
			ags->clear_text_window(text_window, (param == 0) ? true : false);
			break;
		case 2:
			if(param == 0) {
				for(int i = 1; i <= 20; i++) {
					var[i] = 0;
				}
			} else if(param == 1) {
				for(int i = 21; i <= 40; i++) {
					var[i] = 0;
				}
			} else if(param == 2) {
				for(int i = 41; i <= 56; i++) {
					var[i] = 0;
				}
			}
			break;
		case 3:
			{
				Uint32 dwTime = SDL_GetTicks() + param * 1000 / 60;
				for(;;) {
					if(params.terminate) {
						return;
					}
//					RND = get_key();
//					if(RND && wait_keydown) {
//						break;
//					}
					if(dwTime <= SDL_GetTicks()) {
						break;
					}
					SDL_Delay(10);
				}
			}
			break;
		case 4:
			RND = (param == 0 || param == 1) ? 0 : random(param);
			break;
		case 7:
			if(crc32 != CRC32_SDPS_MARIA) {
				ags->draw_box(param);
			}
			break;
		case 8:
			tvar_index = param;
			break;
		case 14:
			RND = 1;
			break;
		case 25:
			ags->menu_font_size = (param == 1) ? 16 : (param == 2) ? 24 : (param == 3) ? 32 : (param == 4) ? 48 : (param == 5) ? 64 : 16;
			break;
		case 26:
			ags->text_font_size = (param == 1) ? 16 : (param == 2) ? 24 : (param == 3) ? 32 : (param == 4) ? 48 : (param == 5) ? 64 : 16;
			break;
		case 40:
		case 42:
			{
				Uint32 dwStart = SDL_GetTicks();
				for(int i = 0; i < 16; i++) {
					ags->fade_in(i);
					DWORD dwTime = dwStart + param * 1000 / 60 * i;
					for(;;) {
						if(params.terminate) {
							return;
						}
						if(dwTime <= SDL_GetTicks()) {
							break;
						}
						SDL_Delay(0);
					}
				}
			}
			break;
		case 41:
		case 43:
			{
				Uint32 dwStart = SDL_GetTicks();
				for(int i = 0; i < 16; i++) {
					ags->fade_out(i, (cmd == 41) ? false : true);
					DWORD dwTime = dwStart + param * 1000 / 60 * i;
					for(;;) {
						if(params.terminate) {
							return;
						}
						if(dwTime <= SDL_GetTicks()) {
							break;
						}
						SDL_Delay(0);
					}
				}
			}
			break;
		case 221:
		case 222:
		case 223:
		case 224:
		case 225:
		case 226:
		case 227:
		case 228:
		case 229:
			if(1 <= param && param <= 10) {
				char string[22];
				int len = cmd - 220, p = 0, q = 0;
				uint8 d;
				while((d = (uint8)tvar[param - 1][p]) != '\0') {
					if((0x81 <= d && d <= 0x9f) || 0xe0 <= d) {
						string[p++] = tvar[param - 1][p];
					}
					string[p++] = tvar[param - 1][p];
					q++;
				}
				for(int i = q; i < len; i++) {
					string[p++] = 0x20;
				}
				string[p] = '\0';
				ags->draw_text(string);
			}
			break;
		case 252:
			RND = 8;
			break;
		case 254:
			RND = 0;
			break;
		case 255:
			post_quit = (param == 1) ? true : false;
			fatal_error = true;
			break;
	}
// Y1
// Y2
// Y3
// Y7
// Y8
// Y11
// Y23
// Y25
// Y26
// Y27
// Y40
// Y50
// Y252
// Y254
// Y255
}

void NACT::cmd_z()
{
	int cmd = cali();
	int param = cali();

#if defined(_DEBUG_CONSOLE)
	char log[128];
	sprintf_s(log, 128, "\nZ %d,%d:", cmd, param);
	output_console(log);
#endif

// Z1
// Z2
// Z3
// Z4
// Z5
}


// 下位関数

uint16 NACT::cali()
{
	uint32 cali[256];
	int p = 1;
	fatal_error = true;

	while(p > 0) {
		uint8 dat = getd();

		if(0x80 <= dat && dat <= 0xbf) {
			cali[p++] = var[dat & 0x3f];
		} else if(0xc0 <= dat && dat <= 0xff) {
			cali[p++] = var[((dat & 0x3f) << 8) | getd()];
		} else if(0x00 <= dat && dat <= 0x3f) {
			cali[p++] = ((dat & 0x3f) << 8) | getd();
		} else if(0x40 <= dat && dat <= 0x76) {
			cali[p++] = dat & 0x3f;
		} else if(dat == 0x77) {
			cali[p - 2] = cali[p - 2] * cali[p - 1];
			if (cali[p - 2] > 65535) {
				cali[p - 2] = 65535;
			}
			p--;
		} else if(dat == 0x78) {
			if(cali[p - 1]) {
				cali[p - 2] = cali[p - 2] / cali[p - 1];
			} else {
				cali[p - 2] = 0;
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
		}else if(dat == 0x7d) {
			cali[p - 2] = (cali[p - 2] > cali[p - 1]) ? 1 : 0;
			p--;
		} else if(dat == 0x7e) {
			cali[p - 2] = (cali[p - 2] != cali[p - 1]) ? 1 : 0;
			p--;
		} else if(dat == 0x7f) {
			if(p == 2) {
				fatal_error = false;
			}
			p = 0;
		}
	}
	return (uint16)(cali[1] & 0xffff);
}

uint16 NACT::cali2()
{
	uint16 val = 0;
	uint16 dat = getd();

	if(0x80 <= dat && dat <= 0xbf) {
		val = dat & 0x3f;
	} else if(0xc0 <= dat && dat <= 0xff) {
		val = ((dat & 0x3f) << 8) | getd();
	} else {
		fatal_error = true;
	}
	if(getd() != 0x7f) {
		fatal_error = true;
	}
	return val;
}

uint8 NACT::getd()
{
	return scenario_data[scenario_addr++];
}

uint16 NACT::getw()
{
	uint16 val = scenario_data[scenario_addr] | (scenario_data[scenario_addr + 1] << 8);
	scenario_addr += 2;
	return val;
}

