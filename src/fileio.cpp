/*
	ALICE SOFT SYSTEM 3 for Win32

	[ File I/O ]
*/

#include "fileio.h"

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

const char *FILEIO::GetRootPath()
{
	static bool initialized = false;
	static char root_path[_MAX_PATH];
	
	// モジュールパス取得
	if(!initialized) {
		char tmp_path[_MAX_PATH], *ptr = NULL;
		
		if(GetModuleFileNameA(NULL, tmp_path, _MAX_PATH) != 0 && GetFullPathNameA(tmp_path, _MAX_PATH, root_path, &ptr) != 0 && ptr != NULL) {
			*ptr = '\0';
		} else {
			strcpy_s(root_path, _MAX_PATH, ".\\");
		}
		initialized = true;
	}
	return root_path;
}

const char *FILEIO::GetFilePath(const char *file_name)
{
	static char file_path[_MAX_PATH];
	
	sprintf_s(file_path, _MAX_PATH, "%s%s", GetRootPath(), file_name);
	return file_path;
}

bool FILEIO::Fopen(const char *file_name, int mode)
{
	if(fp != NULL) {
		Fclose();
	}
	fp = NULL;
	
	if(mode == FILEIO_READ_BINARY) {
		fopen_s(&fp, GetFilePath(file_name), "rb");
	} else if(mode == FILEIO_WRITE_BINARY) {
		fopen_s(&fp, GetFilePath(file_name), "wb");
	} else if(mode == FILEIO_READ_WRITE_BINARY) {
		fopen_s(&fp, GetFilePath(file_name), "r+b");
	} else if(mode == FILEIO_READ_ASCII) {
		fopen_s(&fp, GetFilePath(file_name), "r");
	} else if(mode == FILEIO_WRITE_ASCII) {
		fopen_s(&fp, GetFilePath(file_name), "w");
	} else if(mode == FILEIO_READ_WRITE_ASCII) {
		fopen_s(&fp, GetFilePath(file_name), "r+w");
	}
	return (fp != NULL);
}

void FILEIO::Fclose()
{
	if(fp) {
		fclose(fp);
	}
	fp = NULL;
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

uint32 FILEIO::Fread(void *buffer, uint32 size, uint32 count)
{
	return fread(buffer, size, count, fp);
}

uint32 FILEIO::Fwrite(void *buffer, uint32 size, uint32 count)
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
			if((0x81 <= c && c <= 0x9f) || 0xe0 <= c) {
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

