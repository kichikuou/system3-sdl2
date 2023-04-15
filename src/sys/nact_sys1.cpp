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
#include "msgskip.h"
#include "crc32.h"
#include "../fileio.h"
#include "encoding.h"
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

NACT_Sys1::NACT_Sys1(uint32 crc32_a, uint32 crc32_b, const Config& config)
	: NACT(1, crc32_a, crc32_b, config)
{
	switch (crc32_a) {
	case CRC32_DPS:
	case CRC32_DPS_SG:
	//case CRC32_DPS_SG2:
	case CRC32_DPS_SG3:
		text_refresh = false;
		strcpy(tvar[0], strings.dps_custom.c_str());
		strcpy(tvar[1], strings.dps_linus.c_str());
		strcpy(tvar[2], strings.dps_katsumi.c_str());
		strcpy(tvar[3], strings.dps_yumiko.c_str());
		strcpy(tvar[4], strings.dps_itsumi.c_str());
		strcpy(tvar[5], strings.dps_hitomi.c_str());
		strcpy(tvar[6], strings.dps_mariko.c_str());
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
	case CRC32_VAMPIRE_ENG:
		mako->play_music(4);
		ags->load_cg(3, -1);
		WAIT(2000);
		ags->load_cg(38, -1);
		cmd_a();
		ags->draw_box(0);
		break;
	case CRC32_GAKUEN:
	case CRC32_GAKUEN_ENG:
		mako->set_cd_track(1, 1);
		ags->load_cg(302, -1);
		WAIT(3000);
		ags->load_cg(303, -1);
		WAIT(3000);
		mako->play_music(1);
		ags->load_cg(304, -1);
		WAIT(4000);
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
			} else if (cmd == '\'' || cmd == '"') {  // SysEng
				skip_string(cmd);
			} else if (is_message(cmd)) {
				ungetd();
				scenario_addr += encoding->mblen(scenario_data + scenario_addr);
			} else if (cmd >= 0x20 && cmd < 0x7f) {
				fatal("Unknown Command: '%c' at page = %d, addr = %d", cmd, scenario_page, prev_addr);
			} else {
				fatal("Unknown Command: %02x at page = %d, addr = %d", cmd, scenario_page, prev_addr);
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

		if (crc32_a == CRC32_GAKUEN || crc32_a == CRC32_GAKUEN_ENG)
			menu_window = 2;

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

	int selection = menu_select(menu_index);
	if (terminate)
		return;

	if (selection != -1) {
		scenario_addr = menu_addr[selection];
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
	
	if (crc32_a == CRC32_GAKUEN || crc32_a == CRC32_GAKUEN_ENG)
		menu_window = 1;
	int menu_max = ags->calculate_menu_max(menu_window);

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

	if(cnt <= menu_max) {
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
			if(index == menu_max - 1) {
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
		ags->draw_text(strings.next_page.c_str());
		id[index++] = -1;
		ags->menu_dest_y += ags->menu_font_size + 2;
	}
	ags->draw_menu = false;

	int selection = menu_select(index);
	if (terminate)
		return;

	if(selection == -1) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
	} else if(id[selection] == -1) {
		goto top;
	} else {
		cmd_open_obj(id[selection]);
	}
	menu_index = 0;
}

void NACT_Sys1::cmd_open_obj(int verb)
{
	int menu_max = ags->calculate_menu_max(menu_window);

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

	if(cnt <= menu_max - 1) {
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
		ags->draw_text(strings.back.c_str());
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
			if(index == menu_max - 2) {
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
		ags->draw_text(strings.back.c_str());
		id[index++] = 0;
		ags->menu_dest_y += ags->menu_font_size + 2;

		// 次のページを追加
		ags->menu_dest_x = 2;
		ags->menu_dest_y += 2;
		ags->draw_text(strings.next_page.c_str());
		id[index++] = -1;
		ags->menu_dest_y += ags->menu_font_size + 2;
	}
	ags->draw_menu = false;

	int selection = menu_select(index);
	if (terminate)
		return;

	if(selection == -1) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
	} else if(id[selection] == -1) {
		goto top;
	} else {
		scenario_addr = addr[id[selection]];
	}
}

void NACT_Sys1::cmd_a()
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

	if (crc32_a == CRC32_INTRUDER) {
		page = (page == 97) ? 96 : (page == 98) ? 97 : page;
	} else if (crc32_a == CRC32_GAKUEN || crc32_a == CRC32_GAKUEN_ENG) {
		page = (page == 3) ? 94 : page;
	}

	if (enable_graphics)
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
		snprintf(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + index - 1);

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
		snprintf(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + index - 1);

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

void NACT_Sys1::cmd_r()
{
	texthook_newline();

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
	} else if (crc32_a == CRC32_GAKUEN || crc32_a == CRC32_GAKUEN_ENG) {
		switch (cmd) {
			case 1:
				RND = (param == 0 || param == 1) ? 0 : random(param);
				break;
			case 2:
				// Clear variables up to param, though in practice param is always 21.
				for (int i = 0; i <= param; i++) {
					var[i] = 0;
				}
				break;
			case 3:
				{
					char buf[16];
					snprintf(buf, sizeof(buf), "%d", param);
					ags->draw_text(buf);
				}
				break;
			case 4:
				if (param == 0) {
					mako->play_music(1);
				} else {
					mako->stop_music();
				}
				break;
			case 5:
				enable_graphics = (param == 0);
				break;
			case 6:
				// Dummied. 0 - "Display text slowly," 1 - disable.
				break;
			case 7:
				// Dummied. 0 - "Switch to interlace mode," 1 - disable.
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
			case 240:
				ags->draw_hankaku = (param == 1) ? true : false;
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

	case CRC32_GAKUEN:
	case CRC32_GAKUEN_ENG:
		if (param != 30 && (param < 34 || param > 46)) {
			if (enable_graphics) {
				ags->load_cg(param + 250, -1);
			}
		}

		// Display the Gakuen Senki party status bar for party members. The Z commands make the slots appear in reverse, so 39 and 45 draw
		// the box for character 1, 38 and 44 draw for character 2, and so on. Z 1, 30: draws the first character slot, which is always occupied.
		//
		// One the actual MSX, this bar was constructed in an irregular fashion. This code constructs a more consistent version, where each square
		// is 35px wide with 2px borders on each side. Since the borders are overlapped, this means each sub-sprite is placed 37px apart.
		else if (param == 46) {
			// Originally this would have loaded Image 296, the party status bar, to screen 1 for future reference. However, due to trouble with
			// the palette, I decided to load it before using each subcommand, instead.
			/*ags->dest_screen = 1;
			ags->load_cg(296, -1);
			ags->dest_screen = 0;*/
		}
		else if (param == 30) {
			// Load party status bar to screen 1.
			ags->dest_screen = 1;
			ags->load_cg(296, -1);
			ags->dest_screen = 0;

			// Draw Wataru's icon.
			ags->src_screen = 1;
			ags->copy(10, 182, 48, 224, 7, 210);
			ags->src_screen = 0;
		}
		else if (param >= 34 && param <= 39) {
			int x = (39 - param) * 37 + 45;

			// Load party status bar to screen 1.
			ags->dest_screen = 1;
			ags->load_cg(296, -1);
			ags->dest_screen = 0;

			// Draw a blank party icon to init or clear the party box.
			ags->src_screen = 1;
			ags->copy(269, 182, 307, 224, x, 210);
			ags->src_screen = 0;
		}
		else {
			int x = (45 - param) * 37 + 45;

			// Load party status bar to screen 1.
			ags->dest_screen = 1;
			ags->load_cg(296, -1);
			ags->dest_screen = 0;

			// Draw a party member's icon to the party bar.
			ags->src_screen = 1;
			ags->copy(x + 2, 182, x + 38, 224, x, 210);
			ags->src_screen = 0;
		}
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
