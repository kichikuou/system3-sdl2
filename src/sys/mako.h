/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#ifndef _MAKO_H_
#define _MAKO_H_

#include <vector>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "../common.h"
#include "nact.h"

enum BGMDevice {
	BGM_FM,
	BGM_MIDI,
	BGM_CD
};

struct Config;

class MAKO
{
public:
	MAKO(NACT* parent, const Config& config);
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
#if defined(__ANDROID__)
	void select_synthesizer(bool use_fm);
#endif

	bool use_fm;

	// AMUS.DAT, AMSE.DAT
	char amus[16];
	char amse[16];

	int current_music;
	int next_loop;		// Y19

private:
	NACT* nact;
	int cd_track[100];	// Z

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
	bool load_playlist(const char* path);

	std::vector<const char*> playlist;
#endif
};

#endif
