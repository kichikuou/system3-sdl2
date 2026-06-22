#include <string>
#include <unordered_set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "common.h"
#include "texthook.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace {

#ifdef __EMSCRIPTEN__

class EmscriptenTextHookHandler {
public:
	void character(int page, int c) {
		EM_ASM(xsystem35.texthook.message(String.fromCharCode($0)), c);
	}
	void newline(void) {
		EM_ASM(xsystem35.texthook.newline());
	}
	void nextpage(void) {
		EM_ASM(xsystem35.texthook.nextpage());
	}
	void keywait(void) {
		EM_ASM(xsystem35.texthook.keywait());
	}
};
EmscriptenTextHookHandler handler_;
EmscriptenTextHookHandler* handler = &handler_;

#else  // __EMSCRIPTEN__

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

#endif  // __EMSCRIPTEN__

std::unordered_set<int> suppression_set;
enum {
	INIT,
	SUPPRESSING,
	EMITTING,
} suppression_state = INIT;

}  // namespace

void texthook_set_mode(TexthookMode mode) {
#ifndef __EMSCRIPTEN__
	switch (mode) {
	case TexthookMode::NONE:
		handler = &none;
		break;
	case TexthookMode::PRINT:
		handler = &print;
		break;
	case TexthookMode::COPY:
		handler = &copy;
		break;
	}
#endif
}

// suppressions is a comma-separated list of page numbers to suppress.
EMSCRIPTEN_KEEPALIVE
void texthook_set_suppression_list(const char *suppressions) {
	suppression_set.clear();
	if (!suppressions || !*suppressions)
		return;

	char *buf = strdup(suppressions);
	char *token = strtok(buf, ",");
	while (token) {
		suppression_set.insert(atoi(token));
		token = strtok(NULL, ",");
	}
	free(buf);
}

void texthook_character(int page, int c) {
	if (suppression_state == INIT)
		suppression_state = suppression_set.count(page) ? SUPPRESSING : EMITTING;
	if (suppression_state == SUPPRESSING)
		return;
	handler->character(page, c);
}

void texthook_newline(void) {
	handler->newline();
	suppression_state = INIT;
}

void texthook_nextpage(void) {
	handler->nextpage();
	suppression_state = INIT;
}

void texthook_keywait(void) {
	handler->keywait();
	suppression_state = INIT;
}
