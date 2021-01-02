#ifndef MAKOFM_H_
#define MAKOFM_H_

#include <stdint.h>

const int MAKO_MAXCH = 9;

// MakoFM emulates the AliceSoft Music Driver (MAKO.COM) for PC-98
// version 2.67 [1].
// The implementation is based on mako60.z80 by Bookworm [2].
// [1] http://hp.vector.co.jp/authors/VA004111/filelist.htm
// [2] http://mydocuments.g2.xrea.com/html/p6/makop6.html
class MakoFM {
public:
	MakoFM(const uint8_t* data, bool free_data);
	~MakoFM();
	void MainLoop();

	double GetTime() const { return time_ms; }
	bool Looped() const { return part_looped == (1 << MAKO_MAXCH) - 1; }

protected:
	enum RegType {
		OPNA_MASTER,
		OPNA_SLAVE,
	};
	virtual void SetReg(RegType type, uint8_t addr, uint8_t val) = 0;

private:
	uint16_t GetWord(int offset) const {
		return data[offset] | data[offset + 1] << 8;
	}

	bool IsFMchannel(int ch) const {
		return ch < 3 || 6 <= ch;
	}

	void WriteFM(int ch, uint8_t addr, uint8_t val) {
		// Use OPNA_MASTER for control registers / PSG
		RegType type = (addr < 0x30 || ch < 6) ? OPNA_MASTER : OPNA_SLAVE;
		SetReg(type, addr, val);
	}

	void WritePSG(uint8_t addr, uint8_t val) {
		SetReg(OPNA_MASTER, addr, val);
	}

	void InitOPNA();
	void InitChannel(int ch);
	void ExecSequence(int ch, bool execcmd);
	const uint8_t* ExecCmd(int ch, const uint8_t* code);
	const uint8_t* ExecNote(int ch, const uint8_t* code);
	void KeyOff(int ch);
	void SetVol(int ch);
	void ExecPan(int ch);
	void SetFMTone(int ch, const uint8_t* tone);
	void ExecFLFO(int ch);
	void ExecVLFO(int ch);
	void ExecVLFOp(int ch);
	void SetFLFO(int ch, bool neg);
	void SetVLFO(int ch, bool neg);
	void ExecHWLFO(int ch);

	struct Work {
		uint16_t part_offset;		// W20A3
		uint16_t jump_ptr;			// W20A5
		uint16_t seq_ptr;			// W20A7
		uint16_t Q;					// W20A9
		uint16_t flags;				// W20AB
		uint8_t tone_num;			// W20AD
		uint8_t vol;				// W20AE
		uint8_t tl1;				// W20AF
		uint8_t tl2;				// W20B0
		uint8_t tl3;				// W20B1
		uint8_t tl4_vol;			// W20B2
		uint8_t use_temp_octave;	// W20B3
		uint8_t octave;				// W20B4
		uint8_t octave_temp;		// W20B5
		uint8_t note;				// W20B6 0=rest, 1-12=scale (cdefgab)
		uint16_t blk_fnum;			// W20B7
		uint16_t default_length;	// W20B9
		uint16_t length;			// W20BB
		uint16_t on_length;			// W20BD
		uint16_t off_length;		// W20BF
		uint16_t length_counter;	// W20C1
		uint8_t rest;				// W20C3 1=rest, 0=note
		uint8_t keyon;				// W20C4 key-on status
		int16_t detune;				// W20C5
		uint8_t alg;				// W20C7
		uint8_t panpot;				// W20C8
		uint8_t part_enable;		// W20C9 02=disable 01,FF=enable

		struct {
			uint16_t delay;			// W20DB
			uint16_t counter;		// W20DD
			uint16_t value;			// W20DF
			uint16_t max;			// W20E1
		} flfo_params;
		uint16_t flq_lfo_counter;	// W20E3
		uint16_t flq_lfo_result;	// W20E5

		struct {
			uint16_t delay;			// W20E7
			uint16_t counter;		// W20E9
			uint16_t value;			// W20EB
			uint16_t max;			// W20ED
		} vlfo_params;
		uint16_t fm_delay;			// W20EF
		uint16_t sw_lfo_val;		// W20F1
		uint16_t vlfo_val;			// W20F3
		uint16_t vlfo_counter;		// W20F5
		uint16_t vlfo_mode;			// W20F7

		uint8_t ams_param;			// W20F9
		uint8_t pms_param;			// W20FA
	};

	Work work[9];
	const uint8_t* data;
	const bool free_data;
	uint16_t tone_offset;
	uint16_t ver;

	uint8_t mixer = 0xbf;
	uint8_t hwlfo = 0;
	uint8_t tempo = 120;

	uint8_t cmdFF;

	unsigned part_looped = 0;
	double time_ms = 0;
	uint8_t current_mark;
};

#endif // MAKOFM_H_
