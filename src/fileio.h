/*
	ALICE SOFT SYSTEM 3 for Win32

	[ File I/O ]
*/

#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <memory>
#include <string>
#include <stdio.h>
#include <sys/stat.h>
#include "common.h"

#define FILEIO_READ_BINARY 1
#define FILEIO_WRITE_BINARY 2
#define FILEIO_SAVEDATA 4

class FILEIO
{
private:
	FILEIO(FILE* fp_, int mode) : fp(fp_), mode_(mode) {}

	static std::string savedir;
	FILE* fp;
	int mode_;

public:
	~FILEIO();

	static void set_savedir(const std::string& dir);
	static int stat_save(const char* filename, struct stat* buf);
	static std::unique_ptr<FILEIO> open(const char *file_name, int mode);
	int seek(long offset, int whence) { return fseek(fp, offset, whence); }
	long tell() { return ftell(fp); }
	bool read(void* buffer, size_t size) {
		return fread(buffer, size, 1, fp) == 1;
	}
	bool write(const void* buffer, size_t size) {
		return fwrite(buffer, size, 1, fp) == 1;
	}
	std::string read_string(size_t size);
	bool write_string(const std::string& str, size_t size);
	int getc() { return fgetc(fp); }
	int getw() {
		int val = getc();
		return val | (getc() << 8);
	}
	uint32_t getdw() {
		uint32_t val = getc();
		val |= getc() << 8;
		val |= getc() << 16;
		val |= getc() << 24;
		return val;
	}
	void putc(int c) { fputc(c, fp); }
	void putw(int w) {
		putc(w & 0xff);
		putc((w >> 8) & 0xff);
	}
	std::string gets();
};

#endif
