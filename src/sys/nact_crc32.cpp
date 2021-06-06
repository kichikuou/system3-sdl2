/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - crc32 ]
*/

#include "nact.h"
#include <string.h>
#include "crc32.h"
#include "../fileio.h"

namespace {

const struct CRCTable {
	const char* id;
	int sys_ver;
	const char* title;
	uint32 crc32_a;
	uint32 crc32_b;
} crc_table[] = {
	{"bunkasai", 1, "あぶない文化祭前夜", CRC32_BUNKASAI},
	{"crescent", 1, "クレセントムーンがぁる", CRC32_CRESCENT},
	{"dps", 1, "D.P.S - Dream Program System", CRC32_DPS},
	{"dps_sg_fahren", 1, "D.P.S SG - Fahren Fliegen", CRC32_DPS_SG, CRC32_DPS_SG_FAHREN},
	{"dps_sg_katei", 1, "D.P.S SG - 家庭教師はステキなお仕事", CRC32_DPS_SG, CRC32_DPS_SG_KATEI},
	{"dps_sg_nobunaga", 1, "D.P.S SG - 信長の淫謀", CRC32_DPS_SG, CRC32_DPS_SG_NOBUNAGA},
	{"dps_sg2_antique", 1, "D.P.S SG set2 - ANTIQUE HOUSE", CRC32_DPS_SG2, CRC32_DPS_SG2_ANTIQUE},
	{"dps_sg2_ikenai", 1, "D.P.S SG set2 - いけない内科検診再び", CRC32_DPS_SG2, CRC32_DPS_SG2_IKENAI},
	{"dps_sg2_akai", 1, "D.P.S SG set2 - 朱い夜", CRC32_DPS_SG2, CRC32_DPS_SG2_AKAI},
	{"dps_sg3_rabbit", 1, "D.P.S SG set3 - Rabbit P4P", CRC32_DPS_SG3, CRC32_DPS_SG3_RABBIT},
	{"dps_sg3_shinkon", 1, "D.P.S SG set3 - しんこんさんものがたり", CRC32_DPS_SG3, CRC32_DPS_SG3_SHINKON},
	{"dps_sg3_sotsugyou", 1, "D.P.S SG set3 - 卒業", CRC32_DPS_SG3, CRC32_DPS_SG3_SOTSUGYOU},
	{"fukei", 1, "婦警さんＶＸ", CRC32_FUKEI},
	{"intruder", 1, "Intruder -桜屋敷の探索-", CRC32_INTRUDER},
	{"tengu", 1, "あぶないてんぐ伝説", CRC32_TENGU},
	{"toushin_hint", 1, "闘神都市 ヒントディスク", CRC32_TOUSHIN_HINT},
	{"little_vampire", 1, "Little Vampire", CRC32_VAMPIRE},
	{"little_vampire_eng", 1, "Little Vampire", CRC32_VAMPIRE_ENG},
	{"yakata", 1, "ALICEの館", CRC32_YAKATA},

	{"ayumi_fd", 2, "あゆみちゃん物語", CRC32_AYUMI_FD},
	{"ayumi_hint", 2, "あゆみちゃん物語 ヒントディスク", CRC32_AYUMI_HINT},
	{"ayumi_proto", 2, "あゆみちゃん物語 PROTO", CRC32_AYUMI_PROTO},
	{"dalk_hint", 2, "DALK ヒントディスク", CRC32_DALK_HINT},
	{"drstop", 2, "Dr. STOP!", CRC32_DRSTOP},
	{"prog_fd", 2, "Prostudent G", CRC32_PROSTUDENTG_FD},
	{"rance3_hint", 2, "Rance3 ヒントディスク", CRC32_RANCE3_HINT},
	{"sdps_maria", 2, "Super D.P.S - マリアとカンパン", CRC32_SDPS, CRC32_SDPS_MARIA},
	{"sdps_tono", 2, "Super D.P.S - 遠野の森", CRC32_SDPS, CRC32_SDPS_TONO},
	{"sdps_kaizoku", 2, "Super D.P.S - うれしたのし海賊稼業", CRC32_SDPS, CRC32_SDPS_KAIZOKU},
	{"yakata2", 2, "ALICEの館II", CRC32_YAKATA2},

	{"ambivalenz_fd", 3, "AmbivalenZ −二律背反−", CRC32_AMBIVALENZ_FD},
	{"ambivalenz_cd", 3, "AmbivalenZ −二律背反−", CRC32_AMBIVALENZ_CD},
	{"dps_all", 3, "D.P.S. 全部", CRC32_DPSALL},
	{"funnybee_cd", 3, "宇宙快盗ファニーBee", CRC32_FUNNYBEE_CD},
//	{"funnybee_patch", 3, "宇宙快盗ファニーBee", CRC32_FUNNYBEE_PATCH},
	{"funnybee_fd", 3, "宇宙快盗ファニーBee", CRC32_FUNNYBEE_FD},
	{"onlyyou", 3, "Only You −世紀末のジュリエット達−", CRC32_ONLYYOU},
	{"onlyyou_demo", 3, "Only You −世紀末のジュリエット達− デモ版", CRC32_ONLYYOU_DEMO},
	{"prog_cd", 3, "Prostudent G", CRC32_PROSTUDENTG_CD},
	{"prog_omake", 3, "Prostudent G おまけ", CRC32_PROG_OMAKE},
	{"rance41", 3, "ランス 4.1 〜お薬工場を救え！〜", CRC32_RANCE41},
	{"rance41_eng", 3, "Rance 4.1 ~Save the Medicine Plant!~", CRC32_RANCE41_ENG},
	{"rance42", 3, "ランス 4.2 〜エンジェル組〜", CRC32_RANCE42},
	{"rance42_eng", 3, "Rance 4.2 ~Angel Army~", CRC32_RANCE42_ENG},
	{"ayumi_cd", 3, "あゆみちゃん物語", CRC32_AYUMI_CD},
	{"ayumi_live_256", 3, "あゆみちゃん物語 実写版", CRC32_AYUMI_JISSHA_256},
	{"ayumi_live_full", 3, "あゆみちゃん物語 フルカラー実写版", CRC32_AYUMI_JISSHA_FULL},
	{"yakata3_cd", 3, "アリスの館3", CRC32_YAKATA3_CD},
	{"yakata3_fd", 3, "アリスの館3", CRC32_YAKATA3_FD},
	{"hashirionna2", 3, "走り女2", CRC32_HASHIRIONNA2},
	{"toushin2_gd", 3,"闘神都市2 グラフィックディスク", CRC32_TOUSHIN2_GD},
	{"toushin2_sp", 3, "闘神都市2 そして、それから…", CRC32_TOUSHIN2_SP},
	{"otome", 3, "乙女戦記", CRC32_OTOMESENKI},
	{"ningyo", 3, "人魚 -蘿子-", CRC32_NINGYO},
	{"mugen", 3, "夢幻泡影", CRC32_MUGENHOUYOU},

	{NULL},
};

const CRCTable* lookup(uint32 crc32_a, uint32 crc32_b)
{
	for (const CRCTable* t = crc_table; t->id; t++) {
		if (crc32_a == t->crc32_a && (!t->crc32_b || crc32_b == t->crc32_b))
			return t;
	}
	return NULL;
}

} // namespace

uint32 NACT::calc_crc32(const char* file_name, const std::string& game_id)
{
	if (!game_id.empty()) {
		for (const CRCTable* t = crc_table; t->id; t++) {
			if (t->id == game_id)
				return file_name[0] == 'A' ? t->crc32_a : t->crc32_b;
		}
	}

	uint32 crc = 0;
	FILEIO* fio = new FILEIO();

	if(fio->Fopen(file_name, FILEIO_READ_BINARY)) {
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
	delete fio;
	return crc;
}

const char* NACT::get_game_id()
{
	if (const CRCTable* entry = lookup(crc32_a, crc32_b))
		return entry->id;
	return NULL;
}

const int NACT::get_sys_ver(uint32 crc32_a, uint32 crc32_b)
{
	if (const CRCTable* entry = lookup(crc32_a, crc32_b))
		return entry->sys_ver;
	return 3;
}

const char* NACT::get_title()
{
	if (const CRCTable* entry = lookup(crc32_a, crc32_b))
		return entry->title;
	return NULL;
}

Language NACT::get_language()
{
	// TODO: Add Language as a field of the CRCTable struct
	if (crc32_a == CRC32_VAMPIRE_ENG ||
		crc32_a == CRC32_RANCE41_ENG ||
		crc32_a == CRC32_RANCE42_ENG)
		return ENGLISH;
	return JAPANESE;
}
