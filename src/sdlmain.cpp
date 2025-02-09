#include <SDL3/SDL_main.h>
#include <string>
#include "common.h"
#include "config.h"
#include "fileio.h"
#include "texthook.h"
#include "nact.h"
#include "debugger/debugger.h"
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
std::unique_ptr<NACT> g_nact;

namespace {

SDL_Window* create_window(const GameId& game_id)
{
	std::string title = "System3-sdl2 " SYSTEM3_VERSION;
	if (game_id.title) {
		title += ": ";
		title += game_id.title;
	}

	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#ifdef __EMSCRIPTEN__
	// Stop SDL from calling emscripten_sleep() in functions that are called
	// indirectly, which does not work with ASYNCIFY_IGNORE_INDIRECT=1. For
	// details, see https://github.com/emscripten-core/emscripten/issues/10746.
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");
#endif

#ifdef __ANDROID__
	SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	Uint32 flags = SDL_WINDOW_FULLSCREEN;
#else
	Uint32 flags = SDL_WINDOW_RESIZABLE;
#endif

#ifdef __EMSCRIPTEN__
	EM_ASM_ARGS({ xsystem35.shell.setWindowTitle(UTF8ToString($0), UTF8ToString($1)); }, title.c_str(), game_id.name);
	const char *window_title = NULL;  // Don't let SDL change document.title
#else
	const char *window_title = title.c_str();
#endif
	float display_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	return SDL_CreateWindow(window_title, 640 * display_scale, 400 * display_scale, flags);
}

} // namespace

uint32_t sdl_custom_event_type;

int main(int argc, char *argv[])
{
#ifdef __SWITCH__
	romfsInit();
#endif
	Config config(argc, argv);
	if (config.print_version) {
		puts(SYSTEM3_VERSION);
		return 0;
	}
	GameId game_id(config);

	g_window = create_window(game_id);
	g_renderer = SDL_CreateRenderer(g_window, NULL);
	sdl_custom_event_type = SDL_RegisterEvents(1);

	// system3 初期化
	g_nact.reset(NACT::create(config, game_id));

	if (!config.save_dir.empty()) {
		std::string path = config.save_dir;
		if (path[path.size() - 1] == '@') {
			path.pop_back();
			if (game_id.name) {
				path += game_id.name;
#ifdef WIN32
				mkdir(path.c_str());
#else
				mkdir(path.c_str(), 0777);
#endif
			}
		}
		FILEIO::set_savedir(path);
	}

#ifdef __EMSCRIPTEN__
	// Prevent SDL from calling emscripten_exit_fullscreen on visibilitychange
	emscripten_set_visibilitychange_callback(NULL, 0, NULL);

	EM_ASM( xsystem35.shell.windowSizeChanged(); );
#endif

	texthook_set_mode(config.texthook_mode);
	texthook_set_suppression_list(config.texthook_suppressions.c_str());

#ifdef ENABLE_DEBUGGER
	if (config.debugger_mode != DebuggerMode::DISABLED) {
		g_debugger = std::make_unique<debugger::Debugger>("ADISK.DAT.symbols", config.debugger_mode);
		g_debugger->init();
	}
#endif

	int exit_code = 0;
	while (g_nact) {
		exit_code = g_nact->mainloop();
		g_nact.reset();
		if (exit_code == NACT_RESTART)
			g_nact.reset(NACT::create(config, game_id));
	}

	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	SDL_Quit();

#ifdef __SWITCH__
	romfsExit();
#endif

    return exit_code;
}

[[noreturn]] void sys_error(const char* format, ...) {
	char buf[512];
	va_list args;
	va_start(args, format);
	vsnprintf(buf, sizeof buf, format, args);
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fatal Error: %s", buf);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "system3", buf, g_window);
	exit(1);
}

void sys_warning(const char* format, ...)
{
	va_list args;
	va_start(args, format);
#ifdef ENABLE_DEBUGGER
	if (g_debugger && g_debugger->console_vprintf(format, args))
		return;
#endif
	char buf[1024];
	vsnprintf(buf, sizeof buf, format, args);
	SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", buf);
}

#ifdef __EMSCRIPTEN__

extern "C"
EMSCRIPTEN_KEEPALIVE
void sys_restart()
{
	g_nact->quit(NACT_RESTART);
}

#endif
