/*
	ALICE SOFT SYSTEM 3 for Win32

	[ DRI ]
*/

#ifndef _DRI_H_
#define _DRI_H_

#include <string>
#include <stdint.h>
#include <vector>

struct GameId;

class Dri {
public:
	void open(const char* file_name);
	std::vector<uint8_t> load(int page);
	bool loaded() const { return !link_table.empty(); }

	static std::vector<uint8_t> load_mda(const GameId& game_id, int page);

private:
	std::string fname;
	std::vector<uint8_t> link_table;
};

#endif
