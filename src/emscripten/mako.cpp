/*
	ALICE SOFT SYSTEM 3 for Emscripten
	[ MAKO ]
*/

#include <memory>
#include <emscripten.h>
#include "mako.h"
#include "mako_midi.h"
#include "fm/mako_fmgen.h"
#include "dri.h"

namespace {

MAKO *g_mako;
std::unique_ptr<MakoFMgen> fm;

} // namespace

MAKO::MAKO(NACT* parent, const MAKOConfig& config) :
	use_fm(config.use_fm),
	current_music(0),
	next_loop(0),
	nact(parent)
{
	g_mako = this;
	strcpy(amus, "AMUS.DAT");
	for (int i = 1; i <= 99; i++)
		cd_track[i] = 0;
}

MAKO::~MAKO() {}

void MAKO::play_music(int page)
{
	if (current_music == page)
		return;

	int track = page < 100 ? cd_track[page] : 0;
	if (track) {
		EM_ASM_ARGS({ xsystem35.cdPlayer.play($0, $1); },
					cd_track[page] + 1, next_loop ? 0 : 1);
	} else if (use_fm) {
		DRI dri;
		int size;
		uint8* data = dri.load(amus, page, &size);
		if (!data)
			return;
		int rate = EM_ASM_INT({ return xsystem35.audio.enable_audio_hook(); });
		fm = std::make_unique<MakoFMgen>(rate, data, true);
	} else {
		auto midi = std::make_unique<MAKOMidi>(nact, amus);
		if (!midi->load_mml(page)) {
			WARNING("load_mml(%d) failed", page);
			return;
		}
		midi->load_mda(page);
		std::vector<uint8_t> smf = midi->generate_smf(next_loop);

		EM_ASM_ARGS({ xsystem35.midiPlayer.play($0, $1, $2); },
					next_loop, smf.data(), smf.size());
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
		xsystem35.midiPlayer.stop();
	});

	fm = nullptr;
	current_music = 0;
}

bool MAKO::check_music()
{
	return current_music != 0;
}

void MAKO::get_mark(int* mark, int* loop)
{
	WARNING("not implemented");
	*mark = *loop = 0;
}

void MAKO::play_pcm(int page, bool loop)
{
	WARNING("not implemented");
}

void MAKO::stop_pcm() {}

bool MAKO::check_pcm()
{
	return false;
}

void MAKO::select_synthesizer(bool use_fm_) {
	if (use_fm == use_fm_)
		return;
	use_fm = use_fm_;
	int page = current_music;
	stop_music();
	play_music(page);
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
bool audio_callback(Uint8 *stream, int len) {
	if (!fm)
		return false;
	fm->Process(reinterpret_cast<int16*>(stream), len);
	return true;
}

EMSCRIPTEN_KEEPALIVE
void select_synthesizer(int use_fm) {
	g_mako->select_synthesizer(use_fm);
}

} // extern "C"
