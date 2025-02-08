/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - bmp loader ]
*/

#include "ags.h"
#include "fileio.h"

void AGS::load_bmp(const char* file_name)
{
	// ヘッダ取得
	int sx = 0, sy = 0;

	// Jコマンドの処理
	if(set_cg_dest) {
		sx = cg_dest_x;
		sy = cg_dest_y;
		set_cg_dest = false;
	}

	if(extract_cg) {
		WARNING("not implemented");
	}
}
