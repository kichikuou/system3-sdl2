#ifndef MAKO_MUSIC_H_
#define MAKO_MUSIC_H_

#include <memory>
#include <string>
#include <SDL.h>

class MakoMusicDecoder;

// Plays a single BGM file (MP3, OGG, or WAV, chosen by file extension),
// decoding on demand and mixing into the audio callback's output buffer.
class MakoMusic {
public:
	// loops: number of times to play; 0 means loop forever.
	MakoMusic(const std::string& path, int loops, const SDL_AudioSpec& device_spec);
	~MakoMusic();

	bool is_open() const { return stream != nullptr; }
	bool is_playing() const { return playing; }

	// Called from the audio callback.
	void mix(Uint8* out, int len);

private:
	// Decodes and queues one chunk, handling EOF and decoder errors.
	void decode();

	std::unique_ptr<MakoMusicDecoder> decoder;
	SDL_AudioStream* stream = nullptr;
	int loops_;  // number of times to play, 0 for infinite loop
	bool playing = false;
	bool input_finished = false;
};

#endif // MAKO_MUSIC_H_
