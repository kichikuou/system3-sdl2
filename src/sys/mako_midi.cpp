/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO - midi ]
*/

#include <windows.h>
#include <mmsystem.h>
#include "mako.h"
#include "dri.h"

#define next_mml(p) mml[p].data[mml[p].addr++]

// MIDI音源
union UNION_MIDI_DATA {
	DWORD msg;
	BYTE data[4];
};
static UNION_MIDI_DATA midi;
static HMIDIOUT hMidi;

#define send_2bytes(d1, d2) { midi.data[0] = (BYTE)d1; midi.data[1] = (BYTE)d2; midiOutShortMsg(hMidi,midi.msg); }
#define send_3bytes(d1, d2, d3) { midi.data[0] = (BYTE)d1; midi.data[1] = (BYTE)d2; midi.data[2] = (BYTE)d3 ;midiOutShortMsg(hMidi, midi.msg); }

void MAKO::initialize_midi()
{
	// MIDI音源の初期化
	midiOutOpen(&hMidi, MIDI_MAPPER, NULL, NULL, CALLBACK_NULL);
}

void MAKO::release_midi()
{
	// MIDI音源の開放
	midiOutClose(hMidi);
}

void MAKO::stop_midi()
{
	for(int i = 0; i < 10; i++) {
		send_3bytes(0xb0 + i, 0x78, 0x00);
	}
	for(int i = 0; i < 10; i++) {
		send_3bytes(0xb0 + i, 0x79, 0x00);
	}
}

void MAKO::start_midi()
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
		mml[i].addr = 0;
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

	// MIDI音源のGS-RESET
	BYTE gs_reset[11] = {0xf0, 0x41, 0x20, 0x42, 0x12, 0x40, 0x00, 0x7f, 0x00, 0x41, 0xf7};
	MIDIHDR mhMidi;
	ZeroMemory(&mhMidi, sizeof(mhMidi));
	mhMidi.lpData = (LPSTR)gs_reset;
	mhMidi.dwBufferLength = 11;
	mhMidi.dwBytesRecorded = 11;

	midiOutPrepareHeader(hMidi, &mhMidi, sizeof(mhMidi));
	midiOutLongMsg(hMidi, &mhMidi, sizeof(mhMidi));
	while(!(mhMidi.dwFlags & MHDR_DONE)) {
		SDL_Delay(10);
	}
	midiOutUnprepareHeader(hMidi, &mhMidi, sizeof(mhMidi));

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

void MAKO::play_midi()
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
				int d0 = next_mml(i), d1, d2, d3;
				if(d0 == 0xff) {
					d1 = next_mml(i);
					d2 = next_mml(i);
					d3 = next_mml(i);
					mml[i].addr = d1 | (d2 << 8) | (d3 << 16);
					if(!play[i].note_flag || mute_flag) {
						// 再生停止または未使用
						play[i].loop_flag = false;
						if(!play[0].loop_flag && !play[1].loop_flag && !play[2].loop_flag &&
						   !play[3].loop_flag && !play[4].loop_flag && !play[5].loop_flag &&
						   !play[6].loop_flag && !play[7].loop_flag && !play[8].loop_flag) {
							// 全チャンネルが再生停止 (ループしない曲)
							for(int j = 0; j < 10; j++) {
								send_3bytes(0xb0 + j, 0x78, 0x00);
							}
							for(int j = 0; j < 10; j++) {
								send_3bytes(0xb0 + j, 0x79, 0x00);
							}
							current_loop = 1;
							current_music = 0;
							params.current_music = params.next_music = 0;
							return;
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
						current_loop = loop;
						if(current_loop >= current_max && current_max) {
							// 指定回数だけ再生完了
							for(int j = 0; j < 10; j++) {
								send_3bytes(0xb0 + j, 0x78, 0x00);
							}
							for(int j = 0; j < 10; j++) {
								send_3bytes(0xb0 + j, 0x79, 0x00);
							}
							current_music = 0;
							params.current_music = params.next_music = 0;
							return;
						}
					}
				} else if(d0 < 0x80) {
					int note = d0;
					play[i].wait_time = next_mml(i);
					play[i].wait_time |= next_mml(i) << 8;
					if(play[i].timbre_type != 128) {
						note = drum_map[play[i].timbre_type][note];
					} else {
						note = (note + play[i].key_shift) - 64;
					}
					play[i].note_flag = true;
					play[i].note = note;
					send_3bytes(0x90 + play[i].channel, note, play[i].velocity);
				} else if(d0 == 0x80) {
					play[i].wait_time = next_mml(i);
					play[i].wait_time |= next_mml(i) << 8;
					if(play[i].note != 128) {
						send_3bytes(0x90 + play[i].channel, play[i].note, 0x00);
					}
					play[i].note = 128;
				} else if(d0 == 0xe0) {
					current_mark = next_mml(i);
				} else if(d0 == 0xe1) {
					d1 = next_mml(i);
					play[i].velocity += (d1 > 127) ? (d1 - 256) : d1;
				} else if(d0 == 0xeb) {
					play[i].pan = next_mml(i);
					send_3bytes(0xb0 + play[i].channel, 0x0a, play[i].pan);
				}
#if !defined(_DPS)
				else if(d0 == 0xec) {
					mute_flag = true;
				}
#endif
				else if(d0 == 0xf5) {
					int n = next_mml(i);
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
					play[i].velocity = next_mml(i);
				} else if(d0 == 0xfc) {
					d1 = next_mml(i);
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
}

bool MAKO::load_mml(int page)
{
	// データ読み込み
	DRI* dri = new DRI();
	int size;
	uint8* data = dri->load(amus, page, &size);
	if(data == NULL) {
		delete dri;
		return false;
	}

	// FM音源データの判定
	int p, d0, d1, d2;

	p = data[0] + 1;
	if(data[p] != 0x0f || (data[p + 1] + data[p + 2] + data[p + 3] + data[p + 4] + data[p + 5]) != 0) {
		free(data);
		delete dri;
		return false;
	}

	// MML部 読み込み＆変換
	for(int i = 0; i < 9; i++) {
		p = 4 * (i + 1);
		int block_offset_top = data[p++];
		block_offset_top |= data[p++] << 8;
		int body_addr = 0;
		mml[i].addr = 0;

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
					body_addr = mml[i].addr;
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
							next_mml(i) = (note - 1) + 12 * (current_octave + 1);
							next_mml(i) = on_time & 0xff;
							next_mml(i) = on_time >> 8;
						}
						next_mml(i) = 0x80;
						next_mml(i) = off_time & 0xff;
						next_mml(i) = off_time >> 8;
						if(current_octave != base_octave) {
							current_octave = base_octave;
						}
					} else if(d0 == 0xe0 || d0 == 0xe1 || d0 == 0xeb || d0 == 0xf5 || d0 == 0xf9 || d0 == 0xfc) {
						next_mml(i) = d0;
						next_mml(i) = data[p++];
					} else if(d0 == 0xea) {
						gate_time = data[p++];
					} else if(d0 == 0xec) {
						next_mml(i) = d0;
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
		next_mml(i) = 0xff;
		next_mml(i) = (body_addr >>  0) & 0xff;
		next_mml(i) = (body_addr >>  8) & 0xff;
		next_mml(i) = (body_addr >> 16) & 0xff;
	}
	free(data);
	delete dri;
	return true;
}

void MAKO::load_mda(int page)
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

	// MDAデータの読み込み
	_TCHAR path[16];
	_tcscpy_s(path, 16, amus);
	int p = _tcslen(path);
	path[p - 3] = _T('M');
	path[p - 2] = _T('D');
	path[p - 1] = _T('A');

	DRI* dri = new DRI();
	int size;
	uint8* data = NULL;
	if((data = dri->load(path, page, &size)) == NULL) {
		data = dri->load_mda(nact->crc32, page, &size);
	}

	if(data) {
		tempo_dif = data[7];
		int map_wide = data[24];
		int inst_num = data[25];
		int drum_format = data[26];

		// SSGパートの音色設定
		for(int i = 0; i < 3; i++) {
			uint8* buf = &data[map_wide * i + 27];
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
			uint8* buf = &data[27 + map_wide * 3 + (map_wide + 1) * i];
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
			uint8* buf = &data[27 + map_wide * 3 + (map_wide + 1) * inst_num];
			int drum_num = buf[0];
			for(int i = 0; i < drum_num; i++) {
				int d = buf[i * 3 + 1];
				int t = buf[i * 3 + 2];
				drum_map[d][t + 12] = buf[i * 3 + 3];
			}
		}
		free(data);
	}
	delete dri;
}

