#ifndef _UTFSJIS_H_
#define _UTFSJIS_H_

#include "common.h"

// Gaiji characters are mapped to Unicode Private Use Area U+E000-U+E0BB.
const int GAIJI_FIRST = 0xE000;
const int GAIJI_LAST = 0xE0BB;

uint16 sjis_to_unicode(uint16 code);
char* sjis2utf(char* src);
char* utf2sjis(char* src);

#endif // _UTFSJIS_H_
