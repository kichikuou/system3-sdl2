#ifndef _TEXTHOOK_H_
#define _TEXTHOOK_H_

extern "C" {

enum TexthookMode {
	TEXTHOOK_NONE,
	TEXTHOOK_PRINT,
	TEXTHOOK_COPY,
};

void texthook_set_mode(TexthookMode mode);
void texthook_set_suppression_list(const char *suppressions);
void texthook_character(int page, int character);
void texthook_newline(void);
void texthook_nextpage(void);
void texthook_keywait(void);

}

#endif /* _TEXTHOOK_H_ */
