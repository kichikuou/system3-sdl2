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
#include "resource.h"

extern SDL_Window* g_window;

namespace {

HWND get_hwnd(SDL_Window* window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);
	return info.info.win.window;
}

void init_menu()
{
	HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);
	HMENU hmenu = LoadMenu(hinst, MAKEINTRESOURCE(IDR_MENU1));
	SetMenu(get_hwnd(g_window), hmenu);
	// Let SDL recalc the window size, taking menu height into account.
	SDL_SetWindowSize(g_window, 640, 400);
}

void init_console(int sys_ver)
{
#if defined(_DEBUG_CONSOLE)
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	char title[] = "SYSTEM1 NACT Tracer";
	title[6] = '0' + sys_ver;
	SetConsoleTitle(title);
#endif
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

INT_PTR CALLBACK TextDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static NACT* nact;
	char string[64];
	wchar_t wstring[64];

	switch(msg) {
	case WM_CLOSE:
		EndDialog(hDlg, IDCANCEL);
		break;

	case WM_INITDIALOG:
		{
			// get this pointer
			nact = (NACT *)lParam;
			// init dialog
			swprintf_s(wstring, 64, L"文字列を入力してください（最大%d文字）", nact->tvar_maxlen);
			SetWindowTextW(GetDlgItem(hDlg, IDC_TEXT), wstring);

			char *oldstr = nact->encoding->toUtf8(nact->tvar[nact->tvar_index - 1]);
			MultiByteToWideChar(CP_UTF8, 0, oldstr, -1, wstring, 64);
			SetWindowTextW(GetDlgItem(hDlg, IDC_EDITBOX), wstring);
			EnableWindow(GetDlgItem(hDlg, IDOK), oldstr[0] != '\0');
			free(oldstr);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_EDITBOX:
			{
				int len = GetWindowTextLengthW(GetDlgItem(hDlg, IDC_EDITBOX));
				if (len == 0 || len > nact->tvar_maxlen) {
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
				char *newstr = nact->encoding->fromUtf8(string);
				strcpy_s(nact->tvar[nact->tvar_index - 1], 22, newstr);
				free(newstr);
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
}

} // namespace

void NACT::text_dialog()
{
	HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);
	DialogBoxParam(hinst, MAKEINTRESOURCE(IDD_DIALOG1), get_hwnd(g_window), TextDialogProc, (LPARAM)this);
}

void NACT::platform_initialize()
{
	init_menu();
	init_console(sys_ver);
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
			terminate = restart_after_terminate = true;
			break;
		case ID_EXIT:
			terminate = true;
			break;
		case ID_SCREEN_WINDOW:
			SDL_SetWindowFullscreen(g_window, 0);
			ags->flush_screen(true);
			break;
		case ID_SCREEN_FULL:
			SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
			ags->flush_screen(true);
			break;
		case ID_SOUND_FM:
			mako->select_sound(BGM_FM);
			break;
		case ID_SOUND_MIDI:
			mako->select_sound(BGM_MIDI);
			break;
		case ID_SOUND_CD:
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
		}
		break;
	case MM_MCINOTIFY:
		mako->on_mci_notify(msg);
		break;
	}
	return true;
}
