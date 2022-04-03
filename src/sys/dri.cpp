/*
	ALICE SOFT SYSTEM 3 for Win32

	[ DRI ]
*/

#include "dri.h"
#include <string.h>
#include "crc32.h"
#include "../fileio.h"

uint8* DRI::load(const char* file_name, int page, int* size)
{
	char tmp_name[20];
	strcpy_s(tmp_name, sizeof(tmp_name), file_name);

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
	const char* name = NULL;

	switch(crc32_a) {
		case CRC32_BUNKASAI:		// あぶない文化祭前夜
			name = "AMUS_AB.MDA";
			break;
		case CRC32_CRESCENT:		// クレセントムーンがぁる
			name = "AMUS_CRS.MDA";
			break;
		case CRC32_DPS:			// D.P.S. - Dream Program System
			name = "AMUS_DPS.MDA";
			break;
		case CRC32_DPS_SG:		// D.P.S. SG
//		case CRC32_DPS_SG2:		// D.P.S. SG set2
		case CRC32_DPS_SG3:		// D.P.S. SG set2
			name = "AMUS_SG.MDA";
			break;
		case CRC32_FUKEI:		// 婦警さんＶＸ
			name = "AMUS_VX.MDA";
			break;
		case CRC32_INTRUDER:		// Intruder -桜屋敷の探索-
			name = "AMUS_INT";
			break;
		case CRC32_TENGU:		// あぶないてんぐ伝説
			name = "AMUS_AT";
			break;
		case CRC32_TOUSHIN_HINT:	// 闘神都市 ヒントディスク
			name = "AMUS_T1";
			break;
		case CRC32_VAMPIRE:		// Little Vampire
		case CRC32_VAMPIRE_ENG:
			name = "AMUS_LP2";
			break;
		case CRC32_YAKATA:		// ALICEの館
			name = "AMUS_AL1";
			break;

		case CRC32_AYUMI_FD:		// あゆみちゃん物語 (FD)
		case CRC32_AYUMI_HINT:		// あゆみちゃん物語 ヒントディスク
		case CRC32_AYUMI_PROTO:		// あゆみちゃん物語 PROTO
			name = "AMUS_AYM";
			break;
		case CRC32_DALK_HINT:		// DALK ヒントディスク
			name = "AMUS_DLK";
			break;
		case CRC32_DRSTOP:		// Dr. STOP!
			name = "AMUS_DRS";
			break;
		case CRC32_PROSTUDENTG_FD:	// Prostudent -G- (FD)
			name = "AMUS_PSG";
			break;
		case CRC32_RANCE3_HINT:		// Rance3 ヒントディスク
			name = "AMUS_R3H";
			break;
		case CRC32_SDPS:		// Super D.P.S
			name = "AMUS_SDP";
			break;
		case CRC32_YAKATA2:		// ALICEの館II
			name = "AMUS_AL2";
			break;

		case CRC32_AMBIVALENZ_FD:	// AmbivalenZ (FD)
		case CRC32_AMBIVALENZ_CD:	// AmbivalenZ (CD)
			name = "AMUS_AMB.MDA";
			break;
		case CRC32_DPSALL:		// DPS全部
			name = "AMUS_ALL.MDA";
			break;
		case CRC32_FUNNYBEE_CD:		// Funny Bee (CD)
//		case CRC32_FUNNYBEE_PATCH:	// Funny Bee (CD + Patch)
		case CRC32_FUNNYBEE_FD:		// Funny Bee (FD)
			name = "AMUS_BEE.MDA";
			break;
		case CRC32_ONLYYOU:		// Only You
		case CRC32_ONLYYOU_DEMO:	// Only You (DEMO)
			name = "AMUS_OY.MDA";
			break;
		case CRC32_PROSTUDENTG_CD:	// Prostudent -G- (CD)
			name = "AMUS_PSG.MDA";
			break;
		case CRC32_RANCE41:		// Rance 4.1
		case CRC32_RANCE41_ENG:
			name = "AMUS_R41.MDA";
			break;
		case CRC32_RANCE42:		// Rance 4.2
		case CRC32_RANCE42_ENG:
			name = "AMUS_R42.MDA";
			break;
		case CRC32_AYUMI_CD:		// あゆみちゃん物語 (CD)
		case CRC32_AYUMI_JISSHA_256:	// あゆみちゃん物語 実写版
		case CRC32_AYUMI_JISSHA_FULL:	// あゆみちゃん物語 フルカラー実写版
			name = "AMUS_AYM.MDA";
			break;
		case CRC32_YAKATA3_CD:		// アリスの館３ (CD)
		case CRC32_YAKATA3_FD:		// アリスの館３ (FD)
			name = "AMUS_AL3.MDA";
			break;
		case CRC32_HASHIRIONNA2:	// 走り女２ (Rance 4.x ヒントディスク)
			name = "AMUS_RG2.MDA";
			break;
		case CRC32_TOUSHIN2_GD:		// 闘神都市２ グラフィックディスク
			name = "AMUS_T2";
			break;
		case CRC32_OTOMESENKI:		// 乙女戦記
			name = "AMUS_OTM.MDA";
			break;
		case CRC32_MUGENHOUYOU:		// 夢幻泡影
			name = "AMUS_MGN.MDA";
			break;
	}

	if(name == NULL) {
		return NULL;
	}

	SDL_RWops* rw = open_resource(name, "mda");
	if (!rw)
		return NULL;
	uint8 buf[4];

	// ページの位置を取得
	SDL_RWread(rw, buf, 4, 1);
	int link_sector = buf[0] | (buf[1] << 8);
	int data_sector = buf[2] | (buf[3] << 8);

	if(page > (data_sector - link_sector) * 128 - 1) {
		// ページ番号不正
		SDL_RWclose(rw);
		return NULL;
	}

	SDL_RWseek(rw, (link_sector - 1) * 256 + (page - 1) * 2, RW_SEEK_SET);
	SDL_RWread(rw, buf, 2, 1);

	int disk_index = buf[0];
	int link_index = buf[1];

	if(disk_index == 0 || disk_index == 0x1a) {
		// 欠番
		SDL_RWclose(rw);
		return NULL;
	}

	// AMUS.MDA以外にリンクされている場合はリソースを開き直す
	if(disk_index == 2) {
		SDL_RWclose(rw);
		switch(crc32_b) {
			case CRC32_DPS_SG_FAHREN:	// D.P.S. SG - Fahren Fliegen
				name = "BMUS_FAH";
				break;
			case CRC32_DPS_SG_KATEI:	// D.P.S. SG - 家庭教師はステキなお仕事
				name = "BMUS_KAT";
				break;
			case CRC32_DPS_SG_NOBUNAGA:	// D.P.S. SG - 信長の淫謀
				name = "BMUS_NOB";
				break;
			case CRC32_DPS_SG2_ANTIQUE:	// D.P.S. SG set2 - ANTIQUE HOUSE
				name = "BMUS_ANT";
				break;
			case CRC32_DPS_SG2_IKENAI:	// D.P.S. SG set2 - いけない内科検診再び
				name = "BMUS_NAI";
				break;
			case CRC32_DPS_SG2_AKAI:	// D.P.S. SG set2 - 朱い夜
				name = "BMUS_AKA";
				break;
			case CRC32_DPS_SG3_RABBIT:	// D.P.S. SG set3 - Rabbit P4P
				name = "BMUS_RAB";
				break;
			case CRC32_DPS_SG3_SHINKON:	// D.P.S. SG set3 - しんこんさんものがたり
				name = "BMUS_SIN";
				break;
			case CRC32_DPS_SG3_SOTSUGYOU:	// D.P.S. SG set3 - 卒業
				name = "BMUS_SOT";
				break;
			case CRC32_SDPS_MARIA:		// Super D.P.S - マリアとカンパン
				name = "BMUS_MTK";
				break;
			case CRC32_SDPS_TONO:		// Super D.P.S - 遠野の森
				name = "BMUS_TNM";
				break;
			case CRC32_SDPS_KAIZOKU:	// Super D.P.S - うれしたのし海賊稼業
				name = "BMUS_KAM";
				break;
			default:
				name = NULL;
				break;
		}
		if(name == NULL) {
			return NULL;
		}
		if((rw = open_resource(name, "mda")) == NULL) {
			return NULL;
		}
	} else if(disk_index == 3) {
		SDL_RWclose(rw);
		switch(crc32_a) {
			case CRC32_TOUSHIN_HINT:	// 闘神都市 ヒントディスク
				name = "CMUS_T1";
				break;
			default:
				name = NULL;
				break;
		}
		if(name == NULL) {
			return NULL;
		}
		if((rw = open_resource(name, "mda")) == NULL) {
			return NULL;
		}
	} else if(disk_index != 1) {
		// AMUS.MDA以外にリンクされている場合は失敗
		SDL_RWclose(rw);
		return NULL;
	}

	// データ取得
	SDL_RWseek(rw, link_index * 2, RW_SEEK_SET);
	SDL_RWread(rw, buf, 4, 1);
	int start_sector = buf[0] | (buf[1] << 8);
	int end_sector = buf[2] | (buf[3] << 8);

	if((*size = (end_sector - start_sector) * 256) == 0) {
		// サイズ不正
		SDL_RWclose(rw);
		return NULL;
	}
	uint8* buffer = (uint8*)malloc(*size);
	SDL_RWseek(rw, (start_sector - 1) * 256, RW_SEEK_SET);
	SDL_RWread(rw, buffer, *size, 1);

	SDL_RWclose(rw);

	return buffer;
}
