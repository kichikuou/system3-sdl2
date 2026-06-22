/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#include <memory>
#include <string>
#include <vector>
#include <limits.h>
#include <SDL.h>

#include "mako.h"
#include "mako_midi.h"
#include "mako_music.h"
#include "fm/mako_ymfm.h"
#include "config.h"
#include "dri.h"
#include "game_id.h"

namespace {

const int SAMPLE_RATE = 44100;

#ifdef _WIN32
// Per-game mapping from music numbers to CD tracks.  This is necessary to
// forcibly change the sound device with a menu command.
//
// When the game uses "Z 100+x,y" command, the xth element of the array should
// be y.  The array must be terminated with -1.
const int8_t RANCE41_tracks[] = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,1,-1};
const int8_t RANCE42_tracks[] = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,1,-1};
const int8_t DPSALL_tracks[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,1,2,3,-1};
#endif

SDL_AudioDeviceID g_device;
SDL_AudioSpec g_device_spec;

std::unique_ptr<MakoMusic> music;
std::unique_ptr<MakoYmfm> fm;
std::unique_ptr<MAKOMidi> midi;

// PCM (sound effect) playback.  pcm_loops is the number of remaining plays,
// or -1 for an infinite loop.
SDL_AudioStream* pcm_stream;
std::vector<uint8_t> pcm_src;
int pcm_loops;
bool pcm_input_finished;

void mix_pcm(Uint8* out, int len)
{
	while (SDL_AudioStreamAvailable(pcm_stream) < len && !pcm_input_finished) {
		if (pcm_loops == 0) {
			SDL_AudioStreamFlush(pcm_stream);
			pcm_input_finished = true;
			break;
		}
		SDL_AudioStreamPut(pcm_stream, pcm_src.data(), static_cast<int>(pcm_src.size()));
		if (pcm_loops > 0)
			pcm_loops--;
	}
	Uint8* tmp = SDL_stack_alloc(Uint8, len);
	int got = SDL_AudioStreamGet(pcm_stream, tmp, len);
	if (got > 0)
		SDL_MixAudioFormat(out, tmp, AUDIO_S16SYS, got, SDL_MIX_MAXVOLUME);
	SDL_stack_free(tmp);
}

void audio_callback(void*, Uint8* stream, int len)
{
	SDL_memset(stream, 0, len);
	if (fm) {
		int frames = len / 4;
		int16_t* tmp = SDL_stack_alloc(int16_t, frames * 2);
		fm->Process(tmp, frames);
		SDL_MixAudioFormat(stream, reinterpret_cast<Uint8*>(tmp), AUDIO_S16SYS, len, SDL_MIX_MAXVOLUME);
		SDL_stack_free(tmp);
	}
	if (music)
		music->mix(stream, len);
	if (pcm_stream)
		mix_pcm(stream, len);
}

} // namespace

MAKO::MAKO(const Config& config, const GameId& game_id) :
	use_fm(config.use_fm),
	current_music(0),
	next_loop(0),
	game_id(game_id)
{
	if (!config.playlist.empty())
		load_playlist(config.playlist.c_str());

#ifdef _WIN32
	if (!is_cd_available()) {
		SDL_Event event = {};
		event.user.type = sdl_custom_event_type;
		event.user.code = DISABLE_CD_MENU;
		SDL_PushEvent(&event);
	}
#endif

	amus.open("AMUS.DAT");
	awav.open("AWAV.DAT");
	amse.open("AMSE.DAT");
	mda.open("AMUS.MDA");
	for (int i = 1; i <= 99; i++)
		cd_track[i] = 0;

	SDL_InitSubSystem(SDL_INIT_AUDIO);
	SDL_AudioSpec want;
	SDL_zero(want);
	want.freq = SAMPLE_RATE;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = 4096;
	want.callback = &audio_callback;
	g_device = SDL_OpenAudioDevice(nullptr, 0, &want, &g_device_spec, 0);
	if (!g_device) {
		WARNING("SDL_OpenAudioDevice failed: %s", SDL_GetError());
		use_fm = false;
	} else {
		SDL_PauseAudioDevice(g_device, 0);
	}

	midi = std::make_unique<MAKOMidi>(config.midi_device);
	if (!midi->is_available())
		use_fm = true;
}

MAKO::~MAKO()
{
	stop_music();
	stop_pcm();
	midi.reset();

	if (g_device)
		SDL_CloseAudioDevice(g_device);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
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

	size_t track = page < 100 ? cd_track[page] : 0;
	if (track && is_cd_available()) {
		if (track >= playlist.size() || !playlist[track])
			return;
		const char* file = playlist[track];
#ifdef __ANDROID__
		// dr_mp3/stb_vorbis open the file via fopen, which requires an absolute
		// path on Android.
		char abspath[PATH_MAX];
		if (!realpath(file, abspath))
			return;
		file = abspath;
#endif
		auto m = std::make_unique<MakoMusic>(file, next_loop, g_device_spec);
		if (!m->is_open())
			return;
		SDL_LockAudioDevice(g_device);
		music = std::move(m);
		SDL_UnlockAudioDevice(g_device);
	} else if (use_fm) {
		std::vector<uint8_t> data = amus.load(page);
		if (data.empty())
			return;
		auto f = std::make_unique<MakoYmfm>(SAMPLE_RATE, std::move(data));
		SDL_LockAudioDevice(g_device);
		fm = std::move(f);
		SDL_UnlockAudioDevice(g_device);
	} else if (midi->is_available()) {
		if (!midi->play(game_id, amus, mda, page, next_loop))
			return;
	}
	current_music = page;
	next_loop = 0;
}

void MAKO::stop_music()
{
	if (music || fm) {
		SDL_LockAudioDevice(g_device);
		std::unique_ptr<MakoMusic> old_music = std::move(music);
		std::unique_ptr<MakoYmfm> old_fm = std::move(fm);
		SDL_UnlockAudioDevice(g_device);
		// old_music/old_fm are destroyed here, outside the lock.
	}
	if (midi->is_available())
		midi->stop();
	current_music = 0;
}

bool MAKO::check_music()
{
	SDL_LockAudioDevice(g_device);
	if (fm) {
		int mark, loop;
		fm->get_mark(&mark, &loop);
		SDL_UnlockAudioDevice(g_device);
		return !loop;
	}
	if (music) {
		bool playing = music->is_playing();
		SDL_UnlockAudioDevice(g_device);
		return playing;
	}
	SDL_UnlockAudioDevice(g_device);
	return midi->is_playing();
}

#ifdef _WIN32
void MAKO::select_sound(BGMDevice dev)
{
	// 強制的に音源を変更する
	int page = current_music;
	int old_dev = (1 <= page && page <= 99 && cd_track[page]) ? BGM_CD :
		use_fm ? BGM_FM : BGM_MIDI;

	switch (dev) {
	case BGM_FM:
	case BGM_MIDI:
		for (int i = 1; i <= 99; i++)
			cd_track[i] = 0;
		if (midi->is_available())
			use_fm = dev == BGM_FM;
		else
			dev = BGM_FM;
		break;

	case BGM_CD:
		const int8_t* tracks;

		switch (game_id.game) {
		case GameId::RANCE41:
			tracks = RANCE41_tracks;
			break;
		case GameId::RANCE42:
			tracks = RANCE42_tracks;
			break;
		case GameId::DPS_ALL:
			tracks = DPSALL_tracks;
			break;

		// For the following games, the default mapping (cd_track[i] = i) works.
		case GameId::AYUMI_CD:
		case GameId::FUNNYBEE_CD:
		case GameId::ONLYYOU:
		default:
			tracks = nullptr;
		}

		if (tracks) {
			for (int i = 0; tracks[i] >= 0; i++)
				cd_track[i + 1] = tracks[i];
		} else {
			for (int i = 1; i <= 99; i++)
				cd_track[i] = i;
		}
		break;
	}

	// デバイスが変更された場合は再演奏する
	if (dev != old_dev && page) {
		stop_music();
		play_music(page);
	}
}
#endif

void MAKO::get_mark(int* mark, int* loop)
{
	SDL_LockAudioDevice(g_device);
	if (fm) {
		fm->get_mark(mark, loop);
		SDL_UnlockAudioDevice(g_device);
		return;
	}
	SDL_UnlockAudioDevice(g_device);
	midi->get_mark(mark, loop);
}

void MAKO::play_pcm(int page, int loops)
{
	stop_pcm();

	SDL_AudioStream* stream = nullptr;
	std::vector<uint8_t> src;

	// WAV形式 (Only You)
	std::vector<uint8_t> data = awav.load(page);
	if (!data.empty()) {
		SDL_AudioSpec spec;
		Uint8* wav;
		Uint32 wavlen;
		if (!SDL_LoadWAV_RW(SDL_RWFromConstMem(data.data(), static_cast<int>(data.size())), 1, &spec, &wav, &wavlen)) {
			WARNING("SDL_LoadWAV_RW failed: %s", SDL_GetError());
			return;
		}
		src.assign(wav, wav + wavlen);
		SDL_FreeWAV(wav);
		stream = SDL_NewAudioStream(spec.format, spec.channels, spec.freq,
								   g_device_spec.format, g_device_spec.channels, g_device_spec.freq);
	} else {
		// AMSE形式 (乙女戦記)
		data = amse.load(page);
		if (data.empty())
			return;
		uint32_t amse_size = SDL_SwapLE32(*reinterpret_cast<uint32_t*>(&data[8]));
		// 4-bit PCM -> 8-bit PCM, mono, 8000Hz
		for (uint32_t i = 12; i < amse_size; i++) {
			src.push_back(data[i] & 0xf0);
			src.push_back((data[i] & 0x0f) << 4);
		}
		stream = SDL_NewAudioStream(AUDIO_U8, 1, 8000,
								   g_device_spec.format, g_device_spec.channels, g_device_spec.freq);
	}
	if (!stream) {
		WARNING("SDL_NewAudioStream failed: %s", SDL_GetError());
		return;
	}
	SDL_LockAudioDevice(g_device);
	pcm_stream = stream;
	pcm_src = std::move(src);
	pcm_loops = loops ? loops : -1;
	pcm_input_finished = false;
	SDL_UnlockAudioDevice(g_device);
}

void MAKO::stop_pcm()
{
	SDL_LockAudioDevice(g_device);
	SDL_AudioStream* old = pcm_stream;
	pcm_stream = nullptr;
	pcm_src.clear();
	pcm_input_finished = false;
	SDL_UnlockAudioDevice(g_device);
	if (old)
		SDL_FreeAudioStream(old);
}

bool MAKO::check_pcm()
{
	// 再生中でtrue
	SDL_LockAudioDevice(g_device);
	bool playing = pcm_stream &&
		(!pcm_input_finished || SDL_AudioStreamAvailable(pcm_stream) > 0);
	SDL_UnlockAudioDevice(g_device);
	return playing;
}

bool MAKO::is_cd_available() const
{
	return !playlist.empty();
}
