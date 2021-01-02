#include "makofm.h"
#include <algorithm>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

namespace {

const int PART_DISABLED = 2;
enum Flags {
	ALT_Q_FORMAT = 1 << 0,
	SKIP_KEYOFF = 1 << 1,
	LFO_FLAG = 1 << 2,
	SW_FRQLFO_ENABLED = 1 << 4,
	SW_VOLLFO_ENABLED = 1 << 5,
	USE_HW_LFO = 1 << 6,
	FRQLFO_DIRECTION = 1 << 7,
	VOLLFO_DIRECTION = 1 << 8,
	HW_LFO_WRITE_REQUEST = 1 << 9,
};

const uint16_t FMFreqTable[12] = {
	0x0269, 0x028E, 0x02B4, 0x02DE, 0x0309, 0x0338,
	0x0369, 0x039C, 0x03D3, 0x040E, 0x044B, 0x048D,
};
const uint16_t PSGFreqTable[12] = {
	0x0EED, 0x0E17, 0x0D4C, 0x0C8D, 0x0BD9, 0x0B2F,
	0x0A8E, 0x09F6, 0x0967, 0x08E0, 0x0860, 0x07E8,
};

const int TONE_DATA_SIZE = 43;
const uint8_t PresetFMTone[TONE_DATA_SIZE] = {
	// alg, keyon, reserved*5
	0x03, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00,

	// DT/ML, TL, KS/AR, DR, SR, SL/RR, reserved*3
	0x02, 0x0F, 0x1F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
	0x06, 0x28, 0x1F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
	0x04, 0x28, 0x1F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x1F, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
};

} // namespace

MakoFM::MakoFM(const uint8_t* data, bool free_data) :
	data(data),
	free_data(free_data),
	tone_offset(GetWord(0)),
	ver(GetWord(2)) {
	memset(work, 0, sizeof(work));
	for (int ch = 0; ch < MAKO_MAXCH; ch++) {
		work[ch].Q = 7;
		work[ch].default_length = 64;
		work[ch].octave = 4;
		work[ch].octave_temp = 4;
		work[ch].vol = 120;
		work[ch].panpot = 0x40;
	}
}

MakoFM::~MakoFM() {
	if (free_data)
		free((void*)data);
}

void MakoFM::MainLoop() {
	if (time_ms == 0.0) {
		InitOPNA();
		for (int ch = MAKO_MAXCH - 1; ch >= 0; ch--) {
			InitChannel(ch);
			ExecPan(ch);
			SetFMTone(ch, PresetFMTone);
			if (work[ch].part_enable == PART_DISABLED)
				continue;
			ExecPan(ch);
			ExecSequence(ch, true);
		}
	} else {
		for (int ch = MAKO_MAXCH - 1; ch >= 0; ch--) {
			ExecSequence(ch, false);
			ExecFLFO(ch);
			ExecVLFO(ch);
		}
	}
	time_ms += 545455.0 / (480.0 * tempo);
}

void MakoFM::InitOPNA() {
	WritePSG(0x00, 0x57);
	WritePSG(0x00, 0x41);
	WritePSG(0x00, 0x4f);
	WriteFM(0, 0x27, 0x30);
	WritePSG(0x07, 0xBf);
	WriteFM(0, 0x90, 0x00);
	WriteFM(0, 0x91, 0x00);
	WriteFM(0, 0x92, 0x00);
	WriteFM(0, 0x29, 0x83);
}

void MakoFM::InitChannel(int ch) {
	work[ch].part_offset = GetWord(4 + ch * 4);
	if (!work[ch].part_offset) {
		work[ch].part_enable = PART_DISABLED;
		part_looped |= 1 << ch;
		return;
	}
	uint16_t block0_offset = GetWord(work[ch].part_offset);
	if (block0_offset >= 0xff00) {
		work[ch].part_enable = PART_DISABLED;
		part_looped |= 1 << ch;
		return;
	}
	work[ch].jump_ptr = work[ch].part_offset + 2;
	work[ch].seq_ptr = block0_offset;
}

void MakoFM::ExecSequence(int ch, bool execcmd) {
	if (execcmd)
		goto execcmd;

	if (work[ch].part_enable != 1)
		return;
	if (work[ch].length_counter > 1) {
		work[ch].length_counter--;
		return;
	}
	if (!work[ch].rest)
		goto note1;

 execcmd:
	cmdFF = 0;
	work[ch].seq_ptr = ExecCmd(ch, data + work[ch].seq_ptr) - data;
	if (cmdFF) {
		uint16_t block_addr = GetWord(work[ch].jump_ptr);
		work[ch].jump_ptr += 2;
		if (block_addr >> 8 == 0xff) {
			part_looped |= 1 << ch;
			if (work[ch].part_enable == PART_DISABLED)
				return;
			if (work[ch].part_enable == 0) {
				work[ch].part_enable = 0xff;
				cmdFF = 0;
				return;
			}
			work[ch].jump_ptr = work[ch].part_offset + (block_addr & 0xff) * 2;
			block_addr = GetWord(work[ch].jump_ptr);
			work[ch].jump_ptr += 2;
		}
		work[ch].seq_ptr = block_addr;
		cmdFF = 0;
		goto execcmd;
	}

	if (work[ch].rest) {
		if (work[ch].off_length) {
			work[ch].length_counter = work[ch].off_length;
			return;
		}
		KeyOff(ch);
		goto execcmd;
	}

	// .note3
	if (work[ch].on_length) {
		work[ch].length_counter = work[ch].on_length;
	} else {
	note1:
		work[ch].rest = 1;
		if (work[ch].off_length == 0) {
			KeyOff(ch);
			goto execcmd;
		}
		// .note2
		work[ch].length_counter = work[ch].off_length;
	}

	// .note4
	if (work[ch].rest) {
		KeyOff(ch);
		return;
	}

	if (IsFMchannel(ch)) {
		uint8_t reg = 0xa4 + ch % 6;
		uint16_t freq = work[ch].blk_fnum + work[ch].detune;
		WriteFM(ch, reg, freq >> 8);
		WriteFM(ch, reg - 4, freq & 0xff);
		if (work[ch].flags & LFO_FLAG) {
			work[ch].flags &= ~LFO_FLAG;
			return;
		}
		SetFLFO(ch, false);
		SetVLFO(ch, false);
		if (work[ch].flags & USE_HW_LFO) {
			work[ch].flags |= HW_LFO_WRITE_REQUEST;
			ExecPan(ch);
		}
		WriteFM(ch, 0x28, work[ch].keyon + (ch < 6 ? ch : ch - 2));
	} else {
		uint16_t freq = work[ch].blk_fnum - work[ch].detune / 4;
		WritePSG((ch - 3) * 2 + 1, freq >> 8);
		WritePSG((ch - 3) * 2, freq & 0xff);
		if (work[ch].flags & LFO_FLAG) {
			work[ch].flags &= ~LFO_FLAG;
			return;
		}
		SetFLFO(ch, false);
		SetVLFO(ch, false);

		mixer &= (ch - 2 == 3 ? 4 : ch - 2) ^ 0xff;
		mixer |= 0xb8;
		WritePSG(7, mixer);
	}
}

const uint8_t* MakoFM::ExecCmd(int ch, const uint8_t* code) {
	bool cont = true;
	while (cont) {
		if (*code < 0xe0)
			return ExecNote(ch, code);
		cont = false;
		uint8_t a = *code++;
		switch (a) {
		case 0xec:	// Part end
			work[ch].part_enable = PART_DISABLED;
			part_looped |= 1 << ch;
			break;

		case 0xff:	// Jump
			cmdFF = 0xff;
			break;

		case 0xeb:	// Pan
			work[ch].panpot = *code++;
			ExecPan(ch);
			cont = true;
			break;

		case 0xee:  // Octave++
			work[ch].octave++;
			cont = true;
			break;

		case 0xef:  // Octave--
			work[ch].octave--;
			cont = true;
			break;

		case 0xf0:  // Octave
			work[ch].octave = *code++;
			cont = true;
			break;

		case 0xf1:  // Temporary octave
			work[ch].octave_temp = *code++;
			cont = true;
			break;

		case 0xf2:  // Q (1-8)
			work[ch].flags &= ~ALT_Q_FORMAT;
			work[ch].Q = *code++ + 1;
			cont = true;
			break;

		case 0xea:  // Q (alt)
			work[ch].flags |= ALT_Q_FORMAT;
			work[ch].Q = *code++;
			cont = true;
			break;

		case 0xe9:  // Tie
			work[ch].flags |= SKIP_KEYOFF;
			cont = true;
			break;

		case 0xe7:  // Frq LFO
			work[ch].flfo_params.delay   = code[0] | code[1] << 8;
			work[ch].flfo_params.counter = code[2] | code[3] << 8;
			work[ch].flfo_params.value   = code[4] | code[5] << 8;
			work[ch].flfo_params.max     = code[6] | code[7] << 8;
			code += 8;
			work[ch].flags |= SW_FRQLFO_ENABLED;
			work[ch].flags &= ~FRQLFO_DIRECTION;
			SetFLFO(ch, false);
			cont = true;
			break;

		case 0xe6:  // Vol LFO (PSG)
			work[ch].vlfo_params.delay   = code[0] | code[1] << 8;
			work[ch].vlfo_params.counter = code[2] | code[3] << 8;
			work[ch].vlfo_params.value   = code[4] | code[5] << 8;
			work[ch].vlfo_params.max     = code[6] | code[7] << 8;
			code += 8;
			work[ch].fm_delay = work[ch].vlfo_params.delay;
			work[ch].sw_lfo_val = 0;
			work[ch].flags |= SW_VOLLFO_ENABLED;
			work[ch].flags &= ~VOLLFO_DIRECTION;
			SetVLFO(ch, false);
			cont = true;
			break;

		case 0xf3:  // Default length
			{
				uint8_t a = *code++;
				if (a < 0x80)
					work[ch].default_length = a;
				else
					work[ch].default_length = (a - 0x80) << 8 | *code++;
			}
			cont = true;
			break;

		case 0xe5:  // LFO flags
			work[ch].flags &= ~0x70;
			work[ch].flags |= (*code++ & 7) << 4;
			cont = true;
			break;

		case 0xe8:  // Vol LFO (FM)
			work[ch].vlfo_params.delay   = code[0] | code[1] << 8;
			work[ch].vlfo_params.counter = code[2] | code[3] << 8;
			work[ch].vlfo_params.value   = code[4] | code[5] << 8;
			work[ch].vlfo_params.max     = code[6] | code[7] << 8;
			work[ch].fm_delay            = code[8] | code[9] << 8;
			code += 10;
			work[ch].flags |= SW_VOLLFO_ENABLED;
			cont = true;
			break;

		case 0xf4:  // Tempo
			tempo = *code++;
			cont = true;
			break;

		case 0xf5:  // FM tone set
			work[ch].tone_num = *code++;
			SetFMTone(ch, data + tone_offset + TONE_DATA_SIZE * work[ch].tone_num);
			ExecHWLFO(ch);
			cont = true;
			break;

		case 0xf7:  // Temp octave--
			work[ch].octave_temp = work[ch].octave - 1;
			work[ch].use_temp_octave = 1;
			cont = true;
			break;

		case 0xf8:  // Temp octave++
			work[ch].octave_temp = work[ch].octave + 1;
			work[ch].use_temp_octave = 1;
			cont = true;
			break;

		case 0xf9:  // Absolute vol
			work[ch].vol = work[ch].tl4_vol = std::min(*code++, static_cast<uint8_t>(127));
			SetVol(ch);
			cont = true;
			break;

		case 0xe1:  // Relative vol
			work[ch].vol = work[ch].tl4_vol =
				std::min(static_cast<uint8_t>(work[ch].vol + *code++),
						 static_cast<uint8_t>(127));
			SetVol(ch);
			cont = true;
			break;

		case 0xe4:  // HW_LFO
			hwlfo = *code++ | 8;
			work[ch].pms_param = *code++;
			work[ch].ams_param = *code++;
			work[ch].flags |= USE_HW_LFO;
			ExecHWLFO(ch);
			cont = true;
			break;

		case 0xfc:  // Detune
			work[ch].detune = *(const int8_t*)code++;
			cont = true;
			break;

		case 0xfe:
			code += 3;
			cont = true;
			break;

		case 0xf6:
			code += 2;
			cont = true;
			break;

		case 0xfa:
		case 0xfd:
			code++;
			cont = true;
			break;

		case 0xe0:
			current_mark = *code++;
			cont = true;
			break;

		default:
			WARNING("MakoFM: unknown command 0x%02x", a);
			work[ch].part_enable = PART_DISABLED;
			part_looped |= 1 << ch;
			break;
		}
	}
	return code;
}

const uint8_t* MakoFM::ExecNote(int ch, const uint8_t* code) {
	uint8_t note = *code++;
	if (note < 13) {
		work[ch].note = note;
		work[ch].length = *code++;
	} else if (note < 26) {
		work[ch].note = note - 13;
		work[ch].length = code[0] | code[1] << 8;
		code += 2;
	} else if (0x80 <= note && note < 0x80 + 13) {
		work[ch].note = note - 0x80;
		work[ch].length = work[ch].default_length;
	} else {
		WARNING("MakoFM: unknown command: 0x%02x", note);
		work[ch].part_enable = PART_DISABLED;
		part_looped |= 1 << ch;
		return code;
	}

	if (!work[ch].use_temp_octave)
		work[ch].octave_temp = work[ch].octave;
	work[ch].use_temp_octave = 0;

	uint16_t freq = 0;
	if (work[ch].note) {
		if (IsFMchannel(ch))
			freq = FMFreqTable[work[ch].note - 1] | work[ch].octave_temp << 11;
		else
			freq = PSGFreqTable[work[ch].note - 1] >> work[ch].octave_temp;
	}

	work[ch].blk_fnum = freq;
	if (work[ch].part_enable != PART_DISABLED)
		work[ch].part_enable = 1;

	if (!work[ch].note) {
		work[ch].on_length = 0;
		work[ch].off_length = work[ch].length;
		work[ch].rest = 1;
		return code;
	}

	uint16_t on_length, off_length;
	if (work[ch].length <= 1) {
		on_length = work[ch].length;
		off_length = 0;
	} else {
		if (work[ch].flags & ALT_Q_FORMAT) {
			if (work[ch].length >= work[ch].Q) {
				on_length = work[ch].length - work[ch].Q;
				off_length = work[ch].Q;
			} else {
				on_length = 1;
				off_length = work[ch].length - 1;
			}
		} else {
			if (work[ch].Q == 8) {
				on_length = work[ch].length - 1;
				off_length = 1;
			} else {
				assert(1 <= work[ch].Q && work[ch].Q <= 7);
				on_length = work[ch].length / 8 * work[ch].Q;
				if (!on_length)
					on_length = 1;
				off_length = work[ch].length - on_length;
				if (off_length == 0) {
					on_length--;
					off_length++;
				}
			}
		}
	}
	work[ch].rest = 0;
	work[ch].on_length = on_length;
	work[ch].off_length = off_length;
	return code;
}

void MakoFM::KeyOff(int ch) {
	if (work[ch].flags & SKIP_KEYOFF) {
		work[ch].flags &= ~SKIP_KEYOFF;
		work[ch].flags |= LFO_FLAG;
		return;
	}
	if (IsFMchannel(ch)) {
		if (work[ch].flags & USE_HW_LFO) {
			work[ch].flags &= ~HW_LFO_WRITE_REQUEST;
			ExecPan(ch);
		}
		WriteFM(ch, 0x28, ch < 3 ? ch : ch - 2);
	} else {
		if (work[ch].flags & SW_VOLLFO_ENABLED &&
			work[ch].vlfo_mode & 0x0f &&
			(work[ch].vlfo_mode & 8) == 0) {
			work[ch].vlfo_val = work[ch].vlfo_params.max;
			work[ch].vlfo_mode = 8;	// SW-VLFO mode 3
			return;
		}
		mixer |= (ch == 5 ? 4 : ch - 2) | 0xB8;
		WritePSG(7, mixer);
	}
}

void MakoFM::SetVol(int ch) {
	work[ch].vol = work[ch].tl4_vol;
	assert(work[ch].vol <= 127);
	if (IsFMchannel(ch)) {
		uint8_t reg = 0x40 + ch % 6;
		uint8_t val = 127 - work[ch].vol;
		switch (work[ch].alg) {
		case 7:
			WriteFM(ch, reg + 0x00, val);
			// fall through
		case 5: case 6:
			WriteFM(ch, reg + 0x04, val);
			// fall through
		case 4:
			WriteFM(ch, reg + 0x08, val);
			// fall through
		case 0: case 1: case 2: case 3:
			WriteFM(ch, reg + 0x0c, val);
		}
	} else {
		if (work[ch].flags & SW_VOLLFO_ENABLED)
			return;
		uint8_t val = work[ch].vol >> 3;
		WritePSG(ch + 5, val);
	}
}

void MakoFM::ExecPan(int ch) {
	if (!IsFMchannel(ch))
		return;
	uint8_t reg = 0xb4 + ch % 6;
	uint8_t panpot = work[ch].panpot;
	uint8_t val = (panpot == 0 || panpot == 0x40) ? 0xc0  // LR
		: (panpot < 0x40) ? 0x80  // L
		: 0x40;  // R
	if ((work[ch].flags & USE_HW_LFO) && (work[ch].flags & HW_LFO_WRITE_REQUEST))
		val |= work[ch].ams_param << 4 | work[ch].pms_param;
	WriteFM(ch, reg, val);
}

void MakoFM::SetFMTone(int ch, const uint8_t* tone) {
	if (!IsFMchannel(ch))
		return;
	uint8_t alg = *tone++;
	work[ch].alg = alg & 7;
	WriteFM(ch, 0xb0 + ch % 6, alg);

	work[ch].keyon = *tone * 16;
	tone += 6;

	work[ch].tl1 = tone[9 * 0 + 1];
	work[ch].tl2 = tone[9 * 1 + 1];
	work[ch].tl3 = tone[9 * 2 + 1];
	work[ch].tl4_vol = tone[9 * 3 + 1];

	const uint8_t regtop[] = {0x30, 0x38, 0x34, 0x3c};
	for (int op = 0; op < 4; op++) {
		uint8_t reg = regtop[op] + ch % 6;
		for (int i = 0; i < 6; i++) {	// 6 parameters (DT/ML,TL,KS/AR,DR,SR,SL/RR)
			uint8_t val = *tone++;
			// Since ver.0x300, write TL only to the modulator
			if (ver >= 0x300 && i == 1) {
				switch (work[ch].alg) {
				case 0: case 1: case 2: case 3:
					if (op < 3)
						WriteFM(ch, reg, val);
					break;
				case 4:
					if (op == 0 || op == 2)
						WriteFM(ch, reg, val);
					break;
				case 5: case 6:
					if (op == 0)
						WriteFM(ch, reg, val);
					break;
				case 7: default:
					break;
				}
			} else {
				WriteFM(ch, reg, val);
			}
			reg += 0x10;
		}
		tone += 3;
	}
}

void MakoFM::ExecFLFO(int ch) {
	if ((work[ch].flags & SW_FRQLFO_ENABLED) == 0)
		return;
	if ((work[ch].flags & (SKIP_KEYOFF | LFO_FLAG)) == 0) {
		if (work[ch].part_enable == PART_DISABLED)
			return;
		if (work[ch].rest || work[ch].note == 0)
			return;
	}
	if (work[ch].flq_lfo_counter-- > 0)
		return;
	work[ch].flq_lfo_counter = work[ch].flfo_params.counter;
	work[ch].flq_lfo_result += work[ch].flfo_params.value;
	if (work[ch].flfo_params.value & 0x8000) {
		// .minus
		if ((int16_t)work[ch].flfo_params.max >= (int16_t)work[ch].flq_lfo_result) {
			work[ch].flq_lfo_result = work[ch].flfo_params.max;
			SetFLFO(ch, true);
			work[ch].flags &= ~FRQLFO_DIRECTION;
		}
	} else {
		if ((int16_t)work[ch].flq_lfo_result >= (int16_t)work[ch].flfo_params.max) {
			work[ch].flq_lfo_result = work[ch].flfo_params.max;
			SetFLFO(ch, true);
			work[ch].flags |= FRQLFO_DIRECTION;
		}
	}
	// .output
	uint16_t result = work[ch].flq_lfo_result + work[ch].detune;
	if (IsFMchannel(ch)) {
		result += work[ch].blk_fnum;
		uint8_t reg = 0xa4 + ch % 6;
		WriteFM(ch, reg, result >> 8);
		WriteFM(ch, reg - 4, result & 0xff);
	} else {
		uint8_t reg = (ch - 3) * 2 + 1;
		result = -result;
		int16_t s_result = result;
		s_result >>= 2;
		s_result += work[ch].blk_fnum;
		result = s_result;
		WritePSG(reg, (result >> 8) & 0xf);
		WritePSG(reg - 1, result & 0xff);
	}
}

void MakoFM::ExecVLFO(int ch) {
	if ((work[ch].flags & SW_VOLLFO_ENABLED) == 0)
		return;
	if (work[ch].part_enable == PART_DISABLED)
		return;

	work[ch].vol = work[ch].tl4_vol;

	if (!IsFMchannel(ch)) {
		ExecVLFOp(ch);
		return;
	}

	if (work[ch].fm_delay) {
		work[ch].fm_delay--;  // counter--
		return;
	}
	work[ch].fm_delay = work[ch].vlfo_params.counter;
	work[ch].sw_lfo_val = work[ch].vlfo_params.value;  // bug (should be +=)?
	if (work[ch].vlfo_params.value & 0x8000) {
		if (work[ch].vlfo_params.max >= work[ch].sw_lfo_val) {
			work[ch].sw_lfo_val = work[ch].vlfo_params.max;
			SetVLFO(ch, true);  // negate add_value, max
			work[ch].flags |= VOLLFO_DIRECTION;
		}
	} else {
		if (work[ch].vlfo_params.max >= work[ch].sw_lfo_val) {
			work[ch].sw_lfo_val = work[ch].vlfo_params.max;
			SetVLFO(ch, true);  // negate add_value, max
			work[ch].flags |= VOLLFO_DIRECTION;
		}
	}
	// .skip1
	uint8_t val = 127 - work[ch].vol - (work[ch].sw_lfo_val & 0xff) & 0x7f;
	uint8_t reg = 0x40 + ch % 6;
	switch (work[ch].alg) {
	case 7:
		WriteFM(ch, reg, val);
		// fall through
	case 5: case 6:
		WriteFM(ch, reg + 4, val);
		// fall through
	case 4:
		WriteFM(ch, reg + 8, val);
		// fall through
	case 0: case 1: case 2: case 3:
		WriteFM(ch, reg + 12, val);
	}
}

void MakoFM::ExecVLFOp(int ch) {
	if (work[ch].vlfo_mode & 1) {
		// .mode0
		if (work[ch].sw_lfo_val < 0x3fff) {
			work[ch].sw_lfo_val += work[ch].vlfo_val;
		} else {
			work[ch].sw_lfo_val = 0x4000;
			work[ch].vlfo_counter = work[ch].vlfo_params.counter;  // counter = counter source
			work[ch].vlfo_val = work[ch].fm_delay;
			work[ch].vlfo_mode = 2;	// to mode1
		}
	} else if (work[ch].vlfo_mode & 2) {
		// .mode1
		if (work[ch].sw_lfo_val < work[ch].vlfo_val) {
			work[ch].vlfo_mode = 0;
			work[ch].sw_lfo_val = 0;
			KeyOff(ch);
			return;
		}
		work[ch].sw_lfo_val -= work[ch].vlfo_val;

		if (work[ch].vlfo_counter--) {
			work[ch].vlfo_val = work[ch].vlfo_params.value;	// value2
			work[ch].vlfo_mode = 4;	// to mode2
		}
	} else if (work[ch].vlfo_mode & 4) {
		// .mode2
		if (work[ch].sw_lfo_val < work[ch].vlfo_val) {
			work[ch].vlfo_mode = 0;
			work[ch].sw_lfo_val = 0;
			KeyOff(ch);
			return;
		}
		work[ch].sw_lfo_val -= work[ch].vlfo_val;
	} else if (work[ch].vlfo_mode & 8) {
		// .mode3, same as mode2
		if (work[ch].sw_lfo_val < work[ch].vlfo_val) {
			work[ch].vlfo_mode = 0;
			work[ch].sw_lfo_val = 0;
			KeyOff(ch);
			return;
		}
		work[ch].sw_lfo_val -= work[ch].vlfo_val;
	} else {
		return;
	}
	// .output
	uint8_t val = ((work[ch].sw_lfo_val >> 6) * work[ch].vol) >> 8;
	WritePSG(ch + 5, (val & 0x78) >> 3);
}

void MakoFM::SetFLFO(int ch, bool neg) {
	if (!neg) {
		if ((work[ch].flags & SW_FRQLFO_ENABLED) == 0)
			return;
		work[ch].flq_lfo_counter = work[ch].flfo_params.delay;
		work[ch].flq_lfo_result = 0;
		if ((work[ch].flags & FRQLFO_DIRECTION) == 0)
			return;
		work[ch].flags &= ~FRQLFO_DIRECTION;
	}
	work[ch].flfo_params.value = -work[ch].flfo_params.value;
	work[ch].flfo_params.max = -work[ch].flfo_params.max;
}

void MakoFM::SetVLFO(int ch, bool neg) {
	if (neg)
		goto neg;
	if ((work[ch].flags & SW_VOLLFO_ENABLED) == 0)
		return;
	if (IsFMchannel(ch)) {
		work[ch].fm_delay = work[ch].vlfo_params.delay;
		work[ch].sw_lfo_val = 0;
		if ((work[ch].flags & VOLLFO_DIRECTION) == 0)
			return;
		work[ch].flags &= ~VOLLFO_DIRECTION;
	neg:
		work[ch].vlfo_params.value = -work[ch].vlfo_params.value;
		work[ch].vlfo_params.max = -work[ch].vlfo_params.max;
	} else {
		work[ch].vlfo_mode = 1;
		work[ch].vlfo_val = work[ch].vlfo_params.delay;
		work[ch].sw_lfo_val = work[ch].vlfo_params.delay;
	}
}

void MakoFM::ExecHWLFO(int ch) {
	if ((work[ch].flags & USE_HW_LFO) == 0)
		return;
	assert(!"not implemented");
}
