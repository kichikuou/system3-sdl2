/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#include <memory>
#include <string>
#include <vector>
#include <limits.h>
#include <SDL3/SDL.h>

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

// The main output stream, opened together with the audio device via
// SDL_OpenAudioDeviceStream().  Its callback mixes fm/music/pcm together.
SDL_AudioStream* g_stream;
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
	while (SDL_GetAudioStreamAvailable(pcm_stream) < len && !pcm_input_finished) {
		if (pcm_loops == 0) {
			SDL_FlushAudioStream(pcm_stream);
			pcm_input_finished = true;
			break;
		}
		SDL_PutAudioStreamData(pcm_stream, pcm_src.data(), static_cast<int>(pcm_src.size()));
		if (pcm_loops > 0)
			pcm_loops--;
	}
	Uint8* tmp = SDL_stack_alloc(Uint8, len);
	int got = SDL_GetAudioStreamData(pcm_stream, tmp, len);
	if (got > 0)
		SDL_MixAudio(out, tmp, SDL_AUDIO_S16, got, 1.0f);
	SDL_stack_free(tmp);
}

// SDL3 audio callback: rather than handing us a buffer to fill, SDL asks us to
// feed `additional_amount` bytes into the stream via SDL_PutAudioStreamData().
void SDLCALL audio_callback(void*, SDL_AudioStream* stream, int additional_amount, int /*total_amount*/)
{
	// Mix in bounded chunks so the scratch buffers (and the per-source
	// SDL_stack_alloc() below) stay small no matter how much SDL requests.
	const int CHUNK = 4096;  // bytes; 1024 stereo S16 frames
	Uint8 buffer[CHUNK];
	while (additional_amount > 0) {
		int len = additional_amount < CHUNK ? additional_amount : CHUNK;
		SDL_memset(buffer, 0, len);
		if (fm) {
			int frames = len / 4;
			int16_t* tmp = SDL_stack_alloc(int16_t, frames * 2);
			fm->Process(tmp, frames);
			SDL_MixAudio(buffer, reinterpret_cast<Uint8*>(tmp), SDL_AUDIO_S16, len, 1.0f);
			SDL_stack_free(tmp);
		}
		if (music)
			music->mix(buffer, len);
		if (pcm_stream)
			mix_pcm(buffer, len);
		SDL_PutAudioStreamData(stream, buffer, len);
		additional_amount -= len;
	}
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
	// g_device_spec is the format our callback produces; the resampling streams
	// used by MakoMusic/MakoYmfm/pcm convert their sources into this format.
	SDL_zero(g_device_spec);
	g_device_spec.freq = SAMPLE_RATE;
	g_device_spec.format = SDL_AUDIO_S16;
	g_device_spec.channels = 2;
	g_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &g_device_spec, audio_callback, nullptr);
	if (!g_stream) {
		WARNING("SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
		use_fm = false;
	} else {
		SDL_ResumeAudioStreamDevice(g_stream);
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

	if (g_stream)
		SDL_DestroyAudioStream(g_stream);  // also closes the audio device
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
		SDL_LockAudioStream(g_stream);
		music = std::move(m);
		SDL_UnlockAudioStream(g_stream);
	} else if (use_fm) {
		std::vector<uint8_t> data = amus.load(page);
		if (data.empty())
			return;
		auto f = std::make_unique<MakoYmfm>(SAMPLE_RATE, std::move(data));
		SDL_LockAudioStream(g_stream);
		fm = std::move(f);
		SDL_UnlockAudioStream(g_stream);
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
		SDL_LockAudioStream(g_stream);
		std::unique_ptr<MakoMusic> old_music = std::move(music);
		std::unique_ptr<MakoYmfm> old_fm = std::move(fm);
		SDL_UnlockAudioStream(g_stream);
		// old_music/old_fm are destroyed here, outside the lock.
	}
	if (midi->is_available())
		midi->stop();
	current_music = 0;
}

bool MAKO::check_music()
{
	SDL_LockAudioStream(g_stream);
	if (fm) {
		int mark, loop;
		fm->get_mark(&mark, &loop);
		SDL_UnlockAudioStream(g_stream);
		return !loop;
	}
	if (music) {
		bool playing = music->is_playing();
		SDL_UnlockAudioStream(g_stream);
		return playing;
	}
	SDL_UnlockAudioStream(g_stream);
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
	SDL_LockAudioStream(g_stream);
	if (fm) {
		fm->get_mark(mark, loop);
		SDL_UnlockAudioStream(g_stream);
		return;
	}
	SDL_UnlockAudioStream(g_stream);
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
		if (!SDL_LoadWAV_IO(SDL_IOFromConstMem(data.data(), static_cast<int>(data.size())), 1, &spec, &wav, &wavlen)) {
			WARNING("SDL_LoadWAV_IO failed: %s", SDL_GetError());
			return;
		}
		src.assign(wav, wav + wavlen);
		SDL_free(wav);
		stream = SDL_CreateAudioStream(&spec, &g_device_spec);
	} else {
		// AMSE形式 (乙女戦記)
		data = amse.load(page);
		if (data.empty())
			return;
		uint32_t amse_size = SDL_Swap32LE(*reinterpret_cast<uint32_t*>(&data[8]));
		// 4-bit PCM -> 8-bit PCM, mono, 8000Hz
		for (uint32_t i = 12; i < amse_size; i++) {
			src.push_back(data[i] & 0xf0);
			src.push_back((data[i] & 0x0f) << 4);
		}
		SDL_AudioSpec src_spec = { SDL_AUDIO_U8, 1, 8000 };
		stream = SDL_CreateAudioStream(&src_spec, &g_device_spec);
	}
	if (!stream) {
		WARNING("SDL_CreateAudioStream failed: %s", SDL_GetError());
		return;
	}
	SDL_LockAudioStream(g_stream);
	pcm_stream = stream;
	pcm_src = std::move(src);
	pcm_loops = loops ? loops : -1;
	pcm_input_finished = false;
	SDL_UnlockAudioStream(g_stream);
}

void MAKO::stop_pcm()
{
	SDL_LockAudioStream(g_stream);
	SDL_AudioStream* old = pcm_stream;
	pcm_stream = nullptr;
	pcm_src.clear();
	pcm_input_finished = false;
	SDL_UnlockAudioStream(g_stream);
	if (old)
		SDL_DestroyAudioStream(old);
}

bool MAKO::check_pcm()
{
	// 再生中でtrue
	SDL_LockAudioStream(g_stream);
	bool playing = pcm_stream &&
		(!pcm_input_finished || SDL_GetAudioStreamAvailable(pcm_stream) > 0);
	SDL_UnlockAudioStream(g_stream);
	return playing;
}

bool MAKO::is_cd_available() const
{
	return !playlist.empty();
}
