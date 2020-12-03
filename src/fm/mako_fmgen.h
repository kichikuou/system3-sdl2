#ifndef MAKOFMGEN_H_
#define MAKOFMGEN_H_

#include "fmgen/opna.h"
#include "makofm.h"

class MakoFMgen : private MakoFM {
public:
	MakoFMgen(int rate, const uint8_t* data, bool free_data);
	void Process(int16* stream, int len);

private:
	void SetReg(RegType type, uint8_t addr, uint8_t val) override;
	void Sync();
	void Mix(int samples);

	const int sample_rate;
	FM::OPNA opna;
	int sync_ms;
	int16* output;
	int out_samples;
	int samples_left;
};

#endif // MAKOFMGEN_H_
