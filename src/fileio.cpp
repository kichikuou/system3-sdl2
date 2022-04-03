/*
	ALICE SOFT SYSTEM 3 for Win32

	[ File I/O ]
*/

#include "fileio.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#ifndef _WIN32
#include <dirent.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

std::string FILEIO::savedir = ".";

namespace {

std::string FindFile(const std::string& dir, const char* filename) {
	std::string path = dir + "/" + filename;

#ifndef _WIN32
	// Case-insensitive search for files in `dir`.
	DIR* d = opendir(dir.c_str());
	if (!d)
		return path;

	while (struct dirent* entry = readdir(d)) {
		if (strcasecmp(filename, entry->d_name) == 0)
			path = dir + "/" + entry->d_name;
	}
	closedir(d);
#endif
	return path;
}

} // namespace

void FILEIO::SetSaveDir(const std::string& dir)
{
	savedir = dir;
	while (!savedir.empty() && savedir.back() == '/')
		savedir.pop_back();
}

int FILEIO::StatSavedata(const char* filename, struct stat* buf)
{
	return stat(FindFile(savedir, filename).c_str(), buf);
}

FILEIO::~FILEIO(void)
{
	if(fp != NULL) {
		Fclose();
	}
}

bool FILEIO::Fopen(const char *file_name, int mode)
{
	mode_ = mode;
	std::string path =
		FindFile((mode & FILEIO_SAVEDATA) ? savedir : ".", file_name);

	mode &= ~FILEIO_SAVEDATA;

	if(fp != NULL) {
		Fclose();
	}
	fp = NULL;
	
	if(mode == FILEIO_READ_BINARY) {
		fp = fopen(path.c_str(), "rb");
	} else if(mode == FILEIO_WRITE_BINARY) {
		fp = fopen(path.c_str(), "wb");
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

size_t FILEIO::Fread(void* buffer, uint32 size, uint32 count)
{
	return fread(buffer, size, count, fp);
}

size_t FILEIO::Fwrite(void* buffer, uint32 size, uint32 count)
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

