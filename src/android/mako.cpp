#include <SDL.h>
#include <android/log.h>
#include <jni.h>
#include "mako.h"

MAKO::MAKO(NACT* parent, const char* playlist)
	: current_music(0)
	, next_loop(0)
{
	for (int i = 1; i <= 99; i++)
		cd_track[i] = i;
}

void MAKO::play_music(int page)
{
	if (current_music == page)
		return;

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	if (env->PushLocalFrame(16) < 0) {
		__android_log_write(ANDROID_LOG_ERROR, "system3",
							"Failed to allocate JVM local references");
		return;
	}
	jobject context = (jobject)SDL_AndroidGetActivity();
	jmethodID mid = env->GetMethodID(env->GetObjectClass(context),
									 "cddaStart", "(IZ)V");
	env->CallVoidMethod(context, mid, cd_track[page] + 1, next_loop ? 0 : 1);
	env->PopLocalFrame(NULL);

	current_music = page;
	next_loop = 0;
}

void MAKO::stop_music()
{
	if (!current_music)
		return;

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	if (env->PushLocalFrame(16) < 0) {
		__android_log_write(ANDROID_LOG_ERROR, "system3",
							"Failed to allocate JVM local references");
		return;
	}
	jobject context = (jobject)SDL_AndroidGetActivity();
	jmethodID mid = env->GetMethodID(env->GetObjectClass(context),
									 "cddaStop", "()V");
	env->CallVoidMethod(context, mid);
	env->PopLocalFrame(NULL);

	current_music = 0;
}
