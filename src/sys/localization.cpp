#include "localization.h"

namespace strings {

const char* back[] = {
	"\x96\xDF\x82\xE9", // "戻る" in SJIS
	"Back",
};
const char* next_page[] = {
	"\x8E\x9F\x82\xCC\x83\x79\x81\x5B\x83\x57", // "次のページ" in SJIS
	"Next Page",
};
const char* dps_initial_tvars[][7] = {
	{
		"\x83\x4A\x83\x58\x83\x5E\x83\x80", // "カスタム" in SJIS
		"\x83\x8A\x81\x5B\x83\x69\x83\x58", // "リーナス" in SJIS
		"\x82\xA9\x82\xC2\x82\xDD", // "かつみ" in SJIS
		"\x97\x52\x94\xFC\x8E\x71", // "由美子" in SJIS
		"\x82\xA2\x82\xC2\x82\xDD", // "いつみ" in SJIS
		"\x82\xD0\x82\xC6\x82\xDD", // "ひとみ" in SJIS
		"\x90\x5E\x97\x9D\x8E\x71", // "真理子" in SJIS
	},
	{
		"Custom",
		"Linus",
		"Katsumi",
		"Yumiko",
		"Itsumi",
		"Hitomi",
		"Mariko",
	},
};

} // namespace strings
