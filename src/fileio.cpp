/*
	ALICE SOFT SYSTEM 3 for Win32

	[ File I/O ]
*/

#include "fileio.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

char g_root[_MAX_PATH];
const char* g_savedir;

void FILEIO::SetSaveDir(const char* savedir)
{
	g_savedir = strdup(savedir);
}

int FILEIO::StatSavedata(const char* filename, struct stat* buf)
{
	char path[_MAX_PATH];
	sprintf_s(path, _MAX_PATH, "%s%s", g_savedir ? g_savedir : g_root, filename);
	return stat(path, buf);
}

FILEIO::FILEIO()
{
	// 初期化
	fp = NULL;
}

FILEIO::~FILEIO(void)
{
	// 後始末
	if(fp != NULL) {
		Fclose();
	}
}

bool FILEIO::Fopen(const char *file_name, int mode)
{
	mode_ = mode;
	char path[_MAX_PATH];
	sprintf_s(path, _MAX_PATH, "%s%s",
			  (g_savedir && mode & FILEIO_SAVEDATA) ? g_savedir : g_root,
			  file_name);

	mode &= ~FILEIO_SAVEDATA;

	if(fp != NULL) {
		Fclose();
	}
	fp = NULL;
	
	if(mode == FILEIO_READ_BINARY) {
		fp = fopen(path, "rb");
	} else if(mode == FILEIO_WRITE_BINARY) {
		fp = fopen(path, "wb");
	}
	return (fp != NULL);
}

void FILEIO::Fclose()
{
	if(fp) {
		fclose(fp);
	}
	fp = NULL;

#ifdef __EMSCRIPTEN__
	if (mode_ & FILEIO_WRITE_BINARY)
		EM_ASM( xsystem35.shell.syncfs(); );
#endif
}

uint32 FILEIO::Fseek(long offset, int origin)
{
	uint32 val = 0xFFFFFFFF;
	if(origin == FILEIO_SEEK_CUR) {
		val = fseek(fp, offset, SEEK_CUR);
	} else if(origin == FILEIO_SEEK_END) {
		val = fseek(fp, offset, SEEK_END);
	} else if(origin == FILEIO_SEEK_SET) {
		val = fseek(fp, offset, SEEK_SET);
	}
	return val;
}

uint32 FILEIO::Ftell()
{
	return ftell(fp);
}

uint32 FILEIO::Fread(void* buffer, uint32 size, uint32 count)
{
	return fread(buffer, size, count, fp);
}

uint32 FILEIO::Fwrite(void* buffer, uint32 size, uint32 count)
{
	return fwrite(buffer, size, count, fp);
}

int FILEIO::Fgetc()
{
	return fgetc(fp);
}

int FILEIO::Fgetw()
{
	int val = Fgetc();
	return val | (Fgetc() << 8);
}

void FILEIO::Fgets(char *dest, int length)
{
	int p = 0;
	
	for(;;) {
		int c = Fgetc();
		if(c == 0x0d || c == EOF) {
			break;
		}
		if(c != 0x0a) {
			if(p < length + 1) {
				dest[p++] = c;
			}
			if(is_2byte_message(c)) {
				if(p < length + 1) {
					dest[p++] = Fgetc();
				}
			}
		}
	}
	dest[p] = '\0';
}

void FILEIO::Fputc(int c)
{
	fputc(c, fp);
}

void FILEIO::Fputw(int w)
{
	Fputc(w & 0xff);
	Fputc((w >> 8) & 0xff);
}

