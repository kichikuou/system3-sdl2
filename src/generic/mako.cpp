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

// Wraps a MakoYmfm sample generator together with the SDL_AudioStream bound to
// the audio device.
class FmStream {
public:
	FmStream(SDL_AudioDeviceID device, std::vector<uint8_t> data)
		: FmStream(device, std::move(data), device_format(device)) {}
	~FmStream() { SDL_DestroyAudioStream(stream); }  // unbinds and stops fill()

	// FM music keeps playing until every channel has looped.
	bool is_playing() { int mark, loop; get_mark(&mark, &loop); return !loop; }

	void get_mark(int* mark, int* loop) {
		SDL_LockAudioStream(stream);
		ymfm.get_mark(mark, loop);
		SDL_UnlockAudioStream(stream);
	}

private:
	FmStream(SDL_AudioDeviceID device, std::vector<uint8_t> data, const SDL_AudioSpec& device_spec)
		: ymfm(device_spec.freq, std::move(data))
	{
		SDL_AudioSpec src_spec = { SDL_AUDIO_S16, 2, device_spec.freq };
		stream = SDL_CreateAudioStream(&src_spec, &device_spec);
		SDL_SetAudioStreamGetCallback(stream, [](void* self, SDL_AudioStream*, int additional_amount, int) {
			static_cast<FmStream*>(self)->fill(additional_amount);
		}, this);
		SDL_BindAudioStream(device, stream);
	}

	static SDL_AudioSpec device_format(SDL_AudioDeviceID device) {
		SDL_AudioSpec spec;
		SDL_GetAudioDeviceFormat(device, &spec, nullptr);
		return spec;
	}

	// SDL3 stream get-callback: synthesize `additional_amount` bytes of audio.
	void fill(int additional_amount) {
		const int CHUNK = 4096;  // bytes; 1024 stereo S16 frames
		int16_t buffer[CHUNK / 2];
		while (additional_amount > 0) {
			int len = additional_amount < CHUNK ? additional_amount : CHUNK;
			ymfm.Process(buffer, len / 4);
			SDL_PutAudioStreamData(stream, buffer, len);
			additional_amount -= len;
		}
	}

	MakoYmfm ymfm;
	SDL_AudioStream* stream;
};

// The audio device.  Each sound source (music, fm, pcm) creates its own
// SDL_AudioStream and binds it to this device.
SDL_AudioDeviceID g_device;

std::unique_ptr<MakoMusic> music;
std::unique_ptr<FmStream> fm;
std::unique_ptr<MAKOMidi> midi;

// PCM playback.  pcm_loops is the number of remaining plays, or -1 for an
// infinite loop.  pcm_stream's get-callback re-feeds pcm_src.
SDL_AudioStream* pcm_stream;
std::vector<uint8_t> pcm_src;
int pcm_loops;
bool pcm_input_finished;

// Get-callback for the pcm stream: re-feed the decoded sample buffer until the
// remaining loop count is exhausted (pcm_loops == 0).  -1 loops forever.
void SDLCALL pcm_audio_callback(void*, SDL_AudioStream* stream, int additional_amount, int /*total_amount*/)
{
	while (SDL_GetAudioStreamAvailable(stream) < additional_amount && !pcm_input_finished) {
		if (pcm_loops == 0) {
			SDL_FlushAudioStream(stream);
			pcm_input_finished = true;
			break;
		}
		SDL_PutAudioStreamData(stream, pcm_src.data(), static_cast<int>(pcm_src.size()));
		if (pcm_loops > 0)
			pcm_loops--;
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
	g_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
	if (!g_device)
		WARNING("Cannot open audio device: %s", SDL_GetError());

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
		music = std::make_unique<MakoMusic>(g_device, file, next_loop);
		if (!music->is_open()) {
			music.reset();
			return;
		}
	} else if (use_fm) {
		std::vector<uint8_t> data = amus.load(page);
		if (data.empty())
			return;
		fm = std::make_unique<FmStream>(g_device, std::move(data));
	} else if (midi->is_available()) {
		if (!midi->play(game_id, amus, mda, page, next_loop))
			return;
	}
	current_music = page;
	next_loop = 0;
}

void MAKO::stop_music()
{
	music.reset();
	fm.reset();
	if (midi->is_available())
		midi->stop();
	current_music = 0;
}

bool MAKO::check_music()
{
	if (fm)
		return fm->is_playing();
	if (music)
		return music->is_playing();
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
	if (fm) {
		fm->get_mark(mark, loop);
		return;
	}
	midi->get_mark(mark, loop);
}

void MAKO::play_pcm(int page, int loops)
{
	stop_pcm();

	SDL_AudioSpec device_spec;
	SDL_GetAudioDeviceFormat(g_device, &device_spec, nullptr);

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
		stream = SDL_CreateAudioStream(&spec, &device_spec);
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
		stream = SDL_CreateAudioStream(&src_spec, &device_spec);
	}
	if (!stream) {
		WARNING("SDL_CreateAudioStream failed: %s", SDL_GetError());
		return;
	}

	pcm_src = std::move(src);
	pcm_loops = loops ? loops : -1;
	pcm_input_finished = false;
	pcm_stream = stream;
	SDL_SetAudioStreamGetCallback(pcm_stream, pcm_audio_callback, nullptr);
	SDL_BindAudioStream(g_device, pcm_stream);
}

void MAKO::stop_pcm()
{
	// Destroy the stream first (unbinds and stops its get-callback), then it is
	// safe to drop the source buffer the callback was reading.
	if (pcm_stream) {
		SDL_DestroyAudioStream(pcm_stream);
		pcm_stream = nullptr;
	}
	pcm_src.clear();
	pcm_input_finished = false;
}

bool MAKO::check_pcm()
{
	// 再生中でtrue
	if (!pcm_stream)
		return false;
	SDL_LockAudioStream(pcm_stream);
	bool playing = pcm_stream &&
		(!pcm_input_finished || SDL_GetAudioStreamAvailable(pcm_stream) > 0);
	SDL_UnlockAudioStream(pcm_stream);
	return playing;
}

bool MAKO::is_cd_available() const
{
	return !playlist.empty();
}
