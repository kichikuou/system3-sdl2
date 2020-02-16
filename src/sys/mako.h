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

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
#include <SDL_mixer.h>
#endif

class MAKO
{
public:
	MAKO(NACT* parent, const char* playlist);
	~MAKO();

	void play_music(int page);
	void stop_music();
	bool check_music();
	void get_mark(int* mark, int* loop);

	void play_pcm(int page, bool loop);
	void stop_pcm();
	bool check_pcm();

	// AMUS.DAT, AMSE.DAT
	char amus[16];
	char amse[16];

	int current_music;
	int next_loop;		// Y19
	int cd_track[100];	// Z

private:
	NACT* nact;

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
	bool load_playlist(const char* path);

	std::vector<const char*> playlist;
	std::vector<uint8> smf;
	Mix_Music *mix_music;
	Mix_Chunk *mix_chunk;
#endif
};

#endif
