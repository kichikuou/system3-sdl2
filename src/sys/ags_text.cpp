/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - text ]
*/

#include <algorithm>
#include <limits.h>
#include <string_view>
#include "ags.h"
#include "encoding.h"

namespace {

bool antialias = true;

} // namespace

int AGS::draw_text(int dest, int x, int y, std::u16string_view codes, int font_size, uint8 color)
{
	uint8 antialias_cache[256*7];
	if (antialias)
		memset(antialias_cache, 0, 256);

	TTF_Font* font = NULL;
	switch (font_size) {
	case 16: font = hFont16; break;
	case 24: font = hFont24; break;
	case 32: font = hFont32; break;
	case 48: font = hFont48; break;
	case 64: font = hFont64; break;
	}
	int ascent = TTF_FontAscent(font);
	int descent = TTF_FontDescent(font);
	// Adjust dest_y if the font height is larger than the specified size.
	int dest_y = y - (ascent - descent - font_size) / 2;

	int dest_x = x;
	for (char16_t code : codes) {
		// 文字出力
		if (GAIJI_FIRST <= code && code <= GAIJI_LAST) {
			// Use unadjusted y here.
			draw_gaiji(dest, dest_x, y, gaiji[code - GAIJI_FIRST], font_size, color);
			dest_x += font_size;
		} else {
			if (antialias)
				draw_char_antialias(dest, dest_x, dest_y, code, font, color, antialias_cache);
			else
				draw_char(dest, dest_x, dest_y, code, font, color);

			int miny, maxy, advance;
			TTF_GlyphMetrics(font, code, NULL, NULL, &miny, &maxy, &advance);
			// Some fonts report incorrect Ascent/Descent value so we need to fix them.
			if (miny < descent) descent = miny;
			if (maxy > ascent) ascent = maxy;
			dest_x += advance;
		}
	}

	// 画面更新
	if (dest == 0)
		invalidate_screen(x, dest_y, dest_x - x, ascent - descent);
	return dest_x;
}

void AGS::draw_char(int dest, int dest_x, int dest_y, uint16 code, TTF_Font* font, uint8 color)
{
	// パターン取得
	SDL_Color white = {0xff, 0xff, 0xff};
	SDL_Surface* fs = TTF_RenderGlyph_Solid(font, code, white);

	// パターン出力
	for (int y = std::max(0, -dest_y); y < fs->h && dest_y + y < 480; y++) {
		uint8_t* pattern = (uint8_t*)fs->pixels + fs->pitch * y;
		for(int x = 0; x < fs->w && dest_x + x < 640; x++) {
			if(pattern[x] != 0) {
				vram[dest][dest_y + y][dest_x + x] = color;
			}
		}
	}

	SDL_FreeSurface(fs);
}

int AGS::nearest_color(int r, int g, int b) {
	int i, col, mind = INT_MAX;
	for (i = 0; i < 256; i++) {
		int dr = r - palR(i);
		int dg = g - palG(i);
		int db = b - palB(i);
		int d = dr*dr*30 + dg*dg*59 + db*db*11;
		if (d < mind) {
			mind = d;
			col = i;
		}
	}
	return col;
}

void AGS::draw_char_antialias(int dest, int dest_x, int dest_y, uint16 code, TTF_Font* font, uint8 color, uint8 cache[])
{
	// パターン取得
	SDL_Color black = {0, 0, 0};
	SDL_Color white = {0xff, 0xff, 0xff};
	SDL_Surface* fs = TTF_RenderGlyph_Shaded(font, code, white, black);

	// パターン出力
	for (int y = std::max(0, -dest_y); y < fs->h && dest_y + y < 480; y++) {
		uint8_t* pattern = (uint8_t*)fs->pixels + fs->pitch * y;
		uint8_t* dp = &vram[dest][dest_y + y][dest_x];
		for(int x = 0; x < fs->w && dest_x + x < 640; x++, dp++) {
			uint8 bg = *dp;
			int alpha = pattern[x] >> 5;
			if (alpha == 0) {
				// Transparent, do nothing
			} else if (alpha == 7) {
				*dp = color;
			} else if (cache[bg] & 1 << alpha) {
				*dp = cache[alpha << 8 | bg];
			} else {
				cache[bg] |= 1 << alpha;
				int c = nearest_color((palR(color) * alpha + palR(bg) * (7 - alpha)) / 7,
									  (palG(color) * alpha + palG(bg) * (7 - alpha)) / 7,
									  (palB(color) * alpha + palB(bg) * (7 - alpha)) / 7);
				cache[alpha << 8 | bg] = c;
				*dp = c;
			}
		}
	}

	SDL_FreeSurface(fs);
}

void AGS::draw_gaiji(int dest, int dest_x, int dest_y, const uint8_t bitmap[32], int size, uint8 color)
{
	bool pattern[16][16];

	// パターン取得
	for(int y = 0; y < 16; y++) {
		uint8 l = bitmap[y * 2 + 0];
		uint8 r = bitmap[y * 2 + 1];

		pattern[y][ 0] = ((l & 0x80) != 0);
		pattern[y][ 1] = ((l & 0x40) != 0);
		pattern[y][ 2] = ((l & 0x20) != 0);
		pattern[y][ 3] = ((l & 0x10) != 0);
		pattern[y][ 4] = ((l & 0x08) != 0);
		pattern[y][ 5] = ((l & 0x04) != 0);
		pattern[y][ 6] = ((l & 0x02) != 0);
		pattern[y][ 7] = ((l & 0x01) != 0);
		pattern[y][ 8] = ((r & 0x80) != 0);
		pattern[y][ 9] = ((r & 0x40) != 0);
		pattern[y][10] = ((r & 0x20) != 0);
		pattern[y][11] = ((r & 0x10) != 0);
		pattern[y][12] = ((r & 0x08) != 0);
		pattern[y][13] = ((r & 0x04) != 0);
		pattern[y][14] = ((r & 0x02) != 0);
		pattern[y][15] = ((r & 0x01) != 0);
	}

	// パターン出力
	for(int y = 0; y < size && dest_y + y < 480; y++) {
		for(int x = 0; x < size && dest_x + x < 640; x++) {
			if(pattern[(y * 16) / size][(x * 16) / size]) {
				vram[dest][dest_y + y][dest_x + x] = color;
			}
		}
	}
}

extern "C" {

void EMSCRIPTEN_KEEPALIVE ags_setAntialiasedStringMode(int on) {
	antialias = on != 0;
}

}
