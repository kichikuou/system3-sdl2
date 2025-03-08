/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#include <memory>
#include <SDL3/SDL.h>
#include <SDL_mixer.h>

#include "mako.h"
#include "mako_midi.h"
#include "fm/mako_ymfm.h"
#include "config.h"
#include "dri.h"

namespace {

const int SAMPLE_RATE = 44100;

Mix_Music *mix_music;
Mix_Chunk *mix_chunk;
SDL_Mutex* fm_mutex;
std::unique_ptr<MakoYmfm> fm;
std::unique_ptr<MAKOMidi> midi;

void FMHook(void*, Uint8* stream, int len) {
	SDL_LockMutex(fm_mutex);
	fm->Process(reinterpret_cast<int16_t*>(stream), len / 4);
	SDL_UnlockMutex(fm_mutex);
}

} // namespace

MAKO::MAKO(const Config& config, const GameId& game_id) :
	use_fm(config.use_fm),
	current_music(0),
	next_loop(0),
	game_id(game_id)
{
	int mix_init_flags = 0;

	if (!config.playlist.empty() && load_playlist(config.playlist.c_str()))
		mix_init_flags |= MIX_INIT_MP3 | MIX_INIT_OGG;

	amus.open("AMUS.DAT");
	awav.open("AWAV.DAT");
	amse.open("AMSE.DAT");
	mda.open("AMUS.MDA");
	for (int i = 1; i <= 99; i++)
		cd_track[i] = 0;

	if (Mix_Init(mix_init_flags) != mix_init_flags)
		WARNING("Mix_Init(0x%x) failed", mix_init_flags);
	if (Mix_OpenAudio(SAMPLE_RATE, AUDIO_S16LSB, 2, 4096) < 0)
		WARNING("Mix_OpenAudio failed: %s", Mix_GetError());

	midi = std::make_unique<MAKOMidi>(config.midi_device);
	if (!midi->is_available())
		use_fm = true;
}

MAKO::~MAKO()
{
	midi.reset();
	stop_pcm();
	Mix_CloseAudio();
	Mix_Quit();
}

bool MAKO::load_playlist(const char* path)
{
	FILE* fp = fopen(path, "r");
	if (!fp) {
		WARNING("Cannot open %s", path);
		return false;
	}
	char buf[256];
	while (fgets(buf, sizeof(buf) - 1, fp)) {
		for (char *p = buf; *p; p++) {
			if (*p == '\\')
				*p = '/';
			else if (*p == '\r' || *p == '\n')
				*p = '\0';
		}
		playlist.push_back(buf[0] ? strdup(buf) : NULL);
	}
	fclose(fp);
	return true;
}

void MAKO::play_music(int page)
{
	if (current_music == page)
		return;

	stop_music();

	int track = page < 100 ? cd_track[page] : 0;
	if (track) {
		if (track < playlist.size() && playlist[track]) {
#ifdef __ANDROID__
			// Mix_LoadMUS uses SDL_RWFromFile which requires absolute path on Android
			char path[PATH_MAX];
			if (!realpath(playlist[track], path))
				return;
			mix_music = Mix_LoadMUS(path);
#else
			mix_music = Mix_LoadMUS(playlist[track]);
#endif
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
		std::vector<uint8_t> data = amus.load(page);
		if (data.empty())
			return;
		if (!fm_mutex)
			fm_mutex = SDL_CreateMutex();

		SDL_LockMutex(fm_mutex);
		fm = std::make_unique<MakoYmfm>(SAMPLE_RATE, std::move(data));
		SDL_UnlockMutex(fm_mutex);
		Mix_HookMusic(&FMHook, this);
	} else if (midi->is_available()) {
		if (!midi->play(game_id, amus, mda, page, next_loop))
			return;
	}

	current_music = page;
	next_loop = 0;
}

void MAKO::stop_music()
{
	if (!current_music)
		return;

	if (mix_music) {
		Mix_FreeMusic(mix_music);
		mix_music = NULL;
	}
	if (fm) {
		Mix_HookMusic(NULL, NULL);
		SDL_LockMutex(fm_mutex);
		fm = nullptr;
		SDL_UnlockMutex(fm_mutex);
	}
	if (midi->is_available()) {
		midi->stop();
	}
	current_music = 0;
}

bool MAKO::check_music()
{
	if (mix_music)
		return Mix_PlayingMusic();
	if (fm) {
		int mark, loop;
		SDL_LockMutex(fm_mutex);
		fm->get_mark(&mark, &loop);
		SDL_UnlockMutex(fm_mutex);
		return !loop;
	}
	return midi->is_playing();
}

void MAKO::get_mark(int* mark, int* loop)
{
	SDL_LockMutex(fm_mutex);
	if (fm) {
		fm->get_mark(mark, loop);
		SDL_UnlockMutex(fm_mutex);
		return;
	}
	SDL_UnlockMutex(fm_mutex);
	midi->get_mark(mark, loop);
}

void MAKO::play_pcm(int page, int loops)
{
	static char header[44] = {
		'R' , 'I' , 'F' , 'F' , 0x00, 0x00, 0x00, 0x00, 'W' , 'A' , 'V' , 'E' , 'f' , 'm' , 't' , ' ' ,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00,
		0x01, 0x00, 0x08, 0x00, 'd' , 'a' , 't' , 'a' , 0x00, 0x00, 0x00, 0x00
	};

	stop_pcm();

	std::vector<uint8_t> buffer = awav.load(page);
	if (!buffer.empty()) {
		// WAV形式 (Only You)
		mix_chunk = Mix_LoadWAV_RW(SDL_IOFromConstMem(buffer.data(), buffer.size()), 1 /* freesrc */);
		Mix_PlayChannel(-1, mix_chunk, loops ? loops : -1);
		return;
	}
	buffer = amse.load(page);
	if (!buffer.empty()) {
		// AMSE形式 (乙女戦記)
		uint32_t amse_size = SDL_Swap32LE(*reinterpret_cast<uint32_t*>(&buffer[8]));
		int samples = (amse_size - 12) * 2;
		int total = samples + 0x24;

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
		for (uint32_t i = 12, p = 44; i < amse_size; i++) {
			wav[p++] = buffer[i] & 0xf0;
			wav[p++] = (buffer[i] & 0x0f) << 4;
		}

		mix_chunk = Mix_LoadWAV_RW(SDL_IOFromConstMem(wav, total + 8), 1 /* freesrc */);
		free(wav);
		Mix_PlayChannel(-1, mix_chunk, loops ? loops : -1);
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
	return Mix_Playing(-1) != 0;
}

