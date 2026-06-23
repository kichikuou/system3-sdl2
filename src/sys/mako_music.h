#ifndef MAKO_MUSIC_H_
#define MAKO_MUSIC_H_

#include <memory>
#include <string>
#include <SDL3/SDL.h>

class MakoMusicDecoder;

// Plays a single BGM file (MP3, OGG, or WAV, chosen by file extension).
class MakoMusic {
public:
	// loops: number of times to play; 0 means loop forever.
	MakoMusic(SDL_AudioDeviceID device, const std::string& path, int loops);
	~MakoMusic();

	bool is_open() const { return stream != nullptr; }
	bool is_playing() const;

private:
	// SDL3 stream get-callback: decode and feed `additional_amount` bytes.
	void AudioCallback(int additional_amount);

	// Decodes and queues one chunk, handling EOF and decoder errors.
	void decode();

	std::unique_ptr<MakoMusicDecoder> decoder;
	SDL_AudioStream* stream = nullptr;
	int loops_;  // number of times to play, 0 for infinite loop
	bool playing = false;
	bool input_finished = false;
};

#endif // MAKO_MUSIC_H_
