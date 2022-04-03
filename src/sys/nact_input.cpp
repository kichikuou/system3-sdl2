/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - input ]
*/

#ifdef _WIN32
#include <windows.h>
#undef ERROR
#endif
#include <SDL_syswm.h>
#include "nact.h"
#include "ags.h"
#include "texthook.h"

extern SDL_Window* g_window;
static int mousex, mousey, fingers;

void NACT::pump_events()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (handle_platform_event(e))
			continue;

		switch (e.type) {
		case SDL_QUIT:
			terminate = true;
			break;

		case SDL_WINDOWEVENT:
			switch (e.window.event) {
			case SDL_WINDOWEVENT_EXPOSED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				ags->flush_screen(false);
				break;
			}
			break;

		case SDL_MOUSEMOTION:
			mousex = e.motion.x;
			mousey = e.motion.y;
			break;

		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION:
			mousex = e.tfinger.x * 640;
			mousey = e.tfinger.y * ags->screen_height;
			fingers = SDL_GetNumTouchFingers(e.tfinger.touchId);
			break;

		case SDL_APP_DIDENTERFOREGROUND:
			ags->flush_screen(false);
			break;
		}
	}
}

uint8 NACT::get_key()
{
	uint8 val = 0;

	texthook_keywait();

	pump_events();

	// キーボード＆マウス
	const Uint8* key = SDL_GetKeyboardState(NULL);
	Uint32 mouse;
	if (fingers) {
		mouse = fingers == 1 ? SDL_BUTTON_LMASK : SDL_BUTTON_RMASK;
	} else {
		mouse = SDL_GetMouseState(NULL, NULL);
	}

	if(key[SDL_SCANCODE_UP    ] || key[SDL_SCANCODE_KP_8 ] ) val |= 0x01;
	if(key[SDL_SCANCODE_DOWN  ] || key[SDL_SCANCODE_KP_2 ] ) val |= 0x02;
	if(key[SDL_SCANCODE_LEFT  ] || key[SDL_SCANCODE_KP_4 ] ) val |= 0x04;
	if(key[SDL_SCANCODE_RIGHT ] || key[SDL_SCANCODE_KP_6 ] ) val |= 0x08;
	if(key[SDL_SCANCODE_RETURN] || mouse & SDL_BUTTON_LMASK) val |= 0x10;
	if(key[SDL_SCANCODE_SPACE ] || mouse & SDL_BUTTON_RMASK) val |= 0x20;
	if(key[SDL_SCANCODE_ESCAPE]                            ) val |= 0x40;
	if(key[SDL_SCANCODE_TAB   ]                            ) val |= 0x80;

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
	pump_events();
	*x = mousex;
	*y = mousey;
}

void NACT::set_cursor(int x, int y)
{
	ags->translate_mouse_coords(&x, &y);
	SDL_WarpMouseInWindow(g_window, x, y);
}
