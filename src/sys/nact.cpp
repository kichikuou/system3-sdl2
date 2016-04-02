/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT ]
*/

#include <windows.h>
#include "nact.h"
#include "ags.h"
#include "mako.h"
#include "dri.h"
#include "../fileio.h"

extern _TCHAR g_root[_MAX_PATH];

// 初期化

NACT::NACT()
{
	mouse_x = mouse_y = 0;

	// デバッグコンソール起動
	initialize_console();

	// SYSTEM3 初期化

	crc32 = calc_crc32();
	fatal_error = post_quit = false;

	// AG00.DAT読み込み
	FILEIO* fio = new FILEIO();
	_TCHAR file_path[_MAX_PATH];
	_stprintf_s(file_path, _MAX_PATH, _T("%sAG00.DAT"), g_root);

	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		int d0, d1, d2, d3;
		char string[MAX_CAPTION];
		fio->Fgets(string);
		sscanf_s(string, "%d,%d,%d,%d", &d0, &d1, &d2, &d3);
		for(int i = 0; i < d1; i++) {
			// 動詞の読み込み
			fio->Fgets(string);
			memcpy(caption_verb[i], string, sizeof(string));
		}
		for(int i = 0; i < d2; i++) {
			// 目的語の読み込み
			fio->Fgets(string);
			memcpy(caption_obj[i], string, sizeof(string));
		}
		fio->Fclose();
	}
	delete fio;

	// ADISK.DAT
#if defined(_PROG_OMAKE)
	_tcscpy_s(adisk, 16, _T("AGAME.DAT"));
#else
	_tcscpy_s(adisk, 16, _T("ADISK.DAT"));
#endif

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

	// SYETEM1 初期化
#if defined(_DPS)
	text_refresh = false;
	_tcscpy_s(tvar[0], 22, _T("カスタム"));
	_tcscpy_s(tvar[1], 22, _T("リーナス"));
	_tcscpy_s(tvar[2], 22, _T("かつみ"));
	_tcscpy_s(tvar[3], 22, _T("由美子"));
	_tcscpy_s(tvar[4], 22, _T("いつみ"));
	_tcscpy_s(tvar[5], 22, _T("ひとみ"));
	_tcscpy_s(tvar[6], 22, _T("真理子"));
#elif defined(_INTRUDER)
	paint_x = paint_y = map_page = 0;
#endif

	// 各種クラス生成
	ags = new AGS(this);
	mako = new MAKO(this);

#ifdef USE_JOY
	// 入力初期化
	joy_num = joyGetNumDevs();
	if(joy_num) {
		joyGetDevCaps(JOYSTICKID1, &joycaps, sizeof(JOYCAPS));
	}
#endif

	terminate = false;
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

	// デバッグコンソール開放
	release_console();
}

void NACT::mainloop()
{
	int sleep_cnt = 0;

	while(!terminate) {
		execute();
		// 512コマンド実行毎にSleep(10)
		if(!(sleep_cnt = (sleep_cnt + 1) & 0x1ff)) {
			SDL_Delay(10);
		} else {
			SDL_Delay(0);
		}
	}
}

// コマンドパーサ

void NACT::execute()
{
	// アドレスの確認
	if(scenario_addr < 2 || scenario_addr >= scenario_size) {
		fatal_error = true;
	}

	// 致命的なエラー発生 or 正常終了
	if(fatal_error) {
		if(!post_quit) {
			terminate = true;
		}
		post_quit = true;
		return;
	}

#if defined(_SYSTEM1)
	if(scenario_page == 0 && scenario_addr == 2) {
		opening();
	}
#endif

	// １コマンド実行
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
				char string[2];
				string[0] = cmd;
				string[1] = '\0';
				ags->draw_text(string);
#if defined(_DPS)
				if(!ags->draw_menu) {
					text_refresh = false;
				}
#endif
				
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
						SDL_Delay(10);
					}
				}

#if defined(_DEBUG_CONSOLE)
				output_console(string);
#endif
			} else if((0x81 <= cmd && cmd <= 0x9f) || 0xe0 <= cmd) {
				// message (2 bytes)
				char string[3];
				string[0] = cmd;
				string[1] = getd();
				string[2] = '\0';
				ags->draw_text(string);
#if defined(_DPS)
				if(!ags->draw_menu) {
					text_refresh = false;
				}
#endif
				
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
						SDL_Delay(10);
					}
				}

#if defined(_DEBUG_CONSOLE)
				output_console(string);
#endif
			} else {
				fatal_error = true;

#if defined(_DEBUG_CONSOLE)
				char log[128];
				memset(log, 0, sizeof(log));
				sprintf_s(log, 128, "\nFAILED: %2x, %d", cmd, scenario_addr);
				output_console(log);
#endif
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
		fatal_error = true;
	}
	delete dri;
}

// 下位関数

uint16 NACT::random(uint16 range)
{
	// xorhift32
	seed = seed ^ (seed << 13);
	seed = seed ^ (seed >> 17);
	seed = seed ^ (seed << 15);
	return (uint16)(((uint32)range * (seed & 0xffff)) >> 16) + 1;
}

// WinMainとのインターフェース

int NACT::get_screen_height()
{
	return ags->screen_height;
}

void NACT::notify_mci(int status)
{
	mako->notify_mci(status);
}

void NACT::select_cursor()
{
	ags->select_cursor();
}

void NACT::select_sound(int dev)
{
	// 強制的に音源を変更する
	int page = mako->current_music;
	int old_dev = (1 <= page && page <= 99 && mako->cd_track[page]) ? 1 : 0;

	for(int i = 1; i <= 99; i++) {
		mako->cd_track[i] = dev ? i : 0;
	}

	// デバイスが変更された場合は再演奏する
	if(dev != old_dev && page) {
		mako->stop_music();
		mako->play_music(page);
	}
}

// デバッグコンソール

void NACT::initialize_console()
{
#if defined(_DEBUG_CONSOLE)
	AllocConsole();
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#if defined(_SYSTEM1)
	SetConsoleTitle("SYSTEM1 NACT Tracer");
#elif defined(_SYSTEM2)
	SetConsoleTitle("SYSTEM2 NACT Tracer");
#else
	SetConsoleTitle("SYSTEM3 NACT Tracer");
#endif
#endif
}

void NACT::release_console()
{
#if defined(_DEBUG_CONSOLE)
	FreeConsole();
#endif
}

void NACT::output_console(char log[])
{
#if defined(_DEBUG_CONSOLE)
	DWORD dwWritten;
	WriteConsole(hConsole, log, strlen(log), &dwWritten, NULL);
#endif
}
