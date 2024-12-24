/*
	ALICE SOFT SYSTEM2 for Win32

	Super D.P.S. - Dream Program System

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

void NACT_Sys2::cmd_branch()
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
				sco.getd();
				cali();
			} else if(cmd == 'I') {
				cali();
				cali();
//				sco.getd();
				cali();
			} else if(cmd == 'J') {
				cali();
				cali();
			} else if(cmd == 'K') {
				
			} else if(cmd == 'L') {
				sco.getd();
			} else if(cmd == 'M') {
				uint8 val = sco.getd();
				if (val == '\'' || val == '"') {  // SysEng
					sco.skip_syseng_string(encoding.get(), val);
				} else {
					while (val != ':')
						val = sco.getd();
				}
			} else if(cmd == 'N') {
				cali();
				cali();
			} else if(cmd == 'O') {
				cali();
				cali();
				cali();
			} else if(cmd == 'P') {
				sco.getd();
			} else if(cmd == 'Q') {
				sco.getd();
			} else if(cmd == 'R') {
				
			} else if(cmd == 'S') {
				sco.getd();
			} else if(cmd == 'T') {
				cali();
				cali();
				cali();
			} else if(cmd == 'U') {
				if (game_id.crc32_a == CRC32_YAKATA2) {
					cali();
					cali();
				} else {
					sco.getd();
					sco.getd();
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

void NACT_Sys2::cmd_open_verb()
{
	// 動詞メニューの表示
	output_console("\nopen verb-obj menu");
	verb_obj = false;

	// 表示する動詞のチェック
	bool chk[MAX_VERB] = {};
	int page = 0;

	for (const MenuItem& item : menu_items) {
		chk[item.verb] = true;
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
			ags->draw_text(caption_verb[i].c_str());
			id[index++] = i;
			ags->menu_dest_y += ags->menu_font_size + 2;
		}
	}
	ags->draw_menu = false;

	int selection = menu_select(index);
	if (terminate)
		return;

	if(selection == -1) {
		sco.jump_to(sco.default_addr());
	} else {
		cmd_open_obj(id[selection]);
	}
	menu_items.clear();
}

void NACT_Sys2::cmd_open_obj(int verb)
{
	// 目的語メニューの表示
	verb_obj = false;

	// 表示する目的語のチェック
	bool chk[MAX_OBJ] = {};
	int addr[MAX_OBJ], page = 0;

	for (const MenuItem& item : menu_items) {
		if (item.verb == verb) {
			chk[item.obj] = true;
			addr[item.obj] = item.addr;
		}
	}
	// 目的語がない場合
	if(chk[0]) {
		sco.jump_to(addr[0]);
		return;
	}
	// 以後、obj=0は戻るとして扱う
	chk[0] = false;
	addr[0] = sco.default_addr();

	// メニュー項目の準備
	int id[32], index = 0;

	ags->clear_menu_window();
	ags->menu_dest_y = 0;
	ags->draw_menu = true;

	for(int i = 0; i < MAX_OBJ; i++) {
		if(chk[i]) {
			ags->menu_dest_x = 2;
			ags->menu_dest_y += 2;
			ags->draw_text(caption_obj[i].c_str());
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
	ags->draw_menu = false;

	int selection = menu_select(index);
	if (terminate)
		return;

	if(selection == -1) {
		sco.jump_to(sco.default_addr());
	} else {
		sco.jump_to(addr[id[selection]]);
	}
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
		if (game_id.crc32_a == CRC32_AYUMI_FD || game_id.crc32_a == CRC32_AYUMI_HINT || game_id.crc32_a == CRC32_DRSTOP) {
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
		if (game_id.crc32_a == CRC32_AYUMI_FD || game_id.crc32_a == CRC32_AYUMI_HINT || game_id.crc32_a == CRC32_DRSTOP) {
			p1 = 1;
		}
//		if (game_id.crc32_a == CRC32_PROSTUDENTG_FD) {
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
		if (game_id.crc32_a == CRC32_AYUMI_FD || game_id.crc32_a == CRC32_AYUMI_HINT) {
			p1 = 1;
			//p5 ? 0 : 1; // 逆？
			p5 = 0;
		} else if (game_id.crc32_a == CRC32_DRSTOP) {
			p1 = 1;
		}
		if(p5 == 0) {
			// ウィンドウ退避
//			if (game_id.crc32_a == CRC32_PROSTUDENTG_FD) {
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

void NACT_Sys2::cmd_g()
{
	int page = cali();

	output_console("\nG %d:", page);

	if (game_id.crc32_a == CRC32_SDPS && (game_id.crc32_b == CRC32_SDPS_TONO || game_id.crc32_b == CRC32_SDPS_KAIZOKU)) {
		if(20 <= page && page <= 100) {
			page--;
		}
	}
	ags->load_cg(page, -1);

	if (game_id.crc32_a == CRC32_DALK_HINT) {
		if(page == 3) {
			WAIT(2000)
		}
	} else if (game_id.crc32_a == CRC32_RANCE3_HINT) {
		if(page == 25) {
			WAIT(2000)
		}
	}
}

void NACT_Sys2::cmd_h()
{
	int length = sco.getd();
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
//	int p3 = sco.getd();
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
	int index = sco.getd();

	output_console("\nL %d:", index);

	if (1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		if (load(index))
			RND = 0;
	}
}

void NACT_Sys2::cmd_m()
{
	char string[33];

	int d = sco.getd();
	if (d == '\'' || d == '"') {  // SysEng
		sco.get_syseng_string(string, sizeof(string), encoding.get(), d);
	} else {
		int p = 0;
		while(d != ':') {
			string[p++] = d;
			d = sco.getd();
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
	int param = sco.getd();

	if (game_id.crc32_a != CRC32_YAKATA2 && game_id.crc32_a != CRC32_DALK_HINT && game_id.crc32_a != CRC32_RANCE3_HINT) {
		ags->text_font_color = (uint8)((param & 0x7) + 16);
	}

	output_console("\nP %d:", param);
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

	int index = sco.getd();

	output_console("\nQ %d:", index);

	if (1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		save(index, header);
		RND = 1;
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

	if (game_id.crc32_a == CRC32_YAKATA2) {
		page = cali();
		transparent = cali();
	} else {
		page = sco.getd();
		transparent = sco.getd();
	}

	output_console("\nU %d,%d:", page, transparent);

	ags->load_cg(page, transparent);
}

void NACT_Sys2::cmd_v()
{
	int cmd = sco.getd();
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

	output_console("\nV %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d:",
	cmd, p01, p02, p03, p04, p05, p06, p07, p08, p09, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29);
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
			if (!(game_id.crc32_a == CRC32_SDPS && game_id.crc32_b == CRC32_SDPS_MARIA)) {
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
			ags->fade_in(param * 16 * 1000 / 60);
			break;
		case 41:
		case 43:
			ags->fade_out(param * 16 * 1000 / 60, cmd == 43);
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
				ags->draw_text(tvar[param - 1]);
				int padlen = cmd - 220 - encoding->mbslen(tvar[param - 1]);
				if (padlen > 0) {
					char pad[10] = "         ";
					pad[padlen] = '\0';
					ags->draw_text(pad);
				}
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
			quit(param == 1 ? NACT_HALT : RND);
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
		uint8 dat = sco.getd();

		if(0x80 <= dat && dat <= 0xbf) {
			cali[p++] = var[dat & 0x3f];
		} else if(0xc0 <= dat && dat <= 0xff) {
			cali[p++] = var[((dat & 0x3f) << 8) | sco.getd()];
		} else if(0x00 <= dat && dat <= 0x3f) {
			cali[p++] = ((dat & 0x3f) << 8) | sco.getd();
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
		sys_error("cali: invalid expression at %d:%04x", sco.page(), sco.cmd_addr());
	}
	return (uint16)(cali[1] & 0xffff);
}
