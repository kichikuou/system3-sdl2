/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#include <SDL.h>
#include <SDL_mixer.h>

#include "mako.h"
#include "mako_midi.h"
#include "dri.h"

// MIX_INIT_FLUIDSYNTH was renamed to MIX_INIT_MID in SDL_mixer 2.0.2
#if (SDL_MIXER_MAJOR_VERSION == 2) && (SDL_MIXER_MINOR_VERSION == 0) && (SDL_MIXER_PATCHLEVEL < 2)
#define MIX_INIT_MID MIX_INIT_FLUIDSYNTH
#endif

MAKO::MAKO(NACT* parent) : nact(parent)
{
	// AMUS.DAT, AMSE.DAT
	_tcscpy_s(amus, 16, _T("AMUS.DAT"));
	_tcscpy_s(amse, 16, _T("AMSE.DAT"));	// ŽÀÛ‚É‚ÍŽg‚í‚È‚¢

	// Ä¶ó‹µ
	current_music = current_loop = 0;
	current_max = next_loop = 0;

	// CD-DA
	for(int i = 1; i <= 99; i++) {
		cd_track[i] = 0;
	}
	cdda_play = false;
	if (Mix_Init(MIX_INIT_MID) != MIX_INIT_MID) {
		WARNING("Mix_Init failed");
	}
	if (Mix_OpenAudio(44100, AUDIO_S16LSB, 2, 4096) < 0) {
		WARNING("Mix_OpenAudio failed: %s", Mix_GetError());
	}
}

MAKO::~MAKO()
{
	Mix_CloseAudio();
	Mix_Quit();

	// PCM’âŽ~
#if defined(_USE_PCM)
	PlaySound(NULL, NULL, SND_PURGE);
#endif
}

void MAKO::play_music(int page)
{
	// Šù‚É“¯‚¶‹È‚ðÄ¶‚µ‚Ä‚¢‚é
	if(current_music == page) {
		return;
	}

	stop_music();

	current_music = page;
	current_max = next_loop;

	if(page < 100 && cd_track[page]) {
		// CD-DA‚ÅÄ¶
		// TODO: fix
		// PostMessage(g_hwnd, WM_USER, cd_track[page] + 1, 0);
		cdda_play = true;
	} else {
		MAKOMidi midi(nact, amus);
		if (midi.load_mml(page)) {
			midi.load_mda(page);
			smf = midi.generate_smf(current_max);
			SDL_RWops *rwops = SDL_RWFromConstMem(smf.data(), smf.size());
			mix_music = Mix_LoadMUSType_RW(rwops, MUS_MID, SDL_TRUE /* freesrc */);
			if (!mix_music) {
				WARNING("Mix_LoadMUS failed: %s", Mix_GetError());
				return;
			}
			if (Mix_PlayMusic(mix_music, -1) != 0) {
				WARNING("Mix_PlayMusic failed: %s", Mix_GetError());
				return;
			}
		}
		cdda_play = false;
	}
}

void MAKO::stop_music()
{
	// Œ»ÝÄ¶’†‚Ìê‡‚Í’âŽ~‚·‚é
	if(current_music) {
		if(cdda_play) {
			// TODO: fix
			// PostMessage(g_hwnd, WM_USER, 0, 0);
		} else {
			if (mix_music) {
				Mix_FreeMusic(mix_music);
				mix_music = NULL;
				smf.clear();
			}
		}
	}
	cdda_play = false;
	current_music = current_loop = 0;
}

bool MAKO::check_music()
{
	// Ä¶’†‚Åtrue
	if (!current_music)
		return false;
	if (mix_music)
		return Mix_PlayingMusic();
	return true;
}

void MAKO::get_mark(int* mark, int* loop)
{
	// TODO: fix
	*mark = 0;
	*loop = cdda_play ? 0 : current_loop;
}

void MAKO::notify_mci(int status)
{
	if(status == -1) {
		// Ä¶‚ÉŽ¸”s
		current_music = current_loop = 0;
		return;
	}

	if(cdda_play) {
		current_loop++;
		if(current_loop < current_max || current_max == 0) {
			// TODO: fix
			// PostMessage(g_hwnd, WM_USER, cd_track[current_music] + 1, 0);
		} else {
			current_music = current_loop = 0;
			cdda_play = false;
		}
	}
}

void MAKO::play_pcm(int page, bool loop)
{
	assert(false);
#if defined(_USE_PCM)
	static char header[44] = {
		'R' , 'I' , 'F' , 'F' , 0x00, 0x00, 0x00, 0x00, 'W' , 'A' , 'V' , 'E' , 'f' , 'm' , 't' , ' ' ,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00,
		0x01, 0x00, 0x08, 0x00, 'd' , 'a' , 't' , 'a' , 0x00, 0x00, 0x00, 0x00
	};

	// Œ»ÝÄ¶’†‚ÌPCM‚ð’âŽ~
	PlaySound(NULL, NULL, SND_PURGE);

	uint8* buffer = NULL;
	int size;
	DRI* dri = new DRI();

	if((buffer = dri->load(_T("AWAV.DAT"), page, &size)) != NULL) {
		// WAVŒ`Ž® (Only You)
		memcpy(wav, buffer, (size < MAX_SAMPLES) ? size : MAX_SAMPLES);

		if(loop) {
			PlaySound(wav, NULL, SND_ASYNC | SND_MEMORY | SND_LOOP);
		} else {
			PlaySound(wav, NULL, SND_ASYNC | SND_MEMORY);
		}
		free(buffer);
	} else if((buffer = dri->load(_T("AMSE.DAT"), page, &size)) != NULL) {
		// AMSEŒ`Ž® (‰³—í‹L)
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
			PlaySound(wav, NULL, SND_ASYNC | SND_MEMORY | SND_LOOP);
		} else {
			PlaySound(wav, NULL, SND_ASYNC | SND_MEMORY);
		}
		free(buffer);
	}
#endif
}

void MAKO::stop_pcm()
{
#if defined(_USE_PCM)
	PlaySound(NULL, NULL, SND_PURGE);
#endif
}

bool MAKO::check_pcm()
{
	// Ä¶’†‚Åtrue
#if defined(_USE_PCM)
	static char null_wav[45] =  {
		'R' , 'I' , 'F' , 'F' , 0x25, 0x00, 0x00, 0x00, 'W' , 'A' , 'V' , 'E' , 'f' , 'm' , 't' , ' ' ,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00,
		0x01, 0x00, 0x08, 0x00, 'd' , 'a' , 't' , 'a' , 0x01, 0x00, 0x00, 0x00, 0x00
	};
	return PlaySound(null_wav, NULL, SND_ASYNC | SND_MEMORY | SND_NOSTOP) ? false : true;
#else
	return false;
#endif
}

