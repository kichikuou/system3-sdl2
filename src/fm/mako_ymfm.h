#ifndef MAKOYMFM_H_
#define MAKOYMFM_H_

#include <vector>
#include <SDL3/SDL.h>
#include "ymfm_opn.h"
#include "makofm.h"

class MakoYmfm : private MakoFM, private ymfm::ymfm_interface {
public:
	MakoYmfm(SDL_AudioDeviceID device, std::vector<uint8_t> data);
	~MakoYmfm();
	void get_mark(int* mark, int* loop);

private:
	// we use an int64_t as emulated time, as a 32.32 fixed point value
	using emulated_time = int64_t;

	void SetReg(RegType type, uint8_t addr, uint8_t val) override;
	void AudioCallback(int additional_amount, int total_amount);
	std::vector<int16_t> Generate(int samples);

	SDL_AudioStream* stream;
	int sample_rate;
	ymfm::ym2608 opna;
	emulated_time opna_pos = 0;
	emulated_time output_pos = 0;
	int last_sync = 0;
};

#endif // MAKOYMFM_H_
