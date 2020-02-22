/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - crc32 ]
*/

#include "nact.h"
#include "crc32.h"
#include "../fileio.h"

namespace {

const struct CRCTable {
	const char* id;
	int sys_ver;
	uint32 crc32;
	const char* title;
} crc_table[] = {
	{"crescent",        1, CRC32_CRESCENT,          "クレセントムーンがぁる"},
	{"dps",             1, CRC32_DPS,               "D.P.S - Dream Program System"},
	{"fukei",           1, CRC32_FUKEI,             "婦警さんＶＸ"},
	{"intruder",        1, CRC32_INTRUDER,          "Intruder -桜屋敷の探索-"},
	{"tengu",           1, CRC32_TENGU,             "あぶないてんぐ伝説"},
	{"little_vampire",  1, CRC32_VAMPIRE,           "Little Vampire"},

	{"ayumi_proto",     2, CRC32_AYUMI_PROTO,       "あゆみちゃん物語 PROTO"},
	{"sdps_maria",      2, CRC32_SDPS_MARIA,        "Super D.P.S"},
	{"sdps_tono",       2, CRC32_SDPS_TONO,         "Super D.P.S"},
	{"sdps_kaizoku",    2, CRC32_SDPS_KAIZOKU,      "Super D.P.S"},
	{"prog_fd",         2, CRC32_PROSTUDENTG_FD,    "prostudent G"},

	{"ambivalenz_fd",   3, CRC32_AMBIVALENZ_FD,     "AmbivalenZ −二律背反−"},
	{"ambivalenz_cd",   3, CRC32_AMBIVALENZ_CD,     "AmbivalenZ −二律背反−"},
	{"dps_all",         3, CRC32_DPSALL,            "D.P.S. 全部"},
	{"funnybee_cd",     3, CRC32_FUNNYBEE_CD,       "宇宙快盗ファニーBee"},
//	{"funnybee_patch",  3, CRC32_FUNNYBEE_PATCH,    "宇宙快盗ファニーBee"},
	{"funnybee_fd",     3, CRC32_FUNNYBEE_FD,       "宇宙快盗ファニーBee"},
	{"onlyyou",         3, CRC32_ONLYYOU,           "Only You −世紀末のジュリエット達−"},
	{"onlyyou_demo",    3, CRC32_ONLYYOU_DEMO,      "Only You −世紀末のジュリエット達− デモ版"},
	{"prog_cd",         3, CRC32_PROSTUDENTG_CD,    "prostudent G"},
	{"rance41",         3, CRC32_RANCE41,           "ランス 4.1 〜お薬工場を救え！〜"},
	{"rance42",         3, CRC32_RANCE42,           "ランス 4.2 〜エンジェル組〜"},
	{"ayumi_cd",        3, CRC32_AYUMI_CD,          "あゆみちゃん物語"},
	{"ayumi_live_256",  3, CRC32_AYUMI_JISSHA_256,  "あゆみちゃん物語 実写版"},
	{"ayumi_live_full", 3, CRC32_AYUMI_JISSHA_FULL, "あゆみちゃん物語 フルカラー実写版"},
	{"yakata3_cd",      3, CRC32_YAKATA3_CD,        "アリスの館3"},
	{"yakata3_fd",      3, CRC32_YAKATA3_FD,        "アリスの館3"},
	{"hashirionna2",    3, CRC32_HASHIRIONNA2,      "走り女2"},
	{"toushin2_sp",     3, CRC32_TOUSHIN2_SP,       "闘神都市2 そして、それから…"},
	{"otome",           3, CRC32_OTOMESENKI,        "乙女戦記"},
	{"ningyo",          3, CRC32_NINGYO,            "人魚 -蘿子-"},
	{"mugen",           3, CRC32_MUGENHOUYOU,       "夢幻泡影"},

	{NULL, 0, 0, NULL},
};

const CRCTable* lookup(uint32 crc32)
{
	for (const CRCTable* t = crc_table; t->id; t++) {
		if (crc32 == t->crc32)
			return t;
	}
	return NULL;
}

} // namespace

uint32 NACT::calc_crc32(const char* game_id)
{
	if (game_id) {
		for (const CRCTable* t = crc_table; t->id; t++) {
			if (strcmp(t->id, game_id) == 0)
				return t->crc32;
		}
	}

	uint32 crc = 0;
	FILEIO* fio = new FILEIO();
	if(fio->Fopen("ADISK.DAT", FILEIO_READ_BINARY)) {
		uint32 table[256];
		for(int i = 0; i < 256; i++) {
			uint32 c = i;
			for(int j = 0; j < 8; j++) {
				if(c & 1) {
					c = (c >> 1) ^ 0xedb88320;
				} else {
					c >>= 1;
				}
			}
			table[i] = c;
		}
		// ADISK.DATの先頭256bytes
		for(int i = 0; i < 256; i++) {
			int d = fio->Fgetc();
			uint32 c = ~crc;
			c = table[(c ^ d) & 0xff] ^ (c >> 8);
			crc = ~c;
		}
		fio->Fclose();
	}
	if(crc == CRC32_SDPS) {
		// Super D.P.Sの場合はBDISK.DATのCRCを取る
		if(fio->Fopen("BDISK.DAT", FILEIO_READ_BINARY)) {
			uint32 table[256];
			for(int i = 0; i < 256; i++) {
				uint32 c = i;
				for(int j = 0; j < 8; j++) {
					if(c & 1) {
						c = (c >> 1) ^ 0xedb88320;
					} else {
						c >>= 1;
					}
				}
				table[i] = c;
			}
			// BDISK.DATの先頭256bytes
			crc = 0;
			for(int i = 0; i < 256; i++) {
				int d = fio->Fgetc();
				uint32 c = ~crc;
				c = table[(c ^ d) & 0xff] ^ (c >> 8);
				crc = ~c;
			}
			fio->Fclose();
		}
	}
	delete fio;
#if 0
	printf("CRC: %x\n", crc);
#endif
	return crc;
}

const char* NACT::get_game_id()
{
	if (const CRCTable* entry = lookup(crc32))
		return entry->id;
	return NULL;
}

const int NACT::get_sys_ver(uint32 crc32)
{
	if (const CRCTable* entry = lookup(crc32))
		return entry->sys_ver;
	return 3;
}

const char* NACT::get_title()
{
	if (const CRCTable* entry = lookup(crc32))
		return entry->title;
	return NULL;
}
