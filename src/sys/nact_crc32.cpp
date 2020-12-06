/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - crc32 ]
*/

#include "nact.h"
#include "crc32.h"
#include "../fileio.h"

uint32 NACT::calc_crc32(const char *file_name)
{
	uint32 crc = 0;
	FILEIO *fio = new FILEIO();

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

bool NACT::get_title(_TCHAR *title, int length)
{
#if defined(_SYSTEM1)
	switch(crc32_a) {
		case CRC32_BUNKASAI:
			_tcscpy_s(title, length, _T("あぶない文化祭前夜"));
			return true;
		case CRC32_CRESCENT:
			_tcscpy_s(title, length, _T("クレセントムーンがぁる"));
			return true;
		case CRC32_DPS:
			_tcscpy_s(title, length, _T("D.P.S - Dream Program System"));
			return true;
		case CRC32_DPS_SG:
//		case CRC32_DPS_SG2:
			switch(crc32_b) {
				case CRC32_DPS_SG_FAHREN:
					_tcscpy_s(title, length, _T("D.P.S SG - Fahren Fliegen"));
					return true;
				case CRC32_DPS_SG_KATEI:
					_tcscpy_s(title, length, _T("D.P.S SG - 家庭教師はステキなお仕事"));
					return true;
				case CRC32_DPS_SG_NOBUNAGA:
					_tcscpy_s(title, length, _T("D.P.S SG - 信長の淫謀"));
					return true;
				case CRC32_DPS_SG2_ANTIQUE:
					_tcscpy_s(title, length, _T("D.P.S SG set2 - ANTIQUE HOUSE"));
					return true;
				case CRC32_DPS_SG2_IKENAI:
					_tcscpy_s(title, length, _T("D.P.S SG set2 - いけない内科検診再び"));
					return true;
				case CRC32_DPS_SG2_AKAI:
					_tcscpy_s(title, length, _T("D.P.S SG set2 - 朱い夜"));
					return true;
			}
			_tcscpy_s(title, length, _T("D.P.S SG"));
			return true;
		case CRC32_DPS_SG3:
			switch(crc32_b) {
				case CRC32_DPS_SG3_RABBIT:
					_tcscpy_s(title, length, _T("D.P.S SG set3 - Rabbit P4P"));
					return true;
				case CRC32_DPS_SG3_SHINKON:
					_tcscpy_s(title, length, _T("D.P.S SG set3 - しんこんさんものがたり"));
					return true;
				case CRC32_DPS_SG3_SOTSUGYOU:
					_tcscpy_s(title, length, _T("D.P.S SG set3 - 卒業"));
					return true;
			}
			_tcscpy_s(title, length, _T("D.P.S SG set3"));
			return true;
		case CRC32_FUKEI:
			_tcscpy_s(title, length, _T("婦警さんＶＸ"));
			return true;
		case CRC32_INTRUDER:
			_tcscpy_s(title, length, _T("Intruder -桜屋敷の探索-"));
			return true;
		case CRC32_TENGU:
			_tcscpy_s(title, length, _T("あぶないてんぐ伝説"));
			return true;
		case CRC32_TOUSHIN_HINT:
			_tcscpy_s(title, length, _T("闘神都市 ヒントディスク"));
			return true;
		case CRC32_VAMPIRE:
			_tcscpy_s(title, length, _T("Little Vampire"));
			return true;
		case CRC32_YAKATA:
			_tcscpy_s(title, length, _T("ALICEの館"));
			return true;
	}
#elif defined(_SYSTEM2)
	switch(crc32_a) {
		case CRC32_AYUMI_FD:
			_tcscpy_s(title, length, _T("あゆみちゃん物語"));
			return true;
		case CRC32_AYUMI_HINT:
			_tcscpy_s(title, length, _T("あゆみちゃん物語 ヒントディスク"));
			return true;
		case CRC32_AYUMI_PROTO:
			_tcscpy_s(title, length, _T("あゆみちゃん物語 PROTO"));
			return true;
		case CRC32_DALK_HINT:
			_tcscpy_s(title, length, _T("DALK ヒントディスク"));
			return true;
		case CRC32_DRSTOP:
			_tcscpy_s(title, length, _T("Dr. STOP!"));
			return true;
		case CRC32_PROSTUDENTG_FD:
			_tcscpy_s(title, length, _T("prostudent G"));
			return true;
		case CRC32_RANCE3_HINT:
			_tcscpy_s(title, length, _T("Rance3 ヒントディスク"));
			return true;
		case CRC32_SDPS:
			switch(crc32_b) {
				case CRC32_SDPS_MARIA:
					_tcscpy_s(title, length, _T("Super D.P.S - マリアとカンパン"));
					return true;
				case CRC32_SDPS_TONO:
					_tcscpy_s(title, length, _T("Super D.P.S - 遠野の森"));
					return true;
				case CRC32_SDPS_KAIZOKU:
					_tcscpy_s(title, length, _T("Super D.P.S - うれしたのし海賊稼業"));
					return true;
			}
			_tcscpy_s(title, length, _T("Super D.P.S"));
			return true;
		case CRC32_YAKATA2:
			_tcscpy_s(title, length, _T("ALICEの館II"));
			return true;
	}
#elif defined(_SYSTEM3)
	switch(crc32_a) {
		case CRC32_AMBIVALENZ_FD:
		case CRC32_AMBIVALENZ_CD:
			_tcscpy_s(title, length, _T("AmbivalenZ －二律背反－"));
			return true;
		case CRC32_DPSALL:
			_tcscpy_s(title, length, _T("D.P.S. 全部"));
			return true;
		case CRC32_FUNNYBEE_CD:
//		case CRC32_FUNNYBEE_PATCH:
		case CRC32_FUNNYBEE_FD:
			_tcscpy_s(title, length, _T("宇宙快盗ファニーBee"));
			return true;
		case CRC32_ONLYYOU:
			_tcscpy_s(title, length, _T("Only You －世紀末のジュリエット達－"));
			return true;
		case CRC32_ONLYYOU_DEMO:
			_tcscpy_s(title, length, _T("Only You －世紀末のジュリエット達－ デモ版"));
			return true;
		case CRC32_PROSTUDENTG_CD:
			_tcscpy_s(title, length, _T("prostudent G"));
			return true;
		case CRC32_RANCE41:
			_tcscpy_s(title, length, _T("ランス 4.1 ～お薬工場を救え！～"));
			return true;
		case CRC32_RANCE42:
			_tcscpy_s(title, length, _T("ランス 4.2 ～エンジェル組～"));
			return true;
		case CRC32_AYUMI_CD:
			_tcscpy_s(title, length, _T("あゆみちゃん物語"));
			return true;
		case CRC32_AYUMI_JISSHA_256:
			_tcscpy_s(title, length, _T("あゆみちゃん物語 実写版"));
			return true;
		case CRC32_AYUMI_JISSHA_FULL:
			_tcscpy_s(title, length, _T("あゆみちゃん物語 フルカラー実写版"));
			return true;
		case CRC32_YAKATA3_CD:
		case CRC32_YAKATA3_FD:
			_tcscpy_s(title, length, _T("アリスの館3"));
			return true;
		case CRC32_HASHIRIONNA2:
			_tcscpy_s(title, length, _T("走り女2"));
			return true;
		case CRC32_TOUSHIN2_GD:
			_tcscpy_s(title, length, _T("闘神都市2 グラフィックディスク"));
			return true;
		case CRC32_TOUSHIN2_SP:
			_tcscpy_s(title, length, _T("闘神都市2 そして、それから…"));
			return true;
		case CRC32_OTOMESENKI:
			_tcscpy_s(title, length, _T("乙女戦記"));
			return true;
		case CRC32_NINGYO:
			_tcscpy_s(title, length, _T("人魚 -蘿子-"));
			return true;
		case CRC32_MUGENHOUYOU:
			_tcscpy_s(title, length, _T("夢幻泡影"));
			return true;
	}
#endif
	title[0] = _T('\0');
	return false;
}

