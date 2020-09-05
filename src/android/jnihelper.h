#ifndef _JNIHELPER_H_
#define _JNIHELPER_H_

#include <jni.h>
#include "common.h"

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

#endif // _JNIHELPER_H_
