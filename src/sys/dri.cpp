/*
	ALICE SOFT SYSTEM 3 for Win32

	[ DRI ]
*/

#include "dri.h"
#include "crc32.h"
#include "../fileio.h"

uint8* DRI::load(const char* file_name, int page, int* size)
{
	char tmp_name[_MAX_PATH];
	strcpy_s(tmp_name, _MAX_PATH, file_name);

	FILEIO* fio = new FILEIO();

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

uint8* DRI::load_mda(uint32 crc32_a, uint32 crc32_b, int page, int* size)
{
	// データ取得
	const char* fname = NULL;

	switch(crc32_a) {
		case CRC32_BUNKASAI:		// あぶない文化祭前夜
			fname = "AMUS_AB.MDA";
			break;
		case CRC32_CRESCENT:		// クレセントムーンがぁる
			fname = "AMUS_CRS.MDA";
			break;
		case CRC32_DPS:			// D.P.S. - Dream Program System
			fname = "AMUS_DPS.MDA";
			break;
		case CRC32_DPS_SG:		// D.P.S. SG
//		case CRC32_DPS_SG2:		// D.P.S. SG set2
		case CRC32_DPS_SG3:		// D.P.S. SG set2
			fname = "AMUS_SG.MDA";
			break;
		case CRC32_FUKEI:		// 婦警さんＶＸ
			fname = "AMUS_VX.MDA";
			break;
		case CRC32_INTRUDER:		// Intruder -桜屋敷の探索-
			fname = "AMUS_INT";
			break;
		case CRC32_TENGU:		// あぶないてんぐ伝説
			fname = "AMUS_AT";
			break;
		case CRC32_TOUSHIN_HINT:	// 闘神都市 ヒントディスク
			fname = "AMUS_T1";
			break;
		case CRC32_VAMPIRE:		// Little Vampire
			fname = "AMUS_LP2";
			break;
		case CRC32_YAKATA:		// ALICEの館
			fname = "AMUS_AL1";
			break;

		case CRC32_AYUMI_FD:		// あゆみちゃん物語 (FD)
		case CRC32_AYUMI_HINT:		// あゆみちゃん物語 ヒントディスク
		case CRC32_AYUMI_PROTO:		// あゆみちゃん物語 PROTO
			fname = "AMUS_AYM";
			break;
		case CRC32_DALK_HINT:		// DALK ヒントディスク
			fname = "AMUS_DLK";
			break;
		case CRC32_DRSTOP:		// Dr. STOP!
			fname = "AMUS_DRS";
			break;
		case CRC32_PROSTUDENTG_FD:	// Prostudent -G- (FD)
			fname = "AMUS_PSG";
			break;
		case CRC32_RANCE3_HINT:		// Rance3 ヒントディスク
			fname = "AMUS_R3H";
			break;
		case CRC32_SDPS:		// Super D.P.S
			fname = "AMUS_SDP";
			break;
		case CRC32_YAKATA2:		// ALICEの館II
			fname = "AMUS_AL2";
			break;

		case CRC32_AMBIVALENZ_FD:	// AmbivalenZ (FD)
		case CRC32_AMBIVALENZ_CD:	// AmbivalenZ (CD)
			fname = "AMUS_AMB.MDA";
			break;
		case CRC32_DPSALL:		// DPS全部
			fname = "AMUS_ALL.MDA";
			break;
		case CRC32_FUNNYBEE_CD:		// Funny Bee (CD)
//		case CRC32_FUNNYBEE_PATCH:	// Funny Bee (CD + Patch)
		case CRC32_FUNNYBEE_FD:		// Funny Bee (FD)
			fname = "AMUS_BEE.MDA";
			break;
		case CRC32_ONLYYOU:		// Only You
		case CRC32_ONLYYOU_DEMO:	// Only You (DEMO)
			fname = "AMUS_OY.MDA";
			break;
		case CRC32_PROSTUDENTG_CD:	// Prostudent -G- (CD)
			fname = "AMUS_PSG.MDA";
			break;
		case CRC32_RANCE41:		// Rance 4.1
			fname = "AMUS_R41.MDA";
			break;
		case CRC32_RANCE42:		// Rance 4.2
			fname = "AMUS_R42.MDA";
			break;
		case CRC32_AYUMI_CD:		// あゆみちゃん物語 (CD)
		case CRC32_AYUMI_JISSHA_256:	// あゆみちゃん物語 実写版
		case CRC32_AYUMI_JISSHA_FULL:	// あゆみちゃん物語 フルカラー実写版
			fname = "AMUS_AYM.MDA";
			break;
		case CRC32_YAKATA3_CD:		// アリスの館３ (CD)
		case CRC32_YAKATA3_FD:		// アリスの館３ (FD)
			fname = "AMUS_AL3.MDA";
			break;
		case CRC32_HASHIRIONNA2:	// 走り女２ (Rance 4.x ヒントディスク)
			fname = "AMUS_RG2.MDA";
			break;
		case CRC32_TOUSHIN2_GD:		// 闘神都市２ グラフィックディスク
			fname = "AMUS_T2";
			break;
		case CRC32_OTOMESENKI:		// 乙女戦記
			fname = "AMUS_OTM.MDA";
			break;
		case CRC32_MUGENHOUYOU:		// 夢幻泡影
			fname = "AMUS_MGN.MDA";
			break;
	}

	if(fname == NULL) {
		return NULL;
	}

	FILE* fp = fopen(fname, "rb");
	if (!fp)
		return NULL;
	uint8 buf[4];

	// ページの位置を取得
	fread(buf, 4, 1, fp);
	int link_sector = buf[0] | (buf[1] << 8);
	int data_sector = buf[2] | (buf[3] << 8);

	if(page > (data_sector - link_sector) * 128 - 1) {
		// ページ番号不正
		fclose(fp);
		return NULL;
	}

	fseek(fp, (link_sector - 1) * 256 + (page - 1) * 2, SEEK_SET);
	fread(buf, 2, 1, fp);
	int disk_index = buf[0];
	int link_index = buf[1];

	if(disk_index == 0 || disk_index == 0x1a) {
		// 欠番
		fclose(fp);
		return NULL;
	}

	// AMUS.MDA以外にリンクされている場合はリソースを開き直す
	if(disk_index == 2) {
		fclose(fp);
		switch(crc32_b) {
			case CRC32_DPS_SG_FAHREN:	// D.P.S. SG - Fahren Fliegen
				fname = "BMUS_FAH";
				break;
			case CRC32_DPS_SG_KATEI:	// D.P.S. SG - 家庭教師はステキなお仕事
				fname = "BMUS_KAT";
				break;
			case CRC32_DPS_SG_NOBUNAGA:	// D.P.S. SG - 信長の淫謀
				fname = "BMUS_NOB";
				break;
			case CRC32_DPS_SG2_ANTIQUE:	// D.P.S. SG set2 - ANTIQUE HOUSE
				fname = "BMUS_ANT";
				break;
			case CRC32_DPS_SG2_IKENAI:	// D.P.S. SG set2 - いけない内科検診再び
				fname = "BMUS_NAI";
				break;
			case CRC32_DPS_SG2_AKAI:	// D.P.S. SG set2 - 朱い夜
				fname = "BMUS_AKA";
				break;
			case CRC32_DPS_SG3_RABBIT:	// D.P.S. SG set3 - Rabbit P4P
				fname = "BMUS_RAB";
				break;
			case CRC32_DPS_SG3_SHINKON:	// D.P.S. SG set3 - しんこんさんものがたり
				fname = "BMUS_SIN";
				break;
			case CRC32_DPS_SG3_SOTSUGYOU:	// D.P.S. SG set3 - 卒業
				fname = "BMUS_SOT";
				break;
			case CRC32_SDPS_MARIA:		// Super D.P.S - マリアとカンパン
				fname = "BMUS_MTK";
				break;
			case CRC32_SDPS_TONO:		// Super D.P.S - 遠野の森
				fname = "BMUS_TNM";
				break;
			case CRC32_SDPS_KAIZOKU:	// Super D.P.S - うれしたのし海賊稼業
				fname = "BMUS_KAM";
				break;
			default:
				fname = NULL;
				break;
		}
		if(fname == NULL) {
			return NULL;
		}
		fp = fopen(fname, "rb");
		if (!fp)
			return NULL;
	} else if(disk_index == 3) {
		fclose(fp);
		switch(crc32_a) {
			case CRC32_TOUSHIN_HINT:	// 闘神都市 ヒントディスク
				fname = "CMUS_T1";
				break;
			default:
				fname = NULL;
				break;
		}
		if(fname == NULL) {
			return NULL;
		}
		fp = fopen(fname, "rb");
		if (!fp)
			return NULL;
	} else if(disk_index != 1) {
		// AMUS.MDA以外にリンクされている場合は失敗
		fclose(fp);
		return NULL;
	}

	// データ取得
	fseek(fp, link_index * 2, SEEK_SET);
	fread(buf, 4, 1, fp);
	int start_sector = buf[0] | (buf[1] << 8);
	int end_sector = buf[2] | (buf[3] << 8);

	if((*size = (end_sector - start_sector) * 256) == 0) {
		// サイズ不正
		return NULL;
	}
	uint8* buffer = (uint8*)malloc(*size);
	fseek(fp, (start_sector - 1) * 256, SEEK_SET);
	fread(buffer, *size, 1, fp);

	fclose(fp);

	return buffer;
}

