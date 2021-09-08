#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>

struct Config {
 public:
	std::string font_file;
	std::string game_id;
	std::string encoding;
	std::string save_dir;
	std::string playlist;
	std::string title;
	bool use_fm = false;
	bool no_antialias = false;

	Config(int argc, char *argv[]);

 private:
	void load_ini();
};

#endif // CONFIG_H_
