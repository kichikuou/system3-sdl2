#ifndef MAKOYMFM_H_
#define MAKOYMFM_H_

#include <queue>
#include <vector>
#include "ymfm_opn.h"
#include "makofm.h"

class MakoYmfm : private MakoFM, private ymfm::ymfm_interface {
public:
	MakoYmfm(int rate, std::vector<uint8_t> data);
	void Process(int16_t* stream, int len);
	void get_mark(int* mark, int* loop);

private:
	// we use an int64_t as emulated time, as a 32.32 fixed point value
	using emulated_time = int64_t;

	struct RegWrite {
		uint8_t port;
		uint8_t addr;
		uint8_t val;
	};

	void SetReg(RegType type, uint8_t addr, uint8_t val) override;
	void Generate(int16_t* buf, int samples);

	const int sample_rate;
	ymfm::ym2608 opna;
	emulated_time opna_pos = 0;
	emulated_time output_pos = 0;
	int last_sync = 0;
	int samples_left = 0;
	std::queue<RegWrite> queue;
};

#endif // MAKOYMFM_H_
