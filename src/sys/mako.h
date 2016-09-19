/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#ifndef _MAKO_H_
#define _MAKO_H_

#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "../common.h"
#include "nact.h"

#ifndef EMSCRIPTEN

#define MAX_SAMPLES (128 * 1024)
#define MAX_MMLS (128 * 1024)

#define _USE_REAL_MIDI
//#define _USE_PCM

class MAKO
{
protected:
	NACT* nact;

private:
	void reset_midi();
	void send_2bytes(uint8 d1, uint8 d2);
	void send_3bytes(uint8 d1, uint8 d2, uint8 d3);

	// FM, CD-DA
	bool cdda_play;
	int current_mark;	// Y18
	int current_loop;
	int current_max;	// Y19

	// PCM
	char wav[MAX_SAMPLES];

	// MIDIçƒê∂ÉXÉåÉbÉh
	typedef struct {
		MAKO* mako;
		int current_music;
		int next_music;
		bool request_fill;
		bool terminate;
	} PARAMS, *PPARAMS;
	PARAMS params;

	static int thread(void* pvoid);
	SDL_Thread* hThread;

	// MML
	typedef struct {
		uint8 data[MAX_MMLS];
		int addr;
	} MML;
	MML mml[9];

	// MDA
	typedef struct {
		int bank_select;
		int program_change;
		int level;
		int reverb;
		int chorus;
		int key_shift;
		int pan;
	} MDA;
	MDA mda[259];
	int drum_map[8][128];
	int tempo, tempo_dif;

	// MIDIçƒê∂èÛë‘
	typedef struct {
		int timbre;
		int timbre_type;
		int level;
		int reverb;
		int chorus;
		int pan;
		int key_shift;
		int velocity;
		int channel;
		int note;
		int loop_cnt;
		int wait_time;
		bool loop_flag;
		bool note_flag;
	} PLAY;
	PLAY play[9];
	bool mute_flag;
	float play_time;
	Uint32 prev_time;

public:
	MAKO(NACT* parent);
	~MAKO();

	void play_music(int page);
	void stop_music();
	bool check_music();
	void get_mark(int* mark, int* loop);
	void notify_mci(int status);

	void play_pcm(int page, bool loop);
	void stop_pcm();
	bool check_pcm();

	void initialize_midi();
	void release_midi();
	void stop_midi();
	void start_midi();
	void play_midi();

	bool load_mml(int page);
	void load_mda(int page);

	// AMUS.DAT, AMSE.DAT
	_TCHAR amus[16];
	_TCHAR amse[16];

	int current_music;
	int next_loop;		// Y19
	int cd_track[100];	// Z
};

#else // EMSCRIPTEN

class MAKO {
public:
	MAKO(NACT* parent);

	void play_music(int page);
	void stop_music();
	bool check_music() { return current_music != 0; }
	void get_mark(int* mark, int* loop) {}
	void notify_mci(int status) {}

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
};

#endif // EMSCRIPTEN

#endif
