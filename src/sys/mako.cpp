/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#include "mako.h"
#include "dri.h"

extern HWND g_hwnd;

MAKO::MAKO(NACT *parent) : nact(parent)
{
	// AMUS.DAT, AMSE.DAT
	strcpy_s(amus, 16, "AMUS.DAT");
	strcpy_s(amse, 16, "AMSE.DAT");	// 実際には使わない

	// 再生状況
	current_music = current_mark = current_loop = 0;
	current_max = next_loop = 0;

	// CD-DA
	for(int i = 1; i <= 99; i++) {
		cd_track[i] = 0;
	}
	cdda_play = false;

	// MIDI再生スレッド起動
	params.mako = this;
	params.current_music = params.next_music = 0;
	params.request_fill = params.terminate = false;
	hThread = (HANDLE)_beginthread(thread, 0, &params);
	SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
}

MAKO::~MAKO()
{
	// MIDI再生スレッド終了
	params.terminate = true;
	if(hThread) {
		WaitForSingleObject(hThread, INFINITE);
	}
	hThread = NULL;

	// PCM停止
#if defined(_USE_PCM)
	PlaySound(NULL, NULL, SND_PURGE);
#endif
}

void MAKO::thread(PVOID pvoid)
{
	volatile PPARAMS pparams;
	pparams = (PPARAMS)pvoid;

	// MIDI初期化
	pparams->mako->initialize_midi();

	while(!pparams->terminate) {
		int current_music = pparams->current_music;
		int next_music = pparams->next_music;
		int next_music_bak = next_music;
		if(current_music != next_music) {
			// 停止
			pparams->mako->stop_midi();
			current_music = 0;

			// 次の再生準備
			if(next_music) {
				if(pparams->mako->load_mml(next_music)) {
					pparams->mako->load_mda(next_music);
					pparams->mako->start_midi();
					current_music = next_music;
					Sleep(100);
				} else {
					next_music = 0;
				}
			}
		}
		if(current_music) {
			pparams->mako->play_midi();
		}

		pparams->current_music = current_music;
		if(pparams->next_music == next_music_bak) {
			pparams->next_music = next_music;
		}
		Sleep(10);
	}

	// MIDI開放
	pparams->mako->stop_midi();
	pparams->mako->release_midi();

	_endthread();
}

void MAKO::play_music(int page)
{
	// 既に同じ曲を再生している
	if(current_music == page) {
		return;
	}

	// 演奏するデバイスが変更される場合は停止する
	if(current_music) {
		bool next_cdda = (page < 100 && cd_track[page]) ? true : false;
		if(cdda_play && !next_cdda) {
			PostMessage(g_hwnd, WM_USER, 0, 0);
		} else if(!cdda_play && next_cdda) {
			params.next_music = 0;
		}
	}

	current_music = page;
	current_max = next_loop;
	current_mark = current_loop = next_loop = 0;

	if(page < 100 && cd_track[page]) {
		// CD-DAで再生
		PostMessage(g_hwnd, WM_USER, cd_track[page] + 1, 0);
		cdda_play = true;
	} else {
		// MIDIで再生
		params.next_music = page;
		cdda_play = false;
	}
}

void MAKO::stop_music()
{
	// 現在再生中の場合は停止する
	if(current_music) {
		if(cdda_play) {
			PostMessage(g_hwnd, WM_USER, 0, 0);
		} else {
			params.next_music = 0;
		}
	}
	cdda_play = false;
	current_music = current_mark = current_loop = 0;
}

bool MAKO::check_music()
{
	// 再生中でtrue
	return current_music ? true : false;
}

void MAKO::get_mark(int *mark, int *loop)
{
	*mark = cdda_play ? 0 : current_mark;
	*loop = cdda_play ? 0 : current_loop;
}

void MAKO::notify_mci(int status)
{
	if(status == -1) {
		// 再生に失敗
		current_music = current_mark = current_loop = 0;
		return;
	}

	if(cdda_play) {
		current_loop++;
		if(current_loop < current_max || current_max == 0) {
			PostMessage(g_hwnd, WM_USER, cd_track[current_music] + 1, 0);
		} else {
			current_music = current_mark = current_loop = 0;
			cdda_play = false;
		}
	}
}

void MAKO::play_pcm(int page, bool loop)
{
#if defined(_USE_PCM)
	static char header[44] = {
		'R' , 'I' , 'F' , 'F' , 0x00, 0x00, 0x00, 0x00, 'W' , 'A' , 'V' , 'E' , 'f' , 'm' , 't' , ' ' ,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00,
		0x01, 0x00, 0x08, 0x00, 'd' , 'a' , 't' , 'a' , 0x00, 0x00, 0x00, 0x00
	};

	// 現在再生中のPCMを停止
	PlaySound(NULL, NULL, SND_PURGE);

	uint8 *buffer = NULL;
	int size;
	DRI *dri = new DRI();

	if((buffer = dri->load("AWAV.DAT", page, &size)) != NULL) {
		// WAV形式 (Only You)
		memcpy(wav, buffer, (size < MAX_SAMPLES) ? size : MAX_SAMPLES);

		if(loop) {
			PlaySoundA(wav, NULL, SND_ASYNC | SND_MEMORY | SND_LOOP);
		} else {
			PlaySoundA(wav, NULL, SND_ASYNC | SND_MEMORY);
		}
		free(buffer);
	} else if((buffer = dri->load("AMSE.DAT", page, &size)) != NULL) {
		// AMSE形式 (乙女戦記)
		int total = (size - 12) * 2 + 0x24;
		int samples = (size - 12) * 2;

		memcpy(wav, header, 44);
		wav[ 4] = (total >>  0) & 0xff;
		wav[ 5] = (total >>  8) & 0xff;
		wav[ 6] = (total >> 16) & 0xff;
		wav[ 7] = (total >> 24) & 0xff;
		wav[40] = (samples >>  0) & 0xff;
		wav[41] = (samples >>  8) & 0xff;
		wav[42] = (samples >> 16) & 0xff;
		wav[43] = (samples >> 24) & 0xff;
		for(int i = 12, p = 44; i < size && p < MAX_SAMPLES - 2; i++) {
			wav[p++] = buffer[i] & 0xf0;
			wav[p++] = (buffer[i] & 0x0f) << 4;
		}

		if(loop) {
			PlaySoundA(wav, NULL, SND_ASYNC | SND_MEMORY | SND_LOOP);
		} else {
			PlaySoundA(wav, NULL, SND_ASYNC | SND_MEMORY);
		}
		free(buffer);
	}
#endif
}

void MAKO::stop_pcm()
{
#if defined(_USE_PCM)
	PlaySoundA(NULL, NULL, SND_PURGE);
#endif
}

bool MAKO::check_pcm()
{
	// 再生中でtrue
#if defined(_USE_PCM)
	static char null_wav[45] =  {
		'R' , 'I' , 'F' , 'F' , 0x25, 0x00, 0x00, 0x00, 'W' , 'A' , 'V' , 'E' , 'f' , 'm' , 't' , ' ' ,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00,
		0x01, 0x00, 0x08, 0x00, 'd' , 'a' , 't' , 'a' , 0x01, 0x00, 0x00, 0x00, 0x00
	};
	return PlaySoundA(null_wav, NULL, SND_ASYNC | SND_MEMORY | SND_NOSTOP) ? false : true;
#else
	return false;
#endif
}

