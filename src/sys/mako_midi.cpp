/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO - midi ]
*/

#include <memory>
#include <vector>
#include <SDL3/SDL.h>
#include <RtMidi.h>

#include "mako_midi.h"
#include "game_id.h"
#include "dri.h"
#include "game_id.h"

namespace {

std::unique_ptr<RtMidiOut> midiout;

void initialize_midi(int device)
{
	try {
		midiout = std::make_unique<RtMidiOut>();
		int n = midiout->getPortCount();
		if (n == 0) {
			NOTICE("No MIDI output device available.");
			midiout.reset();
			return;
		}
		for (int i = 0; i < n; i++) {
			NOTICE("MIDI #%d: %s", i, midiout->getPortName(i).c_str());
		}
		if (device < 0) {
			// not specified, use the first device
			device = 0;
		} else if (device >= n) {
			WARNING("Invalid MIDI device number: %d", device);
			midiout.reset();
			return;
		}
		midiout->openPort(device);
	} catch (RtMidiError &error) {
		WARNING("Cannot initialize MIDI: %s", error.getMessage().c_str());
		midiout.reset();
	}
}

void release_midi()
{
	midiout.reset();
}

void reset_midi()
{
	uint8_t gs_reset[11] = {0xf0, 0x41, 0x20, 0x42, 0x12, 0x40, 0x00, 0x7f, 0x00, 0x41, 0xf7};
	try {
		midiout->sendMessage(gs_reset, sizeof(gs_reset));
	} catch (RtMidiError &error) {
		WARNING("MIDI error: %s", error.getMessage().c_str());
	}
}

void send_2bytes(uint8 d1, uint8 d2)
{
	try {
		uint8_t msg[2] = {d1, d2};
		midiout->sendMessage(msg, sizeof(msg));
	} catch (RtMidiError &error) {
		WARNING("MIDI error: %s", error.getMessage().c_str());
	}
}

void send_3bytes(uint8 d1, uint8 d2, uint8 d3)
{
	try {
		uint8_t msg[3] = {d1, d2, d3};
		midiout->sendMessage(msg, sizeof(msg));
	} catch (RtMidiError &error) {
		WARNING("MIDI error: %s", error.getMessage().c_str());
	}
}

void stop_midi()
{
	for(int i = 0; i < 10; i++) {
		send_3bytes(0xb0 + i, 0x78, 0x00);
	}
	for(int i = 0; i < 10; i++) {
		send_3bytes(0xb0 + i, 0x79, 0x00);
	}
}

class Playback {
public:
	static std::unique_ptr<Playback> create(const GameId& game_id, Dri& amus, Dri& mda, int page, int loop, int seq);
	Playback(const GameId& game_id, int loop, int seq) : game_id(game_id), loop_(loop), seq_(seq) {}
	bool load_mml(const std::vector<uint8_t>& data);
	void load_mda(const std::vector<uint8_t>& data);
	void start_midi();
	bool play_midi(SDL_AtomicInt* current_loop, SDL_AtomicInt* current_mark);
	int seq() const { return seq_; }

private:
	const GameId& game_id;
	int loop_;
	int seq_;

	class MML {
	public:
		MML() : addr(0) {}
		void write(uint8_t byte) { data.push_back(byte); }
		uint8_t next() { return data[addr++]; }
		void seek(size_t pos) { addr = pos; }
		uint32_t size() { return static_cast<uint32_t>(data.size()); }
	private:
		std::vector<uint8_t> data;
		size_t addr;
	};
	MML mml[9];

	struct MDA {
		int bank_select;
		int program_change;
		int level;
		int reverb;
		int chorus;
		int key_shift;
		int pan;
	};
	MDA mda[259];
	int drum_map[8][128];
	int tempo, tempo_dif;

	// MIDI playing status
	struct Play {
		int timbre;
		int timbre_type;
		int level;
		int reverb;
		int chorus;
		int pan;
		int key_shift;
		int velocity;
		int channel;
		int note;
		int loop_cnt;
		int wait_time;
		bool loop_flag;
		bool note_flag;
	};
	Play play[9];
	bool mute_flag;
	float play_time;
	Uint32 prev_time;
};

void Playback::start_midi()
{
	for(int i = 0; i < 9; i++) {
		play[i].loop_flag = true;
	}
	mute_flag = false;

	// 再生情報の初期化
	for(int i = 0; i < 9; i++) {
		play[i].timbre = 255;
		play[i].timbre_type = 128;
		play[i].level = 255;
		play[i].reverb = 255;
		play[i].chorus = 255;
		play[i].key_shift = 64;
		play[i].pan = 255;

		play[i].channel = i;
		play[i].velocity = 120;
		play[i].note = 128;

		play[i].note_flag = false;
		play[i].loop_cnt = 0;
		play[i].wait_time = 16;
		mml[i].seek(0);
	}

	for(int i = 0; i < 3; i++) {
		play[i + 3].timbre = i + 256;
		if(mda[i + 256].bank_select < 128) {
			play[i + 3].timbre_type = 128;
			play[i + 3].channel = i + 3;
		} else {
			play[i + 3].timbre_type = 255 - mda[i + 256].bank_select;
			play[i + 3].channel = 9;
		}
		play[i + 3].level = mda[i + 256].level;
		play[i + 3].reverb = mda[i + 256].reverb;
		play[i + 3].chorus = mda[i + 256].chorus;
		play[i + 3].key_shift = mda[i + 256].key_shift;
		play[i + 3].pan = mda[i + 256].pan;
	}

	reset_midi();

	// システムリセット、ボリュームリセット
	for(int i = 0; i < 10; i++) {
		send_3bytes(0xb0 + i, 0x79, 0x00);
	}
	for(int i = 0; i < 10; i++) {
		send_3bytes(0xb0 + i, 0x07, 0x64);
	}

	// SSG 1-3 の音色
	for(int i = 0; i < 3; i++) {
		if(mda[i + 256].bank_select < 128) {
			send_3bytes(0xb3 + i, 0x00, mda[i + 256].bank_select);
			send_3bytes(0xb3 + i, 0x20, 0x00);
			send_2bytes(0xc3 + i, mda[i + 256].program_change);
			send_3bytes(0xb3 + i, 0x07, mda[i + 256].level);
			send_3bytes(0xb3 + i, 0x5b, mda[i + 256].reverb);
			send_3bytes(0xb3 + i, 0x5d, mda[i + 256].chorus);
			send_3bytes(0xb3 + i, 0x0a, mda[i + 256].pan);
		} else {
			send_3bytes(0xb9, 0x07, mda[i + 256].level);
			send_3bytes(0xb9, 0x5b, mda[i + 256].reverb);
			send_3bytes(0xb9, 0x5d, mda[i + 256].chorus);
			send_3bytes(0xb9, 0x0a, mda[i + 256].pan);
		}
	}

	// タイマーリセット
	prev_time = SDL_GetTicks();
	play_time = 0;
}

bool Playback::play_midi(SDL_AtomicInt* current_loop, SDL_AtomicInt* current_mark)
{
	// 経過時間の取得
	Uint32 current_time = SDL_GetTicks();
	if(current_time > prev_time) {
		play_time += (float)(current_time - prev_time);
		prev_time = current_time;
	}

	// 各パートの更新
	while((play[0].loop_flag || play[1].loop_flag || play[2].loop_flag ||
		   play[3].loop_flag || play[4].loop_flag || play[5].loop_flag ||
		   play[6].loop_flag || play[7].loop_flag || play[8].loop_flag) && play_time > 0.0) {
		// 1単位時間だけ経過時間を更新する
		for(int i = 0; i < 9; i++) {
			if(play[i].loop_flag && play[i].wait_time) {
				play[i].wait_time--;
			}
		}
		play_time -= (float)(545455.0 / 480.0 / (float)(tempo + 0x40 - tempo_dif));

		// 1単位時間だけ各パートを更新
		for(int i = 0; i < 9; i++) {
			if(!play[i].loop_flag) {
				play[i].wait_time = 1;
			}
			while(play[i].wait_time == 0) {
				int d0 = mml[i].next(), d1, d2, d3;
				if(d0 == 0xff) {
					d1 = mml[i].next();
					d2 = mml[i].next();
					d3 = mml[i].next();
					mml[i].seek(d1 | (d2 << 8) | (d3 << 16));
					if(!play[i].note_flag || mute_flag) {
						// 再生停止または未使用
						play[i].loop_flag = false;
						if(!play[0].loop_flag && !play[1].loop_flag && !play[2].loop_flag &&
						   !play[3].loop_flag && !play[4].loop_flag && !play[5].loop_flag &&
						   !play[6].loop_flag && !play[7].loop_flag && !play[8].loop_flag) {
							// 全チャンネルが再生停止 (ループしない曲)
							stop_midi();
							SDL_SetAtomicInt(current_loop, 1);
							return false;
						}
						play[i].wait_time = 1;
					} else {
						// 全体としてのループ数を取得
						int loop = ++play[i].loop_cnt;
						for(int j = 0; j < 9; j++) {
							if(loop > play[j].loop_cnt && play[j].loop_flag) {
								loop = play[j].loop_cnt;
							}
						}
						SDL_SetAtomicInt(current_loop, loop);
						if(loop_ && loop >= loop_) {
							// 指定回数だけ再生完了
							stop_midi();
							return false;
						}
					}
				} else if(d0 < 0x80) {
					int note = d0;
					play[i].wait_time = mml[i].next();
					play[i].wait_time |= mml[i].next() << 8;
					if(play[i].timbre_type != 128) {
						note = drum_map[play[i].timbre_type][note];
					} else {
						note = (note + play[i].key_shift) - 64;
					}
					play[i].note_flag = true;
					play[i].note = note;
					send_3bytes(0x90 + play[i].channel, note, play[i].velocity);
				} else if(d0 == 0x80) {
					play[i].wait_time = mml[i].next();
					play[i].wait_time |= mml[i].next() << 8;
					if(play[i].note != 128) {
						send_3bytes(0x90 + play[i].channel, play[i].note, 0x00);
					}
					play[i].note = 128;
				} else if(d0 == 0xe0) {
					SDL_SetAtomicInt(current_mark, mml[i].next());
				} else if(d0 == 0xe1) {
					d1 = mml[i].next();
					play[i].velocity += (d1 > 127) ? (d1 - 256) : d1;
				} else if(d0 == 0xeb) {
					play[i].pan = mml[i].next();
					send_3bytes(0xb0 + play[i].channel, 0x0a, play[i].pan);
				}
				else if(d0 == 0xec) {
					if (!game_id.is_system1_dps())
						mute_flag = true;
				}
				else if(d0 == 0xf5) {
					int n = mml[i].next();
					if(n != play[i].timbre) {
						if(mda[n].bank_select < 128) {
							// 通常のパート
							play[i].timbre_type = 128;
							play[i].channel = i;
							send_3bytes(0xb0 + play[i].channel, 0x00, mda[n].bank_select);
							send_3bytes(0xb0 + play[i].channel, 0x20, 0x00);
							send_2bytes(0xc0 + play[i].channel, mda[n].program_change);
						} else {
							// ドラムパート (ch.10)
							play[i].timbre_type = 255 - mda[n].bank_select;
							play[i].channel = 9;
						}
						if(play[i].level != mda[n].level) {
							send_3bytes(0xb0 + play[i].channel, 0x07, mda[n].level);
						}
						if(play[i].reverb != mda[n].reverb) {
							send_3bytes(0xb0 + play[i].channel, 0x5b, mda[n].reverb);
						}
						if(play[i].chorus != mda[n].chorus) {
							send_3bytes(0xb0 + play[i].channel, 0x5d, mda[n].chorus);
						}
						if(play[i].pan != mda[n].pan) {
							send_3bytes(0xb0 + play[i].channel, 0x0a, mda[n].pan);
						}
					}
					play[i].timbre = n;
					play[i].level = mda[n].level;
					play[i].reverb = mda[n].reverb;
					play[i].chorus = mda[n].chorus;
					play[i].key_shift = mda[n].key_shift;
					play[i].pan = mda[n].pan;
					play[i].velocity = 120;
				} else if(d0 == 0xf9) {
					play[i].velocity = mml[i].next();
				} else if(d0 == 0xfc) {
					d1 = mml[i].next();
					if(d1 > 127) {
						d1 = 256 - d1;
						d1 = 0x40 - (d1 > 63 ? 63 : d1);
					} else {
						d1 = 0x40 + (d1 > 63 ? 63 : d1);
					}
					send_3bytes(0xb0 + play[i].channel, 0x65, 0x00);
					send_3bytes(0xb0 + play[i].channel, 0x64, 0x01);
					send_3bytes(0xb0 + play[i].channel, 0x06, d1);
				}
			}
		}
	}
	return true;
}

//static
std::unique_ptr<Playback> Playback::create(const GameId& game_id, Dri& amus, Dri& mda, int page, int loop, int seq)
{
	auto playback = std::make_unique<Playback>(game_id, loop, seq);

	std::vector<uint8_t> data = amus.load(page);
	if (data.empty())
		return nullptr;
	if (!playback->load_mml(data))
		return nullptr;

	// Load MDA
	data = mda.load(page);
	if (data.empty())
		data = Dri::load_mda(game_id, page);
	playback->load_mda(data);

	return playback;
}

bool Playback::load_mml(const std::vector<uint8_t>& data)
{
	// FM音源データの判定
	int p, d0, d1, d2;

	p = data[0] + 1;
	if(data[p] != 0x0f || (data[p + 1] + data[p + 2] + data[p + 3] + data[p + 4] + data[p + 5]) != 0) {
		return false;
	}

	// MML部 読み込み＆変換
	for(int i = 0; i < 9; i++) {
		p = 4 * (i + 1);
		int block_offset_top = data[p++];
		block_offset_top |= data[p++] << 8;
		uint32_t body_addr = 0;

		if(block_offset_top) {
			// ブロックの探索
			int last_block = 0;
			p = block_offset_top;
			d0 = data[p++];
			d1 = data[p++];
			while(d1 != 0xff) {
				last_block++;
				d0 = data[p++];
				d1 = data[p++];
			}
			int return_block = d0;

			uint16 base_octave = 4, current_octave = 4;
			uint16 default_time = 48;
			uint16 gate_step = 7, gate_time = 256;
			uint16 time, on_time, off_time;
			uint16 note;

			for(int j = 0; j < last_block; j++) {
				if(j == return_block) {
					body_addr = mml[i].size();
				}
				p = block_offset_top + 2 * j;
				int block_offset = data[p++];
				block_offset |= data[p++] << 8;
				p = block_offset;

				while((d0 = data[p++]) != 0xff) {
					if((0x00 <= d0 && d0 <= 0x0c) || (0x0d <= d0 && d0 <= 0x19) || (0x80 <= d0 && d0 <=0x8c)) {
						if(0x00 <= d0 && d0 <= 0x0c) {
							time = data[p++];
							note = d0;
						} else if(0x0d <= d0 && d0 <= 0x19) {
							time = data[p++];
							time |= data[p++] << 8;
							note = d0 - 0x0d;
						} else {
							time = default_time;
							note = d0 - 0x80;
						}
						if((d0 = data[p]) == 0xe9) {
							on_time = time;
							off_time = 0;
							p++;
						} else {
							if(gate_time != 256) {
								if(time > gate_time) {
									on_time = time - gate_time;
									off_time = gate_time;
								} else {
									on_time = 1;
									off_time = time - 1;
								}
							} else {
								if(time == 1) {
									on_time = 1;
									off_time = 0;
								} else if(gate_step == 8) {
									on_time = time -1 ;
									off_time = 1;
								} else if(time < 8) {
									on_time = 1;
									off_time = time - 1;
								} else {
									on_time = (time >> 3) * gate_step;
									off_time = time - on_time;
								}
							}
						}
						if(note == 0) {
							off_time = on_time + off_time;
							on_time = 0;
						}
						if(on_time != 0) {
							mml[i].write((note - 1) + 12 * (current_octave + 1));
							mml[i].write(on_time & 0xff);
							mml[i].write(on_time >> 8);
						}
						mml[i].write(0x80);
						mml[i].write(off_time & 0xff);
						mml[i].write(off_time >> 8);
						if(current_octave != base_octave) {
							current_octave = base_octave;
						}
					} else if(d0 == 0xe0 || d0 == 0xe1 || d0 == 0xeb || d0 == 0xf5 || d0 == 0xf9 || d0 == 0xfc) {
						mml[i].write(d0);
						mml[i].write(data[p++]);
					} else if(d0 == 0xea) {
						gate_time = data[p++];
					} else if(d0 == 0xec) {
						mml[i].write(d0);
					} else if(d0 == 0xee) {
						current_octave = ++base_octave;
					} else if(d0 == 0xef) {
						current_octave = --base_octave;
					} else if(d0 == 0xf0) {
						current_octave = base_octave = data[p++];
					} else if(d0 == 0xf2) {
						gate_step = data[p++] + 1;
						gate_time = 256;
					} else if(d0 == 0xf3) {
						d1 = data[p++];
						if(d1 >= 0x80) {
							d2 = data[p++];
							default_time = ((d1 - 0x80) << 8) | d2;
						} else {
							default_time = d1;
						}
					} else if(d0 == 0xf4) {
						tempo = data[p++];
					} else if(d0 == 0xf7) {
						current_octave--;
					} else if(d0 == 0xf8) {
						current_octave++;
					} else if(d0 == 0xe9) {
						; // Tie Command
					} else if(d0 == 0xe5 || d0 == 0xf1 || d0 == 0xfa || d0 == 0xfd) {
						p++;
					} else if(d0 == 0xf6 || d0 == 0xfb) {
						p += 2;
					} else if(d0 == 0xe4 || d0 == 0xfe) {
						p += 3;
					} else if(d0 == 0xe6 || d0 == 0xe7) {
						p += 8;
					} else if(d0 == 0xe8) {
						p += 10;
					}
				}
			}
		}
		mml[i].write(0xff);
		mml[i].write((body_addr >>  0) & 0xff);
		mml[i].write((body_addr >>  8) & 0xff);
		mml[i].write((body_addr >> 16) & 0xff);
	}
	return true;
}

void Playback::load_mda(const std::vector<uint8_t>& data)
{
	// 未設定時の音色設定 (Piano, Acoustic Bass Drum)
	for(int i = 0; i < 256 + 3; i++) {
		mda[i].bank_select = 0;
		mda[i].program_change = 0;
		mda[i].level = 100;
		mda[i].reverb = 40;
		mda[i].chorus = 40;
		mda[i].key_shift = 64;
		mda[i].pan = 64;
	}
	for(int i = 0; i < 8; i++) {
		for(int j = 0; j < 128; j++) {
			drum_map[i][j] = 35;
		}
	}
	tempo_dif = 0x40;

	if (data.empty())
		return;

	tempo_dif = data[7];
	int map_wide = data[24];
	int inst_num = data[25];
	int drum_format = data[26];

	// SSGパートの音色設定
	for(int i = 0; i < 3; i++) {
		const uint8_t* buf = &data[map_wide * i + 27];
		mda[i + 256].bank_select = buf[0];
		mda[i + 256].program_change = buf[1];
		mda[i + 256].level = buf[2];
		mda[i + 256].reverb = buf[3];
		mda[i + 256].chorus = buf[4];
		mda[i + 256].key_shift = buf[5];
		mda[i + 256].pan = buf[6];
	}

	// 通常の音色設定
	for(int i = 0; i < inst_num; i++) {
		const uint8_t* buf = &data[27 + map_wide * 3 + (map_wide + 1) * i];
		int n = buf[0];
		mda[n].bank_select = buf[1];
		mda[n].program_change = buf[2];
		mda[n].level = buf[3];
		mda[n].reverb = buf[4];
		mda[n].chorus = buf[5];
		mda[n].key_shift = buf[6];
		mda[n].pan = buf[7];
	}

	// ドラムパートの音色設定
	if(drum_format) {
		const uint8_t* buf = &data[27 + map_wide * 3 + (map_wide + 1) * inst_num];
		int drum_num = buf[0];
		for(int i = 0; i < drum_num; i++) {
			int d = buf[i * 3 + 1];
			int t = buf[i * 3 + 2];
			drum_map[d][t + 12] = buf[i * 3 + 3];
		}
	}
}

} // namespace

struct MAKOMidi::Command {
	enum Type {
		PLAY,
		STOP,
		TERMINATE
	} type;
	std::unique_ptr<Playback> playback;

	Command(Type t, std::unique_ptr<Playback> p = {}) : type(t), playback(std::move(p)) {}
};

MAKOMidi::MAKOMidi(int device)
{
	initialize_midi(device);
	if (midiout) {
		thread = SDL_CreateThread(thread_main, "MAKOMidi", this);
		queue_mutex = SDL_CreateMutex();
	}
	SDL_SetAtomicInt(&current_seq, 0);
	SDL_SetAtomicInt(&current_loop, 0);
	SDL_SetAtomicInt(&current_mark, 0);
}

MAKOMidi::~MAKOMidi()
{
	if (thread) {
		SDL_LockMutex(queue_mutex);
		queue.push(std::make_unique<MAKOMidi::Command>(Command::TERMINATE));
		SDL_UnlockMutex(queue_mutex);
		SDL_WaitThread(thread, NULL);
		SDL_DestroyMutex(queue_mutex);
	}
	release_midi();
}

bool MAKOMidi::is_available()
{
	return !!midiout;
}

bool MAKOMidi::play(const GameId& game_id, Dri& amus, Dri& mda, int page, int loop)
{
	int seq = ++next_seq;
	auto playback = Playback::create(game_id, amus, mda, page, loop, seq);
	if (!playback)
		return false;
	SDL_SetAtomicInt(&current_seq, seq);
	SDL_LockMutex(queue_mutex);
	queue.push(std::make_unique<MAKOMidi::Command>(Command::PLAY, std::move(playback)));
	SDL_UnlockMutex(queue_mutex);
	return true;
}

void MAKOMidi::stop()
{
	SDL_SetAtomicInt(&current_seq, 0);
	SDL_LockMutex(queue_mutex);
	queue.push(std::make_unique<MAKOMidi::Command>(Command::STOP));
	SDL_UnlockMutex(queue_mutex);
}

bool MAKOMidi::is_playing()
{
	return SDL_GetAtomicInt(&current_seq) != 0;
}

void MAKOMidi::get_mark(int* mark, int* loop)
{
	*mark = SDL_GetAtomicInt(&current_mark);
	*loop = SDL_GetAtomicInt(&current_loop);
}

void MAKOMidi::thread_loop()
{
	std::unique_ptr<Playback> current;

	for (;;) {
		SDL_LockMutex(queue_mutex);
		while (!queue.empty()) {
			auto cmd = std::move(queue.front());
			queue.pop();
			SDL_UnlockMutex(queue_mutex);
			switch (cmd->type) {
			case Command::PLAY:
				if (current)
					stop_midi();
				current = std::move(cmd->playback);
				current->start_midi();
				SDL_SetAtomicInt(&current_loop, 0);
				SDL_SetAtomicInt(&current_mark, 0);
				SDL_Delay(100);  // ?
				break;
			case Command::STOP:
				if (current) {
					stop_midi();
					current.reset();
				}
				break;
			case Command::TERMINATE:
				return;
			}
			SDL_LockMutex(queue_mutex);
		}
		SDL_UnlockMutex(queue_mutex);
		if (current) {
			if (!current->play_midi(&current_loop, &current_mark)) {
				SDL_CompareAndSwapAtomicInt(&current_seq, current->seq(), 0);
				current.reset();
			}
		}
		SDL_Delay(10);
	}
}

// static
int MAKOMidi::thread_main(void* data)
{
	SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_HIGH);

	MAKOMidi* mm = reinterpret_cast<MAKOMidi*>(data);
	mm->thread_loop();

	stop_midi();

	return 0;
}
