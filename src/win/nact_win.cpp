#include "nact.h"
#include <windows.h>
#include <windowsx.h>
#include "SDL_syswm.h"
#include "../res3/resource.h"

extern SDL_Window* g_window;

static INT_PTR CALLBACK TextDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
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

void NACT::text_dialog()
{
	HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);
	SDL_SysWMinfo info;
	SDL_GetWindowWMInfo(g_window, &info);
	int r = DialogBoxParam(hinst, MAKEINTRESOURCE(IDD_DIALOG1), info.info.win.window, TextDialogProc, (LPARAM)this);
}

void NACT::initialize_console()
{
#if defined(_DEBUG_CONSOLE)
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	char title[] = "SYSTEM1 NACT Tracer";
	title[6] = '0' + sys_ver;
	SetConsoleTitle(title);
#endif
}

void NACT::release_console()
{
#if defined(_DEBUG_CONSOLE)
	FreeConsole();
#endif
}

void NACT::output_console(char log[])
{
#if defined(_DEBUG_CONSOLE)
	fputs(log, stdout);
#endif
}
