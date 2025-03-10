#include "config.h"

#include "common.h"
#include <tuple>
#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include "encoding.h"

#define INIFILENAME "system3.ini"

namespace {

bool to_bool(const char *s, int lineno)
{
	if (!strcasecmp(s, "yes") || !strcasecmp(s, "true") || !strcasecmp(s, "on") || !strcmp(s, "1"))
		return true;
	if (!strcasecmp(s, "no") || !strcasecmp(s, "false") || !strcasecmp(s, "off") || !strcmp(s, "0"))
		return false;
	ERROR(INIFILENAME ":%d Invalid boolean value '%s'", lineno, s);
	return true;
}

int to_int(const char *s, int lineno = -1)
{
	char *end;
	int n = strtol(s, &end, 10);
	if (*end) {
		if (lineno == -1)
			ERROR("Command line: Invalid integer value '%s'", s);
		else
			ERROR(INIFILENAME ":%d Invalid integer value '%s'", lineno, s);
	}
	return n;
}

std::string normalize_path(std::string s)
{
	for (char& c : s) {
		if (c == '\\')
			c = '/';
	}
	return s;
}

bool is_empty_line(const char *s)
{
	for (; *s; s++) {
		if (!isspace((unsigned char)*s))
			return false;
	}
	return true;
}

char *trim(char *s)
{
	while (isspace((unsigned char)*s))
		s++;
	char *p = s + strlen(s) - 1;
	while (p > s && isspace((unsigned char)*p))
		*p-- = '\0';
	return s;
}

std::pair<char*, char*> parse_keyval(char *line)
{
	char *eq = strchr(line, '=');
	if (!eq)
		return {nullptr, nullptr};
	*eq = '\0';
	return { trim(line), trim(eq + 1) };
}

#ifdef ENABLE_DEBUGGER
DebuggerMode parse_debugger_mode(const char *s, int lineno = -1)
{
	if (!strcasecmp(s, "none"))
		return DebuggerMode::DISABLED;
	if (!strcasecmp(s, "cli"))
		return DebuggerMode::CLI;
	if (!strcasecmp(s, "dap"))
		return DebuggerMode::DAP;
	if (lineno == -1)
		ERROR("Command line: Invalid debugger mode '%s'", s);
	else
		ERROR(INIFILENAME ":%d Invalid debugger mode '%s'", lineno, s);
	return DebuggerMode::DISABLED;
}
#endif

TexthookMode parse_texthook_mode(const char *s, int lineno = -1)
{
	if (!strcasecmp(s, "none"))
		return TexthookMode::NONE;
	if (!strcasecmp(s, "print"))
		return TexthookMode::PRINT;
	if (!strcasecmp(s, "copy"))
		return TexthookMode::COPY;
	if (lineno == -1)
		ERROR("Command line: Invalid texthook mode '%s'", s);
	else
		ERROR(INIFILENAME ":%d Invalid texthook mode '%s'", lineno, s);
	return TexthookMode::NONE;
}

void init_string(std::string& s, Encoding* encoding, const char *dflt)
{
	s = encoding->fromUtf8(s.empty() ? dflt : s.c_str());
}

}  // namespace

Config::Config(int argc, char *argv[])
{
	// Process the -gamedir option first so that system3.ini is loaded from
	// the specified directory.
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-gamedir") == 0)
			chdir(argv[++i]);
	}

	load_ini();

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-gamedir") == 0)
			++i;
		else if (strcmp(argv[i], "-version") == 0)
			print_version = true;
		else if (strcmp(argv[i], "-noantialias") == 0)
			no_antialias = true;
		else if (strcmp(argv[i], "-savedir") == 0)
			save_dir = argv[++i];
		else if (strcmp(argv[i], "-fontfile") == 0)
			font_file = argv[++i];
		else if (strcmp(argv[i], "-playlist") == 0)
			playlist = argv[++i];
		else if (strcmp(argv[i], "-fm") == 0)
			use_fm = true;
		else if (strcmp(argv[i], "-mididevice") == 0)
			midi_device = to_int(argv[++i]);
		else if (strcmp(argv[i], "-game") == 0)
			game_id = argv[++i];
		else if (strcmp(argv[i], "-encoding") == 0)
			encoding = argv[++i];
		else if (strcmp(argv[i], "-title") == 0)
			title = argv[++i];
		else if (strcmp(argv[i], "-scanline") == 0)
			scanline = true;
#ifdef ENABLE_DEBUGGER
		else if (strcmp(argv[i], "-debugger") == 0)
			debugger_mode = parse_debugger_mode(argv[++i]);
#endif
		else if (strcmp(argv[i], "-trace") == 0)
			trace = true;
		else if (strcmp(argv[i], "-texthook") == 0)
			texthook_mode = parse_texthook_mode(argv[++i]);
		else if (strcmp(argv[i], "-texthook_suppress") == 0)
			texthook_suppressions = argv[++i];
	}
}

void Config::load_ini()
{
	FILE *fp = fopen(INIFILENAME, "r");
	if (!fp)
		return;

	enum {
		NO_SECTION,
		CONFIG,
		STRING,
	} current_section = NO_SECTION;

	char line[256];
	for (int lineno = 1; fgets(line, sizeof(line), fp); lineno++) {
		char val[256];

		if (line[0] == ';')  // comment
			continue;
		if (is_empty_line(line))
			continue;

		if (sscanf(line, "[%[^]]]", val)) {
			if (strcasecmp(val, "config") == 0)
				current_section = CONFIG;
			else if (strcasecmp(val, "string") == 0)
				current_section = STRING;
			else
				WARNING(INIFILENAME ":%d Unknown section \"%s\"", lineno, val);
		} else if (current_section == CONFIG) {
			char *key, *val;
			std::tie(key, val) = parse_keyval(line);
			if (!key) {
				WARNING(INIFILENAME ":%d parse error", lineno);
				continue;
			}
			if (!strcasecmp(key, "noantialias"))
				no_antialias = to_bool(val, lineno);
			else if (!strcasecmp(key, "savedir"))
				save_dir = normalize_path(val);
			else if (!strcasecmp(key, "fontfile"))
				font_file = normalize_path(val);
			else if (!strcasecmp(key, "playlist"))
				playlist = normalize_path(val);
			else if (!strcasecmp(key, "fm"))
				use_fm = to_bool(val, lineno);
			else if (!strcasecmp(key, "mididevice"))
				midi_device = to_int(val, lineno);
			else if (!strcasecmp(key, "game"))
				game_id = val;
			else if (!strcasecmp(key, "encoding"))
				encoding = val;
			else if (!strcasecmp(key, "title"))
				title = val;
			else if (!strcasecmp(key, "scanline"))
				scanline = to_bool(val, lineno);
#ifdef ENABLE_DEBUGGER
			else if (!strcasecmp(key, "debugger"))
				debugger_mode = parse_debugger_mode(val, lineno);
#endif
			else if (!strcasecmp(key, "trace"))
				trace = to_bool(val, lineno);
			else if (!strcasecmp(key, "texthook"))
				texthook_mode = parse_texthook_mode(val, lineno);
			else if (!strcasecmp(key, "texthook_suppress"))
				texthook_suppressions = val;
			else
				WARNING(INIFILENAME ":%d unknown key '%s'", lineno, key);
		} else if (current_section == STRING) {
			char *key, *val;
			std::tie(key, val) = parse_keyval(line);
			if (!key) {
				WARNING(INIFILENAME ":%d parse error", lineno);
				continue;
			}
			if (!strcasecmp(key, "back"))
				strings.back = val;
			else if (!strcasecmp(key, "next_page"))
				strings.next_page = val;
			else if (!strcasecmp(key, "dps_custom"))
				strings.dps_custom = val;
			else if (!strcasecmp(key, "dps_linus"))
				strings.dps_linus = val;
			else if (!strcasecmp(key, "dps_katsumi"))
				strings.dps_katsumi = val;
			else if (!strcasecmp(key, "dps_yumiko"))
				strings.dps_yumiko = val;
			else if (!strcasecmp(key, "dps_itsumi"))
				strings.dps_itsumi = val;
			else if (!strcasecmp(key, "dps_hitomi"))
				strings.dps_hitomi = val;
			else if (!strcasecmp(key, "dps_mariko"))
				strings.dps_mariko = val;
			else
				WARNING(INIFILENAME ":%d unknown key '%s'", lineno, key);
		} else {
			WARNING(INIFILENAME ":%d parse error", lineno);
		}
	}
	fclose(fp);
}

Strings Config::get_strings(Encoding* encoding, bool english) const
{
	Strings strs = strings;
	init_string(strs.back, encoding, english ? "Back" : u8"戻る");
	init_string(strs.next_page, encoding, english ? "Next Page" : u8"次のページ");
	init_string(strs.dps_custom, encoding, u8"カスタム");
	init_string(strs.dps_linus, encoding, u8"リーナス");
	init_string(strs.dps_katsumi, encoding, u8"かつみ");
	init_string(strs.dps_yumiko, encoding, u8"由美子");
	init_string(strs.dps_itsumi, encoding, u8"いつみ");
	init_string(strs.dps_hitomi, encoding, u8"ひとみ");
	init_string(strs.dps_mariko, encoding, u8"真理子");
	return strs;
}
