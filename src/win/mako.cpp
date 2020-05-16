/*
	ALICE SOFT SYSTEM 3 for Win32

	[ MAKO ]
*/

#include <windows.h>
#undef ERROR
#include <string>
#include <SDL_syswm.h>
#include "mako.h"
#include "mako_midi.h"
#include "dri.h"

extern SDL_Window* g_window;

namespace {

const char* mci_geterror(DWORD err)
{
	static char buf[128];
	mciGetErrorString(err, buf, sizeof(buf));
	return buf;
}

class Music {
public:
	static HWND hwnd_notify;

	Music(const char* fname, int loop) : devid(0) {
		MCI_OPEN_PARMS open;
		DWORD flags = MCI_OPEN_ELEMENT | MCI_WAIT;
		open.lpstrElementName = fname;
		MCIERROR err = mciSendCommand(0, MCI_OPEN, flags, (DWORD_PTR)&open);
		if (err) {
			WARNING("MCI_OPEN failed: %s: %s", fname, mci_geterror(err));
			return;
		}
		devid = open.wDeviceID;
		loops = loop;
	}

	Music(MAKOMidi& midi, int loop) : devid(0) {
		std::vector<uint8> smf = midi.generate_smf(loop);
		char path[MAX_PATH + 1];
		if (!GetTempPath(sizeof(path), path)) {
			WARNING("GetTempPath failed: 0x%x", GetLastError());
			return;
		}
		char fname[MAX_PATH + 1];
		if (!GetTempFileName(path, "mid", 0, fname)) {
			WARNING("GetTempFileName failed: 0x%x", GetLastError());
			return;
		}
		FILE* fp = fopen(fname, "wb");
		if (!fp) {
			WARNING("Cannot open %s: %s", fname, strerror(errno));
			return;
		}
		tmpfile = fname;  // Delete this file in destructor
		if (fwrite(smf.data(), smf.size(), 1, fp) != 1) {
			WARNING("Cannot write to %s: %s", fname, strerror(errno));
			fclose(fp);
			return;
		}
		fclose(fp);

		DWORD flags = MCI_OPEN_ELEMENT | MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_WAIT;
		MCI_OPEN_PARMS open;
		open.lpstrElementName = fname;
		open.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_SEQUENCER;
		MCIERROR err = mciSendCommand(0, MCI_OPEN, flags, (DWORD_PTR)&open);
		if (err) {
			WARNING("MCI_OPEN failed: %s: %s", fname, mci_geterror(err));
			return;
		}
		devid = open.wDeviceID;
		loops = loop ? 1 : 0;
	}

	~Music() {
		close();
		if (!tmpfile.empty())
			DeleteFile(tmpfile.c_str());
	}

	void close() {
		if (devid) {
			mciSendCommand(devid, MCI_CLOSE, 0, 0);
			devid = 0;
		}
	}

	bool play() {
		if (!devid)
			return false;
		MCI_PLAY_PARMS play;
		play.dwCallback = (DWORD_PTR)hwnd_notify;
		play.dwFrom = 0;
		DWORD flags = MCI_FROM | (hwnd_notify ? MCI_NOTIFY : 0);
		MCIERROR err = mciSendCommand(devid, MCI_PLAY, flags, (DWORD_PTR)&play);
		if (err) {
			WARNING("MCI_PLAY failed: %s", mci_geterror(err));
			close();
			return false;
		}
		return true;
	}

	bool is_playing() {
		if (!devid)
			return false;
		MCI_STATUS_PARMS stat;
		stat.dwItem = MCI_STATUS_MODE;
		MCIERROR err = mciSendCommand(devid, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&stat);
		if (err) {
			NOTICE("MCI_STATUS failed: %s", mci_geterror(err));
			return false;
		}
		return stat.dwReturn == MCI_MODE_PLAY;
	}

	bool on_mci_notify(SDL_SysWMmsg* msg) {
		if (msg->msg.win.lParam != devid)
			return true;
		switch (msg->msg.win.wParam) {
		case MCI_NOTIFY_FAILURE:
			close();
			return false;
		case MCI_NOTIFY_SUCCESSFUL:
			if (loops && --loops == 0) {
				close();
				return false;
			}
			return play();
		}
		return true;
	}

private:
	MCIDEVICEID devid;
	int loops;  // number of times to play, 0 for infinite loop
	std::string tmpfile;
};

Music *music;
HWND Music::hwnd_notify;
uint8* wav_buffer;

} // namespace

MAKO::MAKO(NACT* parent, const char* playlist) :
	current_music(0),
	next_loop(0),
	nact(parent)
{
	if (playlist)
		load_playlist(playlist);

	strcpy_s(amus, 16, "AMUS.DAT");
	strcpy_s(amse, 16, "AMSE.DAT");	// 実際には使わない

	for(int i = 1; i <= 99; i++) {
		cd_track[i] = 0;
	}

	SDL_SysWMinfo info;
	if (!SDL_GetWindowWMInfo(g_window, &info)) {
		WARNING("SDL_GetWindowWMInfo failed: %s", SDL_GetError());
	} else {
		Music::hwnd_notify = info.info.win.window;
		SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	}
}

MAKO::~MAKO()
{
	stop_music();
	stop_pcm();
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

	int track = page < 100 ? cd_track[page] : 0;
	if (track) {
		if (track >= playlist.size() || !playlist[track])
			return;
		music = new Music(playlist[track], next_loop);
	} else {
		MAKOMidi midi(nact, amus);
		if (!midi.load_mml(page))
			return;
		midi.load_mda(page);
		music = new Music(midi, next_loop);
	}
	if (music->play())
		current_music = page;
}

void MAKO::stop_music()
{
	if (music) {
		delete music;
		music = nullptr;
	}
	current_music = 0;
}

bool MAKO::check_music()
{
	return music && music->is_playing();
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

	if ((buffer = dri->load("AWAV.DAT", page, &size)) != NULL) {
		// WAV形式 (Only You)
		wav_buffer = buffer;
	} else if ((buffer = dri->load("AMSE.DAT", page, &size)) != NULL) {
		// AMSE形式 (乙女戦記)
		int total = (size - 12) * 2 + 0x24;
		int samples = (size - 12) * 2;

		wav_buffer = (uint8*)malloc(total + 8);
		memcpy(wav_buffer, header, 44);
		wav_buffer[ 4] = (total >>  0) & 0xff;
		wav_buffer[ 5] = (total >>  8) & 0xff;
		wav_buffer[ 6] = (total >> 16) & 0xff;
		wav_buffer[ 7] = (total >> 24) & 0xff;
		wav_buffer[40] = (samples >>  0) & 0xff;
		wav_buffer[41] = (samples >>  8) & 0xff;
		wav_buffer[42] = (samples >> 16) & 0xff;
		wav_buffer[43] = (samples >> 24) & 0xff;
		for(int i = 12, p = 44; i < size; i++) {
			wav_buffer[p++] = buffer[i] & 0xf0;
			wav_buffer[p++] = (buffer[i] & 0x0f) << 4;
		}
		free(buffer);
	}
	PlaySound((LPCTSTR)wav_buffer, NULL, SND_ASYNC | SND_MEMORY | (loop ? SND_LOOP : 0));
}

void MAKO::stop_pcm()
{
	PlaySound(NULL, NULL, SND_PURGE);
	if (wav_buffer) {
		free(wav_buffer);
		wav_buffer = nullptr;
	}
}

bool MAKO::check_pcm()
{
	// 再生中でtrue
	static const char null_wav[45] =  {
		'R' , 'I' , 'F' , 'F' , 0x25, 0x00, 0x00, 0x00, 'W' , 'A' , 'V' , 'E' , 'f' , 'm' , 't' , ' ' ,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00,
		0x01, 0x00, 0x08, 0x00, 'd' , 'a' , 't' , 'a' , 0x01, 0x00, 0x00, 0x00, 0x00
	};
	return !PlaySound(null_wav, NULL, SND_ASYNC | SND_MEMORY | SND_NOSTOP);
}

void MAKO::on_mci_notify(SDL_SysWMmsg* msg)
{
	if (!music)
		return;
	if (!music->on_mci_notify(msg))
		current_music = 0;
}
