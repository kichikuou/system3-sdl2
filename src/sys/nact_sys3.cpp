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
#include "game_id.h"
#include "fileio.h"
#include "encoding.h"
#include "texthook.h"
#include "debugger/debugger.h"

#define D01 var[ 1]
#define D02 var[ 2]
#define D03 var[ 3]
#define D04 var[ 4]
#define D05 var[ 5]
#define D06 var[ 6]
#define D07 var[ 7]
#define D08 var[ 8]
#define D09 var[ 9]
#define D10 var[10]
#define D11 var[11]
#define D12 var[12]
#define D13 var[13]
#define D14 var[14]
#define D15 var[15]
#define D16 var[16]
#define D17 var[17]
#define D18 var[18]
#define D19 var[19]
#define D20 var[20]
#define U01 var[21]
#define U02 var[22]
#define U03 var[23]
#define U04 var[24]
#define U05 var[25]
#define U06 var[26]
#define U07 var[27]
#define U08 var[28]
#define U09 var[29]
#define U10 var[30]
#define U11 var[31]
#define U12 var[32]
#define U13 var[33]
#define U14 var[34]
#define U15 var[35]
#define U16 var[36]
#define U17 var[37]
#define U18 var[38]
#define U19 var[39]
#define U20 var[40]
#define B01 var[41]
#define B02 var[42]
#define B03 var[43]
#define B04 var[44]
#define B05 var[45]
#define B06 var[46]
#define B07 var[47]
#define B08 var[48]
#define B09 var[49]
#define B10 var[50]
#define B11 var[51]
#define B12 var[52]
#define B13 var[53]
#define B14 var[54]
#define B15 var[55]
#define B16 var[56]
#define M_X var[57]
#define M_Y var[58]

#define MAX_PCM 256

namespace {

class NACT_Sys3 : public NACT {
public:
	NACT_Sys3(const Config& config, const GameId& game_id) : NACT(config, game_id) {}

protected:
	void cmd_branch() override;
	void cmd_open_verb() override;
	void cmd_b() override;
	void cmd_e() override;
	void cmd_g() override;
	void cmd_h() override;
	void cmd_i() override;
	void cmd_j() override;
	void cmd_k() override;
	void cmd_l() override;
	void cmd_m() override;
	void cmd_n() override;
	void cmd_o() override;
	void cmd_p() override;
	void cmd_q() override;
	void cmd_t() override;
	void cmd_u() override;
	void cmd_v() override;
	void cmd_w() override;
	void cmd_y() override;
	void cmd_z() override;
	uint16 cali() override;

private:
	int pcm_index = 0;
	int pcm[MAX_PCM] = {};
	bool column = true;		// 座標モード
	int mouse_sence = 16;	// マウス感度

	void cmd_open_obj(int verb);
	uint16 cali2();

	struct K3HackInfo;
	static const K3HackInfo yakata3cd_k3_hack_table[];
	static const K3HackInfo yakata3fd_k3_hack_table[];
	static const K3HackInfo onlyyou_k3_hack_table[];
	bool k3_hack(const K3HackInfo* info_table);
};

void NACT_Sys3::cmd_branch()
{
	int condition = cali();
	int t_addr = sco.current_addr() + 2;
	int f_addr = sco.getw();

	// sigmarion3 最適化誤爆の対策
	sco.jump_to(condition ? t_addr : f_addr);

	TRACE("{%d: T:%4x, F:%4x", condition, t_addr, f_addr);
}

void NACT_Sys3::cmd_open_verb()
{
	// 動詞メニューの表示
	TRACE("open verb-obj menu");
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
	ags->draw_menu = true;

	for(int i = 0; i < MAX_VERB; i++) {
		if(chk[i]) {
			ags->draw_text(caption_verb[i].c_str());
			ags->menu.newline();
			id[index++] = i;
		}
	}
	ags->draw_menu = false;

	int selection = menu_select(index);
	if (terminate)
		return;

	if (selection == -1) {
		sco.jump_to(sco.default_addr());
	} else {
		cmd_open_obj(id[selection]);
	}
	menu_items.clear();
}

void NACT_Sys3::cmd_open_obj(int verb)
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
	ags->draw_menu = true;

	for(int i = 0; i < MAX_OBJ; i++) {
		if(chk[i]) {
			ags->draw_text(caption_obj[i].c_str());
			ags->menu.newline();
			id[index++] = i;
		}
	}
	// 戻るを追加
	ags->draw_text(strings.back.c_str());
	ags->menu.newline();
	id[index++] = 0;
	ags->draw_menu = false;

	int selection = menu_select(index);
	if (terminate)
		return;

	if (selection == -1) {
		sco.jump_to(sco.default_addr());
	} else {
		sco.jump_to(addr[id[selection]]);
	}
}

void NACT_Sys3::cmd_b()
{
	int cmd = sco.getd();
	int index = cali();
	int p1 = cali();
	int p2 = cali();
	int p3 = cali();
	int p4 = cali();
	int p5 = cali();

	TRACE("B %d,%d,%d,%d,%d,%d,%d:", cmd, index, p1, p2, p3, p4, p5);

	switch (cmd) {
	case 1:
		// あゆみちゃん物語
		if (game_id.is(GameId::AYUMI_CD) || game_id.is(GameId::AYUMI_LIVE_256) || game_id.is(GameId::AYUMI_LIVE_FULL)) {
			p5 = 1;
		}
		ags->set_menu_window(index, column ? p1 * 8 : p1 & 0xfff8, p2, column ? p3 * 8 - 1 : (p3 & 0xfff8) - 1, p4, p5);
		break;
	case 2:
		ags->set_menu_window_frame(index, p1);
		menu_window = index;
		break;
	case 3:
		ags->set_text_window(index, column ? p1 * 8 : p1 & 0xfff8, p2, column ? p3 * 8 - 1 : (p3 & 0xfff8) - 1, p4, p5);
		break;
	case 4:
		if(p5 == 0) {
			// ウィンドウ退避
			ags->set_text_window_frame(index, p1);
			ags->open_text_window(index, p4 ? false : true);
			text_window = index;
		} else {
			// ウィンドウ復帰
			ags->close_text_window(index, text_window == index ? true : false);
		}
		break;
	default:
		WARNING("Unimplemented command: B %d,%d,%d,%d,%d,%d,%d:", cmd, index, p1, p2, p3, p4, p5);
		break;
	}
}

void NACT_Sys3::cmd_e()
{
	int index = cali();
	uint8 color = (uint8)cali();
	int sx = cali();
	int sy = cali();
	int ex = cali();
	int ey = cali();

	TRACE("E %d,%d,%d,%d,%d,%d:", index, color, sx, sy, ex, ey);

	if (column) {
		sx = sx * 8;
		ex = ex * 8 - 1;
	}
	ags->set_box(index, color, sx, sy, ex, ey);
}

void NACT_Sys3::cmd_g()
{
	int page = cali();

	TRACE("G %d:", page);

	ags->load_cg(page, -1);
}

void NACT_Sys3::cmd_h()
{
	int length = sco.getd();
	int val = cali();

	TRACE("H %d,%d:", length, val);

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

	TRACE("I %d,%d,%d,%d,%d,%d:", sx, sy, ex, ey, dx, dy);

	// X方向はカラム単位で切り捨て
	sx = column ? sx * 8 : sx & 0xfff8;
	ex = column ? ex * 8 - 1 : game_id.is(GameId::NINGYO) ? (ex & 0xfff8) + 7 : (ex & 0xfff8) - 1;
	dx = column ? dx * 8 : dx & 0xfff8;
	ags->copy(sx, sy, ex, ey, dx, dy);
}

void NACT_Sys3::cmd_j()
{
	int x = cali();
	int y = cali();

	TRACE("J %d,%d:", x, y);

	// x方向はカラム単位で切り捨て
	ags->cg_dest_x = column ? x * 8 : x & 0xfff8;
	ags->cg_dest_y = y;
	ags->set_cg_dest = true;
}

void NACT_Sys3::cmd_k()
{
	int cmd = sco.getd(), val;

	TRACE("K %d:", cmd);

	// K6の場合は、すぐに返る
	if(cmd == 6) {
		RND = get_key();
		return;
	}

	if (cmd == 3) {
		switch (game_id.game) {
		case GameId::YAKATA3_CD:
			if (k3_hack(yakata3cd_k3_hack_table))
				return;
			break;
		case GameId::YAKATA3_FD:
			if (k3_hack(yakata3fd_k3_hack_table))
				return;
			break;
		case GameId::ONLYYOU:
			if (k3_hack(onlyyou_k3_hack_table))
				return;
			break;
		}
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

	TRACE("L %d:", index);

	if (index == 0) {
		// 特殊セーブ
		auto fio = FILEIO::open("ASLEEP.DAT", FILEIO_READ_BINARY | FILEIO_SAVEDATA);
		if (fio) {
			// U01 - U20
			for(int i = 21; i <= 40; i++) {
				var[i] = fio->getw();
			}
			// M1 - M10
			for(int i = 0; i < 10; i++) {
				fio->read(tvar[i], 22);
			}
		}
	} else if (1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		if (load(index))
			RND = 0;
	} else if (101 <= index && index <= 126) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		char file_name[_MAX_PATH];
		snprintf(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + index - 101);

		auto fio = FILEIO::open(file_name, FILEIO_READ_BINARY | FILEIO_SAVEDATA);
		if (fio) {
			fio->seek(112 + 16, SEEK_SET);
			RND = fio->getw();
		} else {
			RND = 255;
		}
	}
}

void NACT_Sys3::cmd_m()
{
	char string[22];

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

	TRACE("M %s:", encoding->toUtf8(string).c_str());

	if(1 <= tvar_index && tvar_index <= 10) {
		memcpy(tvar[tvar_index - 1], string, 22);
	} else if(tvar_index == 31) {
		sco.open(string);
	} else if(tvar_index == 32) {
		ags->set_cg_file(string);
	} else if(tvar_index == 33) {
		mako->amus.open(string);
		strcpy(string + strlen(string) - 3, "MDA");
		mako->mda.open(string);
	} else if(tvar_index == 34) {
		// mako->amse.open(string);
	}
}

void NACT_Sys3::cmd_n()
{
	int cmd = sco.getd();
	int src = cali();
	int dest = cali();

	TRACE_UNIMPLEMENTED("N %d,%d,%d:", cmd, src, dest);
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

		TRACE("O %d,%d:", cmd, val);
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

		TRACE("O %d,var[%d]:", cmd, index);
	}
}

void NACT_Sys3::cmd_p()
{
	int index =cali();
	int r = cali();
	int g = cali();
	int b = cali();

	TRACE("P %d,%d,%d,%d:", index, r, g, b);

	ags->set_palette(index, r, g, b);
#ifdef ENABLE_DEBUGGER
	if (g_debugger)
		g_debugger->on_palette_change();
#endif
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

	TRACE("Q %d:", index);

	if (index == 0) {
		// 特殊セーブ
		auto fio = FILEIO::open("ASLEEP.DAT", FILEIO_WRITE_BINARY | FILEIO_SAVEDATA);
		if (fio) {
			// U01 - U20
			for(int i = 21; i <= 40; i++) {
				fio->putw(var[i]);
			}
			// M1 - M10
			for(int i = 0; i < 10; i++) {
				fio->write(tvar[i], 22);
			}
		}
	} else if (1 <= index && index <= 26) {
		// ASLEEP_A.DAT - ASLEEP_Z.DAT
		save(index, header);
		RND = 1;
	}
}

void NACT_Sys3::cmd_t()
{
	int x = cali();
	int y = cali();

	TRACE("T %d,%d:", x, y);

	// x方向はカラム単位で切り捨て
	ags->text.pos.x = column ? x * 8 : x & 0xfff8;
	ags->text.pos.y = y;
}

void NACT_Sys3::cmd_u()
{
	int page = cali();
	int transparent = cali();

	TRACE("U %d,%d:", page, transparent);

	if (game_id.is_rance4x()) {
		transparent = (transparent == 28) ? 12 : transparent;
	}
	ags->load_cg(page, transparent);
}

void NACT_Sys3::cmd_v()
{
	int cmd = cali();
	int index = cali();

	TRACE("V %d,%d:", cmd, index);

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

	TRACE("W %d,%d,%d", x, y, color);

	ags->paint(x, y, color);
}

void NACT_Sys3::cmd_y()
{
	int cmd = cali();
	int param = cali();

	TRACE("Y %d,%d:", cmd, param);

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
			ags->ignore_palette.insert(param);
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
			ags->menu.font_size = (param == 1) ? 16 : (param == 2) ? 24 : (param == 3) ? 32 : (param == 4) ? 48 : (param == 5) ? 64 : 16;
			break;
		case 26:
			ags->text.font_size = (param == 1) ? 16 : (param == 2) ? 24 : (param == 3) ? 32 : (param == 4) ? 48 : (param == 5) ? 64 : 16;
			break;
		case 27:
			tvar_maxlen = param;
			text_dialog();
			break;
		case 28:
			ags->text.line_space = (param == 0) ? 2 : 0;
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
			ags->fade_in(param * 16 * 1000 / 60);
			break;
		case 41:
		case 43:
			ags->fade_out(param * 16 * 1000 / 60, cmd == 43);
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
			ags->scroll = param - 400;
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
				RND = static_cast<uint16>(sco.label_stack_size());
			} else if(param == 1) {
				RND = static_cast<uint16>(sco.page_stack_size());
			}
			break;
		case 81:
			if(param == 0) {
				sco.label_stack_pop();
			} else if(param == 1) {
				sco.page_stack_pop();
			}
			break;
		case 82:
			if(param == 0) {
				sco.label_stack_clear();
			} else if(param == 1) {
				sco.page_stack_clear();
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
				ags->draw_text(tvar[param - 1]);
				int padlen = cmd - 220 - encoding->mbslen(tvar[param - 1]);
				if (padlen > 0) {
					char pad[10] = "         ";
					pad[padlen] = '\0';
					ags->draw_text(pad);
				}
			}
			break;
		case 230:
			clear_text = param ? true : false;
			break;
		case 231:
			D01 = 640;
			D02 = (param == 1) ? ags->screen_height : 480;
			D03 = game_id.is(GameId::YAKATA3_FD) ? 16 : 256;
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
				snprintf(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + param - 1);

				struct stat statbuf;
				if (FILEIO::stat_save(file_name, &statbuf) != -1) {
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
			RND = game_id.is(GameId::YAKATA3_FD) ? 4 : 8;
			break;
		case 253:
			show_push = (param == 0) ? true : false;
			break;
		case 254:
			RND = (game_id.is(GameId::YAKATA3_CD) || game_id.is(GameId::YAKATA3_FD) || game_id.is(GameId::NINGYO)) ? 1 : 0;
			break;
		case 255:
			quit(param == 1 ? NACT_HALT : RND);
			break;
		default:
			WARNING("Unimplemented command: Y %d, %d", cmd, param);
			break;
	}
}

void NACT_Sys3::cmd_z()
{
	int cmd = cali();
	int param = cali();

	TRACE("Z %d,%d:", cmd, param);

	if(cmd == 0) {
		ags->palette_bank = (uint8)(param & 0xff);
	} else if(cmd == 1) {
		ags->text.font_color = (uint8)(param & 0xff);
	} else if(cmd == 2) {
		ags->menu.font_color = (uint8)(param & 0xff);
	} else if(cmd == 3) {
		ags->menu.frame_color = (uint8)(param & 0xff);
	} else if(cmd == 4) {
		ags->menu.back_color = (uint8)(param & 0xff);
	} else if(cmd == 5) {
		ags->text.frame_color = (uint8)(param & 0xff);
	} else if(cmd == 6) {
		ags->text.back_color = (uint8)(param & 0xff);
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

uint16 NACT_Sys3::cali2()
{
	uint16 val = 0;
	uint16 dat = sco.getd();

	if(0x80 <= dat && dat <= 0xbf) {
		val = dat & 0x3f;
	} else if(0xc0 <= dat && dat <= 0xff) {
		val = ((dat & 0x3f) << 8) | sco.getd();
	} else {
		sys_error("cali2: invalid expression at %d:%04x", sco.page(), sco.cmd_addr());
	}
	if (sco.getd() != 0x7f) {
		sys_error("cali2: invalid expression at %d:%04x", sco.page(), sco.cmd_addr());
	}
	return val;
}

struct NACT_Sys3::K3HackInfo {
	int page;
	int var;  // variable that holds the current selection
	int left;
	int top;
	int rows;
	int cols;
	int w;  // column width
	int h;  // row height
	bool draw_window;
};

const NACT_Sys3::K3HackInfo NACT_Sys3::yakata3cd_k3_hack_table[] = {
	//page, var, left, top, rows, cols,   w,   h, draw_win
	{   71,  61,  463,  28,    5,    1, 162,  48, false },  // main menu
	{   72,  60,   32,   0,    2,    3, 192, 138, false },  // game selection
	{   35,  61,  192, 136,    3,    1, 256,  56, false },  // quiz
	{   37,  61,  192, 136,    3,    1, 256,  56, false },  // quiz
	{   39,  61,  192, 136,    3,    1, 256,  56, false },  // quiz
	{   36,  61,  200, 200,    1,    5,  48,  48,  true },  // quiz (CG selection)
	{   -1 }
};

const NACT_Sys3::K3HackInfo NACT_Sys3::yakata3fd_k3_hack_table[] = {
	//page, var, left, top, rows, cols,   w,   h, draw_win
	{   71,  59,  463,  28,    5,    1, 162,  48, false },  // main menu
	{   72,  58,   32,   0,    2,    3, 192, 138, false },  // game selection
	{   35,  59,  192, 136,    3,    1, 256,  56, false },  // quiz
	{   37,  59,  192, 136,    3,    1, 256,  56, false },  // quiz
	{   39,  59,  192, 136,    3,    1, 256,  56, false },  // quiz
	{   36,  59,  200, 200,    1,    5,  48,  48,  true },  // quiz (CG selection)
	{   -1 }
};

const NACT_Sys3::K3HackInfo NACT_Sys3::onlyyou_k3_hack_table[] = {
	//page, var, left, top, rows, cols,   w,   h, draw_win
	{    7, 235,  392,   0,    2,    3,  48,  48, false },  // map area selection
	{   -1 }
};

bool NACT_Sys3::k3_hack(const K3HackInfo* info_table)
{
	const K3HackInfo* info;
	for (info = info_table; info->page >= 0; info++) {
		if (sco.page() == info->page)
			break;
	}
	if (info->page < 0)
		return false;

	if (info->draw_window) {
		// Draw a simple menu to indicate clickable areas.
		ags->draw_window(
			info->left - 6, info->top - 6,
			info->left + info->w * info->cols + 6, info->top + info->h + 6,
			true, ags->menu.frame_color, ags->menu.back_color);
		int orig_font_size = ags->text.font_size;
		ags->text.pos.x = info->left;
		ags->text.pos.y = info->top;
		ags->text.font_size = info->w;
		ags->draw_text("12345");
		ags->text.font_size = orig_font_size;
		int left = info->left + info->w * (var[info->var] - 1);
		ags->box_line(0, left, info->top, left + info->w, info->top + info->h, ags->text.font_color);
	}

	// Only You: Prevents the cursor from being locked due to unselectable cells.
	static bool row_first;
	row_first = !row_first;

	while (!terminate) {
		int mx, my;
		get_cursor(&mx, &my);
		if (info->left <= mx && mx < info->left + info->w * info->cols &&
			info->top <= my && my < info->top + info->h * info->rows) {
			int target_row = (my - info->top) / info->h;
			int target_col = (mx - info->left) / info->w;
			int current_row = (var[info->var] - 1) / info->cols;
			int current_col = (var[info->var] - 1) % info->cols;

			if (row_first) {
				if (target_row < current_row) {
					RND = 1;  // UP
					break;
				}
				if (target_row > current_row) {
					RND = 2;  // DOWN
					break;
				}
			}
			if (target_col < current_col) {
				RND = 4;  // LEFT
				break;
			}
			if (target_col > current_col) {
				RND = 8;  // RIGHT
				break;
			}
			if (!row_first) {
				if (target_row < current_row) {
					RND = 1;  // UP
					break;
				}
				if (target_row > current_row) {
					RND = 2;  // DOWN
					break;
				}
			}
		}
		int key = get_key();
		if (key) {
			// Wait for key release
			while (!terminate && get_key())
				sys_sleep(16);
			RND = key;
			break;
		}
		sys_sleep(16);
	}

	if (info->draw_window) {
		// Clear the menu window.
		ags->box_fill(
			0, info->left - 6, info->top - 6,
			info->left + info->w * info->cols + 6, info->top + info->h + 6, 0);
	}

	return true;
}

class NACT_Toushin2 final : public NACT_Sys3 {
public:
	NACT_Toushin2(const Config& config, const GameId& game_id)
		: NACT_Sys3(config, game_id) {}

	void cmd_b() override {
		int p1 = cali();
		int p2 = cali();
		TRACE_UNIMPLEMENTED("B %d,%d:", p1, p2);
	}

	void cmd_i() override {
		int p1 = cali();
		int p2 = cali();
		int p3 = cali();
		TRACE_UNIMPLEMENTED("I %d,%d,%d:", p1, p2, p3);
	}

	void cmd_n() override {
		int p1 = cali();
		int p2 = cali();
		TRACE_UNIMPLEMENTED("N %d,%d:", p1, p2);
	}

	void cmd_p() override {
		int p1 = cali();
		TRACE_UNIMPLEMENTED("P %d:", p1);
	}

	void cmd_t() override {
		int p1 = cali();
		int p2 = cali();
		int p3 = cali();
		TRACE_UNIMPLEMENTED("T %d,%d,%d:", p1, p2, p3);
	}

	void cmd_z() override {
		int p1 = cali();
		int p2 = cali();
		int p3 = cali();
		TRACE_UNIMPLEMENTED("Z %d,%d,%d:", p1, p2, p3);
	}
};

}  // namespace

// static
NACT* NACT::create_system3(const Config& config, const GameId& game_id) {
	switch (game_id.game) {
	case GameId::TOUSHIN2:
	case GameId::NISE_NAGURI:
	case GameId::GAKUEN_KING:
		return new NACT_Toushin2(config, game_id);
	default:
		return new NACT_Sys3(config, game_id);
	}
}
