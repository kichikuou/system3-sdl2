#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>

class Encoding;

enum class TexthookMode {
	NONE,
	PRINT,
	COPY,
};

enum class DebuggerMode {
	DISABLED,
	CLI,
	DAP,
};

struct Strings {
	std::string back;
	std::string next_page;
	std::string dps_custom;
	std::string dps_linus;
	std::string dps_katsumi;
	std::string dps_yumiko;
	std::string dps_itsumi;
	std::string dps_hitomi;
	std::string dps_mariko;
};

struct Config {
 public:
	Config(int argc, char *argv[]);
	Strings get_strings(Encoding* encoding, bool english) const;

	std::string font_file;
	std::string game_id;
	std::string encoding;
	std::string save_dir;
	std::string playlist;
	std::string title;
	int midi_device = -1;
	bool print_version = false;
	bool trace = false;
	bool use_fm = false;
	bool no_antialias = false;
	bool scanline = false;
	TexthookMode texthook_mode = TexthookMode::NONE;
	DebuggerMode debugger_mode = DebuggerMode::DISABLED;
	std::string texthook_suppressions;

 private:
	void load_ini();

	Strings strings;
};

#endif // CONFIG_H_
