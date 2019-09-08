#include <stdio.h>
#include "texthook.h"

#ifdef __EMSCRIPTEN__  // ----------------------------------------------
#include <emscripten.h>

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

#elif defined(TEXTHOOK_PRINT)  // --------------------------------------

static int newlines = 0;

void texthook_character(int page, int c) {
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

void texthook_newline(void) {
	if (newlines < 2) {
		putchar('\n');
		fflush(stdout);
		newlines++;
	}
}

void texthook_nextpage(void) {
	while (newlines < 2) {
		putchar('\n');
		fflush(stdout);
		newlines++;
	}
}

void texthook_keywait(void) {
	texthook_newline();
}

#else  // --------------------------------------------------------------

void texthook_character(int page, int c) {
}

void texthook_newline(void) {
}

void texthook_nextpage(void) {
}

void texthook_keywait(void) {
}

#endif  // -------------------------------------------------------------
