/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT ]
*/

#include <stdarg.h>
#include <stdlib.h>
#include "nact.h"
#include "encoding.h"
#include "ags.h"
#include "mako.h"
#include "msgskip.h"
#include "dri.h"
#include "../config.h"
#include "../fileio.h"
#include "crc32.h"

extern SDL_Window* g_window;

// 初期化

NACT::NACT(int sys_ver, uint32 crc32_a, uint32 crc32_b, const Config& config)
	: sys_ver(sys_ver),
	  crc32_a(crc32_a),
	  crc32_b(crc32_b),
	  config(config),
	  encoding(Encoding::create(get_encoding_name())),
	  strings(config.get_strings(encoding.get(), get_language() == ENGLISH))
{
	platform_initialize();

	// AG00.DAT読み込み
	auto fio = FILEIO::open("AG00.DAT", FILEIO_READ_BINARY);
	if (fio) {
		int d0, d1, d2, d3;
		char string[MAX_CAPTION];
		fio->ag00_gets(string, MAX_CAPTION);
		if (sscanf_s(string, "%d,%d,%d,%d", &d0, &d1, &d2, &d3) != 4)
			fatal("AG00.DAT: parse error");
		for(int i = 0; i < d1; i++) {
			// 動詞の読み込み
			fio->ag00_gets(string, MAX_CAPTION);
			memcpy(caption_verb[i], string, sizeof(string));
		}
		for(int i = 0; i < d2; i++) {
			// 目的語の読み込み
			fio->ag00_gets(string, MAX_CAPTION);
			memcpy(caption_obj[i], string, sizeof(string));
		}
		fio.reset();
	}

	// ADISK.DAT
	if (crc32_a == CRC32_PROG_OMAKE)
		adisk.open("AGAME.DAT");
	else
		adisk.open("ADISK.DAT");

	// シナリオ管理
	load_scenario(0);
	scenario_page = 0;
	scenario_addr = 2;
	label_depth = page_depth = 0;

	// 変数初期化
	memset(var, 0, sizeof(var));
	memset(var_stack, 0, sizeof(var_stack));
	memset(tvar, 0, sizeof(tvar));
	memset(tvar_stack, 0, sizeof(tvar_stack));

	// コマンド初期化
	column = true;		// 座標モード
	wait_keydown = true;	// ウェイト時のキー受付
	seed = SDL_GetTicks();	// 乱数の種
	text_wait_time = 100;	// メッセージ表示のウェイト
	text_wait_enb = false;
	mouse_sence = 16;	// マウス移動感知

	menu_window = text_window = 1;
	menu_index = 0;
	show_push = true;
	clear_text = true;

	verb_obj = false;
	set_palette = false;

	tvar_index = 0;

	pcm_index = 0;
	memset(pcm, 0, sizeof(pcm));

	// 各種クラス生成
	ags = new AGS(this, config);
	mako = new MAKO(this, config);
	msgskip = new MsgSkip(this);

	SDL_Init(SDL_INIT_GAMECONTROLLER);
	for (int i = 0; i < SDL_NumJoysticks(); ++i) {
		if (SDL_IsGameController(i)) {
			sdl_gamecontroller = SDL_GameControllerOpen(i);
			if (sdl_gamecontroller) {
				break;
			} else {
				WARNING("Could not open gamecontroller %i: %s\n", i, SDL_GetError());
			}
		}
	}

	terminate = false;
}

NACT::~NACT()
{
	delete ags;
	delete mako;
	delete msgskip;

	platform_finalize();
}

int NACT::mainloop()
{
	msgskip->load_from_file();

	int sleep_cnt = 0;
	while(!terminate) {
		execute();
		// 512コマンド実行毎にSleep(10)
		if(!(sleep_cnt = (sleep_cnt + 1) & 0x1ff)) {
			sys_sleep(10);
		}
	}
	while (exit_code == NACT_HALT) {
		// exit_code can change if the user selects restart or exit from the menu.
		sys_sleep(16);
	}
	return exit_code;
}

void NACT::quit(int code)
{
#ifdef __EMSCRIPTEN__
	code = EM_ASM_INT({ return xsystem35.shell.onExit($0); }, code);
#endif
	exit_code = code;
	terminate = true;
}

// コマンドパーサ

EMSCRIPTEN_KEEPALIVE  // Prevent inlining, because this function is listed in ASYNCIFY_ADD
void NACT::execute()
{
	// アドレスの確認
	if (scenario_addr < 2 || scenario_addr >= static_cast<int>(scenario_data.size())) {
		fatal("Scenario error");
		return;
	}

	if(sys_ver == 1 && scenario_page == 0 && scenario_addr == 2) {
		opening();
	}

	// Skip SysEng's "new style" marker
	if (scenario_page == 0 && scenario_addr == 2 &&
		memcmp(&scenario_data[2], "REV", 3) == 0) {
		scenario_addr = 5;
	}

	// １コマンド実行
	prev_addr = scenario_addr;
	uint8 cmd = getd();

	if(set_palette && cmd != 'P') {
		// パレット設定が終わった
		ags->flush_screen(true);
		set_palette = false;
	}
	if(verb_obj && cmd != '[' && cmd != ':') {
		// 動詞-目的語メニューの表示
		scenario_addr--;
		cmd_open_verb();
		return;
	}

	switch(cmd) {
		case '!':
			cmd_calc();
			break;
		case '{':
			cmd_branch();
			break;
		case '}':
			break;
		case '@':
			cmd_label_jump();
			break;
		case '\\':
			cmd_label_call();
			break;
		case '&':
			cmd_page_jump();
			break;
		case '%':
			cmd_page_call();
			break;
		case '$':
			cmd_set_menu();
			break;
		case '[':
			cmd_set_verbobj();
			break;
		case ':':
			cmd_set_verbobj2();
			break;
		case ']':
			cmd_open_menu();
			break;
		case 'A':
			cmd_a();
			break;
		case 'B':
			cmd_b();
			break;
		case 'D':
			cmd_d();
			break;
		case 'E':
			cmd_e();
			break;
		case 'F':
			cmd_f();
			break;
		case 'G':
			cmd_g();
			break;
		case 'H':
			cmd_h();
			break;
		case 'I':
			cmd_i();
			break;
		case 'J':
			cmd_j();
			break;
		case 'K':
			cmd_k();
			break;
		case 'L':
			cmd_l();
			break;
		case 'M':
			cmd_m();
			break;
		case 'N':
			cmd_n();
			break;
		case 'O':
			cmd_o();
			break;
		case 'P':
			cmd_p();
			break;
		case 'Q':
			cmd_q();
			break;
		case 'R':
			cmd_r();
			break;
		case 'S':
			cmd_s();
			break;
		case 'T':
			cmd_t();
			break;
		case 'U':
			cmd_u();
			break;
		case 'V':
			cmd_v();
			break;
		case 'W':
			cmd_w();
			break;
		case 'X':
			cmd_x();
			break;
		case 'Y':
			cmd_y();
			break;
		case 'Z':
			cmd_z();
			break;
		case '\'': case '"':
			message(cmd);
			break;
		default:
			if (is_message(cmd)) {
				ungetd();
				message(0);
			} else if (cmd >= 0x20 && cmd < 0x7f) {
				fatal("Unknown Command: '%c' at page = %d, addr = %d", cmd, scenario_page, prev_addr);
			} else {
				fatal("Unknown Command: %02x at page = %d, addr = %d", cmd, scenario_page, prev_addr);
			}
			break;
	}
}

void NACT::skip_string(uint8 terminator)
{
	for (uint8 c = getd(); c != terminator; c = getd()) {
		if (c != '\\')
			ungetd();
		scenario_addr += encoding->mblen(&scenario_data[scenario_addr]);
	}
}

void NACT::get_string(char* buf, int size, uint8 terminator)
{
	int start_addr = scenario_addr;

	int i = 0;
	for (uint8 c = getd(); c != terminator; c = getd()) {
		if (c != '\\')
			ungetd();
		int len = encoding->mblen(&scenario_data[scenario_addr]);
		if (i + len >= size)
			fatal("String buffer overrun. page = %d, addr = %d", scenario_page, start_addr);
		memcpy(&buf[i], &scenario_data[scenario_addr], len);
		i += len;
		scenario_addr += len;
	}
	buf[i] = '\0';
}

void NACT::message(uint8 terminator)
{
	char buf[200];
	if (terminator) {  // SysEng
		get_string(buf, sizeof(buf), terminator);
	} else {
		uint8* begin = &scenario_data[scenario_addr];
		uint8* p = begin;
		while (is_message(*p))
			p += encoding->mblen(p);
		int len = p - begin;
		scenario_addr += len;

		strncpy(buf, reinterpret_cast<char*>(begin), len);
		buf[len] = '\0';
	}

	ags->draw_text(buf, text_wait_enb);

	if(crc32_a == CRC32_DPS || crc32_a == CRC32_DPS_SG || crc32_a == CRC32_DPS_SG2 || crc32_a == CRC32_DPS_SG3) {
		if(!ags->draw_menu) {
			text_refresh = false;
		}
	}
	if (!ags->draw_menu)
		msgskip->on_message(scenario_page, scenario_addr);

	// TODO: Convert hankaku to zenkaku
	output_console(buf);
}

void NACT::text_wait()
{
	Uint32 dwTime = SDL_GetTicks() + text_wait_time;
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

void NACT::load_scenario(int page)
{
	scenario_data = adisk.load(page + 1);
	if (scenario_data.empty()) {
		fatal("Cannot load scenario %d", page);
	}
}

// 下位関数

int NACT::menu_select(int num_items)
{
	if (msgskip->get_flags() & MSGSKIP_STOP_ON_MENU)
		msgskip->activate(false);

	// メニュー表示
	ags->open_menu_window(menu_window);

	// マウス移動
	int sx = ags->menu_w[menu_window - 1].sx;
	int sy = ags->menu_w[menu_window - 1].sy;
	int ex = ags->menu_w[menu_window - 1].ex;
	int mx = ex - 16;
	int my = sy + 10;
	int height = ags->menu_font_size + 4;
	int current_index = 0;

	set_cursor(mx, my);
	wait_after_open_menu();

	// メニュー選択
	for(bool selectable = true;;) {
		// 入力待機
		int val = 0, current_mx = mx, current_my = my;
		for(;;) {
			if(terminate) {
				return -1;
			}
			get_cursor(&current_mx, &current_my);
			int dx = mx - current_mx;
			int dy = my - current_my;
			if (dx*dx + dy*dy > 10)
				break;
			if((val = get_key()) != 0) {
				sys_sleep(100);
				break;
			}
			sys_sleep(16);
		}
		if(val) {
			for(;;) {
				if(terminate) {
					return -1;
				}
				if(!get_key()) {
					break;
				}
				sys_sleep(16);
			}
		}

		if(val == 0) {
			// マウス操作
			mx = current_mx; my = current_my;
			int index = (my - sy) / height;
			if(sx <= mx && mx <= ex && 0 <= index && index < num_items) {
				current_index = index;
				ags->redraw_menu_window(menu_window, current_index);
				selectable = true;
			} else {
				selectable = false;
			}
		} else if(val == 1 || val == 2 || val == 4 || val == 8) {
			if(val == 1) {
				current_index = current_index ? current_index - 1 : num_items - 1;
			} else if(val == 2) {
				current_index = (current_index < num_items - 1) ? current_index + 1 : 0;
			} else if(val == 4) {
				current_index = 0;
			} else if(val == 8) {
				current_index = num_items - 1;
			}
			ags->redraw_menu_window(menu_window, current_index);
			selectable = true;
		} else if(val == 16 && selectable) {
			break;
		} else if(val == 32) {
			current_index = -1;
			break;
		}
	}

	// 画面更新
	ags->close_menu_window(menu_window);
	if(clear_text) {
		ags->clear_text_window(text_window, true);
	}

	return current_index;
}

uint16 NACT::random(uint16 range)
{
	// xorshift32
	seed = seed ^ (seed << 13);
	seed = seed ^ (seed >> 17);
	seed = seed ^ (seed << 15);
	return (uint16)(((uint32)range * (seed & 0xffff)) >> 16) + 1;
}

void NACT::wait_after_open_menu()
{
	if (mouse_move_enabled) {
		// 連打による誤クリック防止
		Uint32 dwTime = SDL_GetTicks();
		Uint32 dwWait = dwTime + 400;

		while(dwTime < dwWait) {
			if(terminate) {
				return;
			}
			sys_sleep(10);
			dwTime = SDL_GetTicks();
		}
	}

	// クリック中の間は待機
	for(;;) {
		if(terminate) {
			return;
		}
		if(!get_key()) {
			break;
		}
		sys_sleep(10);
	}
}

void NACT::sys_sleep(int ms) {
	pump_events();
	ags->update_screen();
#ifdef __EMSCRIPTEN__
	emscripten_sleep(ms);
#else
	SDL_Delay(ms);
#endif
}

// WinMainとのインターフェース

int NACT::get_screen_height()
{
	return ags->screen_height;
}

void NACT::select_cursor()
{
	ags->select_cursor();
}

[[noreturn]] void NACT::fatal(const char* format, ...) {
	char buf[512];
	va_list args;
	va_start(args, format);
	vsnprintf(buf, sizeof buf, format, args);
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fatal Error: %s", buf);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "system3", buf, g_window);
	exit(1);
}

NACT* NACT::create(const Config& config) {
	uint32 crc32_a = NACT::calc_crc32("ADISK.DAT", config.game_id);
	uint32 crc32_b = NACT::calc_crc32("BDISK.DAT", config.game_id);
	int sys_ver = NACT::get_sys_ver(crc32_a, crc32_b);
	switch (sys_ver) {
	case 1:
		return new NACT_Sys1(crc32_a, crc32_b, config);
	case 2:
		return new NACT_Sys2(crc32_a, crc32_b, config);
	default:
		return new NACT_Sys3(crc32_a, crc32_b, config);
	}
}
