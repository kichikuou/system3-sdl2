#include "nact.h"
#include <windows.h>
#include <windowsx.h>
#include "SDL_syswm.h"
#if defined(_SYSTEM1)
#include "../res1/resource.h"
#elif defined(_SYSTEM2)
#include "../res2/resource.h"
#else
#include "../res3/resource.h"
#endif

extern SDL_Window* g_window;
extern HINSTANCE g_hinst;

static BOOL CALLBACK TextDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static NACT* nact;
	char string[64];
	int pnt, cnt;
	
	switch(msg) {
		case WM_CLOSE:
			EndDialog(hDlg, IDCANCEL);
			break;
		case WM_INITDIALOG:
			// get this pointer
			nact = (NACT *)lParam;
			// init dialog
			sprintf_s(string, 64, "•¶Žš—ñ‚ð“ü—Í‚µ‚Ä‚­‚¾‚³‚¢iÅ‘å%d•¶Žšj", nact->tvar_maxlen);
			Edit_SetText(GetDlgItem(hDlg, IDC_TEXT), string);
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
	SDL_SysWMinfo info;
	SDL_GetWindowWMInfo(g_window, &info);
	int r = DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG1), info.info.win.window, TextDialogProc, (LPARAM)this);
}
