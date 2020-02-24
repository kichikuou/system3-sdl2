/*
	ALICE SOFT SYSTEM 3 for Win32

	[ DRI ]
*/

#include "dri.h"
#include "crc32.h"
#include "../fileio.h"

uint8* DRI::load(const char* path, int page, int* size)
{
	char base_path[_MAX_PATH];
	strcpy_s(base_path, _MAX_PATH, path);

	FILEIO* fio = new FILEIO();

	// ページの位置を取得
	base_path[0] = 'A';

	if(!fio->Fopen(base_path, FILEIO_READ_BINARY)) {
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
		base_path[0] = 'A' + disk_index - 1;
		fio->Fclose();
		if(!fio->Fopen(base_path, FILEIO_READ_BINARY)) {
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

	int buffer_size = (end_sector - start_sector) * 256;
	uint8* buffer = (uint8*)malloc(buffer_size);
	fio->Fseek((start_sector - 1) * 256, FILEIO_SEEK_SET);
	fio->Fread(buffer, 256, end_sector - start_sector);

	fio->Fclose();
	delete fio;

	*size = buffer_size;
	return buffer;
}

uint8* DRI::load_mda(uint32 crc32, int page, int* size)
{
	// データ取得
	const char* fname = NULL;

	switch(crc32) {
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
		return NULL;
	}

	fseek(fp, (link_sector - 1) * 256 + (page - 1) * 2, SEEK_SET);
	fread(buf, 2, 1, fp);
	int disk_index = buf[0];
	int link_index = buf[1];

	if(disk_index == 0 || disk_index == 0x1a) {
		// 欠番
		return NULL;
	}

	// データ取得
	fseek(fp, link_index * 2, SEEK_SET);
	fread(buf, 4, 1, fp);
	int start_sector = buf[0] | (buf[1] << 8);
	int end_sector = buf[2] | (buf[3] << 8);

	int buffer_size = (end_sector - start_sector) * 256;
	uint8* buffer = (uint8*)malloc(buffer_size);
	fseek(fp, (start_sector - 1) * 256, SEEK_SET);
	fread(buffer, buffer_size, 1, fp);

	fclose(fp);

	*size = buffer_size;
	return buffer;
}

