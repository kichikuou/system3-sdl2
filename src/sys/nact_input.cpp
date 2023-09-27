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

enum TouchState {
	TOUCH_NONE,
	TOUCH_LBUTTON,
	TOUCH_RBUTTON,
};

extern SDL_Window* g_window;
static int mousex, mousey;
static TouchState touch_state = TOUCH_NONE;

void NACT::pump_events()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (handle_platform_event(e))
			continue;

		switch (e.type) {
		case SDL_QUIT:
			quit(0);
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
			mousex = e.motion.x * ags->screen_width / ags->window_width;
			mousey = e.motion.y * ags->screen_height / ags->window_height;
			break;

		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION:
			mousex = e.tfinger.x * ags->screen_width;
			mousey = e.tfinger.y * ags->screen_height;
			switch (SDL_GetNumTouchFingers(e.tfinger.touchId)) {
			case 0:
				touch_state = TOUCH_NONE;
				break;
			case 1:
				// A touch outside of the viewport (SDL clamps it to 0.0-1.0) is
				// a right-click.
				if (e.tfinger.x == 0.0f || e.tfinger.x == 1.0f ||
					e.tfinger.y == 0.0f || e.tfinger.y == 1.0f) {
					touch_state = TOUCH_RBUTTON;
				} else {
					touch_state = TOUCH_LBUTTON;
				}
				break;
			case 2:
				// Two-finger touch is a right-click.
				touch_state = TOUCH_RBUTTON;
				break;
			}
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
	Uint32 mouse = SDL_GetMouseState(NULL, NULL);

	if(key[SDL_SCANCODE_UP    ] || key[SDL_SCANCODE_KP_8 ] ) val |= 0x01;
	if(key[SDL_SCANCODE_DOWN  ] || key[SDL_SCANCODE_KP_2 ] ) val |= 0x02;
	if(key[SDL_SCANCODE_LEFT  ] || key[SDL_SCANCODE_KP_4 ] ) val |= 0x04;
	if(key[SDL_SCANCODE_RIGHT ] || key[SDL_SCANCODE_KP_6 ] ) val |= 0x08;
	if(key[SDL_SCANCODE_RETURN] || mouse & SDL_BUTTON_LMASK || touch_state == TOUCH_LBUTTON) val |= 0x10;
	if(key[SDL_SCANCODE_SPACE ] || mouse & SDL_BUTTON_RMASK || touch_state == TOUCH_RBUTTON) val |= 0x20;
	if(key[SDL_SCANCODE_ESCAPE]                            ) val |= 0x40;
	if(key[SDL_SCANCODE_TAB   ]                            ) val |= 0x80;

	// マウス移動で方向入力はサポートしない

	if(sdl_gamecontroller) {
		if(SDL_GameControllerGetButton(sdl_gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_UP) || SDL_GameControllerGetAxis(sdl_gamecontroller, SDL_CONTROLLER_AXIS_LEFTY) <= -8000) val |= 0x01;
		if(SDL_GameControllerGetButton(sdl_gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) || SDL_GameControllerGetAxis(sdl_gamecontroller, SDL_CONTROLLER_AXIS_LEFTY) >= 8000) val |= 0x02;
		if(SDL_GameControllerGetButton(sdl_gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) || SDL_GameControllerGetAxis(sdl_gamecontroller, SDL_CONTROLLER_AXIS_LEFTX) <= -8000) val |= 0x04;
		if(SDL_GameControllerGetButton(sdl_gamecontroller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) || SDL_GameControllerGetAxis(sdl_gamecontroller, SDL_CONTROLLER_AXIS_LEFTX) >= 8000) val |= 0x08;
		if(SDL_GameControllerGetButton(sdl_gamecontroller, SDL_CONTROLLER_BUTTON_A)) val |= 0x10;
		if(SDL_GameControllerGetButton(sdl_gamecontroller, SDL_CONTROLLER_BUTTON_B)) val |= 0x20;
		if(SDL_GameControllerGetButton(sdl_gamecontroller, SDL_CONTROLLER_BUTTON_X)) val |= 0x40;
		if(SDL_GameControllerGetButton(sdl_gamecontroller, SDL_CONTROLLER_BUTTON_Y)) val |= 0x80;
	}

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
	if (!mouse_move_enabled)
		return;
	ags->translate_mouse_coords(&x, &y);
	SDL_WarpMouseInWindow(g_window, x, y);
}

#ifdef __EMSCRIPTEN__
extern "C" {

EMSCRIPTEN_KEEPALIVE
void simulate_right_button(int pressed) {
	touch_state = pressed ? TOUCH_RBUTTON : TOUCH_NONE;
}

} // extern "C"
#endif
