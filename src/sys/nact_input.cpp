/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - input ]
*/

#include "nact.h"

static bool pump_events()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			return true;
		}
	}
	return false;
}

uint8 NACT::get_key()
{
	uint8 val = 0;

	if (pump_events())
		terminate = true;

	// キーボード＆マウス
	const Uint8* key = SDL_GetKeyboardState(NULL);
	Uint32 mouse = SDL_GetMouseState(NULL, NULL);
	if(key[SDL_SCANCODE_UP    ] || key[SDL_SCANCODE_KP_8 ]) val |= 0x01;
	if(key[SDL_SCANCODE_DOWN  ] || key[SDL_SCANCODE_KP_2 ]) val |= 0x02;
	if(key[SDL_SCANCODE_LEFT  ] || key[SDL_SCANCODE_KP_4 ]) val |= 0x04;
	if(key[SDL_SCANCODE_RIGHT ] || key[SDL_SCANCODE_KP_6 ]) val |= 0x08;
	if(key[SDL_SCANCODE_RETURN] || (mouse & SDL_BUTTON(1))) val |= 0x10;
	if(key[SDL_SCANCODE_SPACE ] || (mouse & SDL_BUTTON(3))) val |= 0x20;
	if(key[SDL_SCANCODE_ESCAPE]                           ) val |= 0x40;
	if(key[SDL_SCANCODE_TAB   ]                           ) val |= 0x80;

	// マウス移動で方向入力はサポートしない

#ifdef USE_JOY
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
#endif

	return val;
}

void NACT::get_cursor(int* x, int* y)
{
	if (pump_events())
		terminate = true;
	SDL_GetMouseState(x, y);
}

void NACT::set_cursor(int x, int y)
{
	// TODO: implement
}

