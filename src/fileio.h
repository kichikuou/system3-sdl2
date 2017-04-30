/*
	ALICE SOFT SYSTEM 3 for Win32

	[ File I/O ]
*/

#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <stdio.h>
#include "common.h"

#define FILEIO_READ_BINARY 1
#define FILEIO_WRITE_BINARY 2
#define FILEIO_READ_WRITE_BINARY 3
#define FILEIO_READ_ASCII 4
#define FILEIO_WRITE_ASCII 5
#define FILEIO_READ_WRITE_ASCII 6

#define FILEIO_SEEK_SET 0
#define FILEIO_SEEK_CUR 1
#define FILEIO_SEEK_END 2

class FILEIO
{
private:
	FILE* fp;
public:
	FILEIO();
	~FILEIO();

	bool Fopen(const _TCHAR *filename, int mode);
	void Fclose();
	uint32 Fseek(long offset, int origin);
	uint32 Ftell();
	uint32 Fread(void* buffer, uint32 size, uint32 count);
	uint32 Fwrite(void* buffer, uint32 size, uint32 count);
	int Fgetc();
	int Fgetw();
	void Fgets(char dest[]);
	void Fputc(int c);
	void Fputw(int w);
};

#endif
