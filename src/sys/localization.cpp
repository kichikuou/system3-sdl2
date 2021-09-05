#include "localization.h"
#include "encoding.h"

Strings::Strings(Language lang, Encoding* encoding)
{
	switch (lang) {
	case JAPANESE:
		back = encoding->fromUtf8(u8"戻る");
		next_page = encoding->fromUtf8(u8"次のページ");
		dps_initial_tvars[0] = encoding->fromUtf8(u8"カスタム");
		dps_initial_tvars[1] = encoding->fromUtf8(u8"リーナス");
		dps_initial_tvars[2] = encoding->fromUtf8(u8"かつみ");
		dps_initial_tvars[3] = encoding->fromUtf8(u8"由美子");
		dps_initial_tvars[4] = encoding->fromUtf8(u8"いつみ");
		dps_initial_tvars[5] = encoding->fromUtf8(u8"ひとみ");
		dps_initial_tvars[6] = encoding->fromUtf8(u8"真理子");
		break;
	case ENGLISH:
		back = "Back";
		next_page = "Next Page";
		dps_initial_tvars[0] = "Custom";
		dps_initial_tvars[1] = "Linus";
		dps_initial_tvars[2] = "Katsumi";
		dps_initial_tvars[3] = "Yumiko";
		dps_initial_tvars[4] = "Itsumi";
		dps_initial_tvars[5] = "Hitomi";
		dps_initial_tvars[6] = "Mariko";
		break;
	}
}
