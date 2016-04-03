#include <stdio.h>
#include "nact.h"

void NACT::text_dialog()
{
}

void NACT::initialize_console()
{
}

void NACT::release_console()
{
}

void NACT::output_console(char log[])
{
#if defined(_DEBUG_CONSOLE)
	fputs(log, stderr);
#endif
}
