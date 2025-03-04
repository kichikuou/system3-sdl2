/*
	ALICE SOFT SYSTEM 3 for Emscripten
	[ MAKO ]
*/

#include <memory>
#include <emscripten.h>
#include "mako.h"
#include "fm/mako_ymfm.h"
#include "dri.h"

namespace {

MAKO *g_mako;
std::unique_ptr<MakoYmfm> fm;

EM_ASYNC_JS(bool, muspcm_load_data, (uint8_t *buf, uint32_t len), {
	return await xsystem35.audio.pcm_load_data(0, buf, len);
});

} // namespace

MAKO::MAKO(const Config& config, const GameId& game_id) :
	use_fm(true),
	current_music(0),
	next_loop(0),
	game_id(game_id)
{
	g_mako = this;
	amus.open("AMUS.DAT");
	awav.open("AWAV.DAT");
	amse.open("AMSE.DAT");
	mda.open("AMUS.MDA");
	for (int i = 1; i <= 99; i++)
		cd_track[i] = 0;
}

MAKO::~MAKO()
{
	stop_pcm();
	stop_music();
}

void MAKO::play_music(int page)
{
	if (current_music == page)
		return;

	stop_music();

	int track = page < 100 ? cd_track[page] : 0;
	if (track) {
		EM_ASM_ARGS({ xsystem35.cdPlayer.play($0, $1); },
					cd_track[page] + 1, next_loop ? 0 : 1);
	} else {
		std::vector<uint8_t> data = amus.load(page);
		if (data.empty())
			return;
		int rate = EM_ASM_INT({ return xsystem35.audio.enable_audio_hook(); });
		fm = std::make_unique<MakoYmfm>(rate, std::move(data));
	}

	current_music = page;
	next_loop = 0;
}

void MAKO::stop_music()
{
	if (!current_music)
		return;

	EM_ASM({
		xsystem35.cdPlayer.stop();
	});

	fm = nullptr;
	current_music = 0;
}

bool MAKO::check_music()
{
	if (!current_music)
		return false;
	if (fm) {
		int mark, loop;
		fm->get_mark(&mark, &loop);
		return !loop;
	}

	return EM_ASM_INT_V( return xsystem35.cdPlayer.getPosition(); ) != 0;
}

void MAKO::get_mark(int* mark, int* loop)
{
	if (fm) {
		fm->get_mark(mark, loop);
	} else {
		*mark = 0;
		*loop = 0;
	}
}

void MAKO::play_pcm(int page, int loops)
{
	static const char header[44] = {
		'R' , 'I' , 'F' , 'F' , 0x00, 0x00, 0x00, 0x00,
		'W' , 'A' , 'V' , 'E' , 'f' , 'm' , 't' , ' ' ,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x40, 0x1f, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00,
		0x01, 0x00, 0x08, 0x00, 'd' , 'a' , 't' , 'a' ,
		0x00, 0x00, 0x00, 0x00
	};

	// WAV形式 (Only You)
	std::vector<uint8_t> wav_buffer = awav.load(page);
	if (wav_buffer.empty()) {
		std::vector<uint8_t> buffer = amse.load(page);
		if (!buffer.empty()) {
			// AMSE形式 (乙女戦記)
			uint32_t amse_size = SDL_SwapLE32(*reinterpret_cast<uint32_t*>(&buffer[8]));
			int samples = (amse_size - 12) * 2;
			int total = samples + 0x24;

			wav_buffer.resize(total + 8);
			memcpy(wav_buffer.data(), header, 44);
			wav_buffer[ 4] = (total >>  0) & 0xff;
			wav_buffer[ 5] = (total >>  8) & 0xff;
			wav_buffer[ 6] = (total >> 16) & 0xff;
			wav_buffer[ 7] = (total >> 24) & 0xff;
			wav_buffer[40] = (samples >>  0) & 0xff;
			wav_buffer[41] = (samples >>  8) & 0xff;
			wav_buffer[42] = (samples >> 16) & 0xff;
			wav_buffer[43] = (samples >> 24) & 0xff;
			for (uint32_t i = 12, p = 44; i < amse_size; i++) {
				wav_buffer[p++] = buffer[i] & 0xf0;
				wav_buffer[p++] = (buffer[i] & 0x0f) << 4;
			}
		}
	}
	if (muspcm_load_data(wav_buffer.data(), wav_buffer.size()))
		EM_ASM({ xsystem35.audio.pcm_start(0, $0); }, loops);
}

void MAKO::stop_pcm() {
	EM_ASM({ xsystem35.audio.pcm_stop(0); });
}

bool MAKO::check_pcm()
{
	return EM_ASM_INT({ return xsystem35.audio.pcm_isplaying(0); });
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
bool audio_callback(Uint8 *stream, int len) {
	if (!fm)
		return false;
	fm->Process(reinterpret_cast<int16_t*>(stream), len);
	return true;
}

} // extern "C"
