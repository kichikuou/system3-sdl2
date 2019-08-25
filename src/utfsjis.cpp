#include <stdlib.h>
#include <string.h>
#include "utfsjis.h"

namespace {
#include "s2utbl.h"

int unicode_to_sjis(int u) {
	for (int b1 = 0x80; b1 <= 0xff; b1++) {
		if (b1 >= 0xa0 && b1 <= 0xdf)
			continue;
		for (int b2 = 0x40; b2 <= 0xff; b2++) {
			if (u == s2u[b1 - 0x80][b2 - 0x40])
				return b1 << 8 | b2;
		}
	}
	return 0;
}

} // namespace

uint16 sjis_to_unicode(uint16 code)
{
	if (code >= 0xa0 && code <= 0xdf)
		return 0xff60 + code - 0xa0;
	if (code < 0x100)
		return code;
	return s2u[(code >> 8) - 0x80][(code & 0xff) - 0x40];
}

char* sjis2utf(char* str) {
	unsigned char* src = (unsigned char*)str;
	unsigned char* dst = (unsigned char*)malloc(strlen(str) * 3 + 1);
	unsigned char* dstp = dst;

	while (*src) {
		if (*src <= 0x7f) {
			*dstp++ = *src++;
			continue;
		}

		int c;
		if (*src >= 0xa0 && *src <= 0xdf) {
			c = 0xff60 + *src - 0xa0;
			src++;
		} else {
			c = s2u[*src - 0x80][*(src+1) - 0x40];
			src += 2;
		}

		if (c <= 0x7f) {
			*dstp++ = c;
		} else if (c <= 0x7ff) {
			*dstp++ = 0xc0 | c >> 6;
			*dstp++ = 0x80 | (c & 0x3f);
		} else {
			*dstp++ = 0xe0 | c >> 12;
			*dstp++ = 0x80 | (c >> 6 & 0x3f);
			*dstp++ = 0x80 | (c & 0x3f);
		}
	}
	*dstp = '\0';
	return (char*)dst;
}

char* utf2sjis(char* str) {
	unsigned char* src = (unsigned char*)str;
	unsigned char* dst = (unsigned char*)malloc(strlen(str) + 1);
	unsigned char* dstp = dst;

	while (*src) {
		if (*src <= 0x7f) {
			*dstp++ = *src++;
			continue;
		}

		int u;
		if (*src <= 0xdf) {
			u = (src[0] & 0x1f) << 6 | (src[1] & 0x3f);
			src += 2;
		} else if (*src <= 0xef) {
			u = (src[0] & 0xf) << 12 | (src[1] & 0x3f) << 6 | (src[2] & 0x3f);
			src += 3;
		} else {
			*dstp++ = '?';
			do src++; while ((*src & 0xc0) == 0x80);
			continue;
		}

		if (u > 0xff60 && u <= 0xff9f) {
			*dstp++ = u - 0xff60 + 0xa0;
		} else {
			int c = unicode_to_sjis(u);
			if (c) {
				*dstp++ = c >> 8;
				*dstp++ = c & 0xff;
			} else {
				*dstp++ = '?';
			}
		}
	}
	*dstp = '\0';
	return (char*)dst;
}
