#include <stdio.h>
#include "nact.h"
#include "utfsjis.h"
#include <emscripten.h>

void NACT::text_dialog()
{
	static char buf[256];
	char *oldstr = sjis2utf(tvar[tvar_index - 1]);
	int ok = EM_ASM_({
			var r = xsystem35.shell.inputString("文字列を入力してください", UTF8ToString($0), $1);
			if (r) {
				stringToUTF8(r, $2, $3);
				return 1;
			}
			return 0;
		}, oldstr, tvar_maxlen, buf, sizeof buf);
	free(oldstr);
	if (ok) {
		char *newstr = utf2sjis(buf);
		strcpy_s(tvar[tvar_index - 1], 22, newstr);
		free(newstr);
	}
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

void NACT::set_skip_menu_state(bool enabled, bool checked)
{
}

bool NACT::handle_platform_event(const SDL_Event& e)
{
	return false;
}
