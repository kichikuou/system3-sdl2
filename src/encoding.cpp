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

int Encoding::mbslen(const unsigned char* s)
{
	int len = 0;
	while (*s) {
		s += mblen(s);
		len++;
	}
	return len;
}

std::unique_ptr<Encoding> Encoding::create()
{
	return std::make_unique<SjisEncoding>();
}
