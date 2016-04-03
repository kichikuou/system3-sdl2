/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - text ]
*/

#include "ags.h"
#include "s2utbl.h"

inline uint16 sjis_to_unicode(uint16 code)
{
	if (code >= 0xa0 && code <= 0xdf)
		return 0xff60 + code - 0xa0;
	if (code < 0x100)
		return code;
	return s2u[(code >> 8) - 0x80][(code & 0xff) - 0x40];
}

void AGS::draw_text(char string[])
{
	int p = 0;

	while(string[p] != '\0') {
		// •¶šƒR[ƒhæ“¾
		uint16 code = (uint8)string[p++];
		if((0x81 <= code && code <= 0x9f) || 0xe0 <= code) {
			code = (code << 8) | (uint8)string[p++];
		}
		if(draw_hankaku) {
			code = convert_hankaku(code);
		} else {
			code = convert_zenkaku(code);
		}

		if(draw_menu) {
			// •¶šo—Í
			if((0xeb9f <= code && code <= 0xebfc) || (0xec40 <= code && code <= 0xec9e)) {
				draw_gaiji(2, menu_dest_x, menu_dest_y, code, menu_font_size, menu_font_color);
			} else {
				draw_char(2, menu_dest_x, menu_dest_y, code, menu_font_size, menu_font_color);
			}
			// o—ÍˆÊ’u‚ÌXV
			menu_dest_x += (code & 0xff00) ? menu_font_size : (menu_font_size >> 1);
		} else {
			// •¶šo—Í
			if((0xeb9f <= code && code <= 0xebfc) || (0xec40 <= code && code <= 0xec9e)) {
				draw_gaiji(dest_screen, text_dest_x, text_dest_y, code, text_font_size, text_font_color);
			} else {
				draw_char(dest_screen, text_dest_x, text_dest_y, code, text_font_size, text_font_color);
			}
			// ‰æ–ÊXV
			if(dest_screen == 0) {
				draw_screen(text_dest_x, text_dest_y, (code & 0xff00) ? text_font_size : (text_font_size >> 1), text_font_size);
			}
			// o—ÍˆÊ’u‚ÌXV
			text_dest_x += (code & 0xff00) ? text_font_size : (text_font_size >> 1);
			if(text_font_maxsize < text_font_size) {
				text_font_maxsize = text_font_size;
			}
		}
	}
}

void AGS::draw_char(int dest, int dest_x, int dest_y, uint16 code, int size, uint8 color)
{
	// ƒpƒ^[ƒ“æ“¾
	TTF_Font* font = NULL;
	switch (size) {
	case 16: font = hFont16; break;
	case 24: font = hFont24; break;
	case 32: font = hFont32; break;
	case 48: font = hFont48; break;
	case 64: font = hFont64; break;
	}

	SDL_Color white = {0xff, 0xff, 0xff};
	SDL_Surface* fs = TTF_RenderGlyph_Solid(font, sjis_to_unicode(code), white);

	// ƒpƒ^[ƒ“o—Í
	// TODO: Blit?
	for(int y = 0; y < size && y < fs->w && dest_y + y < 480; y++) {
		uint8 *pattern = (uint8*)surface_line(fs, y);	// FIXME: do not assume 8bpp
		for(int x = 0; x < size && x < fs->h && dest_x + x < 640; x++) {
			if(pattern[x] != 0) {
				vram[dest][dest_y + y][dest_x + x] = color;
			}
		}
	}

	SDL_FreeSurface(fs);
}

void AGS::draw_gaiji(int dest, int dest_x, int dest_y, uint16 code, int size, uint8 color)
{
	int index = (0xeb9f <= code && code <= 0xebfc) ? code - 0xeb9f : (0xec40 <= code && code <= 0xec9e) ? code - 0xec40 + 94 : 0;
	bool pattern[16][16];

	// ƒpƒ^[ƒ“æ“¾
	for(int y = 0; y < 16; y++) {
		uint8 l = gaiji[index][y * 2 + 0];
		uint8 r = gaiji[index][y * 2 + 1];

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

	// ƒpƒ^[ƒ“o—Í
	for(int y = 0; y < size && dest_y + y < 480; y++) {
		for(int x = 0; x < size && dest_x + x < 640; x++) {
			if(pattern[(y * 16) / size][(x * 16) / size]) {
				vram[dest][dest_y + y][dest_x + x] = color;
			}
		}
	}
}

uint16 AGS::convert_zenkaku(uint16 code)
{
	switch(code) {
		case 0x20: return 0x8140; // '@'
		case 0x21: return 0x8149; // 'I'
		case 0x22: return 0x8168; // 'h'
		case 0x23: return 0x8194; // '”'
		case 0x24: return 0x8190; // ''
		case 0x25: return 0x8193; // '“'
		case 0x26: return 0x8195; // '•'
		case 0x27: return 0x8166; // 'f'
		case 0x28: return 0x8169; // 'i'
		case 0x29: return 0x816a; // 'j'
		case 0x2a: return 0x8196; // '–'
		case 0x2b: return 0x817b; // '{'
		case 0x2c: return 0x8143; // 'C'
		case 0x2d: return 0x817c; // '|'
		case 0x2e: return 0x8144; // 'D'
		case 0x2f: return 0x815e; // '^'
		case 0x30: return 0x824f; // '‚O'
		case 0x31: return 0x8250; // '‚P'
		case 0x32: return 0x8251; // '‚Q'
		case 0x33: return 0x8252; // '‚R'
		case 0x34: return 0x8253; // '‚S'
		case 0x35: return 0x8254; // '‚T'
		case 0x36: return 0x8255; // '‚U'
		case 0x37: return 0x8256; // '‚V'
		case 0x38: return 0x8257; // '‚W'
		case 0x39: return 0x8258; // '‚X'
		case 0x3a: return 0x8146; // 'F'
		case 0x3b: return 0x8147; // 'G'
		case 0x3c: return 0x8183; // 'ƒ'
		case 0x3d: return 0x8181; // ''
		case 0x3e: return 0x8184; // '„'
		case 0x3f: return 0x8148; // 'H'
		case 0x40: return 0x8197; // '—'
		case 0x41: return 0x8260; // '‚`'
		case 0x42: return 0x8261; // '‚a'
		case 0x43: return 0x8262; // '‚b'
		case 0x44: return 0x8263; // '‚c'
		case 0x45: return 0x8264; // '‚d'
		case 0x46: return 0x8265; // '‚e'
		case 0x47: return 0x8266; // '‚f'
		case 0x48: return 0x8267; // '‚g'
		case 0x49: return 0x8268; // '‚h'
		case 0x4a: return 0x8269; // '‚i'
		case 0x4b: return 0x826a; // '‚j'
		case 0x4c: return 0x826b; // '‚k'
		case 0x4d: return 0x826c; // '‚l'
		case 0x4e: return 0x826d; // '‚m'
		case 0x4f: return 0x826e; // '‚n'
		case 0x50: return 0x826f; // '‚o'
		case 0x51: return 0x8270; // '‚p'
		case 0x52: return 0x8271; // '‚q'
		case 0x53: return 0x8272; // '‚r'
		case 0x54: return 0x8273; // '‚s'
		case 0x55: return 0x8274; // '‚t'
		case 0x56: return 0x8275; // '‚u'
		case 0x57: return 0x8276; // '‚v'
		case 0x58: return 0x8277; // '‚w'
		case 0x59: return 0x8278; // '‚x'
		case 0x5a: return 0x8279; // '‚y'
		case 0x5b: return 0x816d; // 'm'
		case 0x5c: return 0x818f; // ''
		case 0x5d: return 0x816e; // 'n'
		case 0x5e: return 0x814f; // 'O'
		case 0x5f: return 0x8151; // 'Q'
		case 0x60: return 0x8165; // 'e'
		case 0x61: return 0x8281; // '‚'
		case 0x62: return 0x8282; // '‚‚'
		case 0x63: return 0x8283; // '‚ƒ'
		case 0x64: return 0x8284; // '‚„'
		case 0x65: return 0x8285; // '‚…'
		case 0x66: return 0x8286; // '‚†'
		case 0x67: return 0x8287; // '‚‡'
		case 0x68: return 0x8288; // '‚ˆ'
		case 0x69: return 0x8289; // '‚‰'
		case 0x6a: return 0x828a; // '‚Š'
		case 0x6b: return 0x828b; // '‚‹'
		case 0x6c: return 0x828c; // '‚Œ'
		case 0x6d: return 0x828d; // '‚'
		case 0x6e: return 0x828e; // '‚'
		case 0x6f: return 0x828f; // '‚'
		case 0x70: return 0x8290; // '‚'
		case 0x71: return 0x8291; // '‚‘'
		case 0x72: return 0x8292; // '‚’'
		case 0x73: return 0x8293; // '‚“'
		case 0x74: return 0x8294; // '‚”'
		case 0x75: return 0x8295; // '‚•'
		case 0x76: return 0x8296; // '‚–'
		case 0x77: return 0x8297; // '‚—'
		case 0x78: return 0x8298; // '‚˜'
		case 0x79: return 0x8299; // '‚™'
		case 0x7a: return 0x829a; // '‚š'
		case 0x7b: return 0x816f; // 'o'
		case 0x7c: return 0x8162; // 'b'
		case 0x7d: return 0x8170; // 'p'
		case 0x7e: return 0x8160; // '`'
		case 0xa1: return 0x8142; // 'B'
		case 0xa2: return 0x8175; // 'u'
		case 0xa3: return 0x8176; // 'v'
		case 0xa4: return 0x8141; // 'A'
		case 0xa5: return 0x8145; // 'E'
		case 0xa6: return 0x82f0; // '‚ğ'
		case 0xa7: return 0x829f; // '‚Ÿ'
		case 0xa8: return 0x82a1; // '‚¡'
		case 0xa9: return 0x82a3; // '‚£'
		case 0xaa: return 0x82a5; // '‚¥'
		case 0xab: return 0x82a7; // '‚§'
		case 0xac: return 0x82e1; // '‚á'
		case 0xad: return 0x82e3; // '‚ã'
		case 0xae: return 0x82e5; // '‚å'
		case 0xaf: return 0x82c1; // '‚Á'
		case 0xb0: return 0x815b; // '['
		case 0xb1: return 0x82a0; // '‚ '
		case 0xb2: return 0x82a2; // '‚¢'
		case 0xb3: return 0x82a4; // '‚¤'
		case 0xb4: return 0x82a6; // '‚¦'
		case 0xb5: return 0x82a8; // '‚¨'
		case 0xb6: return 0x82a9; // '‚©'
		case 0xb7: return 0x82ab; // '‚«'
		case 0xb8: return 0x82ad; // '‚­'
		case 0xb9: return 0x82af; // '‚¯'
		case 0xba: return 0x82b1; // '‚±'
		case 0xbb: return 0x82b3; // '‚³'
		case 0xbc: return 0x82b5; // '‚µ'
		case 0xbd: return 0x82b7; // '‚·'
		case 0xbe: return 0x82b9; // '‚¹'
		case 0xbf: return 0x82bb; // '‚»'
		case 0xc0: return 0x82bd; // '‚½'
		case 0xc1: return 0x82bf; // '‚¿'
		case 0xc2: return 0x82c2; // '‚Â'
		case 0xc3: return 0x82c4; // '‚Ä'
		case 0xc4: return 0x82c6; // '‚Æ'
		case 0xc5: return 0x82c8; // '‚È'
		case 0xc6: return 0x82c9; // '‚É'
		case 0xc7: return 0x82ca; // '‚Ê'
		case 0xc8: return 0x82cb; // '‚Ë'
		case 0xc9: return 0x82cc; // '‚Ì'
		case 0xca: return 0x82cd; // '‚Í'
		case 0xcb: return 0x82d0; // '‚Ğ'
		case 0xcc: return 0x82d3; // '‚Ó'
		case 0xcd: return 0x82d6; // '‚Ö'
		case 0xce: return 0x82d9; // '‚Ù'
		case 0xcf: return 0x82dc; // '‚Ü'
		case 0xd0: return 0x82dd; // '‚İ'
		case 0xd1: return 0x82de; // '‚Ş'
		case 0xd2: return 0x82df; // '‚ß'
		case 0xd3: return 0x82e0; // '‚à'
		case 0xd4: return 0x82e2; // '‚â'
		case 0xd5: return 0x82e4; // '‚ä'
		case 0xd6: return 0x82e6; // '‚æ'
		case 0xd7: return 0x82e7; // '‚ç'
		case 0xd8: return 0x82e8; // '‚è'
		case 0xd9: return 0x82e9; // '‚é'
		case 0xda: return 0x82ea; // '‚ê'
		case 0xdb: return 0x82eb; // '‚ë'
		case 0xdc: return 0x82ed; // '‚í'
		case 0xdd: return 0x82f1; // '‚ñ'
		case 0xde: return 0x814a; // 'J'
		case 0xdf: return 0x814b; // 'K'
	}
	return code;
}

uint16 AGS::convert_hankaku(uint16 code)
{
	switch(code) {
		case 0x8140: return 0x20; // '@'
		case 0x8149: return 0x21; // 'I'
		case 0x8168: return 0x22; // 'h'
		case 0x8194: return 0x23; // '”'
		case 0x8190: return 0x24; // ''
		case 0x8193: return 0x25; // '“'
		case 0x8195: return 0x26; // '•'
		case 0x8166: return 0x27; // 'f'
		case 0x8169: return 0x28; // 'i'
		case 0x816a: return 0x29; // 'j'
		case 0x8196: return 0x2a; // '–'
		case 0x817b: return 0x2b; // '{'
		case 0x8143: return 0x2c; // 'C'
		case 0x817c: return 0x2d; // '|'
		case 0x8144: return 0x2e; // 'D'
		case 0x815e: return 0x2f; // '^'
		case 0x824f: return 0x30; // '‚O'
		case 0x8250: return 0x31; // '‚P'
		case 0x8251: return 0x32; // '‚Q'
		case 0x8252: return 0x33; // '‚R'
		case 0x8253: return 0x34; // '‚S'
		case 0x8254: return 0x35; // '‚T'
		case 0x8255: return 0x36; // '‚U'
		case 0x8256: return 0x37; // '‚V'
		case 0x8257: return 0x38; // '‚W'
		case 0x8258: return 0x39; // '‚X'
		case 0x8146: return 0x3a; // 'F'
		case 0x8147: return 0x3b; // 'G'
		case 0x8183: return 0x3c; // 'ƒ'
		case 0x8181: return 0x3d; // ''
		case 0x8184: return 0x3e; // '„'
		case 0x8148: return 0x3f; // 'H'
		case 0x8197: return 0x40; // '—'
		case 0x8260: return 0x41; // '‚`'
		case 0x8261: return 0x42; // '‚a'
		case 0x8262: return 0x43; // '‚b'
		case 0x8263: return 0x44; // '‚c'
		case 0x8264: return 0x45; // '‚d'
		case 0x8265: return 0x46; // '‚e'
		case 0x8266: return 0x47; // '‚f'
		case 0x8267: return 0x48; // '‚g'
		case 0x8268: return 0x49; // '‚h'
		case 0x8269: return 0x4a; // '‚i'
		case 0x826a: return 0x4b; // '‚j'
		case 0x826b: return 0x4c; // '‚k'
		case 0x826c: return 0x4d; // '‚l'
		case 0x826d: return 0x4e; // '‚m'
		case 0x826e: return 0x4f; // '‚n'
		case 0x826f: return 0x50; // '‚o'
		case 0x8270: return 0x51; // '‚p'
		case 0x8271: return 0x52; // '‚q'
		case 0x8272: return 0x53; // '‚r'
		case 0x8273: return 0x54; // '‚s'
		case 0x8274: return 0x55; // '‚t'
		case 0x8275: return 0x56; // '‚u'
		case 0x8276: return 0x57; // '‚v'
		case 0x8277: return 0x58; // '‚w'
		case 0x8278: return 0x59; // '‚x'
		case 0x8279: return 0x5a; // '‚y'
		case 0x816d: return 0x5b; // 'm'
		case 0x818f: return 0x5c; // ''
		case 0x816e: return 0x5d; // 'n'
		case 0x814f: return 0x5e; // 'O'
		case 0x8151: return 0x5f; // 'Q'
		case 0x8165: return 0x60; // 'e'
		case 0x8281: return 0x61; // '‚'
		case 0x8282: return 0x62; // '‚‚'
		case 0x8283: return 0x63; // '‚ƒ'
		case 0x8284: return 0x64; // '‚„'
		case 0x8285: return 0x65; // '‚…'
		case 0x8286: return 0x66; // '‚†'
		case 0x8287: return 0x67; // '‚‡'
		case 0x8288: return 0x68; // '‚ˆ'
		case 0x8289: return 0x69; // '‚‰'
		case 0x828a: return 0x6a; // '‚Š'
		case 0x828b: return 0x6b; // '‚‹'
		case 0x828c: return 0x6c; // '‚Œ'
		case 0x828d: return 0x6d; // '‚'
		case 0x828e: return 0x6e; // '‚'
		case 0x828f: return 0x6f; // '‚'
		case 0x8290: return 0x70; // '‚'
		case 0x8291: return 0x71; // '‚‘'
		case 0x8292: return 0x72; // '‚’'
		case 0x8293: return 0x73; // '‚“'
		case 0x8294: return 0x74; // '‚”'
		case 0x8295: return 0x75; // '‚•'
		case 0x8296: return 0x76; // '‚–'
		case 0x8297: return 0x77; // '‚—'
		case 0x8298: return 0x78; // '‚˜'
		case 0x8299: return 0x79; // '‚™'
		case 0x829a: return 0x7a; // '‚š'
		case 0x816f: return 0x7b; // 'o'
		case 0x8162: return 0x7c; // 'b'
		case 0x8170: return 0x7d; // 'p'
		case 0x8160: return 0x7e; // '`'
		case 0x8142: return 0xa1; // 'B'
		case 0x8175: return 0xa2; // 'u'
		case 0x8176: return 0xa3; // 'v'
		case 0x8141: return 0xa4; // 'A'
		case 0x8145: return 0xa5; // 'E'
		case 0x82f0: return 0xa6; // '‚ğ'
		case 0x829f: return 0xa7; // '‚Ÿ'
		case 0x82a1: return 0xa8; // '‚¡'
		case 0x82a3: return 0xa9; // '‚£'
		case 0x82a5: return 0xaa; // '‚¥'
		case 0x82a7: return 0xab; // '‚§'
		case 0x82e1: return 0xac; // '‚á'
		case 0x82e3: return 0xad; // '‚ã'
		case 0x82e5: return 0xae; // '‚å'
		case 0x82c1: return 0xaf; // '‚Á'
		case 0x815b: return 0xb0; // '['
		case 0x82a0: return 0xb1; // '‚ '
		case 0x82a2: return 0xb2; // '‚¢'
		case 0x82a4: return 0xb3; // '‚¤'
		case 0x82a6: return 0xb4; // '‚¦'
		case 0x82a8: return 0xb5; // '‚¨'
		case 0x82a9: return 0xb6; // '‚©'
		case 0x82ab: return 0xb7; // '‚«'
		case 0x82ad: return 0xb8; // '‚­'
		case 0x82af: return 0xb9; // '‚¯'
		case 0x82b1: return 0xba; // '‚±'
		case 0x82b3: return 0xbb; // '‚³'
		case 0x82b5: return 0xbc; // '‚µ'
		case 0x82b7: return 0xbd; // '‚·'
		case 0x82b9: return 0xbe; // '‚¹'
		case 0x82bb: return 0xbf; // '‚»'
		case 0x82bd: return 0xc0; // '‚½'
		case 0x82bf: return 0xc1; // '‚¿'
		case 0x82c2: return 0xc2; // '‚Â'
		case 0x82c4: return 0xc3; // '‚Ä'
		case 0x82c6: return 0xc4; // '‚Æ'
		case 0x82c8: return 0xc5; // '‚È'
		case 0x82c9: return 0xc6; // '‚É'
		case 0x82ca: return 0xc7; // '‚Ê'
		case 0x82cb: return 0xc8; // '‚Ë'
		case 0x82cc: return 0xc9; // '‚Ì'
		case 0x82cd: return 0xca; // '‚Í'
		case 0x82d0: return 0xcb; // '‚Ğ'
		case 0x82d3: return 0xcc; // '‚Ó'
		case 0x82d6: return 0xcd; // '‚Ö'
		case 0x82d9: return 0xce; // '‚Ù'
		case 0x82dc: return 0xcf; // '‚Ü'
		case 0x82dd: return 0xd0; // '‚İ'
		case 0x82de: return 0xd1; // '‚Ş'
		case 0x82df: return 0xd2; // '‚ß'
		case 0x82e0: return 0xd3; // '‚à'
		case 0x82e2: return 0xd4; // '‚â'
		case 0x82e4: return 0xd5; // '‚ä'
		case 0x82e6: return 0xd6; // '‚æ'
		case 0x82e7: return 0xd7; // '‚ç'
		case 0x82e8: return 0xd8; // '‚è'
		case 0x82e9: return 0xd9; // '‚é'
		case 0x82ea: return 0xda; // '‚ê'
		case 0x82eb: return 0xdb; // '‚ë'
		case 0x82ed: return 0xdc; // '‚í'
		case 0x82f1: return 0xdd; // '‚ñ'
		case 0x814a: return 0xde; // 'J'
		case 0x814b: return 0xdf; // 'K'
	}
	return code;
}

