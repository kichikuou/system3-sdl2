#include <memory>
#include <SDL.h>
#include <android/log.h>
#include <jni.h>
#include "mako.h"
#include "mako_midi.h"

class JNILocalFrame {
public:
	JNILocalFrame(int n) :
		env_((JNIEnv*)SDL_AndroidGetJNIEnv()),
		context_((jobject)SDL_AndroidGetActivity())
	{
		if (env_->PushLocalFrame(n) < 0) {
			ERROR("Failed to allocate JVM local references");
			env_ = NULL;
		}
	}
	~JNILocalFrame() {
		if (env_)
			env_->PopLocalFrame(NULL);
	}
	JNIEnv* env() { return env_; }
	jobject context() { return context_; }
	jmethodID GetMethodID(const char* name, const char* sig) {
		return env_->GetMethodID(env_->GetObjectClass(context_), name, sig);
	}

private:
	JNIEnv* env_;
	jobject context_;
};

MAKO::MAKO(NACT* parent, const char* playlist) :
	current_music(0),
	next_loop(0),
	nact(parent)
{
	strcpy(amus, "AMUS.DAT");
	for (int i = 1; i <= 99; i++)
		cd_track[i] = 0;
}

void MAKO::play_music(int page)
{
	if (current_music == page)
		return;

	JNILocalFrame jni(16);
	if (!jni.env())
		return;

	int track = page < 100 ? cd_track[page] : 0;
	if (track) {
		jmethodID mid = jni.GetMethodID("cddaStart", "(IZ)V");
		jni.env()->CallVoidMethod(jni.context(), mid, track + 1, next_loop ? 0 : 1);
	} else {
		auto midi = std::make_unique<MAKOMidi>(nact, amus);
		if (!midi->load_mml(page)) {
			WARNING("load_mml(%d) failed", page);
			return;
		}
		midi->load_mda(page);
		std::vector<uint8_t> smf = midi->generate_smf(next_loop);
		jbyteArray array = jni.env()->NewByteArray(smf.size());
		if (!array) {
			WARNING("Failed to allocate an array");
			return;
		}
		jbyte *buf = jni.env()->GetByteArrayElements(array, NULL);
		memcpy(buf, smf.data(), smf.size());
		jni.env()->ReleaseByteArrayElements(array, buf, 0);
		jmethodID mid = jni.GetMethodID("midiStart", "([BZ)V");
		jni.env()->CallVoidMethod(jni.context(), mid, array, next_loop ? 0 : 1);
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

	current_music = 0;
}
