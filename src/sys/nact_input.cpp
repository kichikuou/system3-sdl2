/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - input ]
*/

#ifdef _WIN32
#include <windows.h>
#undef ERROR
#undef min
#undef max
#endif
#include "nact.h"
#include "ags.h"
#include "texthook.h"
#include "debugger/debugger.h"

enum TouchState {
	TOUCH_NONE,
	TOUCH_LBUTTON,
	TOUCH_RBUTTON,
};

extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;

namespace {

int mousex, mousey;
TouchState touch_state = TOUCH_NONE;

int GetNumTouchFingers(SDL_TouchID touchID)
{
	int num = 0;
	SDL_Finger **fingers = SDL_GetTouchFingers(touchID, &num);
	if (!fingers)
		return 0;
	SDL_free(fingers);
	return num;
}

} // namespace

void NACT::handle_event(SDL_Event e)
{
	if (handle_platform_event(e))
		return;

	switch (e.type) {
	case SDL_EVENT_QUIT:
		show_quit_dialog();
		break;

#ifdef __ANDROID__
	case SDL_EVENT_KEY_UP:
		if (e.key.keysym.scancode == SDL_SCANCODE_AC_BACK) {
			show_quit_dialog();
		}
		break;
#endif

	case SDL_EVENT_MOUSE_MOTION :
		SDL_ConvertEventToRenderCoordinates(g_renderer, &e);
		mousex = e.motion.x;
		mousey = e.motion.y;
		break;

	case SDL_EVENT_FINGER_DOWN :
	case SDL_EVENT_FINGER_UP :
	case SDL_EVENT_FINGER_MOTION :
		SDL_ConvertEventToRenderCoordinates(g_renderer, &e);
		mousex = e.tfinger.x;
		mousey = e.tfinger.y;
		switch (GetNumTouchFingers(e.tfinger.touchID)) {
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

	default:
#ifdef ENABLE_DEBUGGER
		if (e.type == sdl_custom_event_type) {
			switch (e.user.code) {
			case DEBUGGER_COMMAND:
				g_debugger->post_command(e.user.data1);
				break;
			}
		}
#endif
		break;
	}
}

void NACT::pump_events()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		handle_event(std::move(e));
	}
}

void NACT::process_next_event()
{
	SDL_Event e;
	SDL_WaitEvent(&e);
	handle_event(std::move(e));
}

uint8 NACT::get_key()
{
	uint8 val = 0;

	texthook_keywait();

	pump_events();

	// キーボード＆マウス
	const bool* key = SDL_GetKeyboardState(NULL);
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
		if(SDL_GetGamepadButton(sdl_gamecontroller, SDL_GAMEPAD_BUTTON_DPAD_UP) || SDL_GetGamepadAxis(sdl_gamecontroller, SDL_GAMEPAD_AXIS_LEFTY) <= -8000) val |= 0x01;
		if(SDL_GetGamepadButton(sdl_gamecontroller, SDL_GAMEPAD_BUTTON_DPAD_DOWN) || SDL_GetGamepadAxis(sdl_gamecontroller, SDL_GAMEPAD_AXIS_LEFTY) >= 8000) val |= 0x02;
		if(SDL_GetGamepadButton(sdl_gamecontroller, SDL_GAMEPAD_BUTTON_DPAD_LEFT) || SDL_GetGamepadAxis(sdl_gamecontroller, SDL_GAMEPAD_AXIS_LEFTX) <= -8000) val |= 0x04;
		if(SDL_GetGamepadButton(sdl_gamecontroller, SDL_GAMEPAD_BUTTON_DPAD_RIGHT) || SDL_GetGamepadAxis(sdl_gamecontroller, SDL_GAMEPAD_AXIS_LEFTX) >= 8000) val |= 0x08;
		if(SDL_GetGamepadButton(sdl_gamecontroller, SDL_GAMEPAD_BUTTON_SOUTH)) val |= 0x10;
		if(SDL_GetGamepadButton(sdl_gamecontroller, SDL_GAMEPAD_BUTTON_EAST)) val |= 0x20;
		if(SDL_GetGamepadButton(sdl_gamecontroller, SDL_GAMEPAD_BUTTON_WEST)) val |= 0x40;
		if(SDL_GetGamepadButton(sdl_gamecontroller, SDL_GAMEPAD_BUTTON_NORTH)) val |= 0x80;
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

void NACT::show_quit_dialog()
{
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Quit" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
	};
	const SDL_MessageBoxData messagebox_data = {
		SDL_MESSAGEBOX_INFORMATION,
		nullptr,
		"System3",
		"Quit game?\nAny unsaved progress will be lost.",
		SDL_arraysize(buttons),
		buttons,
	};
	int buttonid = 0;
	if (!SDL_ShowMessageBox(&messagebox_data, &buttonid)) {
		WARNING("error displaying message box");
		return;
	}
	if (buttonid == 1) {
		quit(0);
	}
}

#ifdef __EMSCRIPTEN__
extern "C" {

EMSCRIPTEN_KEEPALIVE
void simulate_right_button(int pressed) {
	touch_state = pressed ? TOUCH_RBUTTON : TOUCH_NONE;
}

} // extern "C"
#endif
