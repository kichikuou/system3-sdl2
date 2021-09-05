#include <stdio.h>
#include "nact.h"
#include "encoding.h"
#include "msgskip.h"
#include <emscripten.h>

namespace {

enum CustomEventCode {
	SET_MESSAGESKIP_MODE,
	SET_MESSAGESKIP_FLAGS,
	SYNC_MESSAGESKIP_FILE,
};

Uint32 custom_event_type = static_cast<Uint32>(-1);

} // namespace

void NACT::text_dialog()
{
	static char buf[256];
	char *oldstr = encoding->toUtf8(tvar[tvar_index - 1]);
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
		char *newstr = encoding->fromUtf8(buf);
		strcpy_s(tvar[tvar_index - 1], 22, newstr);
		free(newstr);
	}
}

void NACT::platform_initialize()
{
	if (custom_event_type == static_cast<Uint32>(-1)) {
		custom_event_type = SDL_RegisterEvents(1);
		EM_ASM({ setInterval(() => _msgskip_syncFile(), 5000); });
	}
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
	EM_ASM({ xsystem35.shell.setSkipButtonState($0, $1); }, enabled, checked);
}

bool NACT::handle_platform_event(const SDL_Event& e)
{
	if (e.type != custom_event_type)
		return false;
	switch (e.user.code) {
	case SET_MESSAGESKIP_MODE:
		msgskip->activate(static_cast<bool>(e.user.data1));
		break;
	case SET_MESSAGESKIP_FLAGS:
		msgskip->set_flags(reinterpret_cast<unsigned>(e.user.data1),
						   reinterpret_cast<unsigned>(e.user.data2));
		break;
	case SYNC_MESSAGESKIP_FILE:
		if (msgskip->write_to_file())
			EM_ASM( xsystem35.shell.syncfs(); );
		break;
	}
	return true;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
void msgskip_activate(int enable) {
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = custom_event_type;
	event.user.code = SET_MESSAGESKIP_MODE;
	event.user.data1 = (void*)enable;
	SDL_PushEvent(&event);
}

EMSCRIPTEN_KEEPALIVE
void msgskip_setFlags(unsigned flags, unsigned mask) {
	if (!SDL_WasInit(SDL_INIT_EVENTS)) {
		// Retry.
		EM_ASM({ setTimeout(() => _msgskip_setFlags($0, $1), 50); }, flags, mask);
		return;
	}
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = custom_event_type;
	event.user.code = SET_MESSAGESKIP_FLAGS;
	event.user.data1 = (void*)flags;
	event.user.data2 = (void*)mask;
	SDL_PushEvent(&event);
}

EMSCRIPTEN_KEEPALIVE
void msgskip_syncFile() {
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = custom_event_type;
	event.user.code = SYNC_MESSAGESKIP_FILE;
	SDL_PushEvent(&event);
}

} // extern "C"
