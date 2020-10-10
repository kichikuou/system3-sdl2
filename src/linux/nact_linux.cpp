#include <stdio.h>
#include "nact.h"

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
#if defined(_DEBUG_CONSOLE)
	va_list ap;

	va_start(ap, format);
	vfprintf(stdout, format, ap);
	va_end(ap);
#endif
}
