#include <memory>
#include <SDL.h>
#include <android/log.h>
#include "../config.h"
#include "jnihelper.h"
#include "dri.h"
#include "mako.h"
#include "mako_midi.h"
#include "fm/mako_fmgen.h"

namespace {

const int SAMPLE_RATE = 44100;

MAKO *g_mako;
SDL_mutex* fm_mutex;
std::unique_ptr<MakoFMgen> fm;

void audio_callback(void*, Uint8* stream, int len) {
	SDL_LockMutex(fm_mutex);
	fm->Process(reinterpret_cast<int16*>(stream), len/ 4);
	SDL_UnlockMutex(fm_mutex);
}

} // namespace

MAKO::MAKO(NACT* parent, const Config& config) :
	use_fm(config.use_fm),
	current_music(0),
	next_loop(0),
	nact(parent)
{
	g_mako = this;
	strcpy(amus, "AMUS.DAT");
	for (int i = 1; i <= 99; i++)
		cd_track[i] = 0;

	SDL_InitSubSystem(SDL_INIT_AUDIO);
	SDL_AudioSpec fmt;
	SDL_zero(fmt);
	fmt.freq = SAMPLE_RATE;
	fmt.format = AUDIO_S16;
	fmt.channels = 2;
	fmt.samples = 4096;
	fmt.callback = &audio_callback;
	if (SDL_OpenAudio(&fmt, NULL) < 0) {
		WARNING("SDL_OpenAudio: %s", SDL_GetError());
		use_fm = false;
	}
	fm_mutex = SDL_CreateMutex();
}

MAKO::~MAKO() {
	SDL_LockMutex(fm_mutex);
	SDL_CloseAudio();
	SDL_UnlockMutex(fm_mutex);
	SDL_DestroyMutex(fm_mutex);
	fm_mutex = nullptr;
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void MAKO::play_music(int page)
{
	if (current_music == page)
		return;

	stop_music();

	JNILocalFrame jni(16);
	if (!jni.env())
		return;

	int track = page < 100 ? cd_track[page] : 0;
	if (track) {
		jmethodID mid = jni.GetMethodID("cddaStart", "(IZ)V");
		jni.env()->CallVoidMethod(jni.context(), mid, track + 1, next_loop ? 0 : 1);
	} else if (use_fm) {
		DRI dri;
		int size;
		uint8* data = dri.load(amus, page, &size);
		if (!data)
			return;

		SDL_LockMutex(fm_mutex);
		fm = std::make_unique<MakoFMgen>(SAMPLE_RATE, data, true);
		SDL_UnlockMutex(fm_mutex);
		SDL_PauseAudio(0);
	} else {
		auto midi = std::make_unique<MAKOMidi>(nact, amus);
		if (!midi->load_mml(page)) {
			WARNING("load_mml(%d) failed", page);
			return;
		}
		midi->load_mda(page);
		std::vector<uint8_t> smf = midi->generate_smf(next_loop);

		char path[PATH_MAX];
		snprintf(path, PATH_MAX, "%s/tmp.mid", SDL_AndroidGetInternalStoragePath());
		FILE* fp = fopen(path, "w");
		if (!fp) {
			WARNING("Failed to create temporary file");
			return;
		}
		fwrite(smf.data(), smf.size(), 1, fp);
		fclose(fp);

		jstring path_str = jni.env()->NewStringUTF(path);
		if (!path_str) {
			WARNING("Failed to allocate a string");
			return;
		}
		jmethodID mid = jni.GetMethodID("midiStart", "(Ljava/lang/String;Z)V");
		jni.env()->CallVoidMethod(jni.context(), mid, path_str, next_loop ? 0 : 1);
	}

	current_music = page;
	next_loop = 0;
}

void MAKO::stop_music()
{
	if (!current_music)
		return;

	JNILocalFrame jni(16);
	if (!jni.env())
		return;

	if (current_music < 100 && cd_track[current_music]) {
		jmethodID mid = jni.GetMethodID("cddaStop", "()V");
		jni.env()->CallVoidMethod(jni.context(), mid);
	} else {
		jmethodID mid = jni.GetMethodID("midiStop", "()V");
		jni.env()->CallVoidMethod(jni.context(), mid);
	}

	if (use_fm)
		SDL_PauseAudio(1);
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
	int page = current_music;
	stop_music();
	use_fm = use_fm_;
	play_music(page);
}

extern "C" {

JNIEXPORT void JNICALL Java_io_github_kichikuou_system3_GameActivity_selectSynthesizer(
	JNIEnv *env, jobject cls, jboolean use_fm) {
	g_mako->select_synthesizer(use_fm);
}

} // extern "C"
