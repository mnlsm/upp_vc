#pragma once

#ifdef _MSC_VER
#define COMPILER_MSC 1
#define WINVER          0x0501
#define _WIN32_WINNT    0x0501
#define _WIN32_WINDOWS  0x0500
#define _WIN32_IE       0x0501
#define _RICHEDIT_VER   0x0200

#ifndef _CPPRTTI
#error  RTTI must be enabled !!!
#endif  //_CPPRTTI
#if _MSC_VER <= 1300
#error  MSC 6.0 not supported anymore
#endif
//#define _MULTITHREADED
#pragma warning(disable: 4786)
#define _CRT_SECURE_NO_DEPRECATE 1 // we really need strcpy etc. to work with MSC 8.0
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define PLATFORM_WIN32 1

#define CPU_LE 1
#define CPU_LITTLE_ENDIAN 1
#define CPU_UNALIGNED 1
#define CPU_X86 1

#ifdef _WIN64
#define PLATFORM_WIN64 1
#define CPU_64 1
#define CPU_AMD64 1
#define CPU_SSE2 1
#define CPU_IA64 1
#else
#define CPU_32 1
#define CPU_IA32 1
#ifdef flagSSE2
#define CPU_SSE2 1
#endif
#endif
#endif
