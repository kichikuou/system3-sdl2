#include "config.h"

#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

Config::Config(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-gamedir") == 0)
			chdir(argv[++i]);
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
		else if (strcmp(argv[i], "-game") == 0)
			game_id = argv[++i];
	}
}
