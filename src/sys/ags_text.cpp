/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - text ]
*/

#include "ags.h"

void AGS::draw_text(char string[])
{
	int p = 0;

	while(string[p] != '\0') {
		// ï∂éöÉRÅ[ÉhéÊìæ
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
			// ï∂éöèoóÕ
			if((0xeb9f <= code && code <= 0xebfc) || (0xec40 <= code && code <= 0xec9e)) {
				draw_gaiji(2, menu_dest_x, menu_dest_y, code, menu_font_size, menu_font_color);
			} else {
				draw_char(2, menu_dest_x, menu_dest_y, code, menu_font_size, menu_font_color);
			}
			// èoóÕà íuÇÃçXêV
			menu_dest_x += (code & 0xff00) ? menu_font_size : (menu_font_size >> 1);
		} else {
			// ï∂éöèoóÕ
			if((0xeb9f <= code && code <= 0xebfc) || (0xec40 <= code && code <= 0xec9e)) {
				draw_gaiji(dest_screen, text_dest_x, text_dest_y, code, text_font_size, text_font_color);
			} else {
				draw_char(dest_screen, text_dest_x, text_dest_y, code, text_font_size, text_font_color);
			}
			// âÊñ çXêV
			if(dest_screen == 0) {
				draw_screen(text_dest_x, text_dest_y, (code & 0xff00) ? text_font_size : (text_font_size >> 1), text_font_size);
			}
			// èoóÕà íuÇÃçXêV
			text_dest_x += (code & 0xff00) ? text_font_size : (text_font_size >> 1);
			if(text_font_maxsize < text_font_size) {
				text_font_maxsize = text_font_size;
			}
		}
	}
}

void AGS::draw_char(int dest, int dest_x, int dest_y, uint16 code, int size, uint8 color)
{
	char string[3];
	int length;
	SDL_Color white = {0xff, 0xff, 0xff};

	// ÉpÉ^Å[ÉìéÊìæ
	if(code > 0xff) {
		string[0] = code >> 8;
		string[1] = code & 0xff;
		string[2] = '\0';
		length = 2;
	} else {
		string[0] = code & 0xff;
		string[1] = '\0';
		length = 1;
	}
	Uint16 buf[2];
	MultiByteToWideChar(CP_ACP, 0, string, length, (LPWSTR)buf, 2);

	TTF_Font* font = NULL;
	switch (size) {
	case 16: font = hFont16; break;
	case 24: font = hFont24; break;
	case 32: font = hFont32; break;
	case 48: font = hFont48; break;
	case 64: font = hFont64; break;
	}

	SDL_Surface* fs = TTF_RenderGlyph_Solid(font, buf[0], white);

	// ÉpÉ^Å[ÉìèoóÕ
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

	// ÉpÉ^Å[ÉìéÊìæ
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

	// ÉpÉ^Å[ÉìèoóÕ
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
		case 0x20: return 0x8140; // 'Å@'
		case 0x21: return 0x8149; // 'ÅI'
		case 0x22: return 0x8168; // 'Åh'
		case 0x23: return 0x8194; // 'Åî'
		case 0x24: return 0x8190; // 'Åê'
		case 0x25: return 0x8193; // 'Åì'
		case 0x26: return 0x8195; // 'Åï'
		case 0x27: return 0x8166; // 'Åf'
		case 0x28: return 0x8169; // 'Åi'
		case 0x29: return 0x816a; // 'Åj'
		case 0x2a: return 0x8196; // 'Åñ'
		case 0x2b: return 0x817b; // 'Å{'
		case 0x2c: return 0x8143; // 'ÅC'
		case 0x2d: return 0x817c; // 'Å|'
		case 0x2e: return 0x8144; // 'ÅD'
		case 0x2f: return 0x815e; // 'Å^'
		case 0x30: return 0x824f; // 'ÇO'
		case 0x31: return 0x8250; // 'ÇP'
		case 0x32: return 0x8251; // 'ÇQ'
		case 0x33: return 0x8252; // 'ÇR'
		case 0x34: return 0x8253; // 'ÇS'
		case 0x35: return 0x8254; // 'ÇT'
		case 0x36: return 0x8255; // 'ÇU'
		case 0x37: return 0x8256; // 'ÇV'
		case 0x38: return 0x8257; // 'ÇW'
		case 0x39: return 0x8258; // 'ÇX'
		case 0x3a: return 0x8146; // 'ÅF'
		case 0x3b: return 0x8147; // 'ÅG'
		case 0x3c: return 0x8183; // 'ÅÉ'
		case 0x3d: return 0x8181; // 'ÅÅ'
		case 0x3e: return 0x8184; // 'ÅÑ'
		case 0x3f: return 0x8148; // 'ÅH'
		case 0x40: return 0x8197; // 'Åó'
		case 0x41: return 0x8260; // 'Ç`'
		case 0x42: return 0x8261; // 'Ça'
		case 0x43: return 0x8262; // 'Çb'
		case 0x44: return 0x8263; // 'Çc'
		case 0x45: return 0x8264; // 'Çd'
		case 0x46: return 0x8265; // 'Çe'
		case 0x47: return 0x8266; // 'Çf'
		case 0x48: return 0x8267; // 'Çg'
		case 0x49: return 0x8268; // 'Çh'
		case 0x4a: return 0x8269; // 'Çi'
		case 0x4b: return 0x826a; // 'Çj'
		case 0x4c: return 0x826b; // 'Çk'
		case 0x4d: return 0x826c; // 'Çl'
		case 0x4e: return 0x826d; // 'Çm'
		case 0x4f: return 0x826e; // 'Çn'
		case 0x50: return 0x826f; // 'Ço'
		case 0x51: return 0x8270; // 'Çp'
		case 0x52: return 0x8271; // 'Çq'
		case 0x53: return 0x8272; // 'Çr'
		case 0x54: return 0x8273; // 'Çs'
		case 0x55: return 0x8274; // 'Çt'
		case 0x56: return 0x8275; // 'Çu'
		case 0x57: return 0x8276; // 'Çv'
		case 0x58: return 0x8277; // 'Çw'
		case 0x59: return 0x8278; // 'Çx'
		case 0x5a: return 0x8279; // 'Çy'
		case 0x5b: return 0x816d; // 'Åm'
		case 0x5c: return 0x818f; // 'Åè'
		case 0x5d: return 0x816e; // 'Ån'
		case 0x5e: return 0x814f; // 'ÅO'
		case 0x5f: return 0x8151; // 'ÅQ'
		case 0x60: return 0x8165; // 'Åe'
		case 0x61: return 0x8281; // 'ÇÅ'
		case 0x62: return 0x8282; // 'ÇÇ'
		case 0x63: return 0x8283; // 'ÇÉ'
		case 0x64: return 0x8284; // 'ÇÑ'
		case 0x65: return 0x8285; // 'ÇÖ'
		case 0x66: return 0x8286; // 'ÇÜ'
		case 0x67: return 0x8287; // 'Çá'
		case 0x68: return 0x8288; // 'Çà'
		case 0x69: return 0x8289; // 'Çâ'
		case 0x6a: return 0x828a; // 'Çä'
		case 0x6b: return 0x828b; // 'Çã'
		case 0x6c: return 0x828c; // 'Çå'
		case 0x6d: return 0x828d; // 'Çç'
		case 0x6e: return 0x828e; // 'Çé'
		case 0x6f: return 0x828f; // 'Çè'
		case 0x70: return 0x8290; // 'Çê'
		case 0x71: return 0x8291; // 'Çë'
		case 0x72: return 0x8292; // 'Çí'
		case 0x73: return 0x8293; // 'Çì'
		case 0x74: return 0x8294; // 'Çî'
		case 0x75: return 0x8295; // 'Çï'
		case 0x76: return 0x8296; // 'Çñ'
		case 0x77: return 0x8297; // 'Çó'
		case 0x78: return 0x8298; // 'Çò'
		case 0x79: return 0x8299; // 'Çô'
		case 0x7a: return 0x829a; // 'Çö'
		case 0x7b: return 0x816f; // 'Åo'
		case 0x7c: return 0x8162; // 'Åb'
		case 0x7d: return 0x8170; // 'Åp'
		case 0x7e: return 0x8160; // 'Å`'
		case 0xa1: return 0x8142; // 'ÅB'
		case 0xa2: return 0x8175; // 'Åu'
		case 0xa3: return 0x8176; // 'Åv'
		case 0xa4: return 0x8141; // 'ÅA'
		case 0xa5: return 0x8145; // 'ÅE'
		case 0xa6: return 0x82f0; // 'Ç'
		case 0xa7: return 0x829f; // 'Çü'
		case 0xa8: return 0x82a1; // 'Ç°'
		case 0xa9: return 0x82a3; // 'Ç£'
		case 0xaa: return 0x82a5; // 'Ç•'
		case 0xab: return 0x82a7; // 'Çß'
		case 0xac: return 0x82e1; // 'Ç·'
		case 0xad: return 0x82e3; // 'Ç„'
		case 0xae: return 0x82e5; // 'ÇÂ'
		case 0xaf: return 0x82c1; // 'Ç¡'
		case 0xb0: return 0x815b; // 'Å['
		case 0xb1: return 0x82a0; // 'Ç†'
		case 0xb2: return 0x82a2; // 'Ç¢'
		case 0xb3: return 0x82a4; // 'Ç§'
		case 0xb4: return 0x82a6; // 'Ç¶'
		case 0xb5: return 0x82a8; // 'Ç®'
		case 0xb6: return 0x82a9; // 'Ç©'
		case 0xb7: return 0x82ab; // 'Ç´'
		case 0xb8: return 0x82ad; // 'Ç≠'
		case 0xb9: return 0x82af; // 'ÇØ'
		case 0xba: return 0x82b1; // 'Ç±'
		case 0xbb: return 0x82b3; // 'Ç≥'
		case 0xbc: return 0x82b5; // 'Çµ'
		case 0xbd: return 0x82b7; // 'Ç∑'
		case 0xbe: return 0x82b9; // 'Çπ'
		case 0xbf: return 0x82bb; // 'Çª'
		case 0xc0: return 0x82bd; // 'ÇΩ'
		case 0xc1: return 0x82bf; // 'Çø'
		case 0xc2: return 0x82c2; // 'Ç¬'
		case 0xc3: return 0x82c4; // 'Çƒ'
		case 0xc4: return 0x82c6; // 'Ç∆'
		case 0xc5: return 0x82c8; // 'Ç»'
		case 0xc6: return 0x82c9; // 'Ç…'
		case 0xc7: return 0x82ca; // 'Ç '
		case 0xc8: return 0x82cb; // 'ÇÀ'
		case 0xc9: return 0x82cc; // 'ÇÃ'
		case 0xca: return 0x82cd; // 'ÇÕ'
		case 0xcb: return 0x82d0; // 'Ç–'
		case 0xcc: return 0x82d3; // 'Ç”'
		case 0xcd: return 0x82d6; // 'Ç÷'
		case 0xce: return 0x82d9; // 'ÇŸ'
		case 0xcf: return 0x82dc; // 'Ç‹'
		case 0xd0: return 0x82dd; // 'Ç›'
		case 0xd1: return 0x82de; // 'Çﬁ'
		case 0xd2: return 0x82df; // 'Çﬂ'
		case 0xd3: return 0x82e0; // 'Ç‡'
		case 0xd4: return 0x82e2; // 'Ç‚'
		case 0xd5: return 0x82e4; // 'Ç‰'
		case 0xd6: return 0x82e6; // 'ÇÊ'
		case 0xd7: return 0x82e7; // 'ÇÁ'
		case 0xd8: return 0x82e8; // 'ÇË'
		case 0xd9: return 0x82e9; // 'ÇÈ'
		case 0xda: return 0x82ea; // 'ÇÍ'
		case 0xdb: return 0x82eb; // 'ÇÎ'
		case 0xdc: return 0x82ed; // 'ÇÌ'
		case 0xdd: return 0x82f1; // 'ÇÒ'
		case 0xde: return 0x814a; // 'ÅJ'
		case 0xdf: return 0x814b; // 'ÅK'
	}
	return code;
}

uint16 AGS::convert_hankaku(uint16 code)
{
	switch(code) {
		case 0x8140: return 0x20; // 'Å@'
		case 0x8149: return 0x21; // 'ÅI'
		case 0x8168: return 0x22; // 'Åh'
		case 0x8194: return 0x23; // 'Åî'
		case 0x8190: return 0x24; // 'Åê'
		case 0x8193: return 0x25; // 'Åì'
		case 0x8195: return 0x26; // 'Åï'
		case 0x8166: return 0x27; // 'Åf'
		case 0x8169: return 0x28; // 'Åi'
		case 0x816a: return 0x29; // 'Åj'
		case 0x8196: return 0x2a; // 'Åñ'
		case 0x817b: return 0x2b; // 'Å{'
		case 0x8143: return 0x2c; // 'ÅC'
		case 0x817c: return 0x2d; // 'Å|'
		case 0x8144: return 0x2e; // 'ÅD'
		case 0x815e: return 0x2f; // 'Å^'
		case 0x824f: return 0x30; // 'ÇO'
		case 0x8250: return 0x31; // 'ÇP'
		case 0x8251: return 0x32; // 'ÇQ'
		case 0x8252: return 0x33; // 'ÇR'
		case 0x8253: return 0x34; // 'ÇS'
		case 0x8254: return 0x35; // 'ÇT'
		case 0x8255: return 0x36; // 'ÇU'
		case 0x8256: return 0x37; // 'ÇV'
		case 0x8257: return 0x38; // 'ÇW'
		case 0x8258: return 0x39; // 'ÇX'
		case 0x8146: return 0x3a; // 'ÅF'
		case 0x8147: return 0x3b; // 'ÅG'
		case 0x8183: return 0x3c; // 'ÅÉ'
		case 0x8181: return 0x3d; // 'ÅÅ'
		case 0x8184: return 0x3e; // 'ÅÑ'
		case 0x8148: return 0x3f; // 'ÅH'
		case 0x8197: return 0x40; // 'Åó'
		case 0x8260: return 0x41; // 'Ç`'
		case 0x8261: return 0x42; // 'Ça'
		case 0x8262: return 0x43; // 'Çb'
		case 0x8263: return 0x44; // 'Çc'
		case 0x8264: return 0x45; // 'Çd'
		case 0x8265: return 0x46; // 'Çe'
		case 0x8266: return 0x47; // 'Çf'
		case 0x8267: return 0x48; // 'Çg'
		case 0x8268: return 0x49; // 'Çh'
		case 0x8269: return 0x4a; // 'Çi'
		case 0x826a: return 0x4b; // 'Çj'
		case 0x826b: return 0x4c; // 'Çk'
		case 0x826c: return 0x4d; // 'Çl'
		case 0x826d: return 0x4e; // 'Çm'
		case 0x826e: return 0x4f; // 'Çn'
		case 0x826f: return 0x50; // 'Ço'
		case 0x8270: return 0x51; // 'Çp'
		case 0x8271: return 0x52; // 'Çq'
		case 0x8272: return 0x53; // 'Çr'
		case 0x8273: return 0x54; // 'Çs'
		case 0x8274: return 0x55; // 'Çt'
		case 0x8275: return 0x56; // 'Çu'
		case 0x8276: return 0x57; // 'Çv'
		case 0x8277: return 0x58; // 'Çw'
		case 0x8278: return 0x59; // 'Çx'
		case 0x8279: return 0x5a; // 'Çy'
		case 0x816d: return 0x5b; // 'Åm'
		case 0x818f: return 0x5c; // 'Åè'
		case 0x816e: return 0x5d; // 'Ån'
		case 0x814f: return 0x5e; // 'ÅO'
		case 0x8151: return 0x5f; // 'ÅQ'
		case 0x8165: return 0x60; // 'Åe'
		case 0x8281: return 0x61; // 'ÇÅ'
		case 0x8282: return 0x62; // 'ÇÇ'
		case 0x8283: return 0x63; // 'ÇÉ'
		case 0x8284: return 0x64; // 'ÇÑ'
		case 0x8285: return 0x65; // 'ÇÖ'
		case 0x8286: return 0x66; // 'ÇÜ'
		case 0x8287: return 0x67; // 'Çá'
		case 0x8288: return 0x68; // 'Çà'
		case 0x8289: return 0x69; // 'Çâ'
		case 0x828a: return 0x6a; // 'Çä'
		case 0x828b: return 0x6b; // 'Çã'
		case 0x828c: return 0x6c; // 'Çå'
		case 0x828d: return 0x6d; // 'Çç'
		case 0x828e: return 0x6e; // 'Çé'
		case 0x828f: return 0x6f; // 'Çè'
		case 0x8290: return 0x70; // 'Çê'
		case 0x8291: return 0x71; // 'Çë'
		case 0x8292: return 0x72; // 'Çí'
		case 0x8293: return 0x73; // 'Çì'
		case 0x8294: return 0x74; // 'Çî'
		case 0x8295: return 0x75; // 'Çï'
		case 0x8296: return 0x76; // 'Çñ'
		case 0x8297: return 0x77; // 'Çó'
		case 0x8298: return 0x78; // 'Çò'
		case 0x8299: return 0x79; // 'Çô'
		case 0x829a: return 0x7a; // 'Çö'
		case 0x816f: return 0x7b; // 'Åo'
		case 0x8162: return 0x7c; // 'Åb'
		case 0x8170: return 0x7d; // 'Åp'
		case 0x8160: return 0x7e; // 'Å`'
		case 0x8142: return 0xa1; // 'ÅB'
		case 0x8175: return 0xa2; // 'Åu'
		case 0x8176: return 0xa3; // 'Åv'
		case 0x8141: return 0xa4; // 'ÅA'
		case 0x8145: return 0xa5; // 'ÅE'
		case 0x82f0: return 0xa6; // 'Ç'
		case 0x829f: return 0xa7; // 'Çü'
		case 0x82a1: return 0xa8; // 'Ç°'
		case 0x82a3: return 0xa9; // 'Ç£'
		case 0x82a5: return 0xaa; // 'Ç•'
		case 0x82a7: return 0xab; // 'Çß'
		case 0x82e1: return 0xac; // 'Ç·'
		case 0x82e3: return 0xad; // 'Ç„'
		case 0x82e5: return 0xae; // 'ÇÂ'
		case 0x82c1: return 0xaf; // 'Ç¡'
		case 0x815b: return 0xb0; // 'Å['
		case 0x82a0: return 0xb1; // 'Ç†'
		case 0x82a2: return 0xb2; // 'Ç¢'
		case 0x82a4: return 0xb3; // 'Ç§'
		case 0x82a6: return 0xb4; // 'Ç¶'
		case 0x82a8: return 0xb5; // 'Ç®'
		case 0x82a9: return 0xb6; // 'Ç©'
		case 0x82ab: return 0xb7; // 'Ç´'
		case 0x82ad: return 0xb8; // 'Ç≠'
		case 0x82af: return 0xb9; // 'ÇØ'
		case 0x82b1: return 0xba; // 'Ç±'
		case 0x82b3: return 0xbb; // 'Ç≥'
		case 0x82b5: return 0xbc; // 'Çµ'
		case 0x82b7: return 0xbd; // 'Ç∑'
		case 0x82b9: return 0xbe; // 'Çπ'
		case 0x82bb: return 0xbf; // 'Çª'
		case 0x82bd: return 0xc0; // 'ÇΩ'
		case 0x82bf: return 0xc1; // 'Çø'
		case 0x82c2: return 0xc2; // 'Ç¬'
		case 0x82c4: return 0xc3; // 'Çƒ'
		case 0x82c6: return 0xc4; // 'Ç∆'
		case 0x82c8: return 0xc5; // 'Ç»'
		case 0x82c9: return 0xc6; // 'Ç…'
		case 0x82ca: return 0xc7; // 'Ç '
		case 0x82cb: return 0xc8; // 'ÇÀ'
		case 0x82cc: return 0xc9; // 'ÇÃ'
		case 0x82cd: return 0xca; // 'ÇÕ'
		case 0x82d0: return 0xcb; // 'Ç–'
		case 0x82d3: return 0xcc; // 'Ç”'
		case 0x82d6: return 0xcd; // 'Ç÷'
		case 0x82d9: return 0xce; // 'ÇŸ'
		case 0x82dc: return 0xcf; // 'Ç‹'
		case 0x82dd: return 0xd0; // 'Ç›'
		case 0x82de: return 0xd1; // 'Çﬁ'
		case 0x82df: return 0xd2; // 'Çﬂ'
		case 0x82e0: return 0xd3; // 'Ç‡'
		case 0x82e2: return 0xd4; // 'Ç‚'
		case 0x82e4: return 0xd5; // 'Ç‰'
		case 0x82e6: return 0xd6; // 'ÇÊ'
		case 0x82e7: return 0xd7; // 'ÇÁ'
		case 0x82e8: return 0xd8; // 'ÇË'
		case 0x82e9: return 0xd9; // 'ÇÈ'
		case 0x82ea: return 0xda; // 'ÇÍ'
		case 0x82eb: return 0xdb; // 'ÇÎ'
		case 0x82ed: return 0xdc; // 'ÇÌ'
		case 0x82f1: return 0xdd; // 'ÇÒ'
		case 0x814a: return 0xde; // 'ÅJ'
		case 0x814b: return 0xdf; // 'ÅK'
	}
	return code;
}

