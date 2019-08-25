#ifndef _UTFSJIS_H_
#define _UTFSJIS_H_

#include "common.h"

uint16 sjis_to_unicode(uint16 code);
char* sjis2utf(char* src);
char* utf2sjis(char* src);

#endif // _UTFSJIS_H_
