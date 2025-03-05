#ifndef MAKO_MP3_H_
#define MAKO_MP3_H_

#include <string>
#include <SDL3/SDL.h>
#include "dr_mp3.h"

class MakoMP3 {
public:
	MakoMP3(SDL_AudioDeviceID device, const std::string& path, int loops);
	~MakoMP3();
	bool is_playing() const { return playing; }

private:
	void AudioCallback(int additional_amount, int total_amount);

	drmp3 mp3;
	SDL_AudioStream* stream;
	int loops_;  // number of times to play, 0 for infinite loop
	bool playing = false;
};

#endif // MAKO_MP3_H_
