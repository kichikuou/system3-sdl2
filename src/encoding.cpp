#include "encoding.h"
#include "utfsjis.h"

class SjisEncoding : public Encoding {
public:
	int mblen(const unsigned char* s)
	{
		return is_2byte(*s) ? 2 : 1;
	}

	int next_codepoint(const unsigned char** s)
	{
		int code = *(*s)++;
		if (is_2byte(code))
			code = (code << 8) | (uint8)*(*s)++;
		return sjis_to_unicode(code);
	}

private:
	bool is_2byte(unsigned char c) {
		return (0x81 <= c && c <= 0x9f) || 0xe0 <= c;
	}
};

class Utf8Encoding : public Encoding {
public:
	int mblen(const unsigned char* s)
	{
		if (*s <= 0xbf)
			return 1;
		if (*s <= 0xdf)
			return 2;
		if (*s <= 0xef)
			return 3;
		return 4;
	}

	int next_codepoint(const unsigned char** str)
	{
		int code;
		const unsigned char *s = *str;

		if (*s <= 0x7f) {
			code = *s++;
		} else if (*s <= 0xbf) {
			// Invalid UTF-8 sequence
			code = '?';
			s++;
		} else if (*s <= 0xdf) {
			code = (s[0] & 0x1f) << 6 | (s[1] & 0x3f);
			s += 2;
		} else if (*s <= 0xef) {
			code = (s[0] & 0xf) << 12 | (s[1] & 0x3f) << 6 | (s[2] & 0x3f);
			s += 3;
		} else if (*s <= 0xf7) {
			code = (s[0] & 0x7) << 18 | (s[1] & 0x3f) << 12 | (s[2] & 0x3f) << 6 | (s[3] & 0x3f);
			s += 4;
		} else {
			code = 0xfffd;  // REPLACEMENT CHARACTER
			s++;
			while (0x80 <= *s && *s <= 0xbf)
				s++;
		}
		*str = s;
		return code;
	}
};

int Encoding::mbslen(const unsigned char* s)
{
	int len = 0;
	while (*s) {
		s += mblen(s);
		len++;
	}
	return len;
}

std::unique_ptr<Encoding> Encoding::create(const char* name)
{
	if (!strcasecmp(name, "Shift_JIS") ||
		!strcasecmp(name, "Shift-JIS") ||
		!strcasecmp(name, "SJIS") ||
		!strcasecmp(name, "CP932"))
		return std::make_unique<SjisEncoding>();

	if (!strcasecmp(name, "UTF-8") ||
		!strcasecmp(name, "UTF8"))
		return std::make_unique<Utf8Encoding>();

	WARNING("Unrecognized encoding: \"%s\"", name);
	return std::make_unique<SjisEncoding>();
}
