/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT ]
*/

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "nact.h"
#include "encoding.h"
#include "ags.h"
#include "mako.h"
#include "msgskip.h"
#include "texthook.h"
#include "dri.h"
#include "config.h"
#include "fileio.h"
#include "game_id.h"
#include "debugger/debugger.h"

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
	const char* ag00_name = game_id.is(GameId::RANCE2_HINT) ? "GG00.DAT" : "AG00.DAT";
	auto fio = FILEIO::open(ag00_name, FILEIO_READ_BINARY);
	if (fio) {
		int d0, d1, d2, d3;
		std::string line = fio->gets();
		if (sscanf_s(line.c_str(), "%d,%d,%d,%d", &d0, &d1, &d2, &d3) != 4)
			sys_error("AG00.DAT: parse error");
		for (int i = 0; i < d1; i++) {
			caption_verb[i] = fio->gets();
		}
		for (int i = 0; i < d2; i++) {
			caption_obj[i] = fio->gets();
		}
		fio.reset();
	}

	// ADISK.DAT
	const char* adisk_name =
		game_id.is(GameId::RANCE2_HINT) ? "GDISK.DAT" :
		game_id.is(GameId::PROG_OMAKE) ? "AGAME.DAT" :
		"ADISK.DAT";
	sco.open(adisk_name);
	if (!sco.loaded())
		sys_error("Cannot open %s", adisk_name);
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
#ifdef ENABLE_DEBUGGER
		if (g_debugger && g_debugger->trapped()) {
			sco.update_cmd_addr();
			g_debugger->repl(0);
		}
#endif
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
#ifdef ENABLE_DEBUGGER
		if (g_debugger)
			g_debugger->on_palette_change();
#endif
	}
	if(verb_obj && cmd != '[' && cmd != ':') {
		// 動詞-目的語メニューの表示
		sco.ungetd();
		cmd_open_verb();
		return;
	}

#ifdef ENABLE_DEBUGGER
	if (cmd == debugger::BREAKPOINT_INSTRUCTION) {
		if (!g_debugger)
			sys_error("Illegal BREAKPOINT instruction");
		cmd = g_debugger->handle_breakpoint(sco.page(), sco.cmd_addr());
	}
#endif

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

void NACT::cmd_calc()
{
	int index = sco.getd();
	if (0x80 <= index && index <= 0xbf) {
		index &= 0x3f;
	} else {
		index = ((index & 0x3f) << 8) | sco.getd();
	}
	var[index] = cali();

	TRACE("!var[%d]:%d!", index, var[index]);
}

void NACT::cmd_label_jump()
{
	int next_addr = sco.getw();
	sco.jump_to(next_addr);

	TRACE("@%x:", next_addr);
}

void NACT::cmd_label_call()
{
	int next_addr = sco.getw();
	sco.label_call(next_addr);

	TRACE("\\%x:", next_addr);
}

void NACT::cmd_page_jump()
{
	int next_page = cali();
	sco.page_jump(next_page, 2);

	TRACE("&%d:", next_page);
}

void NACT::cmd_page_call()
{
	int next_page = cali();
	sco.page_call(next_page);

	TRACE("%%%d:", next_page);
}

void NACT::cmd_set_menu()
{
	if(ags->draw_menu) {
		ags->menu.newline();
		ags->draw_menu = false;

		TRACE("$");
	} else {
		if (menu_items.empty()) {
			ags->clear_menu_window();
		}
		menu_items.emplace_back(sco.getw());
		ags->draw_menu = true;

		if (game_id.is(GameId::GAKUEN))
			menu_window = 2;

		TRACE("$%x,", menu_items.back().addr);
	}
}

void NACT::cmd_open_menu()
{
	TRACE("]");

	if (menu_items.empty()) {
		sco.jump_to(sco.default_addr());
		return;
	}

	if (game_id.is_system1_dps()) {
		if (!text_refresh) {
			cmd_a();
		}
	}

	int selection = menu_select(static_cast<int>(menu_items.size()));
	if (terminate)
		return;

	if (selection != -1) {
		sco.jump_to(menu_items[selection].addr);
	}
	menu_items.clear();
}

void NACT::cmd_set_verbobj()
{
	int verb = sco.getd();
	int obj = sco.getd();
	int addr = sco.getw();

	menu_items.emplace_back(addr, verb, obj);
	verb_obj = true;

	TRACE("[%x,%s,%s:", addr,
		encoding->toUtf8(caption_verb[verb].c_str()).c_str(),
		encoding->toUtf8(caption_obj[obj].c_str()).c_str());
}

void NACT::cmd_set_verbobj2()
{
	int condition = cali();
	int verb = sco.getd();
	int obj = sco.getd();
	int addr = sco.getw();

	if (condition) {
		menu_items.emplace_back(addr, verb, obj);
	}
	verb_obj = true;

	TRACE(":%d,%x,%s,%s:", condition, addr,
		encoding->toUtf8(caption_verb[verb].c_str()).c_str(),
		encoding->toUtf8(caption_obj[obj].c_str()).c_str());
}

void NACT::cmd_a()
{
	texthook_nextpage();

	TRACE("A");

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

	if (game_id.is_system1_dps()) {
		text_refresh = true;
	}
}

void NACT::cmd_f()
{
	TRACE("F");

	sco.jump_to(2);
}

void NACT::cmd_r()
{
	texthook_newline();

	TRACE("R");

	// ウィンドウの表示範囲外の場合は改ページ
	if(ags->return_text_line(text_window)) {
		cmd_a();
	}
}

void NACT::cmd_s()
{
	int page = sco.getd();

	TRACE("S %d:", page);

	if(page) {
		mako->play_music(page);
	} else {
		mako->stop_music();
	}
}

void NACT::cmd_x()
{
	int index = sco.getd();

	TRACE("X %d:", index);

	if(1 <= index && index <= 10) {
		ags->draw_text(tvar[index - 1]);
	}
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

	if (game_id.is_system1_dps()) {
		if(!ags->draw_menu) {
			text_refresh = false;
		}
	}
	if (!ags->draw_menu)
		msgskip->on_message(sco.page(), sco.current_addr());

	// TODO: Convert hankaku to zenkaku
	TRACE("%s", encoding->toUtf8(buf).c_str());
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

bool NACT::load(int index)
{
	char file_name[_MAX_PATH];
	snprintf(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + index - 1);

	auto fio = FILEIO::open(file_name, FILEIO_READ_BINARY | FILEIO_SAVEDATA);
	if (!fio)
		return false;
	fio->seek(112, SEEK_SET);

	int next_page = fio->getw() - 1;
	fio->getw();
	fio->getw();	// cg no?
	fio->getw();
	int next_music = fio->getw();
	fio->getw();
	int next_addr = fio->getw();
	fio->getw();
	for (int i = 0; i < 512; i++) {
		var[i] = fio->getw();
	}
	ags->load(fio.get());
	for (int i = 0; i < 10; i++) {
		fio->read(tvar[i], 22);
	}
	for (int i = 0; i < 30; i++) {
		for (int j = 0; j < 10; j++) {
			fio->read(tvar_stack[i][j], 22);
		}
	}
	for (int i = 0; i < 30; i++) {
		for (int j = 0; j < 20; j++) {
			var_stack[i][j] = fio->getw();
		}
	}
	fio.reset();

	sco.page_jump(next_page, next_addr);

	mako->play_music(next_music);
	return true;
}

bool NACT::save(int index, const char header[112])
{
	char file_name[_MAX_PATH];
	snprintf(file_name, _MAX_PATH, "ASLEEP_%c.DAT", 'A' + index - 1);

	auto fio = FILEIO::open(file_name, FILEIO_WRITE_BINARY | FILEIO_SAVEDATA);
	if (!fio)
		return false;

	fio->write(header, 112);
	fio->putw(sco.page() + 1);
	fio->putw(0);
	fio->putw(0);	// cg no?
	fio->putw(0);
	fio->putw(mako->current_music);
	fio->putw(0);
	fio->putw(sco.current_addr());
	fio->putw(0);
	for (int i = 0; i < 512; i++) {
		fio->putw(var[i]);
	}
	ags->save(fio.get());
	for (int i = 0; i < 10; i++) {
		fio->write(tvar[i], 22);
	}
	for (int i = 0; i < 30; i++) {
		for (int j = 0; j < 10; j++) {
			fio->write(tvar_stack[i][j], 22);
		}
	}
	for (int i = 0; i < 30; i++) {
		for (int j = 0; j < 20; j++) {
			fio->putw(var_stack[i][j]);
		}
	}

	assert(fio->tell() == 9510);
	return true;
}

int NACT::menu_select(int num_items)
{
	if (msgskip->get_flags() & MSGSKIP_STOP_ON_MENU)
		msgskip->activate(false);

	// メニュー表示
	ags->open_menu_window(menu_window);

	// マウス移動
	int sx, sy, ex;
	ags->get_menu_window_rect(menu_window, &sx, &sy, &ex, nullptr);
	int mx = ex - 16;
	int my = sy + 10;
	int height = ags->menu.font_size + 4;
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
#ifdef ENABLE_DEBUGGER
	if (g_debugger)
		g_debugger->on_sleep();
#endif
#ifdef __EMSCRIPTEN__
	emscripten_sleep(ms);
#else
	SDL_Delay(ms);
#endif
}

void NACT::set_string(int index, const char* value)
{
	const char *src = value;
	char *dst = tvar[index];
	int remaining = sizeof(tvar[0]) - 1;
	while (*src) {
		int len = encoding->mblen(*src);
		if (len > remaining)
			break;
		memcpy(dst, src, len);
		src += len;
		dst += len;
		remaining -= len;
	}
	*dst = '\0';
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
		return create_system1(config, game_id);
	case 2:
		return create_system2(config, game_id);
	default:
		return create_system3(config, game_id);
	}
}
