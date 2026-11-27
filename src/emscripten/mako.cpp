/*
	ALICE SOFT SYSTEM 3 for Emscripten
	[ MAKO ]
*/

#include <memory>
#include "sdl_compat.h"
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

void put_le32(uint8_t* p, uint32_t v)
{
	p[0] = v & 0xff;
	p[1] = (v >> 8) & 0xff;
	p[2] = (v >> 16) & 0xff;
	p[3] = (v >> 24) & 0xff;
}

// Wraps 8-bit mono PCM samples in a WAV container.
std::vector<uint8_t> make_wav_u8_mono(const std::vector<uint8_t>& samples, uint32_t freq)
{
	static const uint8_t header[44] = {
		'R' , 'I' , 'F' , 'F' , 0x00, 0x00, 0x00, 0x00,
		'W' , 'A' , 'V' , 'E' , 'f' , 'm' , 't' , ' ' ,
		0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x08, 0x00, 'd' , 'a' , 't' , 'a' ,
		0x00, 0x00, 0x00, 0x00
	};
	std::vector<uint8_t> wav(header, header + sizeof(header));
	put_le32(&wav[4], static_cast<uint32_t>(samples.size() + sizeof(header) - 8));
	put_le32(&wav[24], freq);  // sample rate
	put_le32(&wav[28], freq);  // byte rate
	put_le32(&wav[40], static_cast<uint32_t>(samples.size()));
	wav.insert(wav.end(), samples.begin(), samples.end());
	return wav;
}

// 4-bit PCM -> 8-bit PCM
void unpack_4bit_pcm(uint8_t b, std::vector<uint8_t>& out)
{
	out.push_back(b & 0xf0);
	out.push_back((b & 0x0f) << 4);
}

std::vector<uint8_t> load_amse_pcm(Dri& archive, int page)
{
	std::vector<uint8_t> data = archive.load(page);
	if (data.empty())
		return {};

	uint32_t size = sdl::Swap32LE(*reinterpret_cast<uint32_t*>(&data[8]));
	std::vector<uint8_t> samples;
	for (uint32_t i = 12; i < size; i++)
		unpack_4bit_pcm(data[i], samples);
	return make_wav_u8_mono(samples, 8000);
}

std::vector<uint8_t> load_amus_pcm(Dri& archive, int page)
{
	// Zero-terminated 4-bit PCM in AMUS.DAT.
	std::vector<uint8_t> data = archive.load(page);
	if (data.empty())
		return {};

	// An optional "MP" header supplies the frequency in units of 100 Hz.
	size_t pos = 0;
	uint32_t freq = 8000;
	if (data.size() >= 16 && data[0] == 'M' && data[1] == 'P' &&
		data[2] == 100 && data[3] == 0) {
		freq = (data[4] | (data[5] << 8)) * 100;
		pos = 16;
	}
	if (freq == 0) {
		WARNING("Invalid PCM freq in AMUS.DAT page %d", page);
		return {};
	}
	std::vector<uint8_t> samples;
	while (pos < data.size() && data[pos] != 0)
		unpack_4bit_pcm(data[pos++], samples);
	if (pos == data.size()) {
		WARNING("Unterminated PCM in AMUS.DAT page %d", page);
		return {};
	}
	return make_wav_u8_mono(samples, freq);
}

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
	if (track && is_cd_available()) {
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
	std::vector<uint8_t> wav = awav.load(page);  // Only You
	if (wav.empty())
		wav = load_amse_pcm(amse, page);  // Otome Senki, Gakuen King
	if (wav.empty())
		wav = load_amus_pcm(amus, page);  // Nise naguri
	if (wav.empty())
		return;
	if (muspcm_load_data(wav.data(), wav.size()))
		EM_ASM({ xsystem35.audio.pcm_start(0, $0); }, loops);
}

void MAKO::stop_pcm() {
	EM_ASM({ xsystem35.audio.pcm_stop(0); });
}

bool MAKO::check_pcm()
{
	return EM_ASM_INT({ return xsystem35.audio.pcm_isplaying(0); });
}

bool MAKO::is_cd_available() const
{
	return EM_ASM_INT({ return xsystem35.cdPlayer.hasAudioTrack(); });
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
