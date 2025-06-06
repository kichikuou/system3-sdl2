/*
	ALICE SOFT SYSTEM 3 for Win32

	[ File I/O ]
*/

#include "fileio.h"
#include <vector>
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

void FILEIO::set_savedir(const std::string& dir)
{
	savedir = dir;
	while (!savedir.empty() && savedir.back() == '/')
		savedir.pop_back();
}

int FILEIO::stat_save(const char* filename, struct stat* buf)
{
	return stat(FindFile(savedir, filename).c_str(), buf);
}

FILEIO::~FILEIO(void)
{
	fclose(fp);
#ifdef __EMSCRIPTEN__
	if (mode_ & FILEIO_WRITE_BINARY)
		EM_ASM( xsystem35.shell.syncfs(); );
#endif
}

// static
std::unique_ptr<FILEIO> FILEIO::open(const char *file_name, int mode)
{
	std::string path =
		FindFile((mode & FILEIO_SAVEDATA) ? savedir : ".", file_name);

	mode &= ~FILEIO_SAVEDATA;

	FILE* fp = NULL;
	if(mode == FILEIO_READ_BINARY) {
		fp = fopen(path.c_str(), "rb");
	} else if(mode == FILEIO_WRITE_BINARY) {
		fp = fopen(path.c_str(), "wb");
	}
	return fp ? std::unique_ptr<FILEIO>(new FILEIO(fp, mode)) : nullptr;
}

std::string FILEIO::gets()
{
	std::string s;
	for (;;) {
		int c = getc();
		if (c == '\r' || c == EOF)
			break;
		if(c != '\n')
			s.push_back(c);
	}
	return s;
}

std::string FILEIO::read_string(size_t size) {
	std::vector<char> buf(size + 1);
	if (fread(buf.data(), size, 1, fp) != 1) {
		return std::string();
	}
	return std::string(buf.data());
}

bool FILEIO::write_string(const std::string& str, size_t size) {
	std::vector<char> buf(size);
	memcpy(buf.data(), str.c_str(), std::min(size, str.size()));
	return fwrite(buf.data(), size, 1, fp) == 1;
}
