#ifndef _GAMEID_H_
#define _GAMEID_H_

#include <stdint.h>

struct Config;

enum Language {
	JAPANESE = 0,
	ENGLISH = 1,
};

struct GameId {
	enum Game {
		UNKNOWN,

		// System 1
		BUNKASAI,
		CRESCENT,
		DPS,
		DPS_SG_FAHREN,
		DPS_SG_KATEI,
		DPS_SG_NOBUNAGA,
		DPS_SG2_ANTIQUE,
		DPS_SG2_IKENAI,
		DPS_SG2_AKAI,
		DPS_SG3_RABBIT,
		DPS_SG3_SHINKON,
		DPS_SG3_SOTSUGYOU,
		FUKEI,
		INTRUDER,
		TENGU,
		TOUSHIN_HINT,
		LITTLE_VAMPIRE,
		LITTLE_VAMPIRE_ENG,
		YAKATA,
		GAKUEN,
		GAKUEN_ENG,

		// System 2
		AYUMI_FD,
		AYUMI_HINT,
		AYUMI_PROTO,
		DALK_HINT,
		DRSTOP,
		PROG_FD,
		RANCE3_HINT,
		SDPS_MARIA,
		SDPS_TONO,
		SDPS_KAIZOKU,
		YAKATA2,

		// System 3
		AMBIVALENZ_FD,
		AMBIVALENZ_CD,
		DPS_ALL,
		FUNNYBEE_CD,
		FUNNYBEE_FD,
		ONLYYOU,
		ONLYYOU_DEMO,
		PROG_CD,
		PROG_OMAKE,
		RANCE41,
		RANCE41_ENG,
		RANCE42,
		RANCE42_ENG,
		AYUMI_CD,
		AYUMI_LIVE_256,
		AYUMI_LIVE_FULL,
		YAKATA3_CD,
		YAKATA3_FD,
		HASHIRIONNA2,
		TOUSHIN2_GD,
		TOUSHIN2_SP,
		OTOME,
		NINGYO,
		MUGEN,
	};
	int game;
	const char* name;
	int sys_ver;
	const char* title;
	Language language;
	const char* encoding;

	explicit GameId(const Config& config);
	bool is(Game g) const { return game == g; }
	bool is_valid() const { return game != UNKNOWN; }
	bool is_gakuen() const { return game == GAKUEN || game == GAKUEN_ENG; }
	bool is_system1_dps() const {
		return DPS <= game && game <= DPS_SG3_SOTSUGYOU;
	}
	bool is_sdps() const {
		return game == SDPS_MARIA || game == SDPS_TONO || game == SDPS_KAIZOKU;
	}
	bool is_rance4x() const {
		return game == RANCE41 || game == RANCE41_ENG || game == RANCE42 || game == RANCE42_ENG;
	}
};

#endif  // _GAMEID_H_
