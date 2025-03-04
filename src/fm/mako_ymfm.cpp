#include "mako_ymfm.h"
#include <algorithm>
#include <assert.h>
#include <SDL3/SDL.h>

namespace {

const int OPNA_CLOCK = 3993600 * 2;

int16_t mixSample(int32_t fm, int32_t ssg) {
	return ymfm::clamp(fm * 2 + ssg / 2, -32768, 32767);
}

} // namespace

MakoYmfm::MakoYmfm(SDL_AudioDeviceID device, std::vector<uint8_t> data) :
	MakoFM(std::move(data)),
	opna(*this)
{
	opna.set_fidelity(ymfm::OPN_FIDELITY_MIN);
	opna.reset();

	SDL_AudioSpec device_spec;
	SDL_GetAudioDeviceFormat(device, &device_spec, nullptr);
	sample_rate = device_spec.freq;
	SDL_AudioSpec fm_spec = device_spec;
	fm_spec.format = SDL_AUDIO_S16LE;
	fm_spec.channels = 2;
	stream = SDL_CreateAudioStream(&fm_spec, &device_spec);
	SDL_SetAudioStreamGetCallback(stream, [](void* this_, SDL_AudioStream* stream, int additional_amount, int total_amount) {
		static_cast<MakoYmfm*>(this_)->AudioCallback(additional_amount, total_amount);
	}, this);
	SDL_BindAudioStream(device, stream);
}

MakoYmfm::~MakoYmfm() {
	SDL_DestroyAudioStream(stream);
}

void MakoYmfm::AudioCallback(int additional_amount, int total_amount) {
	while (additional_amount > 0) {
		MainLoop();
		int t = static_cast<int>(GetTime());
		int dt = t - last_sync;
		last_sync = t;
		int samples = sample_rate * dt / 1000;
		std::vector<int16_t> buf = Generate(samples);
		SDL_PutAudioStreamData(stream, buf.data(), samples * 4);
		additional_amount -= samples * 4;
	}
}

void MakoYmfm::SetReg(RegType type, uint8_t addr, uint8_t val) {
	uint32_t offset = type == OPNA_SLAVE ? 2 : 0;
	opna.write(offset, addr);
	opna.write(offset + 1, val);
}

std::vector<int16_t> MakoYmfm::Generate(int samples) {
	const emulated_time output_step = 0x100000000ull / sample_rate;
	const emulated_time opna_step   = 0x100000000ull / opna.sample_rate(OPNA_CLOCK);

	std::vector<int16_t> buf(samples * 2);
	for (int i = 0; i < samples * 2; i += 2) {
		ymfm::ym2608::output_data opna_output;
		// generate at the appropriate sample rate
		assert(opna_pos <= output_pos);
		while (opna_pos <= output_pos) {
			opna.generate(&opna_output);
			opna_pos += opna_step;
		}
		buf[i] = mixSample(opna_output.data[0], opna_output.data[2]);
		buf[i+1] = mixSample(opna_output.data[1], opna_output.data[2]);
		output_pos += output_step;
	}
	return buf;
}

void MakoYmfm::get_mark(int* mark, int* loop) {
	SDL_LockAudioStream(stream);
	*mark = GetMark();
	*loop = Looped() ? 1 : 0;
	SDL_UnlockAudioStream(stream);
}
