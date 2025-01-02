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

void NACT::trace(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
#ifdef ENABLE_DEBUGGER
	if (g_debugger && g_debugger->console_vprintf(format, ap)) {
		va_end(ap);
		return;
	}
#endif
	vfprintf(stdout, format, ap);
	va_end(ap);
}

void NACT::set_skip_menu_state(bool enabled, bool checked)
{
}

bool NACT::handle_platform_event(const SDL_Event& e)
{
	return false;
}
