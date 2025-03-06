#include "mako_mp3.h"
#include <SDL3/SDL.h>

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

MakoMP3::MakoMP3(SDL_AudioDeviceID device, const std::string& path, int loops)
	: loops_(loops)
{
	if (!drmp3_init_file(&mp3, path.c_str(), NULL)) {
		return;
	}
	SDL_AudioSpec spec, device_spec;
	spec.freq = mp3.sampleRate;
	spec.channels = mp3.channels;
	spec.format = SDL_AUDIO_S16LE;
	SDL_GetAudioDeviceFormat(device, &device_spec, nullptr);
	stream = SDL_CreateAudioStream(&spec, &device_spec);
	SDL_SetAudioStreamGetCallback(stream, [](void* this_, SDL_AudioStream*, int additional_amount, int total_amount) {
		static_cast<MakoMP3*>(this_)->AudioCallback(additional_amount, total_amount);
	}, this);
	SDL_BindAudioStream(device, stream);
	playing = true;
}

MakoMP3::~MakoMP3() {
	if (stream) {
		SDL_DestroyAudioStream(stream);
		drmp3_uninit(&mp3);
	}
}

void MakoMP3::AudioCallback(int additional_amount, int total_amount) {
	if (!playing)
		return;
	int16_t* buf = SDL_stack_alloc(int16_t, additional_amount / 2);
	int frames_read = static_cast<int>(drmp3_read_pcm_frames_s16(&mp3, additional_amount / 4, buf));
	if (frames_read < additional_amount / 4) {
		if (loops_ && --loops_ == 0) {
			playing = false;
		} else {
			drmp3_seek_to_pcm_frame(&mp3, 0);
			frames_read += static_cast<int>(drmp3_read_pcm_frames_s16(&mp3, additional_amount / 4 - frames_read, buf + frames_read * 2));
		}
	}
	SDL_PutAudioStreamData(stream, buf, frames_read * 4);
	SDL_stack_free(buf);
}
