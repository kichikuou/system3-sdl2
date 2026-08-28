/*
	ALICE SOFT SYSTEM 3 for Win32

	[ DRI ]
*/

#include "dri.h"
#include <memory>
#include <string.h>
#include <SDL.h>
#include "game_id.h"
#include "fileio.h"

void Dri::open(const char* file_name)
{
	link_table.clear();
	fname = file_name;

	auto fio = FILEIO::open(file_name, FILEIO_READ_BINARY);
	if (!fio)
		return;

	int link_sector = fio->getw();
	int data_sector = fio->getw();

	link_table.resize((data_sector - link_sector) * 256);
	fio->seek((link_sector - 1) * 256, SEEK_SET);
	fio->read(link_table.data(), 256 * (data_sector - link_sector));
}

std::vector<uint8> Dri::load(int page)
{
	if (static_cast<size_t>(page) >= link_table.size() / 2)
		return {};

	int disk_index = link_table[(page - 1) * 2];
	int link_index = link_table[(page - 1) * 2 + 1];
	if (disk_index == 0 || disk_index == 0x1a)
		return {};

	fname[0] = 'A' + disk_index - 1;
	auto fio = FILEIO::open(fname.c_str(), FILEIO_READ_BINARY);
	if (!fio)
		return {};

	fio->seek(link_index * 2, SEEK_SET);
	int start_sector = fio->getw();
	int end_sector = fio->getw();
	if (end_sector <= start_sector)
		return {};

	std::vector<uint8_t> buffer((end_sector - start_sector) * 256);
	fio->seek((start_sector - 1) * 256, SEEK_SET);
	fio->read(buffer.data(), 256 * (end_sector - start_sector));

	return buffer;
}

// static
std::vector<uint8> Dri::load_mda(const GameId& game_id, int page)
{
	// データ取得
	const char* name = NULL;

	switch (game_id.game) {
		case GameId::BUNKASAI:
			name = "AMUS_AB.MDA";
			break;
		case GameId::CRESCENT:
			name = "AMUS_CRS.MDA";
			break;
		case GameId::DPS:
			name = "AMUS_DPS.MDA";
			break;
		case GameId::DPS_SG_FAHREN:
		case GameId::DPS_SG_KATEI:
		case GameId::DPS_SG_NOBUNAGA:
		case GameId::DPS_SG2_ANTIQUE:
		case GameId::DPS_SG2_IKENAI:
		case GameId::DPS_SG2_AKAI:
		case GameId::DPS_SG3_RABBIT:
		case GameId::DPS_SG3_SHINKON:
		case GameId::DPS_SG3_SOTSUGYOU:
			name = "AMUS_SG.MDA";
			break;
		case GameId::FUKEI:
			name = "AMUS_VX.MDA";
			break;
		case GameId::INTRUDER:
			name = "AMUS_INT.MDA";
			break;
		case GameId::TENGU:
			name = "AMUS_AT.MDA";
			break;
		case GameId::TOUSHIN_HINT:
			name = "AMUS_T1.MDA";
			break;
		case GameId::LITTLE_VAMPIRE:
			name = "AMUS_LP2.MDA";
			break;
		case GameId::YAKATA:
			name = "AMUS_AL1.MDA";
			break;

		case GameId::AYUMI_FD:
		case GameId::AYUMI_HINT:
		case GameId::AYUMI_PROTO:
			name = "AMUS_AYM.MDA";
			break;
		case GameId::DALK_HINT:
			name = "AMUS_DLK.MDA";
			break;
		case GameId::DRSTOP:
			name = "AMUS_DRS.MDA";
			break;
		case GameId::PROG_FD:
			name = "AMUS_PSG.MDA";
			break;
		case GameId::RANCE3_HINT:
			name = "AMUS_R3H.MDA";
			break;
		case GameId::SDPS_MARIA:
		case GameId::SDPS_TONO:
		case GameId::SDPS_KAIZOKU:
			name = "AMUS_SDP.MDA";
			break;
		case GameId::YAKATA2:
			name = "AMUS_AL2.MDA";
			break;

		case GameId::AMBIVALENZ_FD:
		case GameId::AMBIVALENZ_CD:
			name = "AMUS_AMB.MDA";
			break;
		case GameId::DPS_ALL:
			name = "AMUS_ALL.MDA";
			break;
		case GameId::FUNNYBEE_CD:
		case GameId::FUNNYBEE_FD:
			name = "AMUS_BEE.MDA";
			break;
		case GameId::ONLYYOU:
		case GameId::ONLYYOU_DEMO:
			name = "AMUS_OY.MDA";
			break;
		case GameId::PROG_CD:
			name = "AMUS_PSG.MDA";
			break;
		case GameId::RANCE4:
			name = "AMUS_R4.MDA";
			break;
		case GameId::RANCE41:
			name = "AMUS_R41.MDA";
			break;
		case GameId::RANCE42:
			name = "AMUS_R42.MDA";
			break;
		case GameId::AYUMI_CD:
		case GameId::AYUMI_LIVE_256:
		case GameId::AYUMI_LIVE_FULL:
			name = "AMUS_AYM.MDA";
			break;
		case GameId::YAKATA3_CD:
		case GameId::YAKATA3_FD:
			name = "AMUS_AL3.MDA";
			break;
		case GameId::HASHIRIONNA2:
			name = "AMUS_RG2.MDA";
			break;
		case GameId::TOUSHIN2_GD:
			name = "AMUS_T2.MDA";
			break;
		case GameId::OTOME:
			name = "AMUS_OTM.MDA";
			break;
		case GameId::MUGEN:
			name = "AMUS_MGN.MDA";
			break;
		case GameId::GAKUEN_KING:
			name = "AMUS_KNG.MDA";
			break;
	}

	if(name == NULL) {
		return {};
	}

	SDL_RWops* rw = open_resource(name, "mda");
	if (!rw)
		return {};
	uint8 buf[4];

	// ページの位置を取得
	SDL_RWread(rw, buf, 4, 1);
	int link_sector = buf[0] | (buf[1] << 8);
	int data_sector = buf[2] | (buf[3] << 8);

	if(page > (data_sector - link_sector) * 128 - 1) {
		// ページ番号不正
		SDL_RWclose(rw);
		return {};
	}

	SDL_RWseek(rw, (link_sector - 1) * 256 + (page - 1) * 2, RW_SEEK_SET);
	SDL_RWread(rw, buf, 2, 1);

	int disk_index = buf[0];
	int link_index = buf[1];

	if(disk_index == 0 || disk_index == 0x1a) {
		// 欠番
		SDL_RWclose(rw);
		return {};
	}

	// AMUS.MDA以外にリンクされている場合はリソースを開き直す
	if(disk_index == 2) {
		SDL_RWclose(rw);
		switch (game_id.game) {
			case GameId::DPS_SG_FAHREN:
				name = "BMUS_FAH.MDA";
				break;
			case GameId::DPS_SG_KATEI:
				name = "BMUS_KAT.MDA";
				break;
			case GameId::DPS_SG_NOBUNAGA:
				name = "BMUS_NOB.MDA";
				break;
			case GameId::DPS_SG2_ANTIQUE:
				name = "BMUS_ANT.MDA";
				break;
			case GameId::DPS_SG2_IKENAI:
				name = "BMUS_NAI.MDA";
				break;
			case GameId::DPS_SG2_AKAI:
				name = "BMUS_AKA.MDA";
				break;
			case GameId::DPS_SG3_RABBIT:
				name = "BMUS_RAB.MDA";
				break;
			case GameId::DPS_SG3_SHINKON:
				name = "BMUS_SIN.MDA";
				break;
			case GameId::DPS_SG3_SOTSUGYOU:
				name = "BMUS_SOT.MDA";
				break;
			case GameId::SDPS_MARIA:
				name = "BMUS_MTK.MDA";
				break;
			case GameId::SDPS_TONO:
				name = "BMUS_TNM.MDA";
				break;
			case GameId::SDPS_KAIZOKU:
				name = "BMUS_KAM.MDA";
				break;
			default:
				name = NULL;
				break;
		}
		if(name == NULL) {
			return {};
		}
		if((rw = open_resource(name, "mda")) == NULL) {
			return {};
		}
	} else if(disk_index == 3) {
		SDL_RWclose(rw);
		switch (game_id.game) {
			case GameId::TOUSHIN_HINT:
				name = "CMUS_T1.MDA";
				break;
			default:
				name = NULL;
				break;
		}
		if(name == NULL) {
			return {};
		}
		if((rw = open_resource(name, "mda")) == NULL) {
			return {};
		}
	} else if(disk_index != 1) {
		// AMUS.MDA以外にリンクされている場合は失敗
		SDL_RWclose(rw);
		return {};
	}

	// データ取得
	SDL_RWseek(rw, link_index * 2, RW_SEEK_SET);
	SDL_RWread(rw, buf, 4, 1);
	int start_sector = buf[0] | (buf[1] << 8);
	int end_sector = buf[2] | (buf[3] << 8);

	int size = (end_sector - start_sector) * 256;
	if (size == 0) {
		// サイズ不正
		SDL_RWclose(rw);
		return {};
	}
	std::vector<uint8_t> buffer(size);
	SDL_RWseek(rw, (start_sector - 1) * 256, RW_SEEK_SET);
	SDL_RWread(rw, buffer.data(), size, 1);

	SDL_RWclose(rw);

	return buffer;
}
