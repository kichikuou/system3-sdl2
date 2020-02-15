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

#define MAX_SAMPLES (128 * 1024)

//#define _USE_PCM

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

	void initialize_midi();
	void release_midi();
	void stop_midi();
	void start_midi();
	void play_midi();

	// AMUS.DAT, AMSE.DAT
	_TCHAR amus[16];
	_TCHAR amse[16];

	int current_music;
	int next_loop;		// Y19
	int cd_track[100];	// Z

private:
	bool load_playlist(const char* path);

	NACT* nact;
	std::vector<const char*> playlist;
	Mix_Music *mix_music;
	std::vector<uint8> smf;
	char wav[MAX_SAMPLES];
};

#else // Emscripten or Android

class MAKO {
public:
	MAKO(NACT* parent, const char* playlist);

	void play_music(int page);
	void stop_music();
	bool check_music() { return current_music != 0; }
	void get_mark(int* mark, int* loop) {}

	void play_pcm(int page, bool loop) {}
	void stop_pcm() {}
	bool check_pcm() { return false; }

	void initialize_midi() {}
	void release_midi() {}
	void stop_midi() {}
	void start_midi() {}
	void play_midi() {}

	bool load_mml(int page) { return false; }
	void load_mda(int page) {}

	// AMUS.DAT, AMSE.DAT
	_TCHAR amus[16];
	_TCHAR amse[16];

	int current_music;
	int next_loop;		// Y19
	int cd_track[100];	// Z

private:
	NACT* nact;
};

#endif // EMSCRIPTEN

#endif
