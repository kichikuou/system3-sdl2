/*
	ALICE SOFT SYSTEM 3 for Win32

	[ WinMain ]
*/

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <commctrl.h>
#include <stdio.h>
#include <tchar.h>
#include "common.h"
#if defined(_SYSTEM1)
#include "res1/resource.h"
#elif defined(_SYSTEM2)
#include "res2/resource.h"
#elif defined(_SYSTEM3)
#include "res3/resource.h"
#endif
#include "sys/nact.h"

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

// フルスクリーン
void set_window(HWND hWnd, BOOL full);
void show_menu_bar(HWND hWnd);
void hide_menu_bar(HWND hWnd);

BOOL fullscreen_now = FALSE;
BOOL menuloop_now = FALSE;

// MCIデバイス
void initialize_mci();
void release_mci();
void play_mci(HWND hWnd, int index);
void stop_mci();

HINSTANCE g_hinst;
HWND g_hwnd = NULL;
HMENU g_hmenu = NULL;

NACT *nact;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow)
{
	g_hinst = hInstance;

	// ウィンドウの表示位置を取得する
	RECT rect = {0, 0, 640, 400};
	AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, TRUE);
	
	HDC hdc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	int width = GetDeviceCaps(hdc, HORZRES);
	int height = GetDeviceCaps(hdc, VERTRES);
	int dest_x = (int)((width - (rect.right - rect.left)) / 2);
	int dest_y = (int)((height - (rect.bottom - rect.top)) / 2);
//	dest_x = (dest_x < 0) ? 0 : dest_x;
	dest_y = (dest_y < 0) ? 0 : dest_y;

	// ウィンドウ生成
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = g_hinst;
	wndclass.hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hCursor = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wndclass.lpszClassName = _T("CWINDOW");
	RegisterClass(&wndclass);

	// ウィンドウ表示
	_TCHAR buf[128];
#if defined(_SYSTEM1)
	_tcscpy_s(buf, 128, _T("Scenario Decoder SYSTEM1"));
#elif defined(_SYSTEM2)
	_tcscpy_s(buf, 128, _T("Scenario Decoder SYSTEM2"));
#elif defined(_SYSTEM3)
	_tcscpy_s(buf, 128, _T("Scenario Decoder SYSTEM3"));
#endif

	g_hwnd = CreateWindow(_T("CWINDOW"), buf,
	                      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
	                      dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top,
	                      NULL, NULL, g_hinst, NULL);
	ShowWindow(g_hwnd, iCmdShow);
	UpdateWindow(g_hwnd);

	// メニュー表示
	show_menu_bar(g_hwnd);

	// MCI 初期化
	initialize_mci();

	// system3 初期化
	nact = new NACT();

	_TCHAR title[128];
	if(nact->get_title(title, 128)) {
#if defined(_SYSTEM1)
		_stprintf_s(buf, 128, _T("Scenario Decoder SYSTEM1  -  %s"), title);
#elif defined(_SYSTEM2)
		_stprintf_s(buf, 128, _T("Scenario Decoder SYSTEM2  -  %s"), title);
#elif defined(_SYSTEM3)
		_stprintf_s(buf, 128, _T("Scenario Decoder SYSTEM3  -  %s"), title);
#endif
		SetWindowText(g_hwnd, buf);
	}

	// メインループ
	MSG msg;
	while(GetMessage(&msg,NULL,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// system3 開放
	if(nact) {
		delete nact;
	}

	// MCI 開放
	release_mci();

	// フルスクリーンから復帰
	if(fullscreen_now) {
		ChangeDisplaySettings(NULL, 0);
	}

#if defined(_DEBUG)
	// メモリリーク検出
	_CrtDumpMemoryLeaks();
#endif

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	HDC hdc;
	PAINTSTRUCT ps;

	switch(iMsg) {
		case WM_CLOSE:
			if(g_hmenu != NULL && IsMenu(g_hmenu)) {
				DestroyMenu(g_hmenu);
			}
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_PAINT:
			if(GetUpdateRect(hWnd, &rect, TRUE)) {
				hdc = BeginPaint(hWnd, &ps);
				if(nact) {
					nact->update_screen(hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
				}
				EndPaint(hWnd, &ps);
			}
			return 0;

		case WM_ENTERMENULOOP:
			menuloop_now = true;
			break;

		case WM_EXITMENULOOP:
			if(fullscreen_now && menuloop_now) {
				hide_menu_bar(hWnd);
			}
			menuloop_now = false;
			break;

		case WM_MOUSEMOVE:
			if(fullscreen_now && !menuloop_now) {
				POINTS p = MAKEPOINTS(lParam);
				if(p.y == 0) {
					show_menu_bar(hWnd);
				} else if(p.y > 32) {
					hide_menu_bar(hWnd);
				}
			}
			break;

		case WM_LBUTTONDOWN:
			if(nact) {
				nact->key_down(VK_LBUTTON);
			}
			break;

		case WM_LBUTTONUP:
			if(nact) {
				nact->key_up(VK_LBUTTON);
			}
			break;

		case WM_RBUTTONDOWN:
			if(nact) {
				nact->key_down(VK_RBUTTON);
			}
			break;

		case WM_RBUTTONUP:
			if(nact) {
				nact->key_up(VK_RBUTTON);
			}
			break;

		case WM_KILLFOCUS:
			if(nact) {
				nact->key_up(-1);
			}
			break;

		case WM_KEYDOWN:
			if(nact) {
				uint8 code = LOBYTE(wParam);
				nact->key_down(code);
			}
			break;

		case WM_KEYUP:
			if(nact) {
				uint8 code = LOBYTE(wParam);
				nact->key_up(code);
			}
			break;

		case WM_SETCURSOR:
			if(nact) {
				nact->select_cursor();
			}
			break;

		case MM_MCINOTIFY:
			if(nact) {
				if(wParam == MCI_NOTIFY_SUCCESSFUL) {
					nact->notify_mci(0);
				} else if(wParam == MCI_NOTIFY_FAILURE) {
					nact->notify_mci(-1);
				}
			}
			break;

		case WM_USER:
			if(wParam) {
				play_mci(hWnd, wParam);
			} else {
				stop_mci();
			}
			break;

		case WM_USER + 2:
//			if(fullscreen_now) {
//				set_window(hWnd, FALSE);
//			}
			rect.left = rect.top = 0;
			rect.right = 640;
			rect.bottom = wParam;
			AdjustWindowRectEx(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, TRUE, 0);
			SetWindowPos(hWnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
			break;

		case WM_ACTIVATEAPP:
			if(fullscreen_now) {
				set_window(hWnd, FALSE);
			}
			break;

		case WM_INITMENUPOPUP:
			if(LOWORD(lParam) == 1) {
				EnableMenuItem((HMENU)wParam, ID_SCREEN_WINDOW, fullscreen_now ? MF_ENABLED : MF_GRAYED);
				EnableMenuItem((HMENU)wParam, ID_SCREEN_FULL, fullscreen_now ? MF_GRAYED : MF_ENABLED);
			} else if(LOWORD(lParam) == 3) {
				if(nact) {
					CheckMenuItem((HMENU)wParam, ID_TEXT_SKIP, nact->text_skip_enb ? MF_CHECKED : MF_UNCHECKED);
					CheckMenuItem((HMENU)wParam, ID_TEXT_WAIT, nact->text_wait_enb ? MF_CHECKED : MF_UNCHECKED);
				}
			}
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_RESTART:
					// restart nact
					if(nact) {
						delete nact;
					}
					nact = new NACT();
					
					// update screen
					rect.left = rect.top = 0;
					rect.right = 640;
					rect.bottom = 480;
					InvalidateRect(hWnd, &rect, FALSE);
					UpdateWindow(g_hwnd);
					break;

				case ID_EXIT:
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					break;

				case ID_SCREEN_WINDOW:
					if(fullscreen_now) {
						set_window(hWnd, FALSE);
					}
					break;

				case ID_SCREEN_FULL:
					if(!fullscreen_now) {
						set_window(hWnd, TRUE);
					}
					break;

				case ID_SOUND_FM:
					if(nact) {
						nact->select_sound(0);
					}
					break;

				case ID_SOUND_CD:
					if(nact) {
						nact->select_sound(1);
					}
					break;

				case ID_TEXT_SKIP:
					if(nact) {
						nact->text_skip_enb = !nact->text_skip_enb;
					}
					break;

				case ID_TEXT_WAIT:
					if(nact) {
						nact->text_wait_enb = !nact->text_wait_enb;
					}
					break;
			}
			break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam) ;
}

// フルスクリーン

void set_window(HWND hWnd, BOOL full)
{
	static LONG style = WS_VISIBLE;
	static int dest_x = 0, dest_y = 0;
	WINDOWPLACEMENT place;
	place.length = sizeof(WINDOWPLACEMENT);

	if(full && !fullscreen_now) {
		// フルスクリーンに移行
		DEVMODE dev;
		HDC hdc = GetDC(NULL);
		ZeroMemory(&dev, sizeof(dev));
		dev.dmSize = sizeof(dev);
		dev.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		dev.dmBitsPerPel = GetDeviceCaps(hdc, BITSPIXEL);
		dev.dmPelsWidth = 640;
		dev.dmPelsHeight = 480;
		ReleaseDC(NULL, hdc);

		if(ChangeDisplaySettings(&dev, CDS_TEST) == DISP_CHANGE_SUCCESSFUL) {
			GetWindowPlacement(hWnd, &place);
			dest_x = place.rcNormalPosition.left;
			dest_y = place.rcNormalPosition.top;
			ChangeDisplaySettings(&dev, CDS_FULLSCREEN);
			style = GetWindowLong(hWnd, GWL_STYLE);
			SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, 640, 480, SWP_SHOWWINDOW);
			SetCursorPos(320, 200);
			fullscreen_now = TRUE;
			hide_menu_bar(hWnd);
		}
	} else if(!full && fullscreen_now) {
		// フルスクリーンから復帰
		RECT rect = {0, 0, 640, nact ? nact->get_screen_height() : 400};
//		if(nact) {
//			rect.bottom = nact->get_screen_height();
//		}
		AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, TRUE);
		ChangeDisplaySettings(NULL, 0);
		SetWindowLong(hWnd, GWL_STYLE, style);
		SetWindowPos(hWnd, HWND_TOP, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
		fullscreen_now = FALSE;
		show_menu_bar(g_hwnd);
	}
}

void show_menu_bar(HWND hWnd)
{
	if(!(g_hmenu != NULL && IsMenu(g_hmenu))) {
		g_hmenu = LoadMenu((HINSTANCE)GetModuleHandle(0), MAKEINTRESOURCE(IDR_MENU1));
		SetMenu(hWnd, g_hmenu);
	}
}

void hide_menu_bar(HWND hWnd)
{
	if(g_hmenu != NULL && IsMenu(g_hmenu)) {
		SetMenu(hWnd, NULL);
		DestroyMenu(g_hmenu);
		g_hmenu = NULL;
	}
}

// MCIデバイス

void initialize_mci()
{
	mciSendString(_T("open cdaudio"), NULL, 0, NULL);
	mciSendString(_T("set cdaudio time format tmsf"), NULL, 0, NULL);
	mciSendString(_T("set cdaudio audio all on"), NULL, 0, NULL);
	mciSendString(_T("stop cdaudio"), NULL, 0, NULL);
}

void release_mci()
{
	mciSendString(_T("stop cdaudio"), NULL, 0, NULL);
	mciSendString(_T("close cdaudio"), NULL, 0, NULL);
}

void play_mci(HWND hWnd, int index)
{
	_TCHAR string[_MAX_PATH];
	
	_stprintf_s(string, _MAX_PATH, _T("play cdaudio from %d to %d notify"), index, index + 1);
	mciSendString(string, NULL, 0, hWnd);
}

void stop_mci()
{
	mciSendString(_T("stop cdaudio"), NULL, 0, NULL);
}

