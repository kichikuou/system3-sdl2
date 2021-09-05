#ifndef _LOCALIZATION_H_
#define _LOCALIZATION_H_

class Encoding;

enum Language {
	JAPANESE = 0,
	ENGLISH = 1,
};

struct Strings {
	Strings(Language lang, Encoding* encoding);

	const char* back;
	const char* next_page;
	const char* dps_initial_tvars[7];
};

#endif // _LOCALIZATION_H_
