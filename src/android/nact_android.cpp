#include <stdio.h>
#include "nact.h"
#include "encoding.h"
#include "jnihelper.h"

#define STRING "Ljava/lang/String;"

void NACT::text_dialog()
{
	JNILocalFrame jni(16);
	if (!jni.env())
		return;

	std::string oldstr = encoding->toUtf8(tvar[tvar_index - 1]);
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
	std::string newstr = encoding->fromUtf8(newstr_utf8);
	strcpy_s(tvar[tvar_index - 1], 22, newstr.c_str());
	jni.env()->ReleaseStringUTFChars(jnewstr, newstr_utf8);
}

void NACT::platform_initialize()
{
	mouse_move_enabled = false;
}

void NACT::platform_finalize()
{
}

void NACT::output_console(const char *format, ...)
{
#if defined(_DEBUG_CONSOLE)
	va_list ap;

	va_start(ap, format);
	vfprintf(stdout, format, ap);
	va_end(ap);
#endif
}

void NACT::set_skip_menu_state(bool enabled, bool checked)
{
}

bool NACT::handle_platform_event(const SDL_Event& e)
{
	if (e.type == SDL_FINGERDOWN && SDL_GetNumTouchFingers(e.tfinger.touchId) == 3) {
		JNILocalFrame jni(16);
		if (jni.env()) {
			jmethodID mid = jni.GetMethodID("popupMenu", "()V");
			jni.env()->CallVoidMethod(jni.context(), mid);
			return true;
		}
	}
	return false;
}
