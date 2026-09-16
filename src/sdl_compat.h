#ifndef SYSTEM3_SDL_COMPAT_H_
#define SYSTEM3_SDL_COMPAT_H_

#include <stddef.h>
#include <stdint.h>

#ifndef SYSTEM3_SDL_VERSION
#define SYSTEM3_SDL_VERSION 2
#endif

#if SYSTEM3_SDL_VERSION == 2
#include <SDL.h>
#elif SYSTEM3_SDL_VERSION == 3
#include <SDL3/SDL.h>
#else
#error "Unsupported SYSTEM3_SDL_VERSION"
#endif

static_assert(SDL_MAJOR_VERSION == SYSTEM3_SDL_VERSION,
			  "The selected SDL headers do not match SYSTEM3_SDL_VERSION");

// windows.h defines CreateWindow as a macro.
#ifdef CreateWindow
#undef CreateWindow
#endif

// Compatibility aliases and wrappers use SDL3 names when SDL2 and SDL3 use
// different names.
namespace sdl {

#if SYSTEM3_SDL_VERSION == 2
using Gamepad = SDL_GameController;
using PixelFormat = SDL_PixelFormatEnum;
using Mutex = SDL_mutex;
using AtomicInt = SDL_atomic_t;
using IOStream = SDL_RWops;
using GamepadButton = SDL_GameControllerButton;
using GamepadAxis = SDL_GameControllerAxis;

inline constexpr Uint32 INIT_GAMEPAD = SDL_INIT_GAMECONTROLLER;
inline constexpr Uint32 EVENT_QUIT = SDL_QUIT;
inline constexpr Uint32 EVENT_KEY_UP = SDL_KEYUP;
inline constexpr Uint32 EVENT_MOUSE_MOTION = SDL_MOUSEMOTION;
inline constexpr Uint32 EVENT_MOUSE_WHEEL = SDL_MOUSEWHEEL;
inline constexpr Uint32 EVENT_FINGER_DOWN = SDL_FINGERDOWN;
inline constexpr Uint32 EVENT_FINGER_UP = SDL_FINGERUP;
inline constexpr Uint32 EVENT_FINGER_MOTION = SDL_FINGERMOTION;
inline constexpr SDL_WindowFlags WINDOW_FULLSCREEN = SDL_WINDOW_FULLSCREEN_DESKTOP;
inline constexpr SDL_AudioFormat AudioS16 = AUDIO_S16SYS;
inline constexpr GamepadButton GAMEPAD_BUTTON_DPAD_UP = SDL_CONTROLLER_BUTTON_DPAD_UP;
inline constexpr GamepadButton GAMEPAD_BUTTON_DPAD_DOWN = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
inline constexpr GamepadButton GAMEPAD_BUTTON_DPAD_LEFT = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
inline constexpr GamepadButton GAMEPAD_BUTTON_DPAD_RIGHT = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
inline constexpr GamepadButton GAMEPAD_BUTTON_SOUTH = SDL_CONTROLLER_BUTTON_A;
inline constexpr GamepadButton GAMEPAD_BUTTON_EAST = SDL_CONTROLLER_BUTTON_B;
inline constexpr GamepadButton GAMEPAD_BUTTON_WEST = SDL_CONTROLLER_BUTTON_X;
inline constexpr GamepadButton GAMEPAD_BUTTON_NORTH = SDL_CONTROLLER_BUTTON_Y;
inline constexpr GamepadAxis GAMEPAD_AXIS_LEFTX = SDL_CONTROLLER_AXIS_LEFTX;
inline constexpr GamepadAxis GAMEPAD_AXIS_LEFTY = SDL_CONTROLLER_AXIS_LEFTY;

#else
using Gamepad = SDL_Gamepad;
using PixelFormat = SDL_PixelFormat;
using Mutex = SDL_Mutex;
using AtomicInt = SDL_AtomicInt;
using IOStream = SDL_IOStream;
using GamepadButton = SDL_GamepadButton;
using GamepadAxis = SDL_GamepadAxis;

inline constexpr Uint32 INIT_GAMEPAD = SDL_INIT_GAMEPAD;
inline constexpr Uint32 EVENT_QUIT = SDL_EVENT_QUIT;
inline constexpr Uint32 EVENT_KEY_UP = SDL_EVENT_KEY_UP;
inline constexpr Uint32 EVENT_MOUSE_MOTION = SDL_EVENT_MOUSE_MOTION;
inline constexpr Uint32 EVENT_MOUSE_WHEEL = SDL_EVENT_MOUSE_WHEEL;
inline constexpr Uint32 EVENT_FINGER_DOWN = SDL_EVENT_FINGER_DOWN;
inline constexpr Uint32 EVENT_FINGER_UP = SDL_EVENT_FINGER_UP;
inline constexpr Uint32 EVENT_FINGER_MOTION = SDL_EVENT_FINGER_MOTION;
inline constexpr SDL_WindowFlags WINDOW_FULLSCREEN = SDL_WINDOW_FULLSCREEN;
inline constexpr SDL_AudioFormat AudioS16 = SDL_AUDIO_S16;
inline constexpr GamepadButton GAMEPAD_BUTTON_DPAD_UP = SDL_GAMEPAD_BUTTON_DPAD_UP;
inline constexpr GamepadButton GAMEPAD_BUTTON_DPAD_DOWN = SDL_GAMEPAD_BUTTON_DPAD_DOWN;
inline constexpr GamepadButton GAMEPAD_BUTTON_DPAD_LEFT = SDL_GAMEPAD_BUTTON_DPAD_LEFT;
inline constexpr GamepadButton GAMEPAD_BUTTON_DPAD_RIGHT = SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
inline constexpr GamepadButton GAMEPAD_BUTTON_SOUTH = SDL_GAMEPAD_BUTTON_SOUTH;
inline constexpr GamepadButton GAMEPAD_BUTTON_EAST = SDL_GAMEPAD_BUTTON_EAST;
inline constexpr GamepadButton GAMEPAD_BUTTON_WEST = SDL_GAMEPAD_BUTTON_WEST;
inline constexpr GamepadButton GAMEPAD_BUTTON_NORTH = SDL_GAMEPAD_BUTTON_NORTH;
inline constexpr GamepadAxis GAMEPAD_AXIS_LEFTX = SDL_GAMEPAD_AXIS_LEFTX;
inline constexpr GamepadAxis GAMEPAD_AXIS_LEFTY = SDL_GAMEPAD_AXIS_LEFTY;
#endif

inline IOStream* IOFromFile(const char* path, const char* mode)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_RWFromFile(path, mode);
#else
	return SDL_IOFromFile(path, mode);
#endif
}

inline IOStream* IOFromConstMem(const void* data, size_t size)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_RWFromConstMem(data, static_cast<int>(size));
#else
	return SDL_IOFromConstMem(data, size);
#endif
}

inline size_t ReadIO(IOStream* stream, void* destination, size_t bytes)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_RWread(stream, destination, 1, bytes);
#else
	return SDL_ReadIO(stream, destination, bytes);
#endif
}

inline int64_t SeekIO(IOStream* stream, int64_t offset, int whence)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_RWseek(stream, offset, whence);
#else
	return SDL_SeekIO(stream, offset, static_cast<SDL_IOWhence>(whence));
#endif
}

inline bool CloseIO(IOStream* stream)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_RWclose(stream) == 0;
#else
	return SDL_CloseIO(stream);
#endif
}

inline int SetAtomicInt(AtomicInt* value, int desired)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_AtomicSet(value, desired);
#else
	return SDL_SetAtomicInt(value, desired);
#endif
}

inline int GetAtomicInt(AtomicInt* value)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_AtomicGet(value);
#else
	return SDL_GetAtomicInt(value);
#endif
}

inline bool CompareAndSwapAtomicInt(AtomicInt* value, int expected, int desired)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_AtomicCAS(value, expected, desired) == SDL_TRUE;
#else
	return SDL_CompareAndSwapAtomicInt(value, expected, desired);
#endif
}

inline bool SetCurrentThreadPriority(SDL_ThreadPriority priority)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_SetThreadPriority(priority) == 0;
#else
	return SDL_SetCurrentThreadPriority(priority);
#endif
}

inline int GetNumTouchFingers(SDL_TouchID touch_id)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_GetNumTouchFingers(touch_id);
#else
	int count = 0;
	SDL_Finger** fingers = SDL_GetTouchFingers(touch_id, &count);
	SDL_free(fingers);
	return count;
#endif
}

inline SDL_TouchID GetTouchID(const SDL_TouchFingerEvent& event)
{
#if SYSTEM3_SDL_VERSION == 2
	return event.touchId;
#else
	return event.touchID;
#endif
}

inline SDL_Scancode GetKeyScancode(const SDL_KeyboardEvent& event)
{
#if SYSTEM3_SDL_VERSION == 2
	return event.keysym.scancode;
#else
	return event.scancode;
#endif
}

inline bool RenderCoordinatesFromWindow(SDL_Renderer* renderer, float window_x,
		float window_y, float* x, float* y)
{
#if SYSTEM3_SDL_VERSION == 2
	*x = window_x;
	*y = window_y;
	return true;
#else
	return SDL_RenderCoordinatesFromWindow(renderer, window_x, window_y, x, y);
#endif
}

inline bool RenderCoordinatesToWindow(SDL_Renderer* renderer, SDL_Window* window,
		int* x, int* y)
{
#if SYSTEM3_SDL_VERSION == 2
	float scale_x, scale_y;
	SDL_RenderGetScale(renderer, &scale_x, &scale_y);
	*x *= scale_x;
	*y *= scale_y;

	int logical_width, logical_height;
	SDL_RenderGetLogicalSize(renderer, &logical_width, &logical_height);

	int window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);

	*x += (window_width - logical_width * scale_x) / 2;
	*y += (window_height - logical_height * scale_y) / 2;
	return true;
#else
	(void)window;
	float window_x, window_y;
	bool result = SDL_RenderCoordinatesToWindow(
		renderer, *x, *y, &window_x, &window_y);
	*x = window_x;
	*y = window_y;
	return result;
#endif
}

inline bool GetGamepadButton(Gamepad* gamepad, GamepadButton button)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_GameControllerGetButton(gamepad, button) != 0;
#else
	return SDL_GetGamepadButton(gamepad, button);
#endif
}

inline Sint16 GetGamepadAxis(Gamepad* gamepad, GamepadAxis axis)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_GameControllerGetAxis(gamepad, axis);
#else
	return SDL_GetGamepadAxis(gamepad, axis);
#endif
}

inline bool ShowMessageBox(const SDL_MessageBoxData* data, int* button_id)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_ShowMessageBox(data, button_id) == 0;
#else
	return SDL_ShowMessageBox(data, button_id);
#endif
}

inline void HideCursor()
{
#if SYSTEM3_SDL_VERSION == 2
	SDL_ShowCursor(SDL_DISABLE);
#else
	SDL_HideCursor();
#endif
}

inline void ShowCursor()
{
#if SYSTEM3_SDL_VERSION == 2
	SDL_ShowCursor(SDL_ENABLE);
#else
	SDL_ShowCursor();
#endif
}

inline bool SaveBMP(SDL_Surface* surface, const char* path)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_SaveBMP(surface, path) == 0;
#else
	return SDL_SaveBMP(surface, path);
#endif
}

inline void FreeWAV(Uint8* data)
{
#if SYSTEM3_SDL_VERSION == 2
	SDL_FreeWAV(data);
#else
	SDL_free(data);
#endif
}

inline Uint32 Swap32LE(Uint32 value)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_SwapLE32(value);
#else
	return SDL_Swap32LE(value);
#endif
}

inline SDL_Surface* CreateSurface(int width, int height, PixelFormat format)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_CreateRGBSurfaceWithFormat(0, width, height,
		SDL_BITSPERPIXEL(format), format);
#else
	SDL_Surface* surface = SDL_CreateSurface(width, height, format);
	if (surface && format == SDL_PIXELFORMAT_INDEX8)
		SDL_CreateSurfacePalette(surface);
	return surface;
#endif
}

inline SDL_Surface* CreateSurfaceWithMasks(int width, int height, int depth,
		Uint32 red, Uint32 green, Uint32 blue, Uint32 alpha)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_CreateRGBSurface(0, width, height, depth, red, green, blue, alpha);
#else
	return SDL_CreateSurface(width, height,
		SDL_GetPixelFormatForMasks(depth, red, green, blue, alpha));
#endif
}

inline void DestroySurface(SDL_Surface* surface)
{
#if SYSTEM3_SDL_VERSION == 2
	SDL_FreeSurface(surface);
#else
	SDL_DestroySurface(surface);
#endif
}

inline SDL_Palette* GetSurfacePalette(SDL_Surface* surface)
{
#if SYSTEM3_SDL_VERSION == 2
	return surface->format->palette;
#else
	return SDL_GetSurfacePalette(surface);
#endif
}

inline SDL_Window* CreateWindow(const char* title, int width, int height,
								 SDL_WindowFlags flags)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, flags);
#else
	// SDL3 uses physical pixels for window coordinates on platforms such as
	// Windows.  Scale the requested content size so that it has the same
	// apparent size on a high-DPI display.  On platforms whose window
	// coordinates are already logical (macOS and Wayland), the content scale is
	// 1 and HIGH_PIXEL_DENSITY provides the correspondingly larger backbuffer.
	float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	if (scale <= 0.0f)
		scale = 1.0f;
	width = static_cast<int>(width * scale + 0.5f);
	height = static_cast<int>(height * scale + 0.5f);
#ifndef __EMSCRIPTEN__
	flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
#endif
	return SDL_CreateWindow(title, width, height, flags);
#endif
}

inline void SetWindowContentSize(SDL_Window* window, int width, int height)
{
#if SYSTEM3_SDL_VERSION == 2
	SDL_SetWindowSize(window, width, height);
#else
	// Convert content units to window coordinates.  Dividing the window's
	// display scale by its pixel density gives the platform content scale.
	float pixel_density = SDL_GetWindowPixelDensity(window);
	float scale = SDL_GetWindowDisplayScale(window);
	if (pixel_density > 0.0f && scale > 0.0f)
		scale /= pixel_density;
	else
		scale = 1.0f;
	SDL_SetWindowSize(window,
		static_cast<int>(width * scale + 0.5f),
		static_cast<int>(height * scale + 0.5f));
#endif
}

inline bool SetWindowFullscreen(SDL_Window* window, bool fullscreen)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_SetWindowFullscreen(
		window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) == 0;
#else
	return SDL_SetWindowFullscreen(window, fullscreen);
#endif
}

inline SDL_Renderer* CreateRenderer(SDL_Window* window)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_CreateRenderer(window, -1, 0);
#else
	return SDL_CreateRenderer(window, nullptr);
#endif
}

inline bool SetRenderLogicalPresentation(SDL_Renderer* renderer, int width, int height)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_RenderSetLogicalSize(renderer, width, height) == 0;
#else
	return SDL_SetRenderLogicalPresentation(renderer, width, height,
		SDL_LOGICAL_PRESENTATION_LETTERBOX);
#endif
}

inline SDL_Palette* CreatePalette(int colors)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_AllocPalette(colors);
#else
	return SDL_CreatePalette(colors);
#endif
}

inline void DestroyPalette(SDL_Palette* palette)
{
#if SYSTEM3_SDL_VERSION == 2
	SDL_FreePalette(palette);
#else
	SDL_DestroyPalette(palette);
#endif
}

inline void DestroyCursor(SDL_Cursor* cursor)
{
#if SYSTEM3_SDL_VERSION == 2
	SDL_FreeCursor(cursor);
#else
	SDL_DestroyCursor(cursor);
#endif
}

inline bool GetRectIntersection(const SDL_Rect* a, const SDL_Rect* b, SDL_Rect* result)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_IntersectRect(a, b, result) == SDL_TRUE;
#else
	return SDL_GetRectIntersection(a, b, result);
#endif
}

inline void GetRectUnion(const SDL_Rect* a, const SDL_Rect* b, SDL_Rect* result)
{
#if SYSTEM3_SDL_VERSION == 2
	SDL_UnionRect(a, b, result);
#else
	SDL_GetRectUnion(a, b, result);
#endif
}

inline bool RenderTexture(SDL_Renderer* renderer, SDL_Texture* texture,
						   const SDL_Rect* source, const SDL_Rect* destination)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_RenderCopy(renderer, texture, source, destination) == 0;
#else
	SDL_FRect src;
	SDL_FRect dst;
	const SDL_FRect* src_ptr = nullptr;
	const SDL_FRect* dst_ptr = nullptr;
	if (source) {
		src = { static_cast<float>(source->x), static_cast<float>(source->y),
			static_cast<float>(source->w), static_cast<float>(source->h) };
		src_ptr = &src;
	}
	if (destination) {
		dst = { static_cast<float>(destination->x), static_cast<float>(destination->y),
			static_cast<float>(destination->w), static_cast<float>(destination->h) };
		dst_ptr = &dst;
	}
	return SDL_RenderTexture(renderer, texture, src_ptr, dst_ptr);
#endif
}

inline bool SetSurfaceColorKey(SDL_Surface* surface, bool enabled, Uint32 key)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_SetColorKey(surface, enabled ? SDL_TRUE : SDL_FALSE, key) == 0;
#else
	return SDL_SetSurfaceColorKey(surface, enabled, key);
#endif
}

inline bool SurfaceHasColorKey(SDL_Surface* surface)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_HasColorKey(surface) == SDL_TRUE;
#else
	return SDL_SurfaceHasColorKey(surface);
#endif
}

inline bool GetSurfaceColorKey(SDL_Surface* surface, Uint32* key)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_GetColorKey(surface, key) == 0;
#else
	return SDL_GetSurfaceColorKey(surface, key);
#endif
}

inline bool StretchSurface(SDL_Surface* source, const SDL_Rect* source_rect,
							SDL_Surface* destination, SDL_Rect* destination_rect)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_SoftStretch(source, source_rect, destination, destination_rect) == 0;
#else
	return SDL_StretchSurface(source, source_rect, destination, destination_rect,
		SDL_SCALEMODE_NEAREST);
#endif
}

inline bool FillSurfaceRect(SDL_Surface* surface, const SDL_Rect* rect, Uint32 color)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_FillRect(surface, rect, color) == 0;
#else
	return SDL_FillSurfaceRect(surface, rect, color);
#endif
}

inline bool FillSurfaceRects(SDL_Surface* surface, const SDL_Rect* rects,
							   int count, Uint32 color)
{
#if SYSTEM3_SDL_VERSION == 2
	return SDL_FillRects(surface, rects, count, color) == 0;
#else
	return SDL_FillSurfaceRects(surface, rects, count, color);
#endif
}

} // namespace sdl

sdl::IOStream* open_resource(const char* name, const char* type);
sdl::IOStream* open_file(const char* name);

#endif // SYSTEM3_SDL_COMPAT_H_
