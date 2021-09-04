#ifndef _ENCODING_H_
#define _ENCODING_H_

#include <memory>

class Encoding {
 public:
	static std::unique_ptr<Encoding> create();

	virtual ~Encoding() = default;

	// Returns Unicode codepoint of the first character of *s, and advances *s
	// to the next character.
	virtual int next_codepoint(const unsigned char** s) = 0;
	int next_codepoint(const char** s) {
		return next_codepoint(reinterpret_cast<const unsigned char **>(s));
	}
	// Returns byte length of the first character of s.
	virtual int mblen(const unsigned char* s) = 0;
	int mblen(const char* s) {
		return mblen(reinterpret_cast<const unsigned char*>(s));
	}
	// Returns the number of characters in s.
	int mbslen(const unsigned char* s);
	int mbslen(const char* s) {
		return mbslen(reinterpret_cast<const unsigned char*>(s));
	}
};

#endif // _ENCODING_H_
