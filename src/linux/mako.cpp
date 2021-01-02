/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#include <memory>
#include <SDL.h>
#include <SDL_mixer.h>

#include "mako.h"
#include "mako_midi.h"
#include "fm/mako_fmgen.h"
#include "dri.h"

// MIX_INIT_FLUIDSYNTH was renamed to MIX_INIT_MID in SDL_mixer 2.0.2
#if (SDL_MIXER_MAJOR_VERSION == 2) && (SDL_MIXER_MINOR_VERSION == 0) && (SDL_MIXER_PATCHLEVEL < 2)
#define MIX_INIT_MID MIX_INIT_FLUIDSYNTH
#endif

namespace {

const int SAMPLE_RATE = 44100;

std::vector<uint8> smf;
Mix_Music *mix_music;
Mix_Chunk *mix_chunk;
SDL_mutex* fm_mutex;
std::unique_ptr<MakoFMgen> fm;

void FMHook(void *udata, Uint8 *stream, int len) {
	SDL_LockMutex(fm_mutex);
	fm->Process(reinterpret_cast<int16*>(stream), len / 4);
	SDL_UnlockMutex(fm_mutex);
}

} // namespace

MAKO::MAKO(NACT* parent, const MAKOConfig& config) :
	use_fm(config.use_fm),
	current_music(0),
	next_loop(0),
	nact(parent)
{
	int mix_init_flags = MIX_INIT_MID;

	if (config.playlist && load_playlist(config.playlist))
		mix_init_flags |= MIX_INIT_MP3 | MIX_INIT_OGG;

	strcpy_s(amus, 16, "AMUS.DAT");
	strcpy_s(amse, 16, "AMSE.DAT");	// 実際には使わない

	for(int i = 1; i <= 99; i++) {
		cd_track[i] = 0;
	}
	if (Mix_Init(mix_init_flags) != mix_init_flags)
		WARNING("Mix_Init(0x%x) failed", mix_init_flags);
	if (Mix_OpenAudio(SAMPLE_RATE, AUDIO_S16LSB, 2, 4096) < 0)
		WARNING("Mix_OpenAudio failed: %s", Mix_GetError());
}

MAKO::~MAKO()
{
	stop_pcm();
	Mix_CloseAudio();
	Mix_Quit();
}

bool MAKO::load_playlist(const char* path)
{
	FILE* fp = fopen(path, "rt");
	if (!fp) {
		WARNING("Cannot open %s", path);
		return false;
	}
	char buf[256];
	while (fgets(buf, sizeof(buf) - 1, fp)) {
		char *p = &buf[strlen(buf) - 1];
		if (*p == '\n')
			*p = '\0';
		playlist.push_back(buf[0] ? strdup(buf) : NULL);
	}
	fclose(fp);
	return true;
}

void MAKO::play_music(int page)
{
	if(current_music == page)
		return;

	stop_music();

	int track = page < 100 ? cd_track[page] : 0;
	if(track) {
		if (track < playlist.size() && playlist[track]) {
			mix_music = Mix_LoadMUS(playlist[track]);
			if (!mix_music) {
				WARNING("Mix_LoadMUS failed: %s: %s", playlist[track], Mix_GetError());
				return;
			}
			if (Mix_PlayMusic(mix_music, next_loop ? next_loop : -1) != 0) {
				WARNING("Mix_PlayMusic failed: %s", Mix_GetError());
				Mix_FreeMusic(mix_music);
				return;
			}
		}
	} else if (use_fm) {
		DRI dri;
		int size;
		uint8* data = dri.load(amus, page, &size);
		if (!data)
			return;
		if (!fm_mutex)
			fm_mutex = SDL_CreateMutex();

		SDL_LockMutex(fm_mutex);
		fm = std::make_unique<MakoFMgen>(SAMPLE_RATE, data, true);
		Mix_HookMusic(&FMHook, this);
		SDL_UnlockMutex(fm_mutex);
	} else {
		auto midi = std::make_unique<MAKOMidi>(nact, amus);
		if (midi->load_mml(page)) {
			midi->load_mda(page);
			smf = midi->generate_smf(next_loop);
			SDL_RWops *rwops = SDL_RWFromConstMem(smf.data(), smf.size());
			mix_music = Mix_LoadMUSType_RW(rwops, MUS_MID, SDL_TRUE /* freesrc */);
			if (!mix_music) {
				WARNING("Mix_LoadMUS failed: %s", Mix_GetError());
				return;
			}
			if (Mix_PlayMusic(mix_music, -1) != 0) {
				WARNING("Mix_PlayMusic failed: %s", Mix_GetError());
				Mix_FreeMusic(mix_music);
				return;
			}
		}
	}
	current_music = page;
}

void MAKO::stop_music()
{
	if (mix_music) {
		Mix_FreeMusic(mix_music);
		mix_music = NULL;
		smf.clear();
	}
	Mix_HookMusic(NULL, NULL);
	current_music = 0;
}

bool MAKO::check_music()
{
	return Mix_PlayingMusic();
}

void MAKO::get_mark(int* mark, int* loop)
{
	// TODO: fix
	*mark = 0;
	*loop = 0;
}

void MAKO::play_pcm(int page, bool loop)
{
	static char header[44] = {
		'R' , 'I' , 'F' , 'F' , 0x00, 0x00, 0x00, 0x00, 'W' , 'A' , 'V' , 'E' , 'f' , 'm' , 't' , ' ' ,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00,
		0x01, 0x00, 0x08, 0x00, 'd' , 'a' , 't' , 'a' , 0x00, 0x00, 0x00, 0x00
	};

	stop_pcm();

	uint8* buffer = NULL;
	int size;
	DRI* dri = new DRI();

	if((buffer = dri->load("AWAV.DAT", page, &size)) != NULL) {
		// WAV形式 (Only You)
		mix_chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(buffer, size), 1 /* freesrc */);
		free(buffer);
		Mix_PlayChannel(-1, mix_chunk, loop ? -1 : 0);
	} else if((buffer = dri->load("AMSE.DAT", page, &size)) != NULL) {
		// AMSE形式 (乙女戦記)
		int total = (size - 12) * 2 + 0x24;
		int samples = (size - 12) * 2;

		uint8* wav = (uint8*)malloc(total + 8);
		memcpy(wav, header, 44);
		wav[ 4] = (total >>  0) & 0xff;
		wav[ 5] = (total >>  8) & 0xff;
		wav[ 6] = (total >> 16) & 0xff;
		wav[ 7] = (total >> 24) & 0xff;
		wav[40] = (samples >>  0) & 0xff;
		wav[41] = (samples >>  8) & 0xff;
		wav[42] = (samples >> 16) & 0xff;
		wav[43] = (samples >> 24) & 0xff;
		for(int i = 12, p = 44; i < size; i++) {
			wav[p++] = buffer[i] & 0xf0;
			wav[p++] = (buffer[i] & 0x0f) << 4;
		}
		free(buffer);

		mix_chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(wav, total + 8), 1 /* freesrc */);
		free(wav);
		Mix_PlayChannel(-1, mix_chunk, loop ? -1 : 0);
	}
}

void MAKO::stop_pcm()
{
	Mix_HaltChannel(-1);
	if (mix_chunk) {
		Mix_FreeChunk(mix_chunk);
		mix_chunk = NULL;
	}
}

bool MAKO::check_pcm()
{
	// 再生中でtrue
	return Mix_Playing(-1) != 0;
}

