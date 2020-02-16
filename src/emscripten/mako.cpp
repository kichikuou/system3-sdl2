/*
	ALICE SOFT SYSTEM 3 for Emscripten
	[ MAKO ]
*/

#include <memory>
#include <emscripten.h>
#include "mako.h"
#include "mako_midi.h"

MAKO::MAKO(NACT* parent, const char* playlist) :
	current_music(0),
	next_loop(0),
	nact(parent)
{
	strcpy(amus, "AMUS.DAT");
	for (int i = 1; i <= 99; i++)
		cd_track[i] = 0;
}

MAKO::~MAKO() {}

void MAKO::play_music(int page)
{
	if (current_music == page)
		return;

	int track = page < 100 ? cd_track[page] : 0;
	if (track) {
		EM_ASM_ARGS({ xsystem35.cdPlayer.play($0, $1); },
					cd_track[page] + 1, next_loop ? 0 : 1);
	} else {
		auto midi = std::make_unique<MAKOMidi>(nact, amus);
		if (!midi->load_mml(page)) {
			WARNING("load_mml(%d) failed", page);
			return;
		}
		midi->load_mda(page);
		std::vector<uint8_t> smf = midi->generate_smf(next_loop);

		EM_ASM_ARGS({ xsystem35.midiPlayer.play($0, $1, $2); },
					next_loop, smf.data(), smf.size());
	}

	current_music = page;
	next_loop = 0;
}

void MAKO::stop_music()
{
	if (!current_music)
		return;

	EM_ASM({
		xsystem35.cdPlayer.stop();
		xsystem35.midiPlayer.stop();
	});

	current_music = 0;
}

bool MAKO::check_music()
{
	return current_music != 0;
}

void MAKO::get_mark(int* mark, int* loop)
{
	WARNING("not implemented");
	*mark = *loop = 0;
}

void MAKO::play_pcm(int page, bool loop)
{
	WARNING("not implemented");
}

void MAKO::stop_pcm() {}

bool MAKO::check_pcm()
{
	return false;
}
