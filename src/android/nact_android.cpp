#include <stdio.h>
#include <android/log.h>
#include "nact.h"
#include "encoding.h"
#include "jnihelper.h"

#define STRING "Ljava/lang/String;"

void NACT::text_dialog()
{
	JNILocalFrame jni(16);
	if (!jni.env())
		return;

	std::string oldstr = encoding->toUtf8(tvar[tvar_index - 1].c_str());
	jstring joldstr = jni.env()->NewStringUTF(oldstr.c_str());
	if (!joldstr) {
		WARNING("Failed to allocate a string");
		return;
	}
	jmethodID mid = jni.GetMethodID("inputString", "(" STRING "I)" STRING);
	jstring jnewstr = (jstring)jni.env()->CallObjectMethod(jni.context(), mid, joldstr, tvar_maxlen);
	if (!jnewstr)
		return;
	const char* newstr_utf8 = jni.env()->GetStringUTFChars(jnewstr, NULL);
	tvar[tvar_index - 1] = encoding->fromUtf8(newstr_utf8);
	jni.env()->ReleaseStringUTFChars(jnewstr, newstr_utf8);
}

void NACT::platform_initialize()
{
	mouse_move_enabled = false;
}

void NACT::platform_finalize()
{
}

void NACT::trace(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	__android_log_vprint(ANDROID_LOG_INFO, "system3", format, ap);
	va_end(ap);
}

void NACT::set_skip_menu_state(bool enabled, bool checked)
{
}

bool NACT::handle_platform_event(const SDL_Event& e)
{
	return false;
}
