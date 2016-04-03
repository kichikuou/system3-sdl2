#include "common.h"

uint16 sjis_to_unicode(uint16 code)
{
	// TODO: correct impl
	return code;
}

void strcpy_s(char* dst, size_t n, const char* src)
{
	strncpy(dst, src, n);
	dst[n - 1] = '\0';
}
