#include "scenario.h"
#include <string.h>
#include "encoding.h"

void Scenario::skip_string(Encoding *enc, uint8_t terminator)
{
	for (uint8_t c = getd(); c != terminator; c = getd()) {
		if (c != '\\')
			ungetd();
		skip(enc->mblen(ptr()));
	}
}

void Scenario::get_syseng_string(char* buf, int size, Encoding *enc, uint8_t terminator)
{
	int start_addr = addr();

	int i = 0;
	for (uint8_t c = getd(); c != terminator; c = getd()) {
		if (c != '\\')
			ungetd();
		int len = enc->mblen(ptr());
		if (i + len < size - 1) {
			memcpy(&buf[i], ptr(), len);
			i += len;
		}
		skip(len);
	}
	buf[i] = '\0';
}
