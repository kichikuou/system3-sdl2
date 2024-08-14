#ifndef _MAKO_MIDI_H_
#define _MAKO_MIDI_H_

#include <memory>
#include <queue>
#include <SDL.h>
#include "../common.h"

class NACT;

#ifdef USE_MIDI

class MAKOMidi {
public:
	MAKOMidi();
	~MAKOMidi();
	bool is_available();
	bool play(NACT* nact, char* amus, int page, int loop);
	void stop();
	bool is_playing();
	void get_mark(int* mark, int* loop);

private:
	struct Command;
	std::queue<std::unique_ptr<Command>> queue;
	SDL_mutex* queue_mutex = nullptr;
	SDL_Thread* thread = nullptr;
	SDL_atomic_t current_seq;
	SDL_atomic_t current_loop;
	SDL_atomic_t current_mark;
	int next_seq = 0;

	void thread_loop();
	static int thread_main(void* data);
};

#else // USE_MIDI

class MAKOMidi {
public:
	bool is_available() { return false; }
	bool play(NACT* nact, char* amus, int page, int loop) { return false; }
	void stop() {}
	bool is_playing() { return false; }
	void get_mark(int* mark, int* loop) { *mark = *loop = 0; }
};

#endif // USE_MIDI

#endif // _MAKO_MIDI_H_
