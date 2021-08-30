/*
	ALICE SOFT SYSTEM2 for Win32

	Super D.P.S. - Dream Program System

	[ NACT - command ]
*/

#include "nact.h"
#include "ags.h"
#include "mako.h"
#include "msgskip.h"
#include "crc32.h"
#include "../fileio.h"
#include "texthook.h"

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
		sys_sleep(16); \
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
				sys_sleep(16); \
			} \
			break; \
		} \
		if(dwTime <= SDL_GetTicks()) { \
			break; \
		} \
		sys_sleep(16); \
	} \
}

NACT_Sys2::NACT_Sys2(uint32 crc32_a, uint32 crc32_b, const Config& config)
	: NACT(2, crc32_a, crc32_b, config)
{
}

void NACT_Sys2::cmd_calc()
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

void NACT_Sys2::cmd_branch()
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
//				getd();
				cali();
			} else if(cmd == 'J') {
				cali();
				cali();
			} else if(cmd == 'K') {
				
			} else if(cmd == 'L') {
				getd();
			} else if(cmd == 'M') {
				uint8 val = getd();
				if (val == '\'' || val == '"') {  // SysEng
					skip_string(val);
				} else {
					while (val != ':') {
						if(is_2byte_message(val))
							getd();
						val = getd();
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
				if(crc32_a == CRC32_YAKATA2) {
					cali();
					cali();
				} else {
					getd();
					getd();
				}
			} else if(cmd == 'V') {
#if 1
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
				cali();
#else
				cali();
				cali();
#endif
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
			} else if(is_1byte_message(cmd)) {
				// message (1 byte)
			} else if(is_2byte_message(cmd)) {
				// message (2 bytes)
				getd();
			} else if (cmd == '\'' || cmd == '"') {  // SysEng
				skip_string(cmd);
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

void NACT_Sys2::cmd_label_jump()
{
	int next_addr = getw();
	scenario_addr = next_addr;

	output_console("\n@%x:", next_addr);
}

void NACT_Sys2::cmd_label_call()
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

void NACT_Sys2::cmd_page_jump()
{
	int next_page = cali();
	load_scenario(next_page);
	scenario_page = next_page;
	scenario_addr = 2;

	output_console("\n&%d:", next_page);
}

void NACT_Sys2::cmd_page_call()
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

void NACT_Sys2::cmd_set_menu()
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

void NACT_Sys2::cmd_open_menu()
{
	output_console("\n]");

	if(!menu_index) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
		return;
	}

	int selection = menu_select(menu_index);
	if (terminate)
		return;

	if (selection != -1) {
		scenario_addr = menu_addr[selection];
	}
	menu_index = 0;
}

void NACT_Sys2::cmd_set_verbobj()
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

void NACT_Sys2::cmd_set_verbobj2()
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

void NACT_Sys2::cmd_open_verb()
{
	// 動詞メニューの表示
	output_console("\nopen verb-obj menu");
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

	int selection = menu_select(index);
	if (terminate)
		return;

	if(selection == -1) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
	} else {
		cmd_open_obj(id[selection]);
	}
	menu_index = 0;
}

void NACT_Sys2::cmd_open_obj(int verb)
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
	ags->draw_text(strings::back[lang]);
	id[index++] = 0;
	ags->menu_dest_y += ags->menu_font_size + 2;
	ags->draw_menu = false;

	int selection = menu_select(index);
	if (terminate)
		return;

	if(selection == -1) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
	} else {
		scenario_addr = addr[id[selection]];
	}
}

void NACT_Sys2::cmd_a()
{
	texthook_nextpage();

	output_console("A\n");

	if (msgskip->skipping()) {
		if (msgskip->get_flags() & MSGSKIP_STOP_ON_CLICK && get_key())
			msgskip->activate(false);
	} else if (show_push) {
		ags->draw_push(text_window);
	}

	// キーが押されて離されるまで待機
	while (!msgskip->skipping()) {
		if(terminate) {
			return;
		}
		if(get_key()) {
			break;
		}
		sys_sleep(16);
	}
	sys_sleep(30);
	while (!msgskip->skipping()) {
		if(terminate) {
			return;
		}
		if(!(get_key() & 0x18)) {
			break;
		}
		sys_sleep(16);
	}

	// ウィンドウ更新
	ags->clear_text_window(text_window, true);
}

void NACT_Sys2::cmd_b()
{
	int cmd = cali();
	int index = cali();
	int p1 = cali();
	int p2 = cali();
	int p3 = cali();
	int p4 = cali();
	int p5 = cali();

	output_console("\nB %d,%d,%d,%d,%d,%d,%d:", cmd, index, p1, p2, p3, p4, p5);

	if(cmd == 1) {
		if(crc32_a == CRC32_AYUMI_FD || crc32_a == CRC32_AYUMI_HINT || crc32_a == CRC32_DRSTOP) {
			p5 = 1;
		}
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
		if(crc32_a == CRC32_AYUMI_FD || crc32_a == CRC32_AYUMI_HINT || crc32_a == CRC32_DRSTOP) {
			p1 = 1;
		}
//		if(crc32_a == CRC32_PROSTUDENTG_FD) {
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
		if(crc32_a == CRC32_AYUMI_FD || crc32_a == CRC32_AYUMI_HINT) {
			p1 = 1;
			//p5 ? 0 : 1; // 逆？
			p5 = 0;
		} else if(crc32_a == CRC32_DRSTOP) {
			p1 = 1;
		}
		if(p5 == 0) {
			// ウィンドウ退避
//			if(crc32_a == CRC32_PROSTUDENTG_FD) {
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

void NACT_Sys2::cmd_d()
{
	int p1 = cali();
	int p2 = cali();
	int p3 = cali();
	int p4 = cali();
	int p5 = cali();
	int p6 = cali();
	int p7 = cali();
	int p8 = cali();

	output_console("\nD %d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8);
}

void NACT_Sys2::cmd_e()
{
	int p1 = cali();
	int p2 = cali();
	int p3 = cali();

	output_console("\nE %d,%d,%d:", p1, p2, p3);
}

void NACT_Sys2::cmd_f()
{
	output_console("\nF");

	scenario_addr = 2;
}

void NACT_Sys2::cmd_g()
{
	int page = cali();

	output_console("\nG %d:", page);

	if(crc32_a == CRC32_SDPS && (crc32_b == CRC32_SDPS_TONO || crc32_b == CRC32_SDPS_KAIZOKU)) {
		if(20 <= page && page <= 100) {
			page--;
		}
	}
	ags->load_cg(page, -1);

	if(crc32_a == CRC32_DALK_HINT) {
		if(page == 3) {
			WAIT(2000)
		}
	} else if(crc32_a == CRC32_RANCE3_HINT) {
		if(page == 25) {
			WAIT(2000)
		}
	}
}

void NACT_Sys2::cmd_h()
{
	int length = getd();
	int val = cali();

	output_console("\nH %d,%d:", length, val);

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

void NACT_Sys2::cmd_i()
{
	int p1 = cali();
	int p2 = cali();
//	int p3 = getd();
	int p3 = cali();

	output_console("\nI %d,%d,%d:", p1, p2, p3);
}

void NACT_Sys2::cmd_j()
{
	int p1 = cali();
	int p2 = cali();

	output_console("\nJ %d,%d:", p1, p2);
}

void NACT_Sys2::cmd_k()
{
	// 未使用
	cmd_r();
}

void NACT_Sys2::cmd_l()
{
	int index = getd();

	output_console("\nL %d:", index);

	if(1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		char file_name[_MAX_PATH];
		sprintf_s(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + index - 1);

		FILEIO* fio = new FILEIO();
		if(fio->Fopen(file_name, FILEIO_READ_BINARY | FILEIO_SAVEDATA)) {
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

void NACT_Sys2::cmd_m()
{
	char string[33];

	int d = getd();
	if (d == '\'' || d == '"') {  // SysEng
		get_string(string, sizeof(string), d);
	} else {
		int p = 0;
		while(d != ':') {
			if(is_2byte_message(d)) {
				string[p++] = d;
				string[p++] = getd();
			} else {
				string[p++] = d;
			}
			d = getd();
		}
		string[p] = '\0';
	}

	output_console("\nM %s:", string);

	if(1 <= tvar_index && tvar_index <= 10) {
		memcpy(tvar[tvar_index - 1], string, 33);
	}
}

void NACT_Sys2::cmd_n()
{
	int p1 = cali();
	int p2 = cali();

	output_console("\nN %d,%d:", p1, p2);
}

void NACT_Sys2::cmd_o()
{
	int st = cali();
	int width = cali();
	int height = cali();

	output_console("\nO %d,%d,%d:", st, width, height);

#if 0
	// white mesh
	int sx = (st % 80) * 8;
	int sy = (int)(st / 80);
	ags->draw_mesh(sx, sy, width, height);
#endif
}

void NACT_Sys2::cmd_p()
{
	int param = getd();

	if(crc32_a != CRC32_YAKATA2 && crc32_a != CRC32_DALK_HINT && crc32_a != CRC32_RANCE3_HINT) {
		ags->text_font_color = (uint8)((param & 0x7) + 16);
	}

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

void NACT_Sys2::cmd_q()
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

	output_console("\nQ %d:", index);

	if(1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		char file_name[_MAX_PATH];
		sprintf_s(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + index - 1);

		FILEIO* fio = new FILEIO();
		if(fio->Fopen(file_name, FILEIO_WRITE_BINARY | FILEIO_SAVEDATA)) {
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

void NACT_Sys2::cmd_r()
{
	texthook_newline();

	output_console("R\n");

	// ウィンドウの表示範囲外の場合は改ページ
	if(ags->return_text_line(text_window)) {
		cmd_a();
	}
}

void NACT_Sys2::cmd_s()
{
	int page = getd();

	output_console("\nS %d:", page);

	if(page) {
		mako->play_music(page);
	} else {
		mako->stop_music();
	}
}

void NACT_Sys2::cmd_t()
{
	int p1 = cali();
	int p2 = cali();
	int p3 = cali();

	output_console("\nT %d,%d,%d:", p1, p2, p3);
}

void NACT_Sys2::cmd_u()
{
	int page, transparent;

	if(crc32_a == CRC32_YAKATA2) {
		page = cali();
		transparent = cali();
	} else {
		page = getd();
		transparent = getd();
	}

	output_console("\nU %d,%d:", page, transparent);

	ags->load_cg(page, transparent);
}

void NACT_Sys2::cmd_v()
{
#if 1
	int p01 = cali();
	int p02 = cali();
	int p03 = cali();
	int p04 = cali();
	int p05 = cali();
	int p06 = cali();
	int p07 = cali();
	int p08 = cali();
	int p09 = cali();
	int p10 = cali();
	int p11 = cali();
	int p12 = cali();
	int p13 = cali();
	int p14 = cali();
	int p15 = cali();
	int p16 = cali();
	int p17 = cali();
	int p18 = cali();
	int p19 = cali();
	int p20 = cali();
	int p21 = cali();
	int p22 = cali();
	int p23 = cali();
	int p24 = cali();
	int p25 = cali();
	int p26 = cali();
	int p27 = cali();
	int p28 = cali();
	int p29 = cali();

	output_console("\nV %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d:",
	p01, p02, p03, p04, p05, p06, p07, p08, p09, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29);
#else
	int p1 = cali();
	int p2 = cali();

	output_console("\nV %d,%d:", p1, p2);
#endif
}

void NACT_Sys2::cmd_w()
{
	int sx = cali();
	int sy = cali();
	int ex = cali();
	int ey = cali();

	output_console("\nW %d,%d,%d,%d:", sx, sy, ex, ey);

	ags->draw_mesh(sx, sy, ex - sx, ey - sy);
}

void NACT_Sys2::cmd_x()
{
	int index = getd();

	output_console("\nX %d:", index);

	if(1 <= index && index <= 10) {
		ags->draw_text(tvar[index - 1]);
	}
}

void NACT_Sys2::cmd_y()
{
	int cmd = cali();
	int param = cali();

	output_console("\nY %d,%d:", cmd, param);

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
					if(terminate) {
						return;
					}
//					RND = get_key();
//					if(RND && wait_keydown) {
//						break;
//					}
					if(dwTime <= SDL_GetTicks()) {
						break;
					}
					sys_sleep(16);
				}
			}
			break;
		case 4:
			RND = (param == 0 || param == 1) ? 0 : random(param);
			break;
		case 7:
			if(!(crc32_a == CRC32_SDPS && crc32_b == CRC32_SDPS_MARIA)) {
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
		case 27:
			{
				int tmp = tvar_index;
				tvar_index = param;
				tvar_maxlen = 8;//param;
				text_dialog();
				tvar_index = tmp;
			}
			break;
		case 40:
		case 42:
			if(ags->now_fade()) {
				Uint32 dwStart = SDL_GetTicks();
				for(int i = 0; i < 16; i++) {
					ags->fade_in(i);
					int32 ms = dwStart + param * 1000 / 60 * i - SDL_GetTicks();
					if (ms > 0)
						sys_sleep(ms);
				}
				ags->fade_end();
			}
			break;
		case 41:
		case 43:
			if(!ags->now_fade()) {
				ags->fade_start();
				Uint32 dwStart = SDL_GetTicks();
				for(int i = 0; i < 16; i++) {
					ags->fade_out(i, (cmd == 41) ? false : true);
					int32 ms = dwStart + param * 1000 / 60 * i - SDL_GetTicks();
					if (ms > 0)
						sys_sleep(ms);
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
					if(is_2byte_message(d)) {
						string[p] = tvar[param - 1][p];
						p++;
					}
					string[p] = tvar[param - 1][p];
					p++;
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
		case 253:
			// キーが押されて離されるまで待機
			for(;;) {
				if(terminate) {
					return;
				}
				if(get_key()) {
					break;
				}
				sys_sleep(16);
			}
			sys_sleep(100);
			for(;;) {
				if(terminate) {
					return;
				}
				if(!(get_key() & 0x18)) {
					break;
				}
				sys_sleep(16);
			}
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

void NACT_Sys2::cmd_z()
{
	int cmd = cali();
	int param = cali();

	output_console("\nZ %d,%d:", cmd, param);

// Z1
// Z2
// Z3
// Z4
// Z5
}


// 下位関数

uint16 NACT_Sys2::cali()
{
	uint32 cali[256];
	int p = 1;
	bool ok = false;

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

uint16 NACT_Sys2::cali2()
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
