/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - crc32 ]
*/

#include "nact.h"
#include "crc32.h"
#include "../fileio.h"

uint32 NACT::calc_crc32()
{
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
FILE* fp = fopen("crc32.txt", "w");
fprintf(fp, "%x\n", crc);
fclose(fp);
#endif
	return crc;
}

bool NACT::get_title(_TCHAR title[], int length)
{
#if defined(_SYSTEM1)

#if defined(_CRESCENT)
	_tcscpy_s(title, length, _T("クレセントムーンがぁる"));
	return true;
#elif defined(_DPS)
	_tcscpy_s(title, length, _T("D.P.S - Dream Program System"));
	return true;
#elif defined(_FUKEI)
	_tcscpy_s(title, length, _T("婦警さんＶＸ"));
	return true;
#elif defined(_INTRUDER)
	_tcscpy_s(title, length, _T("Intruder -桜屋敷の探索-"));
	return true;
#elif defined(_TENGU)
	_tcscpy_s(title, length, _T("あぶないてんぐ伝説"));
	return true;
#elif defined(_VAMPIRE)
	_tcscpy_s(title, length, _T("Little Vampire"));
	return true;
#else
	title[0] = _T('\0');
	return false;
#endif

#elif defined(_SYSTEM2)

	switch(crc32) {
		case CRC32_AYUMI_PROTO:
			_tcscpy_s(title, length, _T("あゆみちゃん物語 PROTO"));
			return true;
		case CRC32_SDPS_MARIA:
		case CRC32_SDPS_TONO:
		case CRC32_SDPS_KAIZOKU:
			_tcscpy_s(title, length, _T("Super D.P.S"));
			return true;
		case CRC32_PROSTUDENTG_FD:
			_tcscpy_s(title, length, _T("prostudent G"));
			return true;
	}
	title[0] = _T('\0');
	return false;

#else

	switch(crc32) {
		case CRC32_AMBIVALENZ_FD:
		case CRC32_AMBIVALENZ_CD:
			_tcscpy_s(title, length, _T("AmbivalenZ −二律背反−"));
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
			_tcscpy_s(title, length, _T("Only You −世紀末のジュリエット達−"));
			return true;
		case CRC32_ONLYYOU_DEMO:
			_tcscpy_s(title, length, _T("Only You −世紀末のジュリエット達− デモ版"));
			return true;
		case CRC32_PROSTUDENTG_CD:
			_tcscpy_s(title, length, _T("prostudent G"));
			return true;
		case CRC32_RANCE41:
			_tcscpy_s(title, length, _T("ランス 4.1 〜お薬工場を救え！〜"));
			return true;
		case CRC32_RANCE42:
			_tcscpy_s(title, length, _T("ランス 4.2 〜エンジェル組〜"));
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
	title[0] = _T('\0');
	return false;

#endif
}

