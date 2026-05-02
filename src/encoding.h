#ifndef _ENCODING_H_
#define _ENCODING_H_

#include <memory>
#include <string>
#include <string_view>

// Gaiji characters are mapped to Unicode Private Use Area U+E000-U+E0BB.
const int GAIJI_FIRST = 0xE000;
const int GAIJI_LAST = 0xE0BB;

class Encoding {
 public:
	static std::unique_ptr<Encoding> create(const char* name);

	virtual ~Encoding() = default;

	// Returns Unicode codepoint of the first character of s, and advances s
	// to the next character.
	virtual int next_codepoint(std::string_view& s) = 0;
	// Determines the byte length of a character based on the first byte.
	virtual int mblen(unsigned char first_byte) = 0;
	// Returns the number of characters in s.
	int mbslen(std::string_view s);

	// Convert from/to utf-8 encoding.
	virtual std::string fromUtf8(std::string_view s) = 0;
	virtual std::string toUtf8(std::string_view s) = 0;
};

#endif // _ENCODING_H_
