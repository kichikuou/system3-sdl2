/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - command ]
*/

#include <time.h>
#include <sys/stat.h>
#include "nact.h"
#include "ags.h"
#include "mako.h"
#include "msgskip.h"
#include "crc32.h"
#include "../fileio.h"
#include "texthook.h"

NACT_Sys3::NACT_Sys3(uint32 crc32_a, uint32 crc32_b, const Config& config)
	: NACT(3, crc32_a, crc32_b, config)
{
}

void NACT_Sys3::cmd_calc()
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

void NACT_Sys3::cmd_branch()
{
	int condition = cali();
	int t_addr = scenario_addr + 2;
	int f_addr = getw();

	// sigmarion3 最適化誤爆の対策
	scenario_addr = condition ? t_addr : f_addr;

	output_console("\n{%d: T:%4x, F:%4x", condition, t_addr, f_addr);
}

void NACT_Sys3::cmd_label_jump()
{
	int next_addr = getw();
	scenario_addr = next_addr;

	output_console("\n@%x:", next_addr);
}

void NACT_Sys3::cmd_label_call()
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

void NACT_Sys3::cmd_page_jump()
{
	int next_page = cali();
	load_scenario(next_page);
	scenario_page = next_page;
	scenario_addr = 2;

	output_console("\n&%d:", next_page);
}

void NACT_Sys3::cmd_page_call()
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

void NACT_Sys3::cmd_set_menu()
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

void NACT_Sys3::cmd_open_menu()
{
	output_console("\n]");

	if(!menu_index) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
		return;
	}

	int index = menu_select(menu_index);
	if (terminate)
		return;

	if(index != -1) {
		scenario_addr = menu_addr[index];
	}
	menu_index = 0;
}

void NACT_Sys3::cmd_set_verbobj()
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

void NACT_Sys3::cmd_set_verbobj2()
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

void NACT_Sys3::cmd_open_verb()
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

	if (selection == -1) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
	} else {
		cmd_open_obj(id[selection]);
	}
	menu_index = 0;
}

void NACT_Sys3::cmd_open_obj(int verb)
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

	if (selection == -1) {
		scenario_addr = scenario_data[0] | (scenario_data[1] << 8);
	} else {
		scenario_addr = addr[id[selection]];
	}
}

void NACT_Sys3::cmd_a()
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

void NACT_Sys3::cmd_b()
{
	int cmd = getd();
	int index = cali();
	int p1 = cali();
	int p2 = cali();
	int p3 = cali();
	int p4 = cali();
	int p5 = cali();

	output_console("\nB %d,%d,%d,%d,%d,%d,%d:", cmd, index, p1, p2, p3, p4, p5);

	if(cmd == 1) {
		// あゆみちゃん物語
		if(crc32_a == CRC32_AYUMI_CD || crc32_a == CRC32_AYUMI_JISSHA_256 || crc32_a == CRC32_AYUMI_JISSHA_FULL) {
			p5 = 1;
		}
		ags->menu_w[index - 1].sx = column ? p1 * 8 : p1 & 0xfff8;
		ags->menu_w[index - 1].sy = p2;
		ags->menu_w[index - 1].ex = column ? p3 * 8 - 1 : (p3 & 0xfff8) - 1;
		ags->menu_w[index - 1].ey = p4;
		ags->menu_w[index - 1].push = p5 ? true : false;

		// 退避画面の破棄
		if(ags->menu_w[index - 1].screen) {
			free(ags->menu_w[index - 1].screen);
			ags->menu_w[index - 1].screen = NULL;
		}
	} else if(cmd == 2) {
		ags->menu_w[index - 1].frame = p1 ? true : false;
		menu_window = index;
	} else if(cmd == 3) {
		ags->text_w[index - 1].sx = column ? p1 * 8 : p1 & 0xfff8;
		ags->text_w[index - 1].sy = p2;
		ags->text_w[index - 1].ex = column ? p3 * 8 - 1 : (p3 & 0xfff8) - 1;
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
			if(crc32_a == CRC32_PROSTUDENTG_CD && p4) {
				// prostudent G オープニング画面化け対策
				if(ags->text_w[index - 1].window) {
					free(ags->text_w[index - 1].window);
					ags->text_w[index - 1].window = NULL;
				}
			}
			ags->text_w[index - 1].frame = p1 ? true : false;
			ags->open_text_window(index, p4 ? false : true);
			text_window = index;
		} else {
			// ウィンドウ復帰
			ags->close_text_window(index, text_window == index ? true : false);
		}
	}
}

void NACT_Sys3::cmd_d()
{
	// 未使用
	fatal("Unknown Command: 'D' at page = %d, addr = %d", scenario_page, prev_addr);
}

void NACT_Sys3::cmd_e()
{
	int index = cali();
	uint8 color = (uint8)cali();
	int sx = cali();
	int sy = cali();
	int ex = cali();
	int ey = cali();

	output_console("\nE %d,%d,%d,%d,%d,%d:", index, color, sx, sy, ex, ey);

	ags->box[index - 1].color = color;
	ags->box[index - 1].sx = column ? sx * 8 : sx;
	ags->box[index - 1].sy = sy;
	ags->box[index - 1].ex = column ? ex * 8 - 1 : ex; // ?
	ags->box[index - 1].ey = ey;
}

void NACT_Sys3::cmd_f()
{
	output_console("\nF");

	scenario_addr = 2;
}

void NACT_Sys3::cmd_g()
{
	int page = cali();

	output_console("\nG %d:", page);

	ags->load_cg(page, -1);
}

void NACT_Sys3::cmd_h()
{
	int length = getd();
	int val = cali();

	output_console("\nH %d,%d:", length, val);

	// 隠しコマンド？
	if(length >= 9) {
		return;
	}

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
		if(tmp[i] != '0') {
			break;
		}
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

void NACT_Sys3::cmd_i()
{
	int sx = cali();
	int sy = cali();
	int ex = cali();
	int ey = cali();
	int dx = cali();
	int dy = cali();

	output_console("\nI %d,%d,%d,%d,%d,%d:", sx, sy, ex, ey, dx, dy);

	// X方向はカラム単位で切り捨て
	sx = column ? sx * 8 : sx & 0xfff8;
	ex = column ? ex * 8 - 1 : (crc32_a == CRC32_NINGYO) ? (ex & 0xfff8) + 7 : (ex & 0xfff8) - 1;
	dx = column ? dx * 8 : dx & 0xfff8;
	ags->copy(sx, sy, ex, ey, dx, dy);
}

void NACT_Sys3::cmd_j()
{
	int x = cali();
	int y = cali();

	output_console("\nJ %d,%d:", x, y);

	// x方向はカラム単位で切り捨て
	ags->cg_dest_x = column ? x * 8 : x & 0xfff8;
	ags->cg_dest_y = y;
	ags->set_cg_dest = true;
}

void NACT_Sys3::cmd_k()
{
	int cmd = getd(), val;

	output_console("\nK %d:", cmd);

	// K6の場合は、すぐに返る
	if(cmd == 6) {
		RND = get_key();
		return;
	}

	// マウスの初期取得
	int mx, my;
	get_cursor(&mx, &my);

	// キーが押されるまで待機
	for(;;) {
		if(terminate) {
			return;
		}
		if((val = get_key()) != 0) {
			break;
		}
		if(1 <= cmd && cmd <= 3) {
			// マウス対応
			int x, y, dx, dy;
			get_cursor(&x, &y);
			dx = (x < mx) ? mx - x : x - mx;
			dy = (y < my) ? my - y : y - my;
			if(dy > dx && my - y >= mouse_sence) {
				val = 1;
			}
			if(dy > dx && y - my >= mouse_sence) {
				val = 2;
			}
			if(dx > dy && mx - x >= mouse_sence) {
				val = 4;
			}
			if(dx > dy && x - mx >= mouse_sence) {
				val = 8;
			}
			if(val) {
				RND = val;
				return;
			}
		}
		sys_sleep(16);
	}
	if(cmd != 0 && cmd != 4) {
		RND = val;
	}

	// キーが離されるまで待機
	if(cmd != 1) {
		sys_sleep(100);
		for(;;) {
			if(terminate) {
				return;
			}
			if(!(val = get_key())) {
				break;
			}
			sys_sleep(16);
		}
	}

	// K0の場合は改行
	if(cmd == 0) {
		cmd_r();
	}
}

void NACT_Sys3::cmd_l()
{
	int index = cali();

	output_console("\nL %d:", index);

	if(index == 0) {
		// 特殊セーブ
		FILEIO* fio = new FILEIO();
		if(fio->Fopen("ASLEEP.DAT", FILEIO_READ_BINARY | FILEIO_SAVEDATA)) {
			// U01 - U20
			for(int i = 21; i <= 40; i++) {
				var[i] = fio->Fgetw();
			}
			// M1 - M10
			for(int i = 0; i < 10; i++) {
				fio->Fread(tvar[i], 22, 1);
			}
			fio->Fclose();
		}
		delete fio;
	} else if(1 <= index && index <= 26) {
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
	} else if(101 <= index && index <= 126) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		char file_name[_MAX_PATH];
		sprintf_s(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + index - 101);

		FILEIO* fio = new FILEIO();
		if(fio->Fopen(file_name, FILEIO_READ_BINARY | FILEIO_SAVEDATA)) {
			fio->Fseek(112 + 16, FILEIO_SEEK_SET);
			var[0] = fio->Fgetw();
			fio->Fclose();
		}
		delete fio;
	}
}

void NACT_Sys3::cmd_m()
{
	char string[22];

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
		memcpy(tvar[tvar_index - 1], string, 22);
	} else if(tvar_index == 31) {
		strcpy_s(adisk, 16, string);
	} else if(tvar_index == 32) {
		strcpy_s(ags->acg, 16, string);
	} else if(tvar_index == 33) {
		strcpy_s(mako->amus, 16, string);
	} else if(tvar_index == 34) {
		strcpy_s(mako->amse, 16, string);
	}
}

void NACT_Sys3::cmd_n()
{
	int cmd = getd();
	int src = cali();
	int dest = cali();

	output_console("\nN %d,%d,%d:", cmd, src, dest);
}

void NACT_Sys3::cmd_o()
{
	int cmd = cali();
	if(cmd == 0) {
		uint16 val = cali();
		B01 = (val & 0x0001) ? 1 : 0;
		B02 = (val & 0x0002) ? 1 : 0;
		B03 = (val & 0x0004) ? 1 : 0;
		B04 = (val & 0x0008) ? 1 : 0;
		B05 = (val & 0x0010) ? 1 : 0;
		B06 = (val & 0x0020) ? 1 : 0;
		B07 = (val & 0x0040) ? 1 : 0;
		B08 = (val & 0x0080) ? 1 : 0;
		B09 = (val & 0x0100) ? 1 : 0;
		B10 = (val & 0x0200) ? 1 : 0;
		B11 = (val & 0x0400) ? 1 : 0;
		B12 = (val & 0x0800) ? 1 : 0;
		B13 = (val & 0x1000) ? 1 : 0;
		B14 = (val & 0x2000) ? 1 : 0;
		B15 = (val & 0x4000) ? 1 : 0;
		B16 = (val & 0x8000) ? 1 : 0;

		output_console("\nO %d,%d:", cmd, val);
	} else {
		uint16 val = 0;
		val |= B01 ? 0x0001 : 0;
		val |= B02 ? 0x0002 : 0;
		val |= B03 ? 0x0004 : 0;
		val |= B04 ? 0x0008 : 0;
		val |= B05 ? 0x0010 : 0;
		val |= B06 ? 0x0020 : 0;
		val |= B07 ? 0x0040 : 0;
		val |= B08 ? 0x0080 : 0;
		val |= B09 ? 0x0100 : 0;
		val |= B10 ? 0x0200 : 0;
		val |= B11 ? 0x0400 : 0;
		val |= B12 ? 0x0800 : 0;
		val |= B13 ? 0x1000 : 0;
		val |= B14 ? 0x2000 : 0;
		val |= B15 ? 0x4000 : 0;
		val |= B16 ? 0x8000 : 0;
		int index = cali2();
		var[index] = val;

		output_console("\nO %d,var[%d]:", cmd, index);
	}
}

void NACT_Sys3::cmd_p()
{
	int index =cali();
	int r = cali();
	int g = cali();
	int b = cali();

	output_console("\nP %d,%d,%d,%d:", index, r, g, b);

	ags->set_palette(index, r, g, b);
	set_palette = true;
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

void NACT_Sys3::cmd_q()
{
	static char header[112] = {
#if 0
		// 夢幻泡影についてきたASLEEP_A.DATのヘッダ
		'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'a', 'v', 'e', ' ', 'd', 'a', 't',
		'a', ' ', 'f', 'o', 'r', ' ', 'S', 'y', 's', 't', 'e', 'm', '3', ' ', ' ', 'U',
		'n', 'i', 'o', 'n', ' ', '[', 'P', 'C', '9', '8', '0', '1', ':', 'F', 'M', 'T',
		'O', 'W', 'N', 'S', ':', 'X', '6', '8', '0', '0', '0', ']', ' ', 'F', 'o', 'r',
		' ', 'N', 'A', 'C', 'T', '/', 'A', 'D', 'V', ' ', 's', 'y', 's', 't', 'e', 'm',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '(', 'C', ')', '1', '9', '9',
		'4', ' ', 'A', 'L', 'I', 'C', 'E', '-', 'S', 'O', 'F', 'T', ' ', ' ', ' ', ' '
#else
		// 偽ヘッダ
		'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'a', 'v', 'e', ' ', 'd', 'a', 't',
		'a', ' ', 'f', 'o', 'r', ' ', 'S', 'y', 's', 't', 'e', 'm', '3', ' ', ' ', 'U',
		'n', 'i', 'o', 'n', ' ', '[', 'W', 'i', 'n', '9', 'x', '/', 'N', 'T', '4', '0',
		'/', '2', '0', '0', '0', '/', 'X', 'P', '/', 'C', 'E', ']', ' ', 'F', 'o', 'r',
		' ', 'N', 'A', 'C', 'T', '/', 'A', 'D', 'V', ' ', 's', 'y', 's', 't', 'e', 'm',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '(', 'C', ')', '2', '0', '0',
		'4', ' ', 'A', 'L', 'I', 'C', 'E', '-', 'S', 'O', 'F', 'T', ' ', ' ', ' ', ' '
#endif
	};

	int index = cali();

	output_console("\nQ %d:", index);

	if(index == 0) {
		// 特殊セーブ
		FILEIO* fio = new FILEIO();
		if(fio->Fopen("ASLEEP.DAT", FILEIO_WRITE_BINARY | FILEIO_SAVEDATA)) {
			// U01 - U20
			for(int i = 21; i <= 40; i++) {
				fio->Fputw(var[i]);
			}
			// M1 - M10
			for(int i = 0; i < 10; i++) {
				fio->Fwrite(tvar[i], 22, 1);
			}
			fio->Fclose();
		}
		delete fio;
	} else if(1 <= index && index <= 26) {
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

void NACT_Sys3::cmd_r()
{
	texthook_newline();

	output_console("R\n");

	// ウィンドウの表示範囲外の場合は改ページ
	if(ags->return_text_line(text_window)) {
		cmd_a();
	}
}

void NACT_Sys3::cmd_s()
{
	int page = getd();

	output_console("\nS %d:", page);

	if(page) {
		mako->play_music(page);
	} else {
		mako->stop_music();
	}
}

void NACT_Sys3::cmd_t()
{
	int x = cali();
	int y = cali();

	output_console("\nT %d,%d:", x, y);

	// x方向はカラム単位で切り捨て
	ags->text_dest_x = column ? x * 8 : x & 0xfff8;
	ags->text_dest_y = y;
}

void NACT_Sys3::cmd_u()
{
	int page = cali();
	int transparent = cali();

	output_console("\nU %d,%d:", page, transparent);

	if(crc32_a == CRC32_RANCE41 || crc32_a == CRC32_RANCE41_ENG ||
	   crc32_a == CRC32_RANCE42 || crc32_a == CRC32_RANCE42_ENG) {
		transparent = (transparent == 28) ? 12 : transparent;
	}
	ags->load_cg(page, transparent);
}

void NACT_Sys3::cmd_v()
{
	int cmd = cali();
	int index = cali();

	output_console("\nV %d,%d:", cmd, index);

	if(cmd == 0) {
		for(int i = 0; i < 20; i++) {
			var[21 + i] = var_stack[index - 1][i];
		}
		for(int i = 0; i < 10; i++) {
			memcpy(tvar[i], tvar_stack[index - 1][i], 22);
		}
	} else {
		for(int i = 0; i < 20; i++) {
			var_stack[index - 1][i] = var[21 + i];
		}
		for(int i = 0; i< 10; i++) {
			memcpy(tvar_stack[index - 1][i], tvar[i], 22);
		}
	}
}

void NACT_Sys3::cmd_w()
{
	int x = cali();
	int y = cali();
	uint8 color = (uint8)cali();

	output_console("\nW %d,%d,%d", x, y, color);

	ags->paint(x, y, color);
}

void NACT_Sys3::cmd_x()
{
	int index = getd();

	output_console("\nX %d:", index);

	if(1 <= index && index <= 10) {
		ags->draw_text(tvar[index - 1]);
	}
}

void NACT_Sys3::cmd_y()
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
			if(param == 10000) {
				wait_keydown = false;
			} else if(param == 10001) {
				wait_keydown = true;
			} else {
				Uint32 dwTime = SDL_GetTicks() + param * 1000 / 60;
				for(;;) {
					if(terminate) {
						return;
					}
					RND = get_key();
					if(RND && wait_keydown) {
						break;
					}
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
		case 5:
			break;
		case 6:
			break;
		case 7:
			ags->draw_box(param);
			break;
		case 8:
			tvar_index = param;
			break;
		case 10:
			ags->extract_palette_cg[param] = false;
			break;
		case 13:
			if(param == 0 || param == 1) {
				text_wait_time = 0;
			} else {
				text_wait_time = (param - 1) * 1000 / 60;
			}
			break;
		case 14:
			RND = 1;
			break;
		case 15:
			break;
		case 16:
			mako->stop_music();
			break;
		case 17:
			if(param == 0) {
				RND = mako->check_music() ? 1 : 0;
			} else {
				RND = 0;
			}
			break;
		case 18:
			{
				int mark, loop;
				mako->get_mark(&mark, &loop);
				D01 = mark;
				D02 = loop;
			}
			break;
		case 19:
			mako->next_loop = param;
			break;
		case 21:
			break;
		case 22:
			break;
		case 23:
			break;
		case 24:
			break;
		case 25:
			ags->menu_font_size = (param == 1) ? 16 : (param == 2) ? 24 : (param == 3) ? 32 : (param == 4) ? 48 : (param == 5) ? 64 : 16;
			break;
		case 26:
			ags->text_font_size = (param == 1) ? 16 : (param == 2) ? 24 : (param == 3) ? 32 : (param == 4) ? 48 : (param == 5) ? 64 : 16;
			break;
		case 27:
			tvar_maxlen = param;
			text_dialog();
			break;
		case 28:
			ags->text_space = (param == 0) ? 2 : 0;
			break;
		case 30:
			ags->src_screen = param ? 1 : 0;
			break;
		case 31:
			ags->dest_screen = param ? 1 : 0;
			break;
		case 32:
			RND = (param <= 480) ? param : 480;
			break;
		case 33:
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
		case 45:
			ags->extract_palette = param ? true : false;
			break;
		case 46:
			ags->get_palette = (param & 4) ? true : false;
			ags->extract_palette = (param & 2) ? true : false;
			ags->extract_cg = (param & 1) ? true : false;
			break;
		case 60:
			ags->scroll = param;
			ags->flush_screen(false);
			break;
		case 61:
			if(param) {
				ags->set_pixel(D03, D01, D02, (uint8)RND);
			} else {
				RND = ags->get_pixel(D03, D01, D02);
			}
			break;
		case 70:
			if(param == 2) {
				int x, y;
				get_cursor(&x, &y);
				M_X = (x < 0) ? x : (x > 639) ? 639 : x;
				M_Y = (y < 0) ? 0 : (y > 479) ? 479 : y;
				if((RND = get_key())) {
					// キー入力があった場合は、離すまで待機
					for(;;) {
						if(terminate) {
							return;
						}
						if(!get_key()) {
							break;
						}
						sys_sleep(16);
					}
				}
			} else if(param == 3) {
				set_cursor(M_X, M_Y);
			}
			break;
		case 71:
			mouse_sence = param;
			break;
		case 72:
			ags->load_cursor(param);
			break;
		case 73:
			ags->cursor_index = param;
			ags->select_cursor();
			break;
		case 80:
			if(param == 0) {
				RND = label_depth;
			} else if(param == 1) {
				RND = page_depth;
			}
			break;
		case 81:
			if(param == 0) {
				label_depth = label_depth ? label_depth - 1 : 0;
			} else if(param == 1) {
				page_depth = page_depth ? page_depth - 1 : 0;
			}
			break;
		case 82:
			if(param == 0) {
				label_depth = 0;
			} else if(param == 1) {
				page_depth = 0;
			}
			break;
		case 100:
			RND = param;
			break;
		case 101:
			pcm_index = param;
			break;
		case 102:
			if(pcm_index < MAX_PCM) {
				pcm[pcm_index] = param;
			}
			break;
		case 103:
			if(pcm_index < MAX_PCM) {
				mako->play_pcm(pcm[pcm_index], param ? false : true);
			}
			break;
		case 104:
			mako->stop_pcm();
			break;
		case 105:
			RND = mako->check_pcm() ? 1 : 0;
			break;
		case 106:
			// 乙女戦記で使用している謎のコマンド
			// 多分PCM関連
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
			{
				char string[21];
				int s = 0, d = 0;
				for(int i = 0; i < cmd - 220; i++) {
					if(tvar[param - 1][s] == '\0') {
						string[d++] = 0x20;
					} else {
						uint8 code = tvar[param - 1][s++];
						string[d++] = code;
						if(is_2byte_message(code)) {
							string[d++] = tvar[param - 1][s++];
						}
					}
				}
				string[d] = '\0';
				ags->draw_text(string);
			}
			break;
		case 230:
			clear_text = param ? true : false;
			break;
		case 231:
			D01 = 640;
			D02 = (param == 1) ? ags->screen_height : 480;
			D03 = (crc32_a == CRC32_YAKATA3_FD) ? 16 : 256;
			break;
		case 232:
			if(ags->screen_height != (param ? 480 : 400)) {
				// TODO: fix
				// SendMessage(g_hwnd, WM_USER + 2, param ? 480 : 400, 0);
				ags->screen_height = param ? 480 : 400;
			}
			break;
		case 233:
			break;
		case 234:
			ags->menu_fix = param ? true : false;
			break;
		case 235:
			break;
		case 236:
			RND = 1024;
			break;
		case 237:
			break;
		case 238:
			{
				time_t timer;
				time(&timer);
				struct tm *t = localtime(&timer);
				D01 = t->tm_year + 1900;
				D02 = t->tm_mon + 1;
				D03 = t->tm_mday;
				D04 = t->tm_hour;
				D05 = t->tm_min;
				D06 = t->tm_sec;
				D07 = t->tm_wday + 1;
			}
			break;
		case 239:
			if(1 <= param && param <= 26) {
				char file_name[_MAX_PATH];
				sprintf_s(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + param - 1);

				struct stat statbuf;
				if (FILEIO::StatSavedata(file_name, &statbuf) != -1) {
					struct tm *t = localtime(&statbuf.st_mtime);
					D01 = t->tm_year + 1900;
					D02 = t->tm_mon + 1;
					D03 = t->tm_mday;
					D04 = t->tm_hour;
					D05 = t->tm_min;
					D06 = t->tm_sec;
				} else {
					D01 = D02 = D03 = D04 = D05 = D06 = 0;
				}
			} else {
				// 不正なファイル番号
				D01 = D02 = D03 = D04 = D05 = D06 = 0;
			}
			break;
		case 240:
			ags->draw_hankaku = (param == 1) ? true : false;
			break;
		case 241:
			break;
		case 250:
#if 0
			if(FILEIO::GetRootPath()[1] == ':') {
				_TCHAR root_path[4];
				root_path[0] = FILEIO::GetRootPath()[0];
				root_path[1] = ':';
				root_path[2] = '\\';
				root_path[3] = '\0';

				UINT t = GetDriveType(root_path);
				// フロッピーから起動した場合は0が返る
				//RND = (t == DRIVE_REMOVABLE) ? 0 : (t == DRIVE_CDROM) ? 2 : 1;
				RND = (t == DRIVE_CDROM) ? 2 : 1;
			} else {
				// ネットワークドライブ？
				RND = 1;
			}
#else
			RND = 1;
#endif
			break;
		case 251:
			column = param ? false : true;
			break;
		case 252:
			RND = (crc32_a == CRC32_YAKATA3_FD) ? 4 : 8;
			break;
		case 253:
			show_push = (param == 0) ? true : false;
			break;
		case 254:
			RND = (crc32_a == CRC32_YAKATA3_CD || crc32_a == CRC32_YAKATA3_FD || crc32_a == CRC32_NINGYO) ? 1 : 0;
			break;
		case 255:
			post_quit = (param == 1) ? true : false;
			fatal_error = true;
			break;
	}
}

void NACT_Sys3::cmd_z()
{
	int cmd = cali();
	int param = cali();

	output_console("\nZ %d,%d:", cmd, param);

	if(cmd == 0) {
		ags->palette_bank = (uint8)(param & 0xff);
	} else if(cmd == 1) {
		ags->text_font_color = (uint8)(param & 0xff);
	} else if(cmd == 2) {
		ags->menu_font_color = (uint8)(param & 0xff);
	} else if(cmd == 3) {
		ags->menu_frame_color = (uint8)(param & 0xff);
	} else if(cmd == 4) {
		ags->menu_back_color = (uint8)(param & 0xff);
	} else if(cmd == 5) {
		ags->text_frame_color = (uint8)(param & 0xff);
	} else if(cmd == 6) {
		ags->text_back_color = (uint8)(param & 0xff);
	} else if(cmd == 7) {
		ags->cursor_color = (uint8)(param & 0xff);
	} else if(101 <= cmd && cmd <= 199) {
		mako->set_cd_track(cmd - 100, param);
	}
}


// 下位関数

uint16 NACT_Sys3::cali()
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

uint16 NACT_Sys3::cali2()
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
