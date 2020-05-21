#include <stdio.h>
#include "nact.h"
#include "utfsjis.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void NACT::text_dialog()
{
#ifdef __EMSCRIPTEN__
	static char buf[256];
	char *oldstr = sjis2utf(tvar[tvar_index - 1]);
	int ok = EM_ASM_({
			var r = xsystem35.shell.inputString("文字列を入力してください", UTF8ToString($0), 8);
			if (r) {
				stringToUTF8(r, $1, $2);
				return 1;
			}
			return 0;
		}, oldstr, buf, sizeof buf);
	free(oldstr);
	if (ok) {
		char *newstr = utf2sjis(buf);
		strcpy_s(tvar[tvar_index - 1], 22, newstr);
		free(newstr);
	}
#endif
}

void NACT::initialize_console()
{
}

void NACT::release_console()
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
