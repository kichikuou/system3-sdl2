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
#include "game_id.h"
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

NACT_Sys1::NACT_Sys1(const Config& config, const GameId& game_id)
	: NACT(config, game_id)
{
	switch (game_id.crc32_a) {
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
	switch (game_id.crc32_a) {
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

void NACT_Sys1::cmd_branch()
{
	int condition = cali();
	int nest = 0;
	bool set_menu = false;

	if(!condition) {
		// 次の'}'命令までスキップする（ネストも考慮する）
		for(;;) {
			uint8 cmd = sco.fetch_command();

			if(cmd == '!') {
				int index = sco.getd();
				if(!(0x80 <= index && index <= 0xbf)) {
					sco.getd();
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
				sco.getw();
			} else if(cmd == '\\') {
				sco.getw();
			} else if(cmd == '&') {
				cali();
			} else if(cmd == '%') {
				cali();
			} else if(cmd == '$') {
				if(!set_menu) {
					sco.getw();
					set_menu = true;
				} else {
					set_menu = false;
				}
			} else if(cmd == '[') {
				sco.getd();
				sco.getd();
				sco.getw();
			} else if(cmd == ':') {
				cali();
				sco.getd();
				sco.getd();
				sco.getw();
			} else if(cmd == ']') {
				
			} else if(cmd == 'A') {
				
			} else if(cmd == 'F') {
				
			} else if(cmd == 'G') {
				sco.getd();
			} else if(cmd == 'L') {
				sco.getd();
			} else if(cmd == 'P') {
				sco.getd();
			} else if(cmd == 'Q') {
				sco.getd();
			} else if(cmd == 'R') {
				
			} else if(cmd == 'S') {
				sco.getd();
			} else if(cmd == 'U') {
				sco.getd();
				sco.getd();
			} else if(cmd == 'X') {
				sco.getd();
			} else if(cmd == 'Y') {
				cali();
				cali();
			} else if(cmd == 'Z') {
				cali();
				cali();
			} else if (cmd == '\'' || cmd == '"') {  // SysEng
				sco.skip_syseng_string(encoding.get(), cmd);
			} else if (is_message(cmd)) {
				sco.skip(encoding->mblen(cmd) - 1);
			} else {
				sco.unknown_command(cmd);
			}
		}
	}

	output_console("\n{%d:", condition);
}

void NACT_Sys1::cmd_open_verb()
{
	// 動詞メニューの表示
	output_console("\nopen verb-obj menu");
	verb_obj = false;

	// 表示する動詞のチェック
	int chk[MAX_VERB], page = 0, cnt = 0;
	
	if (game_id.is_gakuen())
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
		sco.jump_to(sco.default_addr());
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
		sco.jump_to(addr[0]);
		return;
	}
	// 以後、obj=0は戻るとして扱う
	chk[0] = 0;
	addr[0] = sco.default_addr();

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
		sco.jump_to(sco.default_addr());
	} else if(id[selection] == -1) {
		goto top;
	} else {
		sco.jump_to(addr[id[selection]]);
	}
}

void NACT_Sys1::cmd_b()
{
	// Unused
	sco.unknown_command('B');
}

void NACT_Sys1::cmd_d()
{
	// Unused
	sco.unknown_command('D');
}

void NACT_Sys1::cmd_e()
{
	// Unused
	sco.unknown_command('E');
}

void NACT_Sys1::cmd_g()
{
	int page = sco.getd();

	output_console("\nG %d:", page);

	if (game_id.crc32_a == CRC32_INTRUDER) {
		page = (page == 97) ? 96 : (page == 98) ? 97 : page;
	} else if (game_id.is_gakuen()) {
		page = (page == 3) ? 94 : page;
	}

	if (enable_graphics)
		ags->load_cg(page, -1);

	if (game_id.crc32_a == CRC32_INTRUDER) {
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
	// Unused
	sco.unknown_command('H');
}

void NACT_Sys1::cmd_i()
{
	// Unused
	sco.unknown_command('I');
}

void NACT_Sys1::cmd_j()
{
	// Unused
	sco.unknown_command('J');
}

void NACT_Sys1::cmd_k()
{
	// Unused
	sco.unknown_command('K');
}

void NACT_Sys1::cmd_l()
{
	int index = sco.getd();

	output_console("\nL %d:", index);

	if (1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		load(index);
	}
}

void NACT_Sys1::cmd_m()
{
	// Unused
	sco.unknown_command('M');
}

void NACT_Sys1::cmd_n()
{
	// Unused
	sco.unknown_command('N');
}

void NACT_Sys1::cmd_o()
{
	// Unused
	sco.unknown_command('O');
}

void NACT_Sys1::cmd_p()
{
	int param = sco.getd();

	ags->text_font_color = (uint8)((param & 0x7) + 16);

	output_console("\nP %d:", param);
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

	int index = sco.getd();

	output_console("\nQ %d:", index);

	if (1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		save(index, header);
	}
}

void NACT_Sys1::cmd_t()
{
	// Unused
	sco.unknown_command('T');
}

void NACT_Sys1::cmd_u()
{
	int page = sco.getd();
	int transparent = sco.getd();

	output_console("\nU %d,%d:", page, transparent);

	if (game_id.crc32_a == CRC32_INTRUDER) {
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
	// Unused
	sco.unknown_command('V');
}

void NACT_Sys1::cmd_w()
{
	// Unused
	sco.unknown_command('W');
}

void NACT_Sys1::cmd_y()
{
	int cmd = cali();
	int param = cali();

	output_console("\nY %d,%d:", cmd, param);

	if (game_id.crc32_a == CRC32_INTRUDER) {
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
					quit(NACT_HALT);
				} else {
					WAIT(100)
				}
				break;
			case 3:
				WAIT(param * 1000)
				break;
			case 255:
				quit(NACT_HALT);
				break;
		}
	} else if (game_id.is_gakuen()) {
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
				if (game_id.crc32_a == CRC32_CRESCENT) {
					for(int i = 39; i <= 48; i++) {
						var[i] = 0;
					}
				} else if (game_id.is_system1_dps()) {
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
				if (game_id.is_system1_dps()) {
					if(param == 1) {
						ags->box_fill(0, 40, 8, 598, 270, 0);
					}
				}
				break;
			case 130:
				if (game_id.crc32_a == CRC32_TENGU) {
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
				quit(0);
				break;
			case 254:
				RND = 0;
				break;
			case 255:
				if (game_id.is_system1_dps()) {
					quit(0);
				} else {
					quit(NACT_HALT);
				}
				break;
		}
	}
}

void NACT_Sys1::cmd_z()
{
	if (game_id.crc32_a == CRC32_CRESCENT) {
		cmd_y();
		return;
	}
	int cmd = cali();
	int param = cali();

	output_console("\nZ %d,%d:", cmd, param);

	switch (game_id.crc32_a) {
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
			if (param == 0) {
				// Clear the screen
				ags->box_fill(0, 0, 0, 640, 400, 0);
			} else {
				// Restore the screen
				ags->load_cg(74, 8);
				ags->load_cg(81, -1);
				map_page = 81;
			}
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
		uint8 dat = sco.getd();

		// 除算はサポートしない？
		if(0x80 <= dat && dat <= 0xbf) {
			cali[p++] = var[dat & 0x3f];
		} else if(0xc0 <= dat && dat <= 0xff) {
			cali[p++] = var[((dat & 0x3f) << 8) | sco.getd()];
		} else if(0x00 <= dat && dat <= 0x3f) {
			cali[p++] = ((dat & 0x3f) << 8) | sco.getd();
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
		sys_error("cali: invalid expression at %d:%04x", sco.page(), sco.cmd_addr());
	}
	return (uint16)(cali[1] & 0xffff);
}
