#include <stdio.h>
#include "nact.h"
#include "debugger/debugger.h"

void NACT::text_dialog()
{
}

void NACT::platform_initialize()
{
}

void NACT::platform_finalize()
{
}

void NACT::output_console(const char *format, ...)
{
#ifdef ENABLE_DEBUGGER
	if (g_debugger) {
		va_list ap;
		va_start(ap, format);
		bool handled = g_debugger->console_vprintf(format, ap);
		va_end(ap);
		if (handled)
			return;
	}
#endif
#if defined(_DEBUG_CONSOLE)
	va_list ap;

	va_start(ap, format);
	vfprintf(stdout, format, ap);
	va_end(ap);
#endif
}

void NACT::set_skip_menu_state(bool enabled, bool checked)
{
}

bool NACT::handle_platform_event(const SDL_Event& e)
{
	return false;
}
