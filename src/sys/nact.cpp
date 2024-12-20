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
#include "game_id.h"

extern SDL_Window* g_window;

// 初期化

NACT::NACT(const Config& config, const GameId& game_id)
	: config(config),
	  game_id(game_id),
	  encoding(Encoding::create(game_id.encoding)),
	  strings(config.get_strings(encoding.get(), game_id.language == ENGLISH)),
	  seed(SDL_GetTicks())
{
	platform_initialize();

	// AG00.DAT読み込み
	auto fio = FILEIO::open("AG00.DAT", FILEIO_READ_BINARY);
	if (fio) {
		int d0, d1, d2, d3;
		char string[MAX_CAPTION];
		fio->ag00_gets(string, MAX_CAPTION);
		if (sscanf_s(string, "%d,%d,%d,%d", &d0, &d1, &d2, &d3) != 4)
			sys_error("AG00.DAT: parse error");
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
	if (game_id.crc32_a == CRC32_PROG_OMAKE)
		sco.open("AGAME.DAT");
	else
		sco.open("ADISK.DAT");
	sco.page_jump(0, 2);

	// 各種クラス生成
	ags = new AGS(config, game_id);
	mako = new MAKO(config, game_id);
	msgskip = new MsgSkip();

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
	if (game_id.sys_ver == 1 && sco.page() == 0 && sco.current_addr() == 2) {
		opening();
	}

	// １コマンド実行
	uint8 cmd = sco.fetch_command();

	if(set_palette && cmd != 'P') {
		// パレット設定が終わった
		ags->flush_screen(true);
		set_palette = false;
	}
	if(verb_obj && cmd != '[' && cmd != ':') {
		// 動詞-目的語メニューの表示
		sco.ungetd();
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
				message(cmd);
			} else {
				sco.unknown_command(cmd);
			}
			break;
	}
}

void NACT::cmd_label_jump()
{
	int next_addr = sco.getw();
	sco.jump_to(next_addr);

	output_console("\n@%x:", next_addr);
}

void NACT::cmd_label_call()
{
	int next_addr = sco.getw();
	sco.label_call(next_addr);

	output_console("\n\\%x:", next_addr);
}

void NACT::cmd_page_jump()
{
	int next_page = cali();
	sco.page_jump(next_page, 2);

	output_console("\n&%d:", next_page);
}

void NACT::cmd_page_call()
{
	int next_page = cali();
	sco.page_call(next_page);

	output_console("\n%%%d:", next_page);
}

void NACT::message(uint8_t first_byte)
{
	char buf[200];
	if (first_byte == '\'' || first_byte == '"') {  // SysEng
		sco.get_syseng_string(buf, sizeof(buf), encoding.get(), first_byte);
	} else {
		int i = 0;
		uint8_t c = first_byte;
		while (is_message(c)) {
			int len = encoding->mblen(c);
			buf[i++] = c;
			for (int j = 1; j < len; ++j)
				buf[i++] = sco.getd();
			c = sco.getd();
		}
		sco.ungetd();
		buf[i] = '\0';
	}

	ags->draw_text(buf, text_wait_enb);

	if (game_id.crc32_a == CRC32_DPS || game_id.crc32_a == CRC32_DPS_SG || game_id.crc32_a == CRC32_DPS_SG2 || game_id.crc32_a == CRC32_DPS_SG3) {
		if(!ags->draw_menu) {
			text_refresh = false;
		}
	}
	if (!ags->draw_menu)
		msgskip->on_message(sco.page(), sco.current_addr());

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

NACT* NACT::create(const Config& config, const GameId& game_id) {
	switch (game_id.sys_ver) {
	case 1:
		return new NACT_Sys1(config, game_id);
	case 2:
		return new NACT_Sys2(config, game_id);
	default:
		return new NACT_Sys3(config, game_id);
	}
}
