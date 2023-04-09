#include <string>
#include "common.h"
#include "config.h"
#include "fileio.h"
#include "sys/nact.h"
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#ifdef __SWITCH__
#include <switch.h>
#endif

SDL_Window* g_window;
SDL_Renderer* g_renderer;
NACT* g_nact;

int main(int argc, char *argv[])
{
#ifdef __SWITCH__
	romfsInit();
#endif
	Config config(argc, argv);

	if (!config.timidity_cfg.empty()) {
		SDL_setenv("TIMIDITY_CFG", config.timidity_cfg.c_str(), 1);
	}

	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
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
	Uint32 flags = SDL_WINDOW_RESIZABLE;
#endif
	g_window = SDL_CreateWindow("System3-sdl2 " SYSTEM3_VERSION, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 400, flags);
	g_renderer = SDL_CreateRenderer(g_window, -1, 0);

	// system3 初期化
	g_nact = NACT::create(config);

	if (!config.save_dir.empty()) {
		std::string path = config.save_dir;
		if (path[path.size() - 1] == '@') {
			path.pop_back();
			const char* gid = g_nact->get_game_id();
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

	const char* title = g_nact->get_title();
	if (title) {
		char buf[128];
		sprintf_s(buf, 128, "System3-sdl2 " SYSTEM3_VERSION ": %s", title);
#ifdef __EMSCRIPTEN__
		EM_ASM_ARGS({ xsystem35.shell.setWindowTitle(UTF8ToString($0)); }, buf);
#else
		SDL_SetWindowTitle(g_window, buf);
#endif
	} else {
		WARNING("Cannot determine game id. crc32_a: %08x, crc32_b: %08x", g_nact->crc32_a, g_nact->crc32_b);
	}

#ifdef __EMSCRIPTEN__
	// Prevent SDL from calling emscripten_exit_fullscreen on visibilitychange
	emscripten_set_visibilitychange_callback(NULL, 0, NULL);

	EM_ASM( xsystem35.shell.windowSizeChanged(); );
#endif

	bool restart = true;
	while (restart) {
		restart = g_nact->mainloop();
		delete g_nact;
		if (restart)
			g_nact = NACT::create(config);
	}

	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();

#ifdef __SWITCH__
	romfsExit();
#endif

    return 0;
}

#ifdef __EMSCRIPTEN__

extern "C"
EMSCRIPTEN_KEEPALIVE
void sys_restart()
{
	g_nact->quit(true);
}

#endif
