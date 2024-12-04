#include <string>
#include <stdio.h>
#include <SDL.h>
#include "texthook.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

void texthook_set_mode(TexthookMode m) {
	// Do nothing
}

void texthook_character(int page, int character) {
	EM_ASM_({ xsystem35.texthook.message(String.fromCharCode($0), $1); },
			character, page);
}

EM_JS(void, texthook_newline, (void), {
	xsystem35.texthook.newline();
});

EM_JS(void, texthook_nextpage, (void), {
	xsystem35.texthook.nextpage();
});

EM_JS(void, texthook_keywait, (void), {
	xsystem35.texthook.keywait();
});

#else

namespace {

class TextHookHandler {
public:
	virtual void character(int page, int c) = 0;
	virtual void newline(void) = 0;
	virtual void nextpage(void) = 0;
	virtual void keywait(void) = 0;
};

class None : public TextHookHandler {
public:
	void character(int page, int c) override {}
	void newline(void) override {}
	void nextpage(void) override {}
	void keywait(void) override {}
} none;

class Print : public TextHookHandler {
public:
	void character(int page, int c) override {
		if (newlines)
			printf("%d:", page);
		newlines = 0;

		if (c <= 0x7f) {
			putchar(c);
		} else if (c <= 0x7ff) {
			putchar(0xc0 | c >> 6);
			putchar(0x80 | (c & 0x3f));
		} else {
			putchar(0xe0 | c >> 12);
			putchar(0x80 | (c >> 6 & 0x3f));
			putchar(0x80 | (c & 0x3f));
		}
	}

	void newline(void) override {
		if (newlines < 2) {
			putchar('\n');
			fflush(stdout);
			newlines++;
		}
	}

	void nextpage(void) override {
		while (newlines < 2) {
			putchar('\n');
			fflush(stdout);
			newlines++;
		}
	}

	void keywait(void) override {
		newline();
	}

private:
	int newlines = 0;
} print;

class Copy : public TextHookHandler {
public:
	void character(int page, int c) override {
		if (c <= 0x7f) {
			buf += c;
		} else if (c <= 0x7ff) {
			buf += static_cast<char>(0xc0 | c >> 6);
			buf += static_cast<char>(0x80 | (c & 0x3f));
		} else {
			buf += static_cast<char>(0xe0 | c >> 12);
			buf += static_cast<char>(0x80 | (c >> 6 & 0x3f));
			buf += static_cast<char>(0x80 | (c & 0x3f));
		}
	}

	void newline(void) override {
		buf += "\n";
	}

	void nextpage(void) override {
		if (!buf.empty())
			copy_to_clipboard();
	}

	void keywait(void) override {
		if (!buf.empty())
			copy_to_clipboard();
	}

private:
	std::string buf;

	void copy_to_clipboard(void) {
		SDL_SetClipboardText(buf.c_str());
		buf.clear();
	}
} copy;

TextHookHandler *handler = &none;

}  // namespace

void texthook_set_mode(TexthookMode mode) {
	switch (mode) {
	case TEXTHOOK_NONE:
		handler = &none;
		break;
	case TEXTHOOK_PRINT:
		handler = &print;
		break;
	case TEXTHOOK_COPY:
		handler = &copy;
		break;
	}
}

void texthook_character(int page, int c) {
	handler->character(page, c);
}

void texthook_newline(void) {
	handler->newline();
}

void texthook_nextpage(void) {
	handler->nextpage();
}

void texthook_keywait(void) {
	handler->keywait();
}

#endif
