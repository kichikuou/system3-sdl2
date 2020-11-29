/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT ]
*/

#include <stdarg.h>
#include "nact.h"
#include "ags.h"
#include "mako.h"
#include "dri.h"
#include "../fileio.h"
#include "crc32.h"

extern SDL_Window* g_window;

// 初期化

NACT::NACT(int sys_ver, uint32 crc32_a, uint32 crc32_b, const char* font_file, const MAKOConfig& mako_config)
	: sys_ver(sys_ver), crc32_a(crc32_a), crc32_b(crc32_b)
{
	platform_initialize();

	// AG00.DAT読み込み
	FILEIO* fio = new FILEIO();
	if(fio->Fopen("AG00.DAT", FILEIO_READ_BINARY)) {
		int d0, d1, d2, d3;
		char string[MAX_CAPTION];
		fio->Fgets(string, MAX_CAPTION);
		sscanf_s(string, "%d,%d,%d,%d", &d0, &d1, &d2, &d3);
		for(int i = 0; i < d1; i++) {
			// 動詞の読み込み
			fio->Fgets(string, MAX_CAPTION);
			memcpy(caption_verb[i], string, sizeof(string));
		}
		for(int i = 0; i < d2; i++) {
			// 目的語の読み込み
			fio->Fgets(string, MAX_CAPTION);
			memcpy(caption_obj[i], string, sizeof(string));
		}
		fio->Fclose();
	}
	delete fio;

	// ADISK.DAT
	if (crc32_a == CRC32_PROG_OMAKE)
		strcpy_s(adisk, 16, "AGAME.DAT");
	else
		strcpy_s(adisk, 16, "ADISK.DAT");

	// シナリオ管理
	scenario_data = NULL;
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
	text_skip_enb = false;
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
	ags = new AGS(this, font_file);
	mako = new MAKO(this, mako_config);

#ifdef USE_JOY
	// 入力初期化
	joy_num = joyGetNumDevs();
	if(joy_num) {
		joyGetDevCaps(JOYSTICKID1, &joycaps, sizeof(JOYCAPS));
	}
#endif

	terminate = false;
	restart_after_terminate = false;
}

NACT::~NACT()
{
	// 各種クラス開放
	if(ags) {
		delete ags;
	}
	if(mako) {
		delete mako;
	}

	// シナリオ開放
	if(scenario_data) {
		free(scenario_data);
	}

	platform_finalize();
}

bool NACT::mainloop()
{
	int sleep_cnt = 0;

	while(!terminate) {
		execute();
		// 512コマンド実行毎にSleep(10)
		if(!(sleep_cnt = (sleep_cnt + 1) & 0x1ff)) {
			sys_sleep(10);
		}
	}
	return restart_after_terminate;
}

// コマンドパーサ

void NACT::execute()
{
	// アドレスの確認
	if(scenario_addr < 2 || scenario_addr >= scenario_size) {
		fatal("Scenario error");
	}

	// 致命的なエラー発生 or 正常終了
	if(fatal_error) {
		if(!post_quit) {
			terminate = true;
		}
		post_quit = true;
		return;
	}

	if(sys_ver == 1 && scenario_page == 0 && scenario_addr == 2) {
		opening();
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
		default:
			if(cmd == 0x20 || (0xa1 <= cmd && cmd <= 0xdd)) {
				// message (1 byte)
				char string[3];
				string[0] = cmd;
				string[1] = '\0';
				ags->draw_text(string);
				if(crc32_a == CRC32_DPS || crc32_a == CRC32_DPS_SG || crc32_a == CRC32_DPS_SG2 || crc32_a == CRC32_DPS_SG3) {
					if(!ags->draw_menu) {
						text_refresh = false;
					}
				}
				
				if(!ags->draw_menu && text_wait_enb && cmd != 0x20) {
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
				if(!ags->draw_hankaku) {
					uint16 code = ags->convert_zenkaku(cmd);
					string[0] = code >> 8;
					string[1] = code & 0xff;
					string[2] = '\0';
				}
				output_console(string);
			} else if((0x81 <= cmd && cmd <= 0x9f) || 0xe0 <= cmd) {
				// message (2 bytes)
				char string[3];
				string[0] = cmd;
				string[1] = getd();
				string[2] = '\0';
				ags->draw_text(string);
				if(crc32_a == CRC32_DPS || crc32_a == CRC32_DPS_SG || crc32_a == CRC32_DPS_SG2 || crc32_a == CRC32_DPS_SG3) {
					if(!ags->draw_menu) {
						text_refresh = false;
					}
				}
				
				if(!ags->draw_menu && text_wait_enb) {
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
				output_console(string);
			} else {
				if(cmd >= 0x20 && cmd < 0x7f) {
					fatal("Unknown Command: '%c' at page = %d, addr = %d", cmd, scenario_page, prev_addr);
				} else {
					fatal("Unknown Command: %02x at page = %d, addr = %d", cmd, scenario_page, prev_addr);
				}
			}
			break;
	}
}

void NACT::load_scenario(int page)
{
	if(scenario_data) {
		free(scenario_data);
	}
	DRI* dri = new DRI();
	if((scenario_data = dri->load(adisk, page + 1, &scenario_size)) == NULL) {
		fatal("Cannot load scenario %d", page);
	}
	delete dri;
}

// 下位関数

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
	// 連打による誤クリック防止
	Uint32 dwTime = SDL_GetTicks();
	Uint32 dwWait = dwTime + 400;

	while(dwTime < dwWait) {
		if(terminate) {
			return;
		}
/*
		if(get_key() == 16) {
			break;
		}
*/
		sys_sleep(10);
		dwTime = SDL_GetTicks();
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
	ags->update_screen();
	SDL_Delay(ms);
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

void NACT::fatal(const char* format, ...) {
	char buf[512];
	va_list args;
	va_start(args, format);
	vsnprintf(buf, sizeof buf, format, args);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "system3", buf, g_window);
	fatal_error = true;
}

NACT* NACT::create(const char* game_id, const char* font_file, const MAKOConfig& mako_config) {
	uint32 crc32_a = NACT::calc_crc32("ADISK.DAT", game_id);
	uint32 crc32_b = NACT::calc_crc32("BDISK.DAT", game_id);
	int sys_ver = NACT::get_sys_ver(crc32_a, crc32_b);
	switch (sys_ver) {
	case 1:
		return new NACT_Sys1(crc32_a, crc32_b, font_file, mako_config);
	case 2:
		return new NACT_Sys2(crc32_a, crc32_b, font_file, mako_config);
	default:
		return new NACT_Sys3(crc32_a, crc32_b, font_file, mako_config);
	}
}
