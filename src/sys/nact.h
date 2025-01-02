/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT ]
*/

#ifndef _NACT_H_
#define _NACT_H_

#include <memory>
#include <string>
#include <vector>
#include <stdio.h>
#include "../common.h"
#include "config.h"
#include "dri.h"
#include "game_id.h"
#include "scenario.h"

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

#define MAX_VERB 128
#define MAX_OBJ 256
#define MAX_PCM 256
#define MAX_VAR 512
#define MAX_STRVAR 10

#define TRACE(fmt, ...) if (config.trace) trace(fmt "\n", ##__VA_ARGS__)

class AGS;
class MAKO;
class MsgSkip;
class Encoding;

// special codes for NACT::exit_code
const int NACT_HALT = -1;
const int NACT_RESTART = -2;

class NACT
{
public:
	static NACT* create(const Config& config, const GameId& game_id);
	NACT(const Config& config, const GameId& game_id);
	virtual ~NACT();

	Scenario sco;
	AGS* ags;

protected:
	MAKO* mako;
	MsgSkip* msgskip;

	// コマンドパーサ
	void execute();

	// 変数
	uint16 var[MAX_VAR] = {};
	uint16 var_stack[30][20] = {};
	char tvar[MAX_STRVAR][33] = {};
	char tvar_stack[30][MAX_STRVAR][22] = {};
	int tvar_index = 0;
	int tvar_maxlen;

	void message(uint8_t first_byte);

	// Commands
	void cmd_calc();

	virtual void cmd_branch() = 0;
	void cmd_label_jump();
	void cmd_label_call();
	void cmd_page_jump();
	void cmd_page_call();

	void cmd_set_menu();
	void cmd_open_menu();

	void cmd_set_verbobj();
	void cmd_set_verbobj2();
	virtual void cmd_open_verb() = 0;

	void cmd_a();
	virtual void cmd_b() = 0;
	virtual void cmd_d() = 0;
	virtual void cmd_e() = 0;
	void cmd_f();
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
	void cmd_r();
	void cmd_s();
	virtual void cmd_t() = 0;
	virtual void cmd_u() = 0;
	virtual void cmd_v() = 0;
	virtual void cmd_w() = 0;
	void cmd_x();
	virtual void cmd_y() = 0;
	virtual void cmd_z() = 0;

	// SYSTEM1
	virtual void opening() {}

	// DPS
	bool text_refresh;

	bool column = true;		// 座標モード
	bool wait_keydown = true;	// ウェイト時のキー受付
	int text_wait_time = 100;	// テキスト表示のウェイト
	bool text_wait_enb = false;	// テキスト表示のウェイト有効／無効
	int mouse_sence = 16;	// マウス感度

	int menu_window = 1;	// メニューウィンドウ番号
	int text_window = 1;	// メッセージウィンドウ番号
	bool show_push = true;		// Push表示
	bool clear_text = true;	// メニュー後のメッセージウィンドウ消去

	struct MenuItem {
		uint16_t addr;
		uint8_t verb;
		uint8_t obj;
		explicit MenuItem(uint16_t addr, uint8_t verb = 0, uint8_t obj = 0) : addr(addr), verb(verb), obj(obj) {}
	};
	std::vector<MenuItem> menu_items;

	std::string caption_verb[MAX_VERB];
	std::string caption_obj[MAX_OBJ];

	bool verb_obj = false;	// 動詞-形容詞型メニューの定義中
	bool set_palette = false;

	int pcm_index = 0;
	int pcm[MAX_PCM] = {};

	// 下位関数
	bool is_message(uint8_t c) { return c == ' ' || c & 0x80; }

	uint16 random(uint16 range);
	uint32 seed;	// 乱数の種

	bool load(int index);
	bool save(int index, const char header[112]);

	int menu_select(int num_items);
	void wait_after_open_menu();

	uint8 get_key();
	void get_cursor(int* x, int* y);
	void set_cursor(int x, int y);

	SDL_GameController *sdl_gamecontroller = NULL;

	int mouse_x = 0, mouse_y = 0;

	// Y27 ダイアログ
	void text_dialog();

	// 終了フラグ
	bool terminate = false;
	int exit_code;

	// Platform-specific setup / cleanup code
	void platform_initialize();
	void platform_finalize();

	int last_paint_x = -1, last_paint_y = -1;

public:
	int mainloop();
	void sys_sleep(int ms);
	void quit(int code);
	void process_next_event();
	bool is_terminating() const { return terminate; }

	int get_screen_height();

	void select_cursor();

	void text_wait();
	void set_skip_menu_state(bool enabled, bool checked);

	virtual uint16 cali() = 0;

	bool mouse_move_enabled = true;
	const Config& config;
	const GameId& game_id;
	std::unique_ptr<Encoding> encoding;
	Strings strings;

	void trace(const char *format, ...);

	int get_scenario_page() const { return sco.page(); }
	uint16 get_var(int index) const { return var[index]; }
	void set_var(int index, uint16_t value) { var[index] = value; }
	const char* get_string(int index) const { return tvar[index]; }
	void set_string(int index, const char* value);

private:
	void pump_events();
	void handle_event(SDL_Event e);
	bool handle_platform_event(const SDL_Event& e);
};

class NACT_Sys1 final : public NACT {
public:
	NACT_Sys1(const Config& config, const GameId& game_id);
protected:
	void cmd_branch() override;
	void cmd_open_verb() override;
	void cmd_b() override;
	void cmd_d() override;
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
	void opening() override;
	uint16 cali() override;

private:
	void cmd_open_obj(int verb);

	// Intruder Zコマンド
	int paint_x;
	int paint_y;
	int map_page;

	// GAKUEN
	bool enable_graphics = true;
};

class NACT_Sys2 final : public NACT {
public:
	NACT_Sys2(const Config& config, const GameId& game_id) : NACT(config, game_id) {}
protected:
	void cmd_branch() override;
	void cmd_open_verb() override;
	void cmd_b() override;
	void cmd_d() override;
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
	void cmd_open_obj(int verb);
};

class NACT_Sys3 final : public NACT {
public:
	NACT_Sys3(const Config& config, const GameId& game_id) : NACT(config, game_id) {}
protected:
	void cmd_branch() override;
	void cmd_open_verb() override;
	void cmd_b() override;
	void cmd_d() override;
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
	void cmd_open_obj(int verb);
	uint16 cali2();

	struct K3HackInfo;
	static const K3HackInfo yakata3cd_k3_hack_table[];
	static const K3HackInfo yakata3fd_k3_hack_table[];
	static const K3HackInfo onlyyou_k3_hack_table[];
	bool k3_hack(const K3HackInfo* info_table);
};

extern std::unique_ptr<NACT> g_nact;

#endif // _NACT_H_
