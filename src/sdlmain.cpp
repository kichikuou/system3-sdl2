#include "common.h"
#include "sys/nact.h"

_TCHAR g_root[_MAX_PATH];
HINSTANCE g_hinst;
SDL_Window* g_window;

int main(int argc, char *argv[])
{
	g_hinst = (HINSTANCE)::GetModuleHandle(NULL);

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

	g_window = SDL_CreateWindow(buf, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 400, 0);

	// MCI 初期化
	//initialize_mci();

	// system3 初期化
	NACT* nact = new NACT();

	_TCHAR title[128];
	if(nact->get_title(title, 128)) {
#if defined(_SYSTEM1)
		_stprintf_s(buf, 128, _T("Scenario Decoder SYSTEM1  -  %s"), title);
#elif defined(_SYSTEM2)
		_stprintf_s(buf, 128, _T("Scenario Decoder SYSTEM2  -  %s"), title);
#else
		_stprintf_s(buf, 128, _T("Scenario Decoder SYSTEM3  -  %s"), title);
#endif
		wchar_t wbuf[128];
		MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, 128);
		WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, 128, NULL, NULL);

		SDL_SetWindowTitle(g_window, buf);
	}

	nact->mainloop();

	SDL_DestroyWindow(g_window);

	// system3 開放
	delete nact;

	// MCI 開放
	//release_mci();

    return 0;
}
