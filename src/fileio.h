/*
	ALICE SOFT SYSTEM 3 for Win32

	[ File I/O ]
*/

#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <string>
#include <stdio.h>
#include <sys/stat.h>
#include "common.h"

#define FILEIO_READ_BINARY 1
#define FILEIO_WRITE_BINARY 2
#define FILEIO_SAVEDATA 4

#define FILEIO_SEEK_SET 0
#define FILEIO_SEEK_CUR 1
#define FILEIO_SEEK_END 2

class FILEIO
{
private:
	static std::string savedir;
	FILE* fp;
	int mode_;
public:
	FILEIO() : fp(NULL), mode_(0) {}
	~FILEIO();

	static void SetSaveDir(const std::string& dir);
	static int StatSavedata(const char* filename, struct stat* buf);
	bool Fopen(const char *file_name, int mode);
	void Fclose();
	uint32 Fseek(long offset, int origin);
	uint32 Ftell();
	size_t Fread(void* buffer, uint32 size, uint32 count);
	size_t Fwrite(void* buffer, uint32 size, uint32 count);
	int Fgetc();
	int Fgetw();
	void Fgets(char* dest, int length);
	void Fputc(int c);
	void Fputw(int w);
};

#endif
