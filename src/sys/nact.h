/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT ]
*/

#ifndef _NACT_H_
#define _NACT_H_

#include <stdio.h>
#include "../common.h"

#define RND var[ 0]
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

#define MAX_STACK 100
#define MAX_MENU 64
#define MAX_VERB 128
#define MAX_OBJ 256
#define MAX_CAPTION 32
#define MAX_PCM 256

constexpr char SJIS_BACK[] = "\x96\xDF\x82\xE9"; // "戻る" in SJIS
constexpr char SJIS_NEXT_PAGE [] = "\x8E\x9F\x82\xCC\x83\x79\x81\x5B\x83\x57"; // "次のページ" in SJIS

class AGS;
class MAKO;

class NACT
{
public:
	static NACT* create(const char* game_id, const char* font_file, const char* playlist);
	NACT(int sys_ver, uint32 crc32_a, uint32 crc32_b, const char* font_file, const char* playlist);
	virtual ~NACT();

protected:
	AGS* ags;
	MAKO* mako;

	// コマンドパーサ
	void execute();
	void load_scenario(int page);

	bool fatal_error = false;
	bool post_quit = false;

	// ADISK.DAT, ASLEEP.DAT
	char adisk[16];

	// シナリオデータ
	uint8* scenario_data;
	int scenario_size;

	// アドレス、ページ管理
	int scenario_addr, prev_addr;
	int scenario_page;
	int label_depth;
	int label_stack[MAX_STACK];
	int page_depth;
	int page_stack[MAX_STACK];
	int addr_stack[MAX_STACK];

	// 変数
	uint16 var[512];
	uint16 var_stack[30][20];
//	char tvar[10][22];
	char tvar_stack[30][10][22];
//	int tvar_index;

	// Commands
	virtual void cmd_calc() = 0;

	virtual void cmd_branch() = 0;
	virtual void cmd_label_jump() = 0;
	virtual void cmd_label_call() = 0;
	virtual void cmd_page_jump() = 0;
	virtual void cmd_page_call() = 0;

	virtual void cmd_set_menu() = 0;
	virtual void cmd_open_menu() = 0;

	virtual void cmd_set_verbobj() = 0;
	virtual void cmd_set_verbobj2() = 0;
	virtual void cmd_open_verb() = 0;
	virtual void cmd_open_obj(int verb) = 0;

	virtual void cmd_a() = 0;
	virtual void cmd_b() = 0;
	virtual void cmd_d() = 0;
	virtual void cmd_e() = 0;
	virtual void cmd_f() = 0;
	virtual void cmd_g() = 0;
	virtual void cmd_h() = 0;
	virtual void cmd_i() = 0;
	virtual void cmd_j() = 0;
	virtual void cmd_k() = 0;
	virtual void cmd_l() = 0;
	virtual void cmd_m() = 0;
	virtual void cmd_n() = 0;
	virtual void cmd_o() = 0;
	virtual void cmd_p() = 0;
	virtual void cmd_q() = 0;
	virtual void cmd_r() = 0;
	virtual void cmd_s() = 0;
	virtual void cmd_t() = 0;
	virtual void cmd_u() = 0;
	virtual void cmd_v() = 0;
	virtual void cmd_w() = 0;
	virtual void cmd_x() = 0;
	virtual void cmd_y() = 0;
	virtual void cmd_z() = 0;

	// SYSTEM1
	virtual void opening() {}

	// DPS
	bool text_refresh;

	bool column;		// 座標モード
	bool wait_keydown;	// ウェイト時のキー受付
	int text_wait_time;	// テキスト表示のウェイト
	int mouse_sence;	// マウス感度

	int menu_window;	// メニューウィンドウ番号
	int text_window;	// メッセージウィンドウ番号
	bool show_push;		// Push表示
	bool clear_text;	// メニュー後のメッセージウィンドウ消去

	int menu_index;			// メニュー登録のインデックス
	int menu_addr[64];		// ジャンプ先のアドレス
	int menu_verb[64];		// 登録された動詞
	int menu_obj[64];		// 登録された目的語

	char caption_verb[MAX_VERB][MAX_CAPTION];
	char caption_obj[MAX_OBJ][MAX_CAPTION];

	bool verb_obj;	// 動詞-形容詞型メニューの定義中
	bool set_palette;

	int pcm_index;
	int pcm[MAX_PCM];

	// 下位関数
	virtual uint16 cali() = 0;
	virtual uint16 cali2() = 0;
	uint8 getd() { return scenario_data[scenario_addr++]; }
	uint16 getw() {
		uint16 val = scenario_data[scenario_addr] | (scenario_data[scenario_addr + 1] << 8);
		scenario_addr += 2;
		return val;
	}

	uint16 random(uint16 range);
	uint32 seed;

	void wait_after_open_menu();

	uint8 get_key();
	void get_cursor(int* x, int* y);
	void set_cursor(int x, int y);

#ifdef USE_JOY
	int joy_num;
	JOYCAPS joycaps;
#endif
	int mouse_x = 0, mouse_y = 0;

	static uint32 calc_crc32(const char* file_name, const char* game_id);

	// Y27 ダイアログ
	void text_dialog();

	// 終了フラグ
	bool terminate;

	// Platform-specific setup / cleanup code
	void platform_initialize();
	void platform_finalize();

public:
	void mainloop();

	int get_screen_height();

	void select_cursor();
	void select_sound(int dev);

	const char* get_game_id();
	static const int get_sys_ver(uint32 crc32_a, uint32 crc32_b);
	const char* get_title();

	bool text_skip_enb;	// メッセージスキップ
	bool text_wait_enb;	// テキスト表示のウェイト有効／無効
	int sys_ver;
	uint32 crc32_a;		// ADISK
	uint32 crc32_b;		// BDISK for D.P.S -SG- and Super D.P.S

	// for Y27
	char tvar[10][33];
	int tvar_index, tvar_maxlen;

	// デバッグログ
	void output_console(const char *format, ...);

	int get_scenario_page() const { return scenario_page; }

	void fatal(const char* msg, ...);
};

class NACT_Sys1 final : public NACT {
public:
	NACT_Sys1(uint32 crc32_a, uint32 crc32_b, const char* font_file, const char* playlist);
protected:
	void cmd_calc() override;
	void cmd_branch() override;
	void cmd_label_jump() override;
	void cmd_label_call() override;
	void cmd_page_jump() override;
	void cmd_page_call() override;
	void cmd_set_menu() override;
	void cmd_open_menu() override;
	void cmd_set_verbobj() override;
	void cmd_set_verbobj2() override;
	void cmd_open_verb() override;
	void cmd_open_obj(int verb) override;
	void cmd_a() override;
	void cmd_b() override;
	void cmd_d() override;
	void cmd_e() override;
	void cmd_f() override;
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
	void cmd_r() override;
	void cmd_s() override;
	void cmd_t() override;
	void cmd_u() override;
	void cmd_v() override;
	void cmd_w() override;
	void cmd_x() override;
	void cmd_y() override;
	void cmd_z() override;
	void opening() override;
	uint16 cali() override;
	uint16 cali2() override;
private:
	// Intruder Zコマンド
	int paint_x;
	int paint_y;
	int map_page;
};

class NACT_Sys2 final : public NACT {
public:
	NACT_Sys2(uint32 crc32_a, uint32 crc32_b, const char* font_file, const char* playlist);
protected:
	void cmd_calc() override;
	void cmd_branch() override;
	void cmd_label_jump() override;
	void cmd_label_call() override;
	void cmd_page_jump() override;
	void cmd_page_call() override;
	void cmd_set_menu() override;
	void cmd_open_menu() override;
	void cmd_set_verbobj() override;
	void cmd_set_verbobj2() override;
	void cmd_open_verb() override;
	void cmd_open_obj(int verb) override;
	void cmd_a() override;
	void cmd_b() override;
	void cmd_d() override;
	void cmd_e() override;
	void cmd_f() override;
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
	void cmd_r() override;
	void cmd_s() override;
	void cmd_t() override;
	void cmd_u() override;
	void cmd_v() override;
	void cmd_w() override;
	void cmd_x() override;
	void cmd_y() override;
	void cmd_z() override;
	uint16 cali() override;
	uint16 cali2() override;
};

class NACT_Sys3 final : public NACT {
public:
	NACT_Sys3(uint32 crc32_a, uint32 crc32_b, const char* font_file, const char* playlist);
protected:
	void cmd_calc() override;
	void cmd_branch() override;
	void cmd_label_jump() override;
	void cmd_label_call() override;
	void cmd_page_jump() override;
	void cmd_page_call() override;
	void cmd_set_menu() override;
	void cmd_open_menu() override;
	void cmd_set_verbobj() override;
	void cmd_set_verbobj2() override;
	void cmd_open_verb() override;
	void cmd_open_obj(int verb) override;
	void cmd_a() override;
	void cmd_b() override;
	void cmd_d() override;
	void cmd_e() override;
	void cmd_f() override;
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
	void cmd_r() override;
	void cmd_s() override;
	void cmd_t() override;
	void cmd_u() override;
	void cmd_v() override;
	void cmd_w() override;
	void cmd_x() override;
	void cmd_y() override;
	void cmd_z() override;
	uint16 cali() override;
	uint16 cali2() override;
private:
	int menu_select(int num_items);
};

#endif // _NACT_H_
