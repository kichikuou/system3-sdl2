#include "common.h"
#include <windows.h>

uint16 sjis_to_unicode(uint16 code)
{
	char string[3];
	int length;

	if(code > 0xff) {
		string[0] = code >> 8;
		string[1] = code & 0xff;
		string[2] = '\0';
		length = 2;
	} else {
		string[0] = code & 0xff;
		string[1] = '\0';
		length = 1;
	}
	uint16 buf[2];
	MultiByteToWideChar(CP_ACP, 0, string, length, (LPWSTR)buf, 2);
	return buf[0];
}
