/*
	ALICE SOFT SYSTEM 3 for Win32

	[ DRI ]
*/

#include "dri.h"
#include "crc32.h"
#include "../fileio.h"
#if defined(_SYSTEM1)
#include "../res1/resource.h"
#elif defined(_SYSTEM2)
#include "../res2/resource.h"
#else
#include "../res3/resource.h"
#endif

uint8 *DRI::load(const char *file_name, int page, int *size)
{
	char tmp_name[_MAX_PATH];
	strcpy_s(tmp_name, _MAX_PATH, file_name);

	FILEIO *fio = new FILEIO();

	// ページの位置を取得
	tmp_name[0] = 'A';
	if(!fio->Fopen(tmp_name, FILEIO_READ_BINARY)) {
		delete fio;
		return NULL;
	}

	int link_sector = fio->Fgetc();
	link_sector |= fio->Fgetc() << 8;
	int data_sector = fio->Fgetc();
	data_sector |= fio->Fgetc() << 8;

	if(page > (data_sector - link_sector) * 128 - 1) {
		// ページ番号不正
		fio->Fclose();
		delete fio;
		return NULL;
	}

	fio->Fseek((link_sector - 1) * 256 + (page - 1) * 2, FILEIO_SEEK_SET);
	int disk_index = fio->Fgetc();
	int link_index = fio->Fgetc();

	if(disk_index == 0 || disk_index == 0x1a) {
		// 欠番
		fio->Fclose();
		delete fio;
		return NULL;
	}

	// A??.DAT以外にリンクされている場合はファイルを開き直す
	if(disk_index != 1) {
		tmp_name[0] = 'A' + disk_index - 1;
		fio->Fclose();
		if(!fio->Fopen(tmp_name, FILEIO_READ_BINARY)) {
			delete fio;
			return NULL;
		}
	}

	// データ取得
	fio->Fseek(link_index * 2, FILEIO_SEEK_SET);
	int start_sector = fio->Fgetc();
	start_sector |= fio->Fgetc() << 8;
	int end_sector = fio->Fgetc();
	end_sector |= fio->Fgetc() << 8;

	if((*size = (end_sector - start_sector) * 256) == 0) {
		// サイズ不正
		fio->Fclose();
		delete fio;
		return NULL;
	}

	uint8 *buffer = (uint8 *)malloc(*size);
	fio->Fseek((start_sector - 1) * 256, FILEIO_SEEK_SET);
	fio->Fread(buffer, 256, end_sector - start_sector);

	fio->Fclose();
	delete fio;

	return buffer;
}

uint8 *DRI::load_mda(HINSTANCE hInst, uint32 crc32_a, uint32 crc32_b, int page, int *size)
{
	// データ取得
	HGLOBAL hGlobal = NULL;

#if defined(_SYSTEM1)
	switch(crc32_a) {
		case CRC32_BUNKASAI:		// あぶない文化祭前夜
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_AB ), _T("mda")));
			break;
		case CRC32_CRESCENT:		// クレセントムーンがぁる
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_CRS), _T("mda")));
			break;
		case CRC32_DPS:			// D.P.S. - Dream Program System
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_DPS), _T("mda")));
			break;
		case CRC32_DPS_SG:		// D.P.S. SG
//		case CRC32_DPS_SG2:		// D.P.S. SG set2
		case CRC32_DPS_SG3:		// D.P.S. SG set2
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_SG ), _T("mda")));
			break;
		case CRC32_FUKEI:		// 婦警さんＶＸ
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_VX ), _T("mda")));
			break;
		case CRC32_INTRUDER:		// Intruder -桜屋敷の探索-
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_INT), _T("mda")));
			break;
		case CRC32_TENGU:		// あぶないてんぐ伝説
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_AT ), _T("mda")));
			break;
		case CRC32_TOUSHIN_HINT:	// 闘神都市 ヒントディスク
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_T1 ), _T("mda")));
			break;
		case CRC32_VAMPIRE:		// Little Vampire
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_LP2), _T("mda")));
			break;
		case CRC32_YAKATA:		// ALICEの館
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_AL1), _T("mda")));
			break;
	}
#elif defined(_SYSTEM2)
	switch(crc32_a) {
		case CRC32_AYUMI_FD:		// あゆみちゃん物語 (FD)
		case CRC32_AYUMI_HINT:		// あゆみちゃん物語 ヒントディスク
		case CRC32_AYUMI_PROTO:		// あゆみちゃん物語 PROTO
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_AYM), _T("mda")));
			break;
		case CRC32_DALK_HINT:		// DALK ヒントディスク
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_DLK), _T("mda")));
			break;
		case CRC32_DRSTOP:		// Dr. STOP!
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_DRS), _T("mda")));
			break;
		case CRC32_PROSTUDENTG_FD:	// Prostudent -G- (FD)
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_PSG), _T("mda")));
			break;
		case CRC32_RANCE3_HINT:		// Rance3 ヒントディスク
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_R3H), _T("mda")));
			break;
		case CRC32_SDPS:		// Super D.P.S
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_SDP), _T("mda")));
			break;
		case CRC32_YAKATA2:		// ALICEの館II
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_AL2), _T("mda")));
			break;
	}
#elif defined(_SYSTEM3)
	switch(crc32_a) {
		case CRC32_AMBIVALENZ_FD:	// AmbivalenZ (FD)
		case CRC32_AMBIVALENZ_CD:	// AmbivalenZ (CD)
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_AMB), _T("mda")));
			break;
		case CRC32_DPSALL:		// DPS全部
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_ALL), _T("mda")));
			break;
		case CRC32_FUNNYBEE_CD:		// Funny Bee (CD)
//		case CRC32_FUNNYBEE_PATCH:	// Funny Bee (CD + Patch)
		case CRC32_FUNNYBEE_FD:		// Funny Bee (FD)
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_BEE), _T("mda")));
			break;
		case CRC32_ONLYYOU:		// Only You
		case CRC32_ONLYYOU_DEMO:	// Only You (DEMO)
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_OY ), _T("mda")));
			break;
		case CRC32_PROSTUDENTG_CD:	// Prostudent -G- (CD)
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_PSG), _T("mda")));
			break;
		case CRC32_RANCE41:		// Rance 4.1
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_R41), _T("mda")));
			break;
		case CRC32_RANCE42:		// Rance 4.2
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_R42), _T("mda")));
			break;
		case CRC32_AYUMI_CD:		// あゆみちゃん物語 (CD)
		case CRC32_AYUMI_JISSHA_256:	// あゆみちゃん物語 実写版
		case CRC32_AYUMI_JISSHA_FULL:	// あゆみちゃん物語 フルカラー実写版
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_AYM), _T("mda")));
			break;
		case CRC32_YAKATA3_CD:		// アリスの館３ (CD)
		case CRC32_YAKATA3_FD:		// アリスの館３ (FD)
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_AL3), _T("mda")));
			break;
		case CRC32_HASHIRIONNA2:	// 走り女２ (Rance 4.x ヒントディスク)
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_RG2), _T("mda")));
			break;
		case CRC32_TOUSHIN2_GD:		// 闘神都市２ グラフィックディスク
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_T2 ), _T("mda")));
			break;
		case CRC32_OTOMESENKI:		// 乙女戦記
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_OTM), _T("mda")));
			break;
		case CRC32_MUGENHOUYOU:		// 夢幻泡影
			hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_AMUS_MGN), _T("mda")));
			break;
	}
#endif

	if(hGlobal == NULL) {
		return NULL;
	}

	uint8 *data = (uint8 *)LockResource(hGlobal);

	// ページの位置を取得
	int link_sector = data[0] | (data[1] << 8);
	int data_sector = data[2] | (data[3] << 8);

	if(page > (data_sector - link_sector) * 128 - 1) {
		// ページ番号不正
		return NULL;
	}

	int disk_index = data[(link_sector - 1) * 256 + (page - 1) * 2 + 0];
	int link_index = data[(link_sector - 1) * 256 + (page - 1) * 2 + 1];

	if(disk_index == 0 || disk_index == 0x1a) {
		// 欠番
		return NULL;
	}

	// AMUS.MDA以外にリンクされている場合はリソースを開き直す
#if defined(_SYSTEM1)
	if(disk_index == 2) {
		switch(crc32_b) {
			case CRC32_DPS_SG_FAHREN:	// D.P.S. SG - Fahren Fliegen
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_FAH), _T("mda")));
				break;
			case CRC32_DPS_SG_KATEI:	// D.P.S. SG - 家庭教師はステキなお仕事
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_KAT), _T("mda")));
				break;
			case CRC32_DPS_SG_NOBUNAGA:	// D.P.S. SG - 信長の淫謀
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_NOB), _T("mda")));
				break;
			case CRC32_DPS_SG2_ANTIQUE:	// D.P.S. SG set2 - ANTIQUE HOUSE
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_ANT), _T("mda")));
				break;
			case CRC32_DPS_SG2_IKENAI:	// D.P.S. SG set2 - いけない内科検診再び
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_NAI), _T("mda")));
				break;
			case CRC32_DPS_SG2_AKAI:	// D.P.S. SG set2 - 朱い夜
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_AKA), _T("mda")));
				break;
			case CRC32_DPS_SG3_RABBIT:	// D.P.S. SG set3 - Rabbit P4P
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_RAB), _T("mda")));
				break;
			case CRC32_DPS_SG3_SHINKON:	// D.P.S. SG set3 - しんこんさんものがたり
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_SIN), _T("mda")));
				break;
			case CRC32_DPS_SG3_SOTSUGYOU:	// D.P.S. SG set3 - 卒業
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_SOT), _T("mda")));
				break;
			default:
				hGlobal = NULL;
				break;
		}
		if(hGlobal == NULL) {
			return NULL;
		}
		data = (uint8 *)LockResource(hGlobal);
	} else if(disk_index == 3) {
		switch(crc32_a) {
			case CRC32_TOUSHIN_HINT:	// 闘神都市 ヒントディスク
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_CMUS_T1 ), _T("mda")));
				break;
			default:
				hGlobal = NULL;
				break;
		}
		if(hGlobal == NULL) {
			return NULL;
		}
		data = (uint8 *)LockResource(hGlobal);
	} else
#elif defined(_SYSTEM2)
	if(disk_index == 2) {
		switch(crc32_b) {
			case CRC32_SDPS_MARIA:		// Super D.P.S - マリアとカンパン
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_MTK), _T("mda")));
				break;
			case CRC32_SDPS_TONO:		// Super D.P.S - 遠野の森
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_TNM), _T("mda")));
				break;
			case CRC32_SDPS_KAIZOKU:	// Super D.P.S - うれしたのし海賊稼業
				hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDR_BMUS_KAM), _T("mda")));
				break;
			default:
				hGlobal = NULL;
				break;
		}
		if(hGlobal == NULL) {
			return NULL;
		}
		data = (uint8 *)LockResource(hGlobal);
	} else
#endif
	// AMUS.MDA以外にリンクされている場合は失敗
	if(disk_index != 1) {
		return NULL;
	}

	// データ取得
	int start_sector = data[link_index * 2 + 0] | (data[link_index * 2 + 1] << 8);
	int end_sector = data[link_index * 2 + 2] | (data[link_index * 2 + 3] << 8);

	if((*size = (end_sector - start_sector) * 256) == 0) {
		// サイズ不正
		return NULL;
	}

	uint8 *buffer = (uint8 *)malloc(*size);
	memcpy(buffer, &data[(start_sector - 1) * 256], *size);

	return buffer;
}

