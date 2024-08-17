/*
	ALICE SOFT SYSTEM 3 for Win32

	[ DRI ]
*/

#ifndef _DRI_H_
#define _DRI_H_

#include <stdint.h>
#include <vector>

std::vector<uint8_t> dri_load(const char* file_name, int page);
std::vector<uint8_t> dri_load_mda(uint32_t crc32_a, uint32_t crc32_b, int page);

#endif
