
BOOL CALLBACK NACT::TextDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	_TCHAR string[64];
	char origin[22];

	switch(msg) {
		case WM_CLOSE:
			EndDialog(hDlg, IDCANCEL);
			break;

		case WM_INITDIALOG:
			{
				// ダイアログの初期化
				_TCHAR string[256];
				_stprintf_s(string, 258, _T("文字列を入力してください (最大%d文字)"), tvar_length);
				Edit_SetText(GetDlgItem(hDlg, IDC_TEXT), string);

				char origin[22];
				memcpy(origin, tvar[tvar_index - 1], 22);
				Edit_SetText(GetDlgItem(hDlg, IDC_EDITBOX), origin);
				if(origin[0] == '\0') {
					EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
				} else {
					EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
				}
			}
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_EDITBOX:
					{
						GetDlgItemText(hDlg, IDC_EDITBOX, string, 32);
						int p = 0, cnt = 0;
						while(string[p] != _T('\0')) {
							int c = string[p];
							if((0x81 <= c && c <= 0x9f) || 0xe0 <= c) {
								p++;
							}
							p++;
							cnt++;
						}
						if(cnt == 0 || cnt > tvar_length) {
							EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
						} else {
							EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
						}
					}
					break;

				case IDOK:
					GetDlgItemText(hDlg, IDC_EDITBOX, string, 32);
					strcpy(tvar[tvar_index - 1], string);
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

