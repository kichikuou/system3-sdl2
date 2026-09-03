/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT ]
*/

#ifndef _NACT_H_
#define _NACT_H_

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <stdio.h>
#include <SDL.h>
#include "common.h"
#include "config.h"
#include "cg.h"
#include "dri.h"
#include "game_id.h"
#include "scenario.h"

#define RND var[ 0]

#define MAX_VERB 128
#define MAX_OBJ 256
#define MAX_VAR 768  // Gakuen KING uses up to VAR737.
#define MAX_STRVAR 10

#define TRACE(fmt, ...) if (config.trace) trace(fmt "\n", ##__VA_ARGS__)
#define TRACE_UNIMPLEMENTED(fmt, ...) trace("Unimplemented command at %d:%04x " fmt "\n", sco.page(), sco.cmd_addr(), ##__VA_ARGS__)

class AGS;
class MAKO;
class MsgSkip;
class Encoding;
class FILEIO;

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
	std::string tvar[MAX_STRVAR];
	std::string tvar_stack[30][MAX_STRVAR];
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

	virtual void cmd_a();
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

	// Box (E and Y7 commands)
	struct Box {
		uint8 color = 0;
		int sx = 0;
		int sy = 0;
		int ex = 639;
		int ey = 399;
	};
	Box box[20];
	void draw_box(int index);

	uint8_t cg_flags;
	void load_cg(int page, int transparent);

	// Window (B command)
	struct Window {
		int sx;
		int sy;
		int ex;
		int ey;
		bool frame;
		bool save;
		CG screen;
		CG window;

		void reset(int sx, int sy, int ex, int ey, bool frame, bool save);
	};
	Window menu_w[10];
	Window text_w[10];
	bool menu_fix = false;
	const uint8_t* push_bitmap = nullptr;
	void init_windows();

	void clear_text_window(int index, bool erase);
	bool return_text_line(int index);
	void draw_push(int index);
	void open_text_window(int index, bool erase);
	void close_text_window(int index, bool update);

	void clear_menu_lines();
	void add_menu_line(std::string_view string);
	void draw_menu_lines(int index);
	int menu_window_bottom(int index);
	void open_menu_window(int index);
	void redraw_menu_window(int index, int selected);
	void close_menu_window(int index);
	void get_menu_window_rect(int index, int* sx, int* sy, int* ex, int* ey);
	int calculate_menu_max(int window);

	struct TextStyle {
		int line_space;
		int font_size;
		uint8_t font_color;
		uint8_t frame_color;
		uint8_t back_color;
	};
	TextStyle text_style;
	TextStyle menu_style;

	SDL_Point text_pos;
	int text_origin_x;
	int text_line_height;

	void reset_text_pos(int x, int y) {
		text_pos.x = text_origin_x = x;
		text_pos.y = y;
		text_line_height = 0;
	}
	void text_newline() {
		text_pos.x = text_origin_x;
		text_pos.y += std::max(text_style.font_size, text_line_height) + text_style.line_space;
		text_line_height = 0;
	}

	bool draw_hankaku = false;

	void init_text();
	void draw_text(std::string_view string, bool wait = false);

	struct MenuItem {
		uint16_t addr;
		uint8_t verb;
		uint8_t obj;
		explicit MenuItem(uint16_t addr, uint8_t verb = 0, uint8_t obj = 0) : addr(addr), verb(verb), obj(obj) {}
	};
	std::vector<MenuItem> menu_items;

	std::vector<std::u16string> menu_lines;
	std::optional<std::u16string> pending_menu_line;
	bool defining_menu_item() const { return pending_menu_line.has_value(); }

	std::string caption_verb[MAX_VERB];
	std::string caption_obj[MAX_OBJ];

	bool verb_obj = false;	// 動詞-形容詞型メニューの定義中

	// 下位関数
	bool is_message(uint8_t c) { return c == ' ' || c & 0x80; }

	uint16 random(uint16 range);
	uint32 seed;	// 乱数の種

	bool load(int index);
	bool save(int index, const char header[112]);
	void load_display_state(FILEIO* fio);
	void save_display_state(FILEIO* fio);

	int menu_select();
	void wait_after_open_menu();

	void fade_out(int duration_ms, bool white);
	void fade_in(int duration_ms);

	uint8 get_key(bool notify_texthook = true);
	void wait_key_release(uint8_t mask = 0xff);

	void get_cursor(int* x, int* y);
	void set_cursor(int x, int y);
	int get_wheel();

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
	const char* get_string(int index) const { return tvar[index].c_str(); }
	void set_string(int index, const std::string& value) { tvar[index] = value; }

private:
	std::u16string decode_text(std::string_view string);

	void pump_events();
	void handle_event(SDL_Event e);
	bool handle_platform_event(const SDL_Event& e);
	void show_quit_dialog();

	static NACT* create_system1(const Config& config, const GameId& game_id);
	static NACT* create_system2(const Config& config, const GameId& game_id);
	static NACT* create_system3(const Config& config, const GameId& game_id);
};

class NACT_Sys3 : public NACT {
public:
	NACT_Sys3(const Config& config, const GameId& game_id) : NACT(config, game_id) {}
	static NACT_Sys3* create(const Config& config, const GameId& game_id);

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

	void exec_y(int cmd, int param);
	uint16 cali2();

private:
	static constexpr int MAX_PCM = 256;

	int pcm_index = 0;
	int pcm[MAX_PCM] = {};
	bool column = true;		// 座標モード
	int mouse_sence = 16;	// マウス感度

	void cmd_open_obj(int verb);
	struct K3HackInfo;
	static const K3HackInfo yakata3cd_k3_hack_table[];
	static const K3HackInfo yakata3fd_k3_hack_table[];
	static const K3HackInfo onlyyou_k3_hack_table[];
	bool k3_hack(const K3HackInfo* info_table);
};

NACT_Sys3* create_gakuen_king(const Config& config, const GameId& game_id);

extern std::unique_ptr<NACT> g_nact;

#endif // _NACT_H_
