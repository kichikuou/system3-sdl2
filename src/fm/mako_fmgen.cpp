#include "mako_fmgen.h"
#include <algorithm>
#include <assert.h>
#include <string.h>

const int OPNA_CLOCK = 3993600 * 2;

MakoFMgen::MakoFMgen(int rate, const uint8_t* data, bool free_data) :
	MakoFM(data, free_data),
	sample_rate(rate),
	sync_ms(0),
	samples_left(0) {
	bool ok = opna.Init(OPNA_CLOCK, sample_rate, false);
	assert(ok);
	opna.SetVolumePSG(-14);
	opna.Reset();
}

void MakoFMgen::Process(int16* stream, int len) {
	output = stream;
	out_samples = len;

	if (samples_left > 0) {
		int samples = samples_left;
		samples_left = 0;
		Mix(samples);
	}
	while (out_samples > 0) {
		MainLoop();
		Sync();
	}
}

void MakoFMgen::SetReg(RegType type, uint8_t addr, uint8_t val) {
	opna.SetReg(addr + (type == OPNA_SLAVE ? 0x100 : 0), val);
}

void MakoFMgen::Sync() {
	int t = static_cast<int>(GetTime());
	if (t == sync_ms)
		return;
	int ms = t - sync_ms;
	sync_ms = t;
	Mix(sample_rate * ms / 1000);
}

void MakoFMgen::Mix(int samples) {
	assert(samples_left == 0);
	if (samples > out_samples) {
		samples_left = samples - out_samples;
		samples = out_samples;
	}
	FM_SAMPLETYPE* buf = (FM_SAMPLETYPE*)alloca(samples * 2 * sizeof(FM_SAMPLETYPE));
	memset(buf, 0, samples * sizeof(FM_SAMPLETYPE) * 2);
	opna.Mix(buf, samples);
	for (int i = 0; i < samples * 2; i += 2) {
		output[i] = std::max(-0x7fff, std::min(0x7fff, buf[i]));
		output[i+1] = std::max(-0x7fff, std::min(0x7fff, buf[i+1]));
	}
	out_samples -= samples;
	output += samples * 2;
}
