/*
	ALICE SOFT SYSTEM 3 for Win32

	[ Common Header ]
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <SDL.h>
#include <assert.h>

// tchar.h shim
typedef char _TCHAR;
#ifndef _T
#define _T(s) s
#endif
#define _tfopen_s fopen_s
#define _tcscpy_s strcpy_s
#define _tcslen strlen
#define _stprintf_s sprintf_s

// type definition
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

// utilities
uint16 sjis_to_unicode(uint16 sjis);

#endif
