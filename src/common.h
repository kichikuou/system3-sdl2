/*
	ALICE SOFT SYSTEM 3 for Win32

	[ Common Header ]
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "Gdiplus.lib")
using namespace Gdiplus;

#ifdef _DEBUG
	// detect memory leaks
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
	#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <tchar.h>

#if defined(_MSC_VER) && _MSC_VER == 1200
	// variable scope of 'for' loop for Microsoft Visual C++ 6.0
	#define for if(0);else for
#endif

// type definition
#ifndef uint8
typedef unsigned char uint8;
#endif
#ifndef uint16
typedef unsigned short uint16;
#endif
#ifndef uint32
typedef unsigned long uint32;
#endif

#ifndef int8
typedef signed char int8;
#endif
#ifndef int16
typedef signed short int16;
#endif
#ifndef int32
typedef signed long int32;
#endif

#endif
