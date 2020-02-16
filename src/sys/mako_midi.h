#ifndef _MAKO_MIDI_H_
#define _MAKO_MIDI_H_

#include <vector>
#include "../common.h"

#define MAX_MMLS (128 * 1024)

class NACT;

class MAKOMidi {
public:
	MAKOMidi(NACT* nact, char* amus) : nact(nact), amus(amus) {}
	bool load_mml(int page);
	void load_mda(int page);
	std::vector<uint8> generate_smf(int current_max);

private:
	NACT* nact;
	char* amus;

	struct MML {
		uint8 data[MAX_MMLS];
		int addr;
	};
	MML mml[9];

	struct MDA {
		int bank_select;
		int program_change;
		int level;
		int reverb;
		int chorus;
		int key_shift;
		int pan;
	};
	MDA mda[259];
	int drum_map[8][128];
	int tempo, tempo_dif;
};

#endif // _MAKO_MIDI_H_
