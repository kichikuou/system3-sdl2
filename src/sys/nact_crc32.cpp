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
	uint32 crc32;
	const _TCHAR* title;
} crc_table[] = {
#if defined(_SYSTEM1)
	{"crescent",        CRC32_CRESCENT,          _T("クレセントムーンがぁる")},
	{"dps",             CRC32_DPS,               _T("D.P.S - Dream Program System")},
	{"fukei",           CRC32_FUKEI,             _T("婦警さんＶＸ")},
	{"intruder",        CRC32_INTRUDER,          _T("Intruder -桜屋敷の探索-")},
	{"tengu",           CRC32_TENGU,             _T("あぶないてんぐ伝説")},
	{"little_vampire",  CRC32_VAMPIRE,           _T("Little Vampire")},
#elif defined(_SYSTEM2)
	{"ayumi_proto",     CRC32_AYUMI_PROTO,       _T("あゆみちゃん物語 PROTO")},
	{"sdps_maria",      CRC32_SDPS_MARIA,        _T("Super D.P.S")},
	{"sdps_tono",       CRC32_SDPS_TONO,         _T("Super D.P.S")},
	{"sdps_kaizoku",    CRC32_SDPS_KAIZOKU,      _T("Super D.P.S")},
	{"prog_fd",         CRC32_PROSTUDENTG_FD,    _T("prostudent G")},
#else // System3
	{"ambivalenz_fd",   CRC32_AMBIVALENZ_FD,     _T("AmbivalenZ −二律背反−")},
	{"ambivalenz_cd",   CRC32_AMBIVALENZ_CD,     _T("AmbivalenZ −二律背反−")},
	{"dps_all",         CRC32_DPSALL,            _T("D.P.S. 全部")},
	{"funnybee_cd",     CRC32_FUNNYBEE_CD,       _T("宇宙快盗ファニーBee")},
//	{"funnybee_patch",  CRC32_FUNNYBEE_PATCH,    _T("宇宙快盗ファニーBee")},
	{"funnybee_fd",     CRC32_FUNNYBEE_FD,       _T("宇宙快盗ファニーBee")},
	{"onlyyou",         CRC32_ONLYYOU,           _T("Only You −世紀末のジュリエット達−")},
	{"onlyyou_demo",    CRC32_ONLYYOU_DEMO,      _T("Only You −世紀末のジュリエット達− デモ版")},
	{"prog_cd",         CRC32_PROSTUDENTG_CD,    _T("prostudent G")},
	{"rance41",         CRC32_RANCE41,           _T("ランス 4.1 〜お薬工場を救え！〜")},
	{"rance42",         CRC32_RANCE42,           _T("ランス 4.2 〜エンジェル組〜")},
	{"ayumi_cd",        CRC32_AYUMI_CD,          _T("あゆみちゃん物語")},
	{"ayumi_live_256",  CRC32_AYUMI_JISSHA_256,  _T("あゆみちゃん物語 実写版")},
	{"ayumi_live_full", CRC32_AYUMI_JISSHA_FULL, _T("あゆみちゃん物語 フルカラー実写版")},
	{"yakata3_cd",      CRC32_YAKATA3_CD,        _T("アリスの館3")},
	{"yakata3_fd",      CRC32_YAKATA3_FD,        _T("アリスの館3")},
	{"hashirionna2",    CRC32_HASHIRIONNA2,      _T("走り女2")},
	{"toushin2_sp",     CRC32_TOUSHIN2_SP,       _T("闘神都市2 そして、それから…")},
	{"otome",           CRC32_OTOMESENKI,        _T("乙女戦記")},
	{"ningyo",          CRC32_NINGYO,            _T("人魚 -蘿子-")},
	{"mugen",           CRC32_MUGENHOUYOU,       _T("夢幻泡影")},
#endif
	{NULL, 0, NULL},
};

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
	if(fio->Fopen(_T("ADISK.DAT"), FILEIO_READ_BINARY)) {
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
#if defined(_SYSTEM2)
	if(crc == CRC32_SDPS) {
		// Super D.P.Sの場合はBDISK.DATのCRCを取る
		if(fio->Fopen(_T("BDISK.DAT"), FILEIO_READ_BINARY)) {
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
#endif
	delete fio;
#if 0
	printf("CRC: %x\n", crc);
#endif
	return crc;
}

const _TCHAR* NACT::get_title()
{
	for (const CRCTable* t = crc_table; t->id; t++) {
		if (crc32 == t->crc32)
			return t->title;
	}
	return NULL;
}
