#include <string.h>
#include "game_id.h"
#include "../fileio.h"
#include "config.h"

// ADISK.DATの先頭256bytes
// SYSTEM1

#define CRC32_BUNKASAI		0xc80f99b8	// あぶない文化祭前夜
#define CRC32_CRESCENT		0x42351f2c	// クレセントムーンがぁる
#define CRC32_DPS		0x69ea4865	// D.P.S. - Dream Program System
#define CRC32_DPS_SG		0xab4cda48	// D.P.S. SG
#define CRC32_DPS_SG_FAHREN	0xe405d57c	//	D.P.S. SG - Fahren Fliegen
#define CRC32_DPS_SG_KATEI	 0x23e67d18	//	D.P.S. SG - 家庭教師はステキなお仕事
#define CRC32_DPS_SG_NOBUNAGA	0x2ec116f2	//	D.P.S. SG - 信長の淫謀
#define CRC32_DPS_SG2		0xab4cda48	// D.P.S. SG set2
#define CRC32_DPS_SG2_ANTIQUE	0x41fe8b3d	//	D.P.S. SG set2 - ANTIQUE HOUSE
#define CRC32_DPS_SG2_IKENAI	0x6b562c09	//	D.P.S. SG set2 - いけない内科検診再び
#define CRC32_DPS_SG2_AKAI	0x1098e78c	//	D.P.S. SG set2 - 朱い夜
#define CRC32_DPS_SG3		0xb77ae133	// D.P.S. SG SG set3
#define CRC32_DPS_SG3_RABBIT	0xa3228b6c	//	D.P.S. SG set3 - Rabbit P4P
#define CRC32_DPS_SG3_SHINKON	0x09b4448a	//	D.P.S. SG set3 - しんこんさんものがたり
#define CRC32_DPS_SG3_SOTSUGYOU	0xbc4525d8	//	D.P.S. SG set3 - 卒業
#define CRC32_FUKEI		0x026de326	// 婦警さんＶＸ
#define CRC32_INTRUDER		0xa7520fb2	// Intruder -桜屋敷の探索-
#define CRC32_RANCE		0x2fffbd60	// Rance -光をもとめて-
#define CRC32_RANCE2		0x28f8298f	// Rance 2
#define CRC32_RANCE2_HINT	0x2a85e5fa	// Rance 2 ヒントディスク (CRC32 of GDISK.DAT)
#define CRC32_TENGU		0xc942ff58	// あぶないてんぐ伝説
#define CRC32_TOUSHIN		0x62327908	// 闘神都市
#define CRC32_TOUSHIN_HINT	0xac337537	// 闘神都市 ヒントディスク
#define CRC32_VAMPIRE		0x957bcfbf	// Little Vampire
#define CRC32_VAMPIRE_ENG		0x61985a7f	// Little Vampire (English) Patch 1.5
#define CRC32_YAKATA		0x8cef6fa6	// ALICEの館
#define CRC32_GAKUEN		0xe4d6ec66  // 学園戦記 (unofficial system1 port) 1.0JP
#define CRC32_GAKUEN_ENG	0x6ba8c102  // Gakuen Senki (English) 1.0

// SYSTEM2

#define CRC32_AYUMI_FD		0x4e2fed2a	// あゆみちゃん物語 (FD)
#define CRC32_AYUMI_HINT	0xf6bd963a	// あゆみちゃん物語 ヒントディスク
#define CRC32_AYUMI_PROTO	0x4e2f5678	// あゆみちゃん物語 PROTO
#define CRC32_DALK		0x77227088	// DALK
#define CRC32_DALK_HINT		0x4793b843	// DALK ヒントディスク
#define CRC32_DRSTOP		0x73fa86c4	// Dr. STOP!
#define CRC32_PROSTUDENTG_FD	0x5ffbfee7	// Prostudent -G- (FD)
#define CRC32_RANCE3		0x47a399a1	// Rance 3
#define CRC32_RANCE3_HINT	0x8d5ec610	// Rance3 ヒントディスク
#define CRC32_RANCE4		0xebcfaff1	// Rance 4
#define CRC32_RANCE4_OPT	0xbe91c161	// Rance 4 オプションディスク
#define CRC32_SDPS		0xc7a20cdf	// Super D.P.S
#define CRC32_SDPS_MARIA	0x80d4eaca	//	Super D.P.S - マリアとカンパン
#define CRC32_SDPS_TONO		0xbb1edff1	//	Super D.P.S - 遠野の森
#define CRC32_SDPS_KAIZOKU	0xf81829e3	//	Super D.P.S - うれしたのし海賊稼業
#define CRC32_YAKATA2		0x2df591ff	// ALICEの館II

// SYSTEM3

#define CRC32_AMBIVALENZ_FD	0xa6b48dfe	// AmbivalenZ (FD)
#define CRC32_AMBIVALENZ_CD	0x4b10db69	// AmbivalenZ (CD)
#define CRC32_DPSALL		0xd48b4ec6	// DPS全部
#define CRC32_FUNNYBEE_CD	0xe14e3971	// Funny Bee (CD)
#define CRC32_FUNNYBEE_FD	0x731267fa	// Funny Bee (FD)
#define CRC32_ONLYYOU		0x832aeb97	// Only You
#define CRC32_ONLYYOU_DEMO	0xc1d13e44	// Only You (DEMO)
#define CRC32_PROSTUDENTG_CD	0xfb0e4a63	// Prostudent -G- (CD)
#define CRC32_PROG_OMAKE	0x8ba18bff	// Prostudent G おまけ (CRC32 of AGAME.DAT)
#define CRC32_RANCE41		0xa43fb4b6	// Rance 4.1
#define CRC32_RANCE41_ENG	0x811f4ff3	// Rance 4.1 (English) 1.5 Beta
#define CRC32_RANCE42		0x04d24d1e	// Rance 4.2
#define CRC32_RANCE42_ENG	0xa97cc370	// Rance 4.2 (English) 1.5 Beta
#define CRC32_AYUMI_CD		0xd2bed9ee	// あゆみちゃん物語 (CD)
#define CRC32_AYUMI_JISSHA_256	0x00d15a2b	// あゆみちゃん物語 実写版
#define CRC32_AYUMI_JISSHA_FULL	0x5f66ff1d	// あゆみちゃん物語 フルカラー実写版
#define CRC32_YAKATA3_CD	0x7f8f5e2a	// アリスの館３ (CD)
#define CRC32_YAKATA3_FD	0x58ebcc99	// アリスの館３ (FD)
#define CRC32_HASHIRIONNA2	0x09f47cbd	// 走り女２ (Rance 4.x ヒントディスク)
#define CRC32_TOUSHIN2		0xe27dd441	// 闘神都市２
#define CRC32_TOUSHIN2_GD	0xb5eba798	// 闘神都市２ グラフィックディスク
#define CRC32_TOUSHIN2_SP	0x2172c7b2	// 闘神都市２ そして、それから…
#define CRC32_OTOMESENKI	0x49a4db15	// 乙女戦記
#define CRC32_NINGYO		0xd491e7ab	// 人魚 -蘿子-
#define CRC32_MUGENHOUYOU	0xbb27d1ba	// 夢幻泡影
#define CRC32_NISE_NAGURI	0xfabe6302	// にせなぐりまくりたわぁ (ADISK.PAT)
#define CRC32_GAKUEN_KING	0xd1bf243b	// 学園KING -日出彦 学校をつくる-

namespace {

const struct CRCTable {
	GameId::Game game;
	const char* id;
	int sys_ver;
	const char* title;
	Language language;
	uint32 crc32_a;
	uint32 crc32_b;
} crc_table[] = {
	{GameId::BUNKASAI, "bunkasai", 1, "あぶない文化祭前夜", JAPANESE, CRC32_BUNKASAI},
	{GameId::CRESCENT, "crescent", 1, "クレセントムーンがぁる", JAPANESE, CRC32_CRESCENT},
	{GameId::DPS, "dps", 1, "D.P.S - Dream Program System", JAPANESE, CRC32_DPS},
	{GameId::DPS_SG_FAHREN, "dps_sg_fahren", 1, "D.P.S SG - Fahren Fliegen", JAPANESE, CRC32_DPS_SG, CRC32_DPS_SG_FAHREN},
	{GameId::DPS_SG_KATEI, "dps_sg_katei", 1, "D.P.S SG - 家庭教師はステキなお仕事", JAPANESE, CRC32_DPS_SG, CRC32_DPS_SG_KATEI},
	{GameId::DPS_SG_NOBUNAGA, "dps_sg_nobunaga", 1, "D.P.S SG - 信長の淫謀", JAPANESE, CRC32_DPS_SG, CRC32_DPS_SG_NOBUNAGA},
	{GameId::DPS_SG2_ANTIQUE, "dps_sg2_antique", 1, "D.P.S SG set2 - ANTIQUE HOUSE", JAPANESE, CRC32_DPS_SG2, CRC32_DPS_SG2_ANTIQUE},
	{GameId::DPS_SG2_IKENAI, "dps_sg2_ikenai", 1, "D.P.S SG set2 - いけない内科検診再び", JAPANESE, CRC32_DPS_SG2, CRC32_DPS_SG2_IKENAI},
	{GameId::DPS_SG2_AKAI, "dps_sg2_akai", 1, "D.P.S SG set2 - 朱い夜", JAPANESE, CRC32_DPS_SG2, CRC32_DPS_SG2_AKAI},
	{GameId::DPS_SG3_RABBIT, "dps_sg3_rabbit", 1, "D.P.S SG set3 - Rabbit P4P", JAPANESE, CRC32_DPS_SG3, CRC32_DPS_SG3_RABBIT},
	{GameId::DPS_SG3_SHINKON, "dps_sg3_shinkon", 1, "D.P.S SG set3 - しんこんさんものがたり", JAPANESE, CRC32_DPS_SG3, CRC32_DPS_SG3_SHINKON},
	{GameId::DPS_SG3_SOTSUGYOU, "dps_sg3_sotsugyou", 1, "D.P.S SG set3 - 卒業", JAPANESE, CRC32_DPS_SG3, CRC32_DPS_SG3_SOTSUGYOU},
	{GameId::FUKEI, "fukei", 1, "婦警さんＶＸ", JAPANESE, CRC32_FUKEI},
	{GameId::INTRUDER, "intruder", 1, "Intruder -桜屋敷の探索-", JAPANESE, CRC32_INTRUDER},
	{GameId::RANCE, "rance", 1, "Rance -光をもとめて-", JAPANESE, CRC32_RANCE},
	{GameId::RANCE2, "rance2", 1, "Rance 2 -反逆の少女たち-", JAPANESE, CRC32_RANCE2},
	{GameId::RANCE2_HINT, "rance2_hint", 1, "Rance 2 ヒントディスク", JAPANESE, CRC32_RANCE2_HINT},
	{GameId::TENGU, "tengu", 1, "あぶないてんぐ伝説", JAPANESE, CRC32_TENGU},
	{GameId::TOUSHIN, "toushin", 1, "闘神都市", JAPANESE, CRC32_TOUSHIN},
	{GameId::TOUSHIN_HINT, "toushin_hint", 1, "闘神都市 ヒントディスク", JAPANESE, CRC32_TOUSHIN_HINT},
	{GameId::LITTLE_VAMPIRE, "little_vampire", 1, "Little Vampire", JAPANESE, CRC32_VAMPIRE},
	{GameId::LITTLE_VAMPIRE, "little_vampire_eng", 1, "Little Vampire", ENGLISH, CRC32_VAMPIRE_ENG},
	{GameId::YAKATA, "yakata", 1, "ALICEの館", JAPANESE, CRC32_YAKATA},
	{GameId::GAKUEN, "gakuen", 1, "学園戦記", JAPANESE, CRC32_GAKUEN},
	{GameId::GAKUEN, "gakuen_eng", 1, "Gakuen Senki", ENGLISH, CRC32_GAKUEN_ENG},

	{GameId::AYUMI_FD, "ayumi_fd", 2, "あゆみちゃん物語", JAPANESE, CRC32_AYUMI_FD},
	{GameId::AYUMI_HINT, "ayumi_hint", 2, "あゆみちゃん物語 ヒントディスク", JAPANESE, CRC32_AYUMI_HINT},
	{GameId::AYUMI_PROTO, "ayumi_proto", 2, "あゆみちゃん物語 PROTO", JAPANESE, CRC32_AYUMI_PROTO},
	{GameId::DALK, "dalk", 2, "DALK", JAPANESE, CRC32_DALK},
	{GameId::DALK_HINT, "dalk_hint", 2, "DALK ヒントディスク", JAPANESE, CRC32_DALK_HINT},
	{GameId::DRSTOP, "drstop", 2, "Dr. STOP!", JAPANESE, CRC32_DRSTOP},
	{GameId::PROG_FD, "prog_fd", 2, "Prostudent G", JAPANESE, CRC32_PROSTUDENTG_FD},
	{GameId::RANCE3, "rance3", 2, "Rance 3 -リーザス陥落-", JAPANESE, CRC32_RANCE3},
	{GameId::RANCE3_HINT, "rance3_hint", 2, "Rance3 ヒントディスク", JAPANESE, CRC32_RANCE3_HINT},
	{GameId::RANCE4, "rance4", 2, "Rance 4 教団の遺産", JAPANESE, CRC32_RANCE4},
	{GameId::RANCE4_OPT, "rance4_opt", 2, "Rance 4 オプションディスク", JAPANESE, CRC32_RANCE4_OPT},
	{GameId::SDPS_MARIA, "sdps_maria", 2, "Super D.P.S - マリアとカンパン", JAPANESE, CRC32_SDPS, CRC32_SDPS_MARIA},
	{GameId::SDPS_TONO, "sdps_tono", 2, "Super D.P.S - 遠野の森", JAPANESE, CRC32_SDPS, CRC32_SDPS_TONO},
	{GameId::SDPS_KAIZOKU, "sdps_kaizoku", 2, "Super D.P.S - うれしたのし海賊稼業", JAPANESE, CRC32_SDPS, CRC32_SDPS_KAIZOKU},
	{GameId::YAKATA2, "yakata2", 2, "ALICEの館II", JAPANESE, CRC32_YAKATA2},

	{GameId::AMBIVALENZ_FD, "ambivalenz_fd", 3, "AmbivalenZ −二律背反−", JAPANESE, CRC32_AMBIVALENZ_FD},
	{GameId::AMBIVALENZ_CD, "ambivalenz_cd", 3, "AmbivalenZ −二律背反−", JAPANESE, CRC32_AMBIVALENZ_CD},
	{GameId::DPS_ALL, "dps_all", 3, "D.P.S. 全部", JAPANESE, CRC32_DPSALL},
	{GameId::FUNNYBEE_CD, "funnybee_cd", 3, "宇宙快盗ファニーBee", JAPANESE, CRC32_FUNNYBEE_CD},
	{GameId::FUNNYBEE_FD, "funnybee_fd", 3, "宇宙快盗ファニーBee", JAPANESE, CRC32_FUNNYBEE_FD},
	{GameId::ONLYYOU, "onlyyou", 3, "Only You −世紀末のジュリエット達−", JAPANESE, CRC32_ONLYYOU},
	{GameId::ONLYYOU_DEMO, "onlyyou_demo", 3, "Only You −世紀末のジュリエット達− デモ版", JAPANESE, CRC32_ONLYYOU_DEMO},
	{GameId::PROG_CD, "prog_cd", 3, "Prostudent G", JAPANESE, CRC32_PROSTUDENTG_CD},
	{GameId::PROG_OMAKE, "prog_omake", 3, "Prostudent G おまけ", JAPANESE, CRC32_PROG_OMAKE},
	{GameId::RANCE41, "rance41", 3, "ランス 4.1 〜お薬工場を救え！〜", JAPANESE, CRC32_RANCE41},
	{GameId::RANCE41, "rance41_eng", 3, "Rance 4.1 ~Save the Medicine Plant!~", ENGLISH, CRC32_RANCE41_ENG},
	{GameId::RANCE42, "rance42", 3, "ランス 4.2 〜エンジェル組〜", JAPANESE, CRC32_RANCE42},
	{GameId::RANCE42, "rance42_eng", 3, "Rance 4.2 ~Angel Army~", ENGLISH, CRC32_RANCE42_ENG},
	{GameId::AYUMI_CD, "ayumi_cd", 3, "あゆみちゃん物語", JAPANESE, CRC32_AYUMI_CD},
	{GameId::AYUMI_LIVE_256, "ayumi_live_256", 3, "あゆみちゃん物語 実写版", JAPANESE, CRC32_AYUMI_JISSHA_256},
	{GameId::AYUMI_LIVE_FULL, "ayumi_live_full", 3, "あゆみちゃん物語 フルカラー実写版", JAPANESE, CRC32_AYUMI_JISSHA_FULL},
	{GameId::YAKATA3_CD, "yakata3_cd", 3, "アリスの館3", JAPANESE, CRC32_YAKATA3_CD},
	{GameId::YAKATA3_FD, "yakata3_fd", 3, "アリスの館3", JAPANESE, CRC32_YAKATA3_FD},
	{GameId::HASHIRIONNA2, "hashirionna2", 3, "走り女2", JAPANESE, CRC32_HASHIRIONNA2},
	{GameId::TOUSHIN2, "toushin2", 3, "闘神都市II", JAPANESE, CRC32_TOUSHIN2},
	{GameId::TOUSHIN2_GD, "toushin2_gd", 3, "闘神都市2 グラフィックディスク", JAPANESE, CRC32_TOUSHIN2_GD},
	{GameId::TOUSHIN2_SP, "toushin2_sp", 3, "闘神都市2 そして、それから…", JAPANESE, CRC32_TOUSHIN2_SP},
	{GameId::OTOME, "otome", 3, "乙女戦記", JAPANESE, CRC32_OTOMESENKI},
	{GameId::NINGYO, "ningyo", 3, "人魚 -蘿子-", JAPANESE, CRC32_NINGYO},
	{GameId::MUGEN, "mugen", 3, "夢幻泡影", JAPANESE, CRC32_MUGENHOUYOU},
	{GameId::NISE_NAGURI, "nise_naguri", 3, "にせなぐりまくりたわあ", JAPANESE, CRC32_NISE_NAGURI},
	{GameId::GAKUEN_KING, "gakuen_king", 3, "学園KING -日出彦 学校をつくる-", JAPANESE, CRC32_GAKUEN_KING},

	{GameId::UNKNOWN, NULL, 0, NULL, JAPANESE, 0},
};

const CRCTable* lookup(uint32 crc32_a, uint32 crc32_b)
{
	for (const CRCTable* t = crc_table; t->id; t++) {
		if (crc32_a == t->crc32_a && (!t->crc32_b || crc32_b == t->crc32_b))
			return t;
	}
	return NULL;
}

uint32 calc_crc32(const char* file_name)
{
	uint32 crc = 0;
	auto fio = FILEIO::open(file_name, FILEIO_READ_BINARY);

	if (fio) {
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
			int d = fio->getc();
			uint32 c = ~crc;
			c = table[(c ^ d) & 0xff] ^ (c >> 8);
			crc = ~c;
		}
	}
	return crc;
}

} // namespace

GameId::GameId(const Config& config)
	: game(UNKNOWN), name(nullptr), sys_ver(3), title(nullptr), language(JAPANESE), encoding("Shift_JIS")
{
	const CRCTable* entry = nullptr;
	if (!config.game_id.empty()) {
		for (const CRCTable* t = crc_table; t->id; t++) {
			if (t->id == config.game_id) {
				entry = t;
				break;
			}
		}
		if (!entry)
			sys_error("Unknown game ID: %s", config.game_id.c_str());
	} else {
		uint32_t crc32_a = calc_crc32("ADISK.DAT");
		uint32_t crc32_b = calc_crc32("BDISK.DAT");
		entry = lookup(crc32_a, crc32_b);
		if (crc32_a && !entry) {
			WARNING("Cannot determine game id. crc32_a: %08x, crc32_b: %08x", crc32_a, crc32_b);
			SDL_ShowSimpleMessageBox(
				SDL_MESSAGEBOX_WARNING, "system3",
				"Unable to determine game ID.\n"
				"If you are running a modified game, please specify 'game = <original-game-id>' in system3.ini.\n"
				"See README.md for more information.",
				NULL);
		}
	}
	if (entry) {
		game = entry->game;
		name = entry->id;
		sys_ver = entry->sys_ver;
		title = entry->title;
		language = entry->language;
		// TODO: Determine encoding from crc32
	}
	if (!config.title.empty())
		title = config.title.c_str();
	if (!config.encoding.empty())
		encoding = config.encoding.c_str();
}
