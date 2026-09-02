/*
	ALICE SOFT SYSTEM 3 for Win32

	[ NACT - text ]
*/

#include <string>
#include "nact.h"
#include "ags.h"
#include "encoding.h"
#include "texthook.h"

namespace {

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

} // namespace

void NACT::init_text()
{
	text.reset_pos(text_w[0].sx, text_w[0].sy + 2);
	text.line_space = 2;
	text.font_size = 16;
	if (game_id.sys_ver == 1) {
		text.font_color = 15 + 16;
		text.frame_color = 15 + 16;
		text.back_color = 0 + 16;
	} else {
		text.font_color = 15;
		text.frame_color = 15;
		text.back_color = 0;
	}

	menu.line_space = 4;
	menu.font_size = 16;
	if (game_id.sys_ver == 1) {
		menu.font_color = 15 + 16;
		menu.frame_color = 15 + 16;
		menu.back_color = 0 + 16;
	} else {
		menu.font_color = 15;
		menu.frame_color = 15;
		menu.back_color = 0;
	}
}

std::u16string NACT::decode_text(std::string_view string)
{
	std::u16string codes;
	while (!string.empty()) {
		int code = encoding->next_codepoint(string);
		if (draw_hankaku) {
			code = convert_to_hankaku(code);
		} else {
			code = convert_to_zenkaku(code);
		}
		codes.push_back(code);
	}
	return codes;
}

void NACT::add_menu_line(std::string_view string)
{
	menu_lines.push_back(decode_text(string));
}

void NACT::draw_text(std::string_view string, bool wait)
{
	if (string.empty())
		return;

	std::u16string codes = decode_text(string);

	if (defining_menu_item()) {
		*pending_menu_line += codes;
		return;
	}

	for (char16_t code : codes) {
		if (!(GAIJI_FIRST <= code && code <= GAIJI_LAST))
			texthook_character(sco.page(), code);
	}

	TextContext& ctx = text;
	ScreenId screen = ags->dest_screen;
	if (ctx.current_line_height < ctx.font_size)
		ctx.current_line_height = ctx.font_size;

	if (!wait) {
		ctx.pos.x = ags->draw_text(screen, ctx.pos.x, ctx.pos.y, codes, ctx.font_size, ctx.font_color);
		return;
	}

	// Typewriter effect.
	std::u16string_view rest(codes);
	for (;;) {
		size_t next_nonspace = rest.find_first_not_of(u' ');
		if (next_nonspace == std::u16string_view::npos)
			break;
		ctx.pos.x = ags->draw_text(screen, ctx.pos.x, ctx.pos.y, rest.substr(0, next_nonspace + 1),
								   ctx.font_size, ctx.font_color);
		rest.remove_prefix(next_nonspace + 1);
		text_wait();
	}
	if (!rest.empty())
		ctx.pos.x = ags->draw_text(screen, ctx.pos.x, ctx.pos.y, rest, ctx.font_size, ctx.font_color);
}
