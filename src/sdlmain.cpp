#include "common.h"
#include "fileio.h"
#include "sys/nact.h"
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

extern "C" {
	void ags_setAntialiasedStringMode(int on);
}

SDL_Window* g_window;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	// ウィンドウ表示
	_TCHAR buf[128];
#if defined(_SYSTEM1)
	_tcscpy_s(buf, 128, _T("Scenario Decoder SYSTEM1"));
#elif defined(_SYSTEM2)
	_tcscpy_s(buf, 128, _T("Scenario Decoder SYSTEM2"));
#else
	_tcscpy_s(buf, 128, _T("Scenario Decoder SYSTEM3"));
#endif

#ifdef __ANDROID__
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	Uint32 flags = SDL_WINDOW_FULLSCREEN;
#else
	Uint32 flags = 0;
#endif
	g_window = SDL_CreateWindow(buf, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 400, flags);

	const char* font_file = DEFAULT_FONT_PATH "MTLc3m.ttf";
	const char* game_id = NULL;
	char save_dir[256] = "";

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-gamedir") == 0)
			chdir(argv[++i]);
		else if (strcmp(argv[i], "-antialias") == 0)
			ags_setAntialiasedStringMode(1);
		else if (strcmp(argv[i], "-savedir") == 0)
			strcpy(save_dir, argv[++i]);
		else if (strcmp(argv[i], "-fontfile") == 0)
			font_file = argv[++i];
		else if (strcmp(argv[i], "-game") == 0)
			game_id = argv[++i];
	}

	// system3 初期化
	NACT* nact = new NACT(game_id, font_file);

	if (save_dir[0]) {
#ifdef __ANDROID__
		const char* gid = nact->get_game_id();
		if (gid) {
			strcat(save_dir, gid);
			mkdir(save_dir, 0777);
			strcat(save_dir, "/");
		}
#endif
		FILEIO::SetSaveDir(save_dir);
	}

	const _TCHAR* title = nact->get_title();
	if (title) {
#if defined(_SYSTEM1)
		_stprintf_s(buf, 128, _T("Scenario Decoder SYSTEM1: %s"), title);
#elif defined(_SYSTEM2)
		_stprintf_s(buf, 128, _T("Scenario Decoder SYSTEM2: %s"), title);
#else
		_stprintf_s(buf, 128, _T("Scenario Decoder SYSTEM3: %s"), title);
#endif
#ifdef __EMSCRIPTEN__
		EM_ASM_ARGS({ xsystem35.shell.setWindowTitle(UTF8ToString($0)); }, buf);
#else
		SDL_SetWindowTitle(g_window, buf);
#endif
	}

#ifdef __EMSCRIPTEN__
	// Prevent SDL from calling emscripten_exit_fullscreen on visibilitychange
	emscripten_set_visibilitychange_callback(NULL, 0, NULL);

	EM_ASM( xsystem35.shell.windowSizeChanged(); );
#endif

	nact->mainloop();

	SDL_DestroyWindow(g_window);
	SDL_Quit();

	// system3 開放
	delete nact;

	// MCI 開放
	//release_mci();

    return 0;
}
