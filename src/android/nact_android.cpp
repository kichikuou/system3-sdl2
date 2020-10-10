#include <stdio.h>
#include "nact.h"
#include "utfsjis.h"
#include "jnihelper.h"

#define STRING "Ljava/lang/String;"

void NACT::text_dialog()
{
	JNILocalFrame jni(16);
	if (!jni.env())
		return;

	char *oldstr = sjis2utf(tvar[tvar_index - 1]);
	jstring joldstr = jni.env()->NewStringUTF(oldstr);
	if (!joldstr) {
		WARNING("Failed to allocate a string");
		return;
	}
	jmethodID mid = jni.GetMethodID("inputString", "(" STRING "I)" STRING);
	jstring jnewstr = (jstring)jni.env()->CallObjectMethod(jni.context(), mid, joldstr, tvar_maxlen);
	if (!jnewstr)
		return;
	const char* newstr_utf8 = jni.env()->GetStringUTFChars(jnewstr, NULL);
	char* newstr_sjis = utf2sjis((char*)newstr_utf8);
	strcpy_s(tvar[tvar_index - 1], 22, newstr_sjis);
	free(newstr_sjis);
	jni.env()->ReleaseStringUTFChars(jnewstr, newstr_utf8);
}

void NACT::platform_initialize()
{
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

void NACT::on_syswmevent(SDL_SysWMmsg* msg)
{
}
