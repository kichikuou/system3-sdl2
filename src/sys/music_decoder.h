#ifndef MAKO_MUSIC_DECODER_H_
#define MAKO_MUSIC_DECODER_H_

#include <memory>
#include <string>
#include "sdl_compat.h"

struct DecodedChunk {
	const Uint8* data;
	int frames;
};

class MusicDecoder {
public:
	virtual ~MusicDecoder() = default;
	virtual bool is_open() const = 0;
	virtual const SDL_AudioSpec& spec() const = 0;
	virtual DecodedChunk decode(int max_frames) = 0;
	virtual void seek_start() = 0;
};

std::unique_ptr<MusicDecoder> create_music_decoder(const std::string& path);

#endif // MAKO_MUSIC_DECODER_H_
