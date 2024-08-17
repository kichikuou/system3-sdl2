#include "mako_ymfm.h"
#include <algorithm>

const int OPNA_CLOCK = 3993600 * 2;

int16_t mixSample(int32_t fm, int32_t ssg) {
	return ymfm::clamp(fm * 2 + ssg / 2, -32768, 32767);
}

MakoYmfm::MakoYmfm(int rate, std::vector<uint8_t> data) :
	MakoFM(std::move(data)),
	sample_rate(rate),
	opna(*this)
{
	opna.set_fidelity(ymfm::OPN_FIDELITY_MIN);
	opna.reset();
}

void MakoYmfm::Process(int16_t* stream, int len) {
	int16_t* buf = stream;

	if (samples_left > 0) {
		Generate(buf, samples_left);
		buf += samples_left * 2;
		len -= samples_left;
		samples_left = 0;
	}
	while (len > 0) {
		MainLoop();
		int t = static_cast<int>(GetTime());
		int dt = t - last_sync;
		last_sync = t;
		int samples = sample_rate * dt / 1000;
		if (samples > len) {
			samples_left = samples - len;
			samples = len;
		}
		Generate(buf, samples);
		buf += samples * 2;
		len -= samples;
	}
}

void MakoYmfm::SetReg(RegType type, uint8_t addr, uint8_t val) {
	uint32_t offset = type == OPNA_SLAVE ? 2 : 0;
	opna.write(offset, addr);
	opna.write(offset + 1, val);
}

void MakoYmfm::Generate(int16_t* buf, int samples) {
	const emulated_time output_step = 0x100000000ull / sample_rate;
	const emulated_time opna_step   = 0x100000000ull / opna.sample_rate(OPNA_CLOCK);

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
}

void MakoYmfm::get_mark(int* mark, int* loop) {
	*mark = GetMark();
	*loop = Looped() ? 1 : 0;
}
