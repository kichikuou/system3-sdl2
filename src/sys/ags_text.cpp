/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - text ]
*/

#include <algorithm>
#include <limits.h>
#include <string.h>
#include "ags.h"
#include "nact.h"
#include "encoding.h"
#include "texthook.h"

namespace {

bool antialias = true;

int convert_to_zenkaku(int code)
{
	switch(code) {
	case u' ': return u'　';
	case u'!': return u'！';
	case u'"': return u'”';
	case u'#': return u'＃';
	case u'$': return u'＄';
	case u'%': return u'％';
	case u'&': return u'＆';
	case u'\'': return u'’';
	case u'(': return u'（';
	case u')': return u'）';
	case u'*': return u'＊';
	case u'+': return u'＋';
	case u',': return u'，';
	case u'-': return u'－';
	case u'.': return u'．';
	case u'/': return u'／';
	case u'0': return u'０';
	case u'1': return u'１';
	case u'2': return u'２';
	case u'3': return u'３';
	case u'4': return u'４';
	case u'5': return u'５';
	case u'6': return u'６';
	case u'7': return u'７';
	case u'8': return u'８';
	case u'9': return u'９';
	case u':': return u'：';
	case u';': return u'；';
	case u'<': return u'＜';
	case u'=': return u'＝';
	case u'>': return u'＞';
	case u'?': return u'？';
	case u'@': return u'＠';
	case u'A': return u'Ａ';
	case u'B': return u'Ｂ';
	case u'C': return u'Ｃ';
	case u'D': return u'Ｄ';
	case u'E': return u'Ｅ';
	case u'F': return u'Ｆ';
	case u'G': return u'Ｇ';
	case u'H': return u'Ｈ';
	case u'I': return u'Ｉ';
	case u'J': return u'Ｊ';
	case u'K': return u'Ｋ';
	case u'L': return u'Ｌ';
	case u'M': return u'Ｍ';
	case u'N': return u'Ｎ';
	case u'O': return u'Ｏ';
	case u'P': return u'Ｐ';
	case u'Q': return u'Ｑ';
	case u'R': return u'Ｒ';
	case u'S': return u'Ｓ';
	case u'T': return u'Ｔ';
	case u'U': return u'Ｕ';
	case u'V': return u'Ｖ';
	case u'W': return u'Ｗ';
	case u'X': return u'Ｘ';
	case u'Y': return u'Ｙ';
	case u'Z': return u'Ｚ';
	case u'[': return u'［';
	case u'\\': return u'￥';
	case u']': return u'］';
	case u'^': return u'＾';
	case u'_': return u'＿';
	case u'`': return u'‘';
	case u'a': return u'ａ';
	case u'b': return u'ｂ';
	case u'c': return u'ｃ';
	case u'd': return u'ｄ';
	case u'e': return u'ｅ';
	case u'f': return u'ｆ';
	case u'g': return u'ｇ';
	case u'h': return u'ｈ';
	case u'i': return u'ｉ';
	case u'j': return u'ｊ';
	case u'k': return u'ｋ';
	case u'l': return u'ｌ';
	case u'm': return u'ｍ';
	case u'n': return u'ｎ';
	case u'o': return u'ｏ';
	case u'p': return u'ｐ';
	case u'q': return u'ｑ';
	case u'r': return u'ｒ';
	case u's': return u'ｓ';
	case u't': return u'ｔ';
	case u'u': return u'ｕ';
	case u'v': return u'ｖ';
	case u'w': return u'ｗ';
	case u'x': return u'ｘ';
	case u'y': return u'ｙ';
	case u'z': return u'ｚ';
	case u'{': return u'｛';
	case u'|': return u'｜';
	case u'}': return u'｝';
	case u'~': return u'～';
	case u'｡': return u'。';
	case u'｢': return u'「';
	case u'｣': return u'」';
	case u'､': return u'、';
	case u'･': return u'・';
	case u'ｦ': return u'を';
	case u'ｧ': return u'ぁ';
	case u'ｨ': return u'ぃ';
	case u'ｩ': return u'ぅ';
	case u'ｪ': return u'ぇ';
	case u'ｫ': return u'ぉ';
	case u'ｬ': return u'ゃ';
	case u'ｭ': return u'ゅ';
	case u'ｮ': return u'ょ';
	case u'ｯ': return u'っ';
	case u'ｰ': return u'ー';
	case u'ｱ': return u'あ';
	case u'ｲ': return u'い';
	case u'ｳ': return u'う';
	case u'ｴ': return u'え';
	case u'ｵ': return u'お';
	case u'ｶ': return u'か';
	case u'ｷ': return u'き';
	case u'ｸ': return u'く';
	case u'ｹ': return u'け';
	case u'ｺ': return u'こ';
	case u'ｻ': return u'さ';
	case u'ｼ': return u'し';
	case u'ｽ': return u'す';
	case u'ｾ': return u'せ';
	case u'ｿ': return u'そ';
	case u'ﾀ': return u'た';
	case u'ﾁ': return u'ち';
	case u'ﾂ': return u'つ';
	case u'ﾃ': return u'て';
	case u'ﾄ': return u'と';
	case u'ﾅ': return u'な';
	case u'ﾆ': return u'に';
	case u'ﾇ': return u'ぬ';
	case u'ﾈ': return u'ね';
	case u'ﾉ': return u'の';
	case u'ﾊ': return u'は';
	case u'ﾋ': return u'ひ';
	case u'ﾌ': return u'ふ';
	case u'ﾍ': return u'へ';
	case u'ﾎ': return u'ほ';
	case u'ﾏ': return u'ま';
	case u'ﾐ': return u'み';
	case u'ﾑ': return u'む';
	case u'ﾒ': return u'め';
	case u'ﾓ': return u'も';
	case u'ﾔ': return u'や';
	case u'ﾕ': return u'ゆ';
	case u'ﾖ': return u'よ';
	case u'ﾗ': return u'ら';
	case u'ﾘ': return u'り';
	case u'ﾙ': return u'る';
	case u'ﾚ': return u'れ';
	case u'ﾛ': return u'ろ';
	case u'ﾜ': return u'わ';
	case u'ﾝ': return u'ん';
	case u'ﾞ': return u'゛';
	case u'ﾟ': return u'゜';
	}
	return code;
}

int convert_to_hankaku(int code)
{
	switch(code) {
	case u'　': return u' ';
	case u'！': return u'!';
	case u'”': return u'"';
	case u'＃': return u'#';
	case u'＄': return u'$';
	case u'％': return u'%';
	case u'＆': return u'&';
	case u'’': return u'\'';
	case u'（': return u'(';
	case u'）': return u')';
	case u'＊': return u'*';
	case u'＋': return u'+';
	case u'，': return u',';
	case u'－': return u'-';
	case u'．': return u'.';
	case u'／': return u'/';
	case u'０': return u'0';
	case u'１': return u'1';
	case u'２': return u'2';
	case u'３': return u'3';
	case u'４': return u'4';
	case u'５': return u'5';
	case u'６': return u'6';
	case u'７': return u'7';
	case u'８': return u'8';
	case u'９': return u'9';
	case u'：': return u':';
	case u'；': return u';';
	case u'＜': return u'<';
	case u'＝': return u'=';
	case u'＞': return u'>';
	case u'？': return u'?';
	case u'＠': return u'@';
	case u'Ａ': return u'A';
	case u'Ｂ': return u'B';
	case u'Ｃ': return u'C';
	case u'Ｄ': return u'D';
	case u'Ｅ': return u'E';
	case u'Ｆ': return u'F';
	case u'Ｇ': return u'G';
	case u'Ｈ': return u'H';
	case u'Ｉ': return u'I';
	case u'Ｊ': return u'J';
	case u'Ｋ': return u'K';
	case u'Ｌ': return u'L';
	case u'Ｍ': return u'M';
	case u'Ｎ': return u'N';
	case u'Ｏ': return u'O';
	case u'Ｐ': return u'P';
	case u'Ｑ': return u'Q';
	case u'Ｒ': return u'R';
	case u'Ｓ': return u'S';
	case u'Ｔ': return u'T';
	case u'Ｕ': return u'U';
	case u'Ｖ': return u'V';
	case u'Ｗ': return u'W';
	case u'Ｘ': return u'X';
	case u'Ｙ': return u'Y';
	case u'Ｚ': return u'Z';
	case u'［': return u'[';
	case u'￥': return u'\\';
	case u'］': return u']';
	case u'＾': return u'^';
	case u'＿': return u'_';
	case u'‘': return u'`';
	case u'ａ': return u'a';
	case u'ｂ': return u'b';
	case u'ｃ': return u'c';
	case u'ｄ': return u'd';
	case u'ｅ': return u'e';
	case u'ｆ': return u'f';
	case u'ｇ': return u'g';
	case u'ｈ': return u'h';
	case u'ｉ': return u'i';
	case u'ｊ': return u'j';
	case u'ｋ': return u'k';
	case u'ｌ': return u'l';
	case u'ｍ': return u'm';
	case u'ｎ': return u'n';
	case u'ｏ': return u'o';
	case u'ｐ': return u'p';
	case u'ｑ': return u'q';
	case u'ｒ': return u'r';
	case u'ｓ': return u's';
	case u'ｔ': return u't';
	case u'ｕ': return u'u';
	case u'ｖ': return u'v';
	case u'ｗ': return u'w';
	case u'ｘ': return u'x';
	case u'ｙ': return u'y';
	case u'ｚ': return u'z';
	case u'｛': return u'{';
	case u'｜': return u'|';
	case u'｝': return u'}';
	case u'～': return u'~';
	case u'。': return u'｡';
	case u'「': return u'｢';
	case u'」': return u'｣';
	case u'、': return u'､';
	case u'・': return u'･';
	case u'を': return u'ｦ';
	case u'ぁ': return u'ｧ';
	case u'ぃ': return u'ｨ';
	case u'ぅ': return u'ｩ';
	case u'ぇ': return u'ｪ';
	case u'ぉ': return u'ｫ';
	case u'ゃ': return u'ｬ';
	case u'ゅ': return u'ｭ';
	case u'ょ': return u'ｮ';
	case u'っ': return u'ｯ';
	case u'ー': return u'ｰ';
	case u'あ': return u'ｱ';
	case u'い': return u'ｲ';
	case u'う': return u'ｳ';
	case u'え': return u'ｴ';
	case u'お': return u'ｵ';
	case u'か': return u'ｶ';
	case u'き': return u'ｷ';
	case u'く': return u'ｸ';
	case u'け': return u'ｹ';
	case u'こ': return u'ｺ';
	case u'さ': return u'ｻ';
	case u'し': return u'ｼ';
	case u'す': return u'ｽ';
	case u'せ': return u'ｾ';
	case u'そ': return u'ｿ';
	case u'た': return u'ﾀ';
	case u'ち': return u'ﾁ';
	case u'つ': return u'ﾂ';
	case u'て': return u'ﾃ';
	case u'と': return u'ﾄ';
	case u'な': return u'ﾅ';
	case u'に': return u'ﾆ';
	case u'ぬ': return u'ﾇ';
	case u'ね': return u'ﾈ';
	case u'の': return u'ﾉ';
	case u'は': return u'ﾊ';
	case u'ひ': return u'ﾋ';
	case u'ふ': return u'ﾌ';
	case u'へ': return u'ﾍ';
	case u'ほ': return u'ﾎ';
	case u'ま': return u'ﾏ';
	case u'み': return u'ﾐ';
	case u'む': return u'ﾑ';
	case u'め': return u'ﾒ';
	case u'も': return u'ﾓ';
	case u'や': return u'ﾔ';
	case u'ゆ': return u'ﾕ';
	case u'よ': return u'ﾖ';
	case u'ら': return u'ﾗ';
	case u'り': return u'ﾘ';
	case u'る': return u'ﾙ';
	case u'れ': return u'ﾚ';
	case u'ろ': return u'ﾛ';
	case u'わ': return u'ﾜ';
	case u'ん': return u'ﾝ';
	case u'゛': return u'ﾞ';
	case u'゜': return u'ﾟ';
	}
	return code;
}

} // namespace

void AGS::draw_text(const char* string, bool text_wait)
{
	if (!*string)
		return;
	uint8 antialias_cache[256*7];
	if (antialias)
		memset(antialias_cache, 0, 256);

	TextContext& ctx = draw_menu ? menu : text;
	int screen = draw_menu ? 2 : dest_screen;
	int dest_x = ctx.pos.x;
	int dest_y = ctx.pos.y;
	if (ctx.current_line_height < ctx.font_size)
		ctx.current_line_height = ctx.font_size;

	TTF_Font* font = NULL;
	switch (ctx.font_size) {
	case 16: font = hFont16; break;
	case 24: font = hFont24; break;
	case 32: font = hFont32; break;
	case 48: font = hFont48; break;
	case 64: font = hFont64; break;
	}
	int ascent = TTF_FontAscent(font);
	int descent = TTF_FontDescent(font);
	// Adjust dest_y if the font height is larger than the specified size.
	dest_y -= (ascent - descent - ctx.font_size) / 2;

	while (*string) {
		int code = g_nact->encoding->next_codepoint(&string);
		if(draw_hankaku) {
			code = convert_to_hankaku(code);
		} else {
			code = convert_to_zenkaku(code);
		}

		// 文字出力
		if (GAIJI_FIRST <= code && code <= GAIJI_LAST) {
			// Use unadjusted dest_y here.
			draw_gaiji(screen, dest_x, ctx.pos.y, gaiji[code - GAIJI_FIRST], ctx.font_size, ctx.font_color);
			dest_x += ctx.font_size;
		} else {
			if (!draw_menu)
				texthook_character(g_nact->get_scenario_page(), code);

			if (antialias)
				draw_char_antialias(screen, dest_x, dest_y, code, font, ctx.font_color, antialias_cache);
			else
				draw_char(screen, dest_x, dest_y, code, font, ctx.font_color);

			int miny, maxy, advance;
			TTF_GlyphMetrics(font, code, NULL, NULL, &miny, &maxy, &advance);
			// Some fonts report incorrect Ascent/Descent value so we need to fix them.
			if (miny < descent) descent = miny;
			if (maxy > ascent) ascent = maxy;
			dest_x += advance;
		}

		if (!draw_menu && text_wait && code != ' ') {
			// 画面更新
			if(screen == 0)
				draw_screen(ctx.pos.x, dest_y, dest_x - ctx.pos.x, ascent - descent);
			ctx.pos.x = dest_x;
			// Wait
			g_nact->text_wait();
		}
	}
	if (!draw_menu && !text_wait) {
		// 画面更新
		if(screen == 0)
			draw_screen(ctx.pos.x, dest_y, dest_x - ctx.pos.x, ascent - descent);
	}
	ctx.pos.x = dest_x;
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
