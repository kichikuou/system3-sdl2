#include <string>
#include "common.h"
#include "fileio.h"
#include "sys/nact.h"
#include "sys/mako.h"
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

SDL_Window* g_window;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

#ifdef __EMSCRIPTEN__
	// Stop SDL from calling emscripten_sleep() in functions that are called
	// indirectly, which does not work with ASYNCIFY_IGNORE_INDIRECT=1. For
	// details, see https://github.com/emscripten-core/emscripten/issues/10746.
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");
#endif
#ifdef __ANDROID__
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	Uint32 flags = SDL_WINDOW_FULLSCREEN;
#else
	Uint32 flags = 0;
#endif
	g_window = SDL_CreateWindow("Scenario Decoder SYSTEM3", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 400, flags);

	const char* font_file = NULL;
	MAKOConfig mako_config;
	const char* game_id = NULL;
	const char* save_dir = NULL;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-gamedir") == 0)
			chdir(argv[++i]);
		else if (strcmp(argv[i], "-noantialias") == 0)
			ags_setAntialiasedStringMode(0);
		else if (strcmp(argv[i], "-savedir") == 0)
			save_dir = argv[++i];
		else if (strcmp(argv[i], "-fontfile") == 0)
			font_file = argv[++i];
		else if (strcmp(argv[i], "-playlist") == 0)
			mako_config.playlist = argv[++i];
		else if (strcmp(argv[i], "-fm") == 0)
			mako_config.use_fm = true;
		else if (strcmp(argv[i], "-game") == 0)
			game_id = argv[++i];
	}

	// system3 初期化
	NACT* nact = NACT::create(game_id, font_file, mako_config);

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
		FILEIO::SetSaveDir(path);
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
	} else {
		WARNING("Cannot determine game id. crc32_a: %08x, crc32_b: %08x", nact->crc32_a, nact->crc32_b);
	}

#ifdef __EMSCRIPTEN__
	// Prevent SDL from calling emscripten_exit_fullscreen on visibilitychange
	emscripten_set_visibilitychange_callback(NULL, 0, NULL);

	EM_ASM( xsystem35.shell.windowSizeChanged(); );
#endif

	bool restart = true;
	while (restart) {
		restart = nact->mainloop();
		delete nact;
		if (restart)
			nact = NACT::create(game_id, font_file, mako_config);
	}

	SDL_DestroyWindow(g_window);
	SDL_Quit();

    return 0;
}
