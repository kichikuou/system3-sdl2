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
#include "common.h"
#include "config.h"
#include "dri.h"
#include "game_id.h"
#include "scenario.h"

#define RND var[ 0]

#define MAX_VERB 128
#define MAX_OBJ 256
#define MAX_VAR 512
#define MAX_STRVAR 10

#define TRACE(fmt, ...) if (config.trace) trace(fmt "\n", ##__VA_ARGS__)
#define TRACE_UNIMPLEMENTED(fmt, ...) trace("Unimplemented command " fmt "\n", ##__VA_ARGS__)

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
	virtual void cmd_b() { sco.unknown_command('B'); }
	virtual void cmd_d() { sco.unknown_command('D'); }
	virtual void cmd_e() { sco.unknown_command('E'); }
	void cmd_f();
	virtual void cmd_g() { sco.unknown_command('G'); }
	virtual void cmd_h() { sco.unknown_command('H'); }
	virtual void cmd_i() { sco.unknown_command('I'); }
	virtual void cmd_j() { sco.unknown_command('J'); }
	virtual void cmd_k() { sco.unknown_command('K'); }
	virtual void cmd_l() { sco.unknown_command('L'); }
	virtual void cmd_m() { sco.unknown_command('M'); }
	virtual void cmd_n() { sco.unknown_command('N'); }
	virtual void cmd_o() { sco.unknown_command('O'); }
	virtual void cmd_p() { sco.unknown_command('P'); }
	virtual void cmd_q() { sco.unknown_command('Q'); }
	void cmd_r();
	void cmd_s();
	virtual void cmd_t() { sco.unknown_command('T'); }
	virtual void cmd_u() { sco.unknown_command('U'); }
	virtual void cmd_v() { sco.unknown_command('V'); }
	virtual void cmd_w() { sco.unknown_command('W'); }
	void cmd_x();
	virtual void cmd_y() { sco.unknown_command('Y'); }
	virtual void cmd_z() { sco.unknown_command('Z'); }

	// SYSTEM1
	virtual void opening() {}

	// DPS
	bool text_refresh;

	bool wait_keydown = true;	// ウェイト時のキー受付
	int text_wait_time = 100;	// テキスト表示のウェイト
	bool text_wait_enb = false;	// テキスト表示のウェイト有効／無効

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

	// Y27 ダイアログ
	void text_dialog();

	// 終了フラグ
	bool terminate = false;
	int exit_code;

	// Platform-specific setup / cleanup code
	void platform_initialize();
	void platform_finalize();

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

	static NACT* create_system1(const Config& config, const GameId& game_id);
	static NACT* create_system2(const Config& config, const GameId& game_id);
	static NACT* create_system3(const Config& config, const GameId& game_id);
};

extern std::unique_ptr<NACT> g_nact;

#endif // _NACT_H_
