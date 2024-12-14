#ifndef _TEXTHOOK_H_
#define _TEXTHOOK_H_

#include "config.h"

extern "C" {

void texthook_set_mode(TexthookMode mode);
void texthook_set_suppression_list(const char *suppressions);
void texthook_character(int page, int character);
void texthook_newline(void);
void texthook_nextpage(void);
void texthook_keywait(void);

}

#endif /* _TEXTHOOK_H_ */
