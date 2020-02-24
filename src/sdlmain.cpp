#include <string>
#include <unistd.h>
#include "common.h"
#include "fileio.h"
#include "sys/nact.h"
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

#ifdef __ANDROID__
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	Uint32 flags = SDL_WINDOW_FULLSCREEN;
#else
	Uint32 flags = 0;
#endif
	g_window = SDL_CreateWindow("Scenario Decoder SYSTEM3", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 400, flags);

	const char* font_file = DEFAULT_FONT_PATH "MTLc3m.ttf";
	const char* playlist = NULL;
	const char* game_id = NULL;
	const char* save_dir = NULL;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-gamedir") == 0)
			chdir(argv[++i]);
		else if (strcmp(argv[i], "-antialias") == 0)
			ags_setAntialiasedStringMode(1);
		else if (strcmp(argv[i], "-savedir") == 0)
			save_dir = argv[++i];
		else if (strcmp(argv[i], "-fontfile") == 0)
			font_file = argv[++i];
		else if (strcmp(argv[i], "-playlist") == 0)
			playlist = argv[++i];
		else if (strcmp(argv[i], "-game") == 0)
			game_id = argv[++i];
	}

	// system3 初期化
	NACT* nact = NACT::create(game_id, font_file, playlist);

	if (save_dir && save_dir[0]) {
		std::string path(save_dir);
		if (path[path.size() - 1] == '@') {
			path.pop_back();
			const char* gid = nact->get_game_id();
			if (gid) {
				path += gid;
#ifdef WIN32
				mkdir(path.c_str());
#else
				mkdir(path.c_str(), 0777);
#endif
			}
		}
		if (path[path.size() - 1] != '/') {
			path += '/';
		}
		FILEIO::SetSaveDir(path.c_str());
	}

	const char* title = nact->get_title();
	if (title) {
		char buf[128];
		sprintf_s(buf, 128, "Scenario Decoder SYSTEM%d: %s", nact->sys_ver, title);
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
