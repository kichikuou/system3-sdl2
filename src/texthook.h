#ifndef _TEXTHOOK_H_
#define _TEXTHOOK_H_

extern "C" {

void texthook_character(int page, int character);
void texthook_newline(void);
void texthook_nextpage(void);
void texthook_keywait(void);

}

#endif /* _TEXTHOOK_H_ */
