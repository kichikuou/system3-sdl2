/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#include <windows.h>
#undef ERROR
#include <memory>
#include <string>
#include <SDL3/SDL.h>
#include "mako.h"
#include "mako_midi.h"
#include "fm/mako_ymfm.h"
#include "config.h"
#include "dri.h"
#include "game_id.h"

extern SDL_Window* g_window;

namespace {

// Per-game mapping from music numbers to CD tracks.  This is necessary to
// forcibly change the sound device with a menu command.
//
// When the game uses "Z 100+x,y" command, the xth element of the array should
// be y.  The array must be terminated with -1.
const int8_t RANCE41_tracks[] = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,1,-1};
const int8_t RANCE42_tracks[] = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,1,-1};
const int8_t DPSALL_tracks[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,1,2,3,-1};

enum MakoThreadMessage {
	MAKO_OPEN = WM_APP,
	MAKO_PLAY,
	MAKO_STOP,
	MAKO_EXIT
};

const char* mci_geterror(DWORD err)
{
	static char buf[128];
	mciGetErrorString(err, buf, sizeof(buf));
	return buf;
}

class MCIThread {
public:
	MCIThread() {
		hwnd_notify = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(g_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

		if (!_beginthreadex(NULL, 0, &MCIThread::run, this, 0, &thread_id))
			sys_error("Cannot create thread: %s", strerror(errno));
	}

	bool post_message(UINT msg, WPARAM wparam, LPARAM lparam) {
		return PostThreadMessage(thread_id, msg, wparam, lparam);
	}

	MCIDEVICEID device_id() { return devid; }

private:
	static unsigned __stdcall run(void* param)
	{
		MCIThread* t = reinterpret_cast<MCIThread*>(param);
		t->message_loop();
		delete t;
		return 0;
	}

	void message_loop()
	{
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			switch (msg.message) {
			case MAKO_OPEN:
				close();
				open(reinterpret_cast<std::string*>(msg.wParam));
				break;

			case MAKO_PLAY:
				play();
				break;

			case MAKO_STOP:
				close();
				break;

			case MAKO_EXIT:
				close();
				return;
			}
		}
	}

	void open(std::string* file)
	{
		file_playing = file;

		MCI_OPEN_PARMS open;
		open.lpstrElementName = file_playing->c_str();
		DWORD flags = MCI_OPEN_ELEMENT | MCI_WAIT;
		MCIERROR err = mciSendCommand(0, MCI_OPEN, flags, (DWORD_PTR)&open);
		if (err) {
			WARNING("MCI_OPEN failed: %s: %s",
					file_playing->c_str(), mci_geterror(err));
			return;
		}
		devid = open.wDeviceID;
	}

	void play()
	{
		MCI_PLAY_PARMS play;
		play.dwCallback = (DWORD_PTR)hwnd_notify;
		play.dwFrom = 0;
		DWORD flags = MCI_FROM | MCI_NOTIFY;
		MCIERROR err = mciSendCommand(devid, MCI_PLAY, flags, (DWORD_PTR)&play);
		if (err)
			WARNING("MCI_PLAY failed: %s", mci_geterror(err));
	}

	void close() {
		if (devid) {
			mciSendCommand(devid, MCI_CLOSE, 0, 0);
			devid = 0;
		}
		if (file_playing) {
			delete file_playing;
			file_playing = nullptr;
		}
	}

	HWND hwnd_notify;
	unsigned thread_id;
	std::string* file_playing = nullptr;
	MCIDEVICEID devid = 0;
};

MCIThread* mci_thread;

class Music {
public:
	Music(const char* fname, int loop) : loops(loop) {
		std::string* fname_str = new std::string(fname);
		mci_thread->post_message(MAKO_OPEN, (WPARAM)fname_str, 0);
		mci_thread->post_message(MAKO_PLAY, 0, 0);
		playing = true;
	}

	~Music() {
		close();
	}

	void close() {
		mci_thread->post_message(MAKO_STOP, 0, 0);
		playing = false;
	}

	bool is_playing() {
		return playing;
	}

	bool on_mci_notify(const MSG* msg) {
		if (msg->lParam != mci_thread->device_id())
			return true;
		switch (msg->wParam) {
		case MCI_NOTIFY_FAILURE:
			close();
			return false;
		case MCI_NOTIFY_SUCCESSFUL:
			if (loops && --loops == 0) {
				close();
				return false;
			}
			mci_thread->post_message(MAKO_PLAY, 0, 0);
			return true;
		}
		return true;
	}

private:
	int loops;  // number of times to play, 0 for infinite loop
	bool playing;
};

SDL_AudioDeviceID g_device;
Music* music;
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

	for(int i = 1; i <= 99; i++) {
		cd_track[i] = 0;
	}

	SDL_InitSubSystem(SDL_INIT_AUDIO);
	g_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
	if (!g_device) {
		ERROR("Cannot open audio device: %s", SDL_GetError());
	}

	mci_thread = new MCIThread();

	midi = std::make_unique<MAKOMidi>(config.midi_device);
	if (!midi->is_available())
		use_fm = true;
}

MAKO::~MAKO()
{
	stop_music();
	stop_pcm();
	mci_thread->post_message(MAKO_EXIT, 0, 0);
	mci_thread = nullptr;  // MCIThread self-destructs on the worker thread.
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
	if (current_music == page)
		return;

	stop_music();

	size_t track = page < 100 ? cd_track[page] : 0;
	if (track) {
		if (track >= playlist.size() || !playlist[track])
			return;
		music = new Music(playlist[track], next_loop);
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
}

void MAKO::stop_music()
{
	if (music) {
		delete music;
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
			SDL_PutAudioStreamData(pcm_stream, buf.data(), buf.size());
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

void MAKO::on_mci_notify(const MSG* msg)
{
	if (!music)
		return;
	if (!music->on_mci_notify(msg))
		current_music = 0;
}
