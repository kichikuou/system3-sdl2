/*
	ALICE SOFT SYSTEM 3 for Emscripten
	[ MAKO ]
*/

#include <emscripten.h>
#include "mako.h"

MAKO::MAKO(NACT* parent)
	: current_music(0)
	, next_loop(0)
{
	for (int i = 1; i <= 99; i++)
		cd_track[i] = i;
}

void MAKO::play_music(int page)
{
	if (current_music == page)
		return;

	EM_ASM_ARGS({ xsystem35.cdPlayer.play($0, $1); },
				cd_track[page] + 1, next_loop ? 0 : 1);

	current_music = page;
	next_loop = 0;
}

void MAKO::stop_music()
{
	if (current_music)
		EM_ASM( xsystem35.cdPlayer.stop(); );

	current_music = 0;
}
