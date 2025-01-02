#include <windows.h>
#include <windowsx.h>
#undef ERROR
#include <time.h>
#include "nact.h"
#include "SDL_syswm.h"
#include "encoding.h"
#include "ags.h"
#include "mako.h"
#include "msgskip.h"
#include "texthook.h"
#include "resource.h"
#include "debugger/debugger.h"

extern SDL_Window* g_window;

namespace {

bool auto_copy_enabled = false;

HWND get_hwnd(SDL_Window* window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);
	return info.info.win.window;
}

void init_menu(bool mouse_move_enabled, const Config& config)
{
	HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);
	HMENU hmenu = LoadMenu(hinst, MAKEINTRESOURCE(IDR_MENU1));
	SetMenu(get_hwnd(g_window), hmenu);
	if (mouse_move_enabled)
		CheckMenuItem(hmenu, ID_OPTION_MOUSE_MOVE, MF_BYCOMMAND | MFS_CHECKED);
	if (config.scanline)
		CheckMenuItem(hmenu, ID_SCANLINE, MF_BYCOMMAND | MFS_CHECKED);
	auto_copy_enabled = config.texthook_mode == TexthookMode::COPY;
	if (auto_copy_enabled)
		CheckMenuItem(hmenu, ID_TEXT_AUTO_COPY, MF_BYCOMMAND | MFS_CHECKED);
}

void init_console(int sys_ver)
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	freopen("CONIN$", "r", stdin);
	char title[] = "SYSTEM1 Debug Console";
	title[6] = '0' + sys_ver;
	SetConsoleTitle(title);
}

void save_screenshot(AGS* ags)
{
	char pathbuf[MAX_PATH];
	time_t t = time(NULL);
	struct tm *lt = localtime(&t);
	strftime(pathbuf, sizeof(pathbuf), "system3-%Y%m%d-%H%M%S.bmp", lt);

	OPENFILENAME ofn = {sizeof(OPENFILENAME)};
	ofn.hwndOwner = get_hwnd(g_window);
	ofn.lpstrFilter = "Bitmap files (*.bmp)\0*.bmp\0All files (*.*)\0*.*\0";
	ofn.lpstrFile = pathbuf;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

	if (GetSaveFileName(&ofn))
		ags->save_screenshot(pathbuf);
}

} // namespace

void NACT::text_dialog()
{
	auto dialog_proc = [](HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) -> INT_PTR
#ifdef __MINGW32__
		CALLBACK  // MinGW requires this but MSVC doesn't like it
#endif
	{
		char string[64];
		wchar_t wstring[64];

		switch(msg) {
		case WM_CLOSE:
			EndDialog(hDlg, IDCANCEL);
			break;

		case WM_INITDIALOG:
			{
				// init dialog
				swprintf_s(wstring, 64, L"文字列を入力してください（最大%d文字）", g_nact->tvar_maxlen);
				SetWindowTextW(GetDlgItem(hDlg, IDC_TEXT), wstring);

				std::string oldstr = g_nact->encoding->toUtf8(g_nact->tvar[g_nact->tvar_index - 1]);
				MultiByteToWideChar(CP_UTF8, 0, oldstr.c_str(), -1, wstring, 64);
				SetWindowTextW(GetDlgItem(hDlg, IDC_EDITBOX), wstring);
				EnableWindow(GetDlgItem(hDlg, IDOK), oldstr[0] != '\0');
			}
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
			case IDC_EDITBOX:
				{
					int len = GetWindowTextLengthW(GetDlgItem(hDlg, IDC_EDITBOX));
					if (len == 0 || len > g_nact->tvar_maxlen) {
						EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
					} else {
						EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
					}
				}
				break;
			case IDOK:
				{
					GetDlgItemTextW(hDlg, IDC_EDITBOX, wstring, 64);
					WideCharToMultiByte(CP_UTF8, 0, wstring, -1, string, sizeof(string), NULL, NULL);
					strcpy_s(g_nact->tvar[g_nact->tvar_index - 1], 22, g_nact->encoding->fromUtf8(string).c_str());
					EndDialog(hDlg, IDOK);
				}
				break;
			default:
				return FALSE;
			}
			break;

		default:
			return FALSE;
		}
		return TRUE;
	};
	HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);
	DialogBoxParam(hinst, MAKEINTRESOURCE(IDD_DIALOG1), get_hwnd(g_window), dialog_proc, 0);
}

void NACT::platform_initialize()
{
	init_menu(mouse_move_enabled, config);
#ifndef _DEBUG_CONSOLE
	if (config.debugger_mode == DebuggerMode::CLI)
#endif
		init_console(game_id.sys_ver);
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
}

void NACT::platform_finalize()
{
#if defined(_DEBUG_CONSOLE)
	FreeConsole();
#endif
}

void NACT::output_console(const char *format, ...)
{
#ifdef ENABLE_DEBUGGER
	if (g_debugger) {
		va_list ap;
		va_start(ap, format);
		bool handled = g_debugger->console_vprintf(format, ap);
		va_end(ap);
		if (handled)
			return;
	}
#endif
#if defined(_DEBUG_CONSOLE)
	va_list ap;

	va_start(ap, format);
	vfprintf(stdout, format, ap);
	va_end(ap);
#endif
}

void NACT::set_skip_menu_state(bool enabled, bool checked)
{
	HWND hwnd = get_hwnd(g_window);
	HMENU hmenu = GetMenu(hwnd);
	EnableMenuItem(hmenu, ID_TEXT_SKIP, enabled ? MF_ENABLED : MF_GRAYED);
	CheckMenuItem(hmenu, ID_TEXT_SKIP, MF_BYCOMMAND |
				  checked ? MFS_CHECKED : MFS_UNCHECKED);
	DrawMenuBar(hwnd);
}

bool NACT::handle_platform_event(const SDL_Event& e)
{
	if (e.type != SDL_SYSWMEVENT)
		return false;
	const SDL_SysWMmsg* msg = e.syswm.msg;

	switch (msg->msg.win.msg) {
	case WM_COMMAND:
		switch (msg->msg.win.wParam) {
		case ID_SCREENSHOT:
			save_screenshot(ags);
			break;
		case ID_RESTART:
			quit(NACT_RESTART);
			break;
		case ID_EXIT:
			quit(0);
			break;
		case ID_SCREEN_WINDOW:
			SDL_SetWindowFullscreen(g_window, 0);
			ags->flush_screen(true);
			break;
		case ID_SCREEN_FULL:
			SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
			ags->flush_screen(true);
			break;
		case ID_SCANLINE:
			ags->set_scanline_mode(!ags->get_scanline_mode());
			CheckMenuItem(GetMenu(get_hwnd(g_window)), ID_SCANLINE, MF_BYCOMMAND |
						  ags->get_scanline_mode() ? MFS_CHECKED : MFS_UNCHECKED);
			break;
		case ID_OPTION_MOUSE_MOVE:
			mouse_move_enabled = !mouse_move_enabled;
			CheckMenuItem(GetMenu(get_hwnd(g_window)), ID_OPTION_MOUSE_MOVE,
						  MF_BYCOMMAND | mouse_move_enabled ? MFS_CHECKED : MFS_UNCHECKED);
			break;
		case ID_OPTION_FM:
			mako->select_sound(BGM_FM);
			break;
		case ID_OPTION_MIDI:
			mako->select_sound(BGM_MIDI);
			break;
		case ID_OPTION_CD:
			mako->select_sound(BGM_CD);
			break;
		case ID_TEXT_SKIP:
			msgskip->activate(!msgskip->is_activated());
			break;
		case ID_TEXT_WAIT:
			text_wait_enb = !text_wait_enb;
			CheckMenuItem(GetMenu(get_hwnd(g_window)), ID_TEXT_WAIT, MF_BYCOMMAND |
						  text_wait_enb ? MFS_CHECKED : MFS_UNCHECKED);
			break;
		case ID_TEXT_AUTO_COPY:
			auto_copy_enabled = !auto_copy_enabled;
			CheckMenuItem(GetMenu(get_hwnd(g_window)), ID_TEXT_AUTO_COPY, MF_BYCOMMAND |
						  auto_copy_enabled ? MFS_CHECKED : MFS_UNCHECKED);
			texthook_set_mode(auto_copy_enabled ? TexthookMode::COPY : TexthookMode::NONE);
			break;
		}
		break;
	case MM_MCINOTIFY:
		mako->on_mci_notify(msg);
		break;
	}
	return true;
}
