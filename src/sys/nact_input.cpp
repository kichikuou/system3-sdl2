/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - input ]
*/

#include "nact.h"

extern HWND g_hwnd;

uint8 NACT::get_key()
{
	uint8 val = 0;

	// キーボード＆マウス
	if(key[VK_UP    ] || key[VK_NUMPAD8]) val |= 0x01;
	if(key[VK_DOWN  ] || key[VK_NUMPAD2]) val |= 0x02;
	if(key[VK_LEFT  ] || key[VK_NUMPAD4]) val |= 0x04;
	if(key[VK_RIGHT ] || key[VK_NUMPAD6]) val |= 0x08;
	if(key[VK_RETURN] || key[VK_LBUTTON]) val |= 0x10;
	if(key[VK_SPACE ] || key[VK_RBUTTON]) val |= 0x20;
	if(key[VK_ESCAPE]                   ) val |= 0x40;
	if(key[VK_TAB   ]                   ) val |= 0x80;

	// マウス移動で方向入力はサポートしない

	// ジョイスティック
	if(joy_num > 0) {
		JOYINFO joyinfo;
		if(joyGetPos(JOYSTICKID1, &joyinfo) == JOYERR_NOERROR) {
			if(joyinfo.wYpos == joycaps.wYmin) val |= 0x01;
			if(joyinfo.wYpos == joycaps.wYmax) val |= 0x02;
			if(joyinfo.wXpos == joycaps.wXmin) val |= 0x04;
			if(joyinfo.wXpos == joycaps.wXmax) val |= 0x08;
			if(joyinfo.wButtons & JOY_BUTTON1) val |= 0x10;
			if(joyinfo.wButtons & JOY_BUTTON2) val |= 0x20;
			if(joyinfo.wButtons & JOY_BUTTON3) val |= 0x40;
			if(joyinfo.wButtons & JOY_BUTTON4) val |= 0x80;
		}
	}

	return val;
}

void NACT::get_cursor(int* x, int* y)
{
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(g_hwnd, &pt);
	*x = pt.x;
	*y = pt.y;
}

void NACT::set_cursor(int x, int y)
{
	POINT pt;
	pt.x = x;
	pt.y = y;
	ClientToScreen(g_hwnd, &pt);
	SetCursorPos(pt.x, pt.y);
}

