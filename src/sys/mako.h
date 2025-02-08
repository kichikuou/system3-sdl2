/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#ifndef _MAKO_H_
#define _MAKO_H_

#include <memory>
#include <vector>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "common.h"
#include "game_id.h"
#include "dri.h"

enum BGMDevice {
	BGM_FM,
	BGM_MIDI,
	BGM_CD
};

struct Config;

class MAKO
{
public:
	MAKO(const Config& config, const GameId& game_id);
	~MAKO();

	void play_music(int page);
	void stop_music();
	bool check_music();
	void get_mark(int* mark, int* loop);

	void play_pcm(int page, bool loop);
	void stop_pcm();
	bool check_pcm();

	void set_cd_track(int num, int track) { cd_track[num] = track; }

#ifdef _WIN32
	void select_sound(BGMDevice dev);
	void on_mci_notify(const SDL_SysWMmsg* msg);
#endif

	bool use_fm;

	Dri amus;
	Dri awav;
	Dri amse;
	Dri mda;

	int current_music;
	int next_loop;		// Y19

private:
	const GameId& game_id;
	int cd_track[100];	// Z

#ifndef __EMSCRIPTEN__
	bool load_playlist(const char* path);

	std::vector<const char*> playlist;
#endif
};

#endif
