#include "nact.h"
#include <windows.h>
#include <windowsx.h>
#include "SDL_syswm.h"
#include "ags.h"
#include "mako.h"
#include "../res3/resource.h"

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

INT_PTR CALLBACK TextDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static NACT* nact;
	char string[64];
	wchar_t wstring[64];
	int pnt, cnt;
	
	switch(msg) {
		case WM_CLOSE:
			EndDialog(hDlg, IDCANCEL);
			break;
		case WM_INITDIALOG:
			// get this pointer
			nact = (NACT *)lParam;
			// init dialog
			swprintf_s(wstring, 64, L"文字列を入力してください（最大%d文字）", nact->tvar_maxlen);
			SetWindowTextW(GetDlgItem(hDlg, IDC_TEXT), wstring);
			Edit_SetText(GetDlgItem(hDlg, IDC_EDITBOX), nact->tvar[nact->tvar_index - 1]);
			if(nact->tvar[nact->tvar_index - 1][0] == '\0') {
				EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
			} else {
				EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
			}
			break;
		
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_EDITBOX:
					GetDlgItemText(hDlg, IDC_EDITBOX, string, 32);
					pnt = cnt = 0;
					while(string[pnt] != '\0') {
						unsigned char dat = string[pnt];
						if((0x81 <= dat && dat <= 0x9f) || 0xe0 <= dat) {
							pnt++;
						}
						pnt++;
						cnt++;
					}
					if(cnt == 0 || cnt > nact->tvar_maxlen) {
						EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
					} else {
						EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
					}
					break;
				case IDOK:
					GetDlgItemText(hDlg, IDC_EDITBOX, string, 32);
					strcpy_s(nact->tvar[nact->tvar_index - 1], 22, string);
					EndDialog(hDlg, IDOK);
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
	int r = DialogBoxParam(hinst, MAKEINTRESOURCE(IDD_DIALOG1), get_hwnd(g_window), TextDialogProc, (LPARAM)this);
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

void NACT::on_syswmevent(SDL_SysWMmsg* msg)
{
	switch (msg->msg.win.msg) {
	case WM_COMMAND:
		switch (msg->msg.win.wParam) {
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
			select_sound(0);
			break;
		case ID_SOUND_CD:
			select_sound(1);
			break;
		case ID_TEXT_SKIP:
			text_skip_enb = !text_skip_enb;
			CheckMenuItem(GetMenu(get_hwnd(g_window)), ID_TEXT_SKIP, MF_BYCOMMAND |
						  text_skip_enb ? MFS_CHECKED : MFS_UNCHECKED);
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
}
