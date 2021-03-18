#ifndef _LOCALIZATION_H_
#define _LOCALIZATION_H_

enum Language {
	JAPANESE = 0,
	ENGLISH = 1,
};

namespace strings {

extern const char* back[];
extern const char* next_page[];
extern const char* dps_initial_tvars[][7];

} // namespace strings

#endif // _LOCALIZATION_H_
