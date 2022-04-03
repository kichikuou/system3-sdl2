/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - bmp loader ]
*/

#include "ags.h"
#include "../fileio.h"

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
		assert(false);
#if 0
		// BMP読み込み
		WCHAR wszFilePath[_MAX_PATH];
		MultiByteToWideChar(CP_ACP, 0, FILEIO::GetFilePath(file_name), -1, wszFilePath, _MAX_PATH);
		Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromFile(wszFilePath);
		for(unsigned int y = 0; y < pBitmap->GetHeight(); y++) {
			for(unsigned int x = 0; x < pBitmap->GetWidth(); x++) {
				Gdiplus::Color srcColor;
				pBitmap->GetPixel(x, y, &srcColor);
				vram[dest_screen][y + sy][sx + x] = SETPALETTE256(srcColor.GetR(), srcColor.GetG(), srcColor.GetB()) | 0x80000000;
			}
		}

		// 画面更新
		if(dest_screen == 0) {
			draw_screen(sx, sy, pBitmap->GetWidth(), pBitmap->GetHeight());
		}

		// BMP開放
		delete pBitmap;
#endif
	}
}
