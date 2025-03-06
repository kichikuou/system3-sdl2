/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#include <memory>
#include <limits.h>
#include <string>
#include <SDL3/SDL.h>
#include "mako.h"
#include "mako_midi.h"
#include "fm/mako_ymfm.h"
#include "fm/mako_mp3.h"
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

SDL_AudioDeviceID g_device;
std::unique_ptr<MakoMP3> music;
SDL_AudioStream* pcm_stream;
std::unique_ptr<MakoYmfm> fm;
std::unique_ptr<MAKOMidi> midi;

} // namespace

MAKO::MAKO(const Config& config, const GameId& game_id) :
	use_fm(config.use_fm),
	current_music(0),
	next_loop(0),
	game_id(game_id)
{
	if (!config.playlist.empty())
		load_playlist(config.playlist.c_str());

	amus.open("AMUS.DAT");
	awav.open("AWAV.DAT");
	amse.open("AMSE.DAT");
	mda.open("AMUS.MDA");

	for (int i = 1; i <= 99; i++) {
		cd_track[i] = 0;
	}

	SDL_InitSubSystem(SDL_INIT_AUDIO);
	g_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
	if (!g_device) {
		ERROR("Cannot open audio device: %s", SDL_GetError());
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

	SDL_CloseAudioDevice(g_device);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
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
	if (track) {
		if (track >= playlist.size() || !playlist[track])
			return;
		music = std::make_unique<MakoMP3>(g_device, playlist[track], next_loop);
	} else if (use_fm) {
		std::vector<uint8_t> data = amus.load(page);
		if (data.empty())
			return;
		fm = std::make_unique<MakoYmfm>(g_device, std::move(data));
	} else if (midi->is_available()) {
		if (!midi->play(game_id, amus, mda, page, next_loop))
			return;
	}
	current_music = page;
	next_loop = 0;
}

void MAKO::stop_music()
{
	if (music) {
		music = nullptr;
	}
	if (fm) {
		fm = nullptr;
	}
	if (midi->is_available()) {
		midi->stop();
	}
	current_music = 0;
}

bool MAKO::check_music()
{
	if (fm) {
		int mark, loop;
		fm->get_mark(&mark, &loop);
		return !loop;
	}
	else if (music) {
		return music->is_playing();
	} else {
		return midi->is_playing();
	}
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

	// WAV形式 (Only You)
	std::vector<uint8_t> data = awav.load(page);
	if (!data.empty()) {
		SDL_IOStream* io = SDL_IOFromConstMem(data.data(), data.size());
		SDL_AudioSpec spec;
		uint8_t *buf;
		uint32_t buflen;
		if (!SDL_LoadWAV_IO(io, true, &spec, &buf, &buflen)) {
			WARNING("SDL_LoadWAV_IO failed: %s", SDL_GetError());
			return;
		}
		pcm_stream = SDL_CreateAudioStream(&spec, &device_spec);
		for (int i = 0; i < loops; i++) {
			SDL_PutAudioStreamData(pcm_stream, buf, buflen);
		}
		SDL_FlushAudioStream(pcm_stream);
		SDL_BindAudioStream(g_device, pcm_stream);
		SDL_free(buf);
		return;
	}

	// AMSE形式 (乙女戦記)
	data = amse.load(page);
	if (!data.empty()) {
		uint32_t amse_size = SDL_Swap32LE(*reinterpret_cast<uint32_t*>(&data[8]));
		std::vector<uint8_t> buf;
		// 4-bit PCM -> 8-bit PCM
		for (size_t i = 12; i < amse_size; i++) {
			buf.push_back(data[i] & 0xf0);
			buf.push_back((data[i] & 0x0f) << 4);
		}

		SDL_AudioSpec spec = { SDL_AUDIO_U8, 1, 8000 };
		pcm_stream = SDL_CreateAudioStream(&spec, &device_spec);
		for (int i = 0; i < loops; i++) {
			SDL_PutAudioStreamData(pcm_stream, buf.data(), static_cast<int>(buf.size()));
		}
		SDL_FlushAudioStream(pcm_stream);
		SDL_BindAudioStream(g_device, pcm_stream);
	}
}

void MAKO::stop_pcm()
{
	if (pcm_stream) {
		SDL_DestroyAudioStream(pcm_stream);
		pcm_stream = nullptr;
	}
}

bool MAKO::check_pcm()
{
	// 再生中でtrue
	return pcm_stream && SDL_GetAudioStreamQueued(pcm_stream) > 0;
}
