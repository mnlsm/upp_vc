
#ifndef UPP_CORE_H
#define UPP_CORE_H

#define QLIB3

#if defined(flagMT)
#define _MULTITHREADED
#ifdef flagDLL
#define flagUSEMALLOC
#endif
#endif

#ifdef flagDLL
#define _USRDLL
#endif

#ifdef flagHEAPDBG
#define HEAPDBG
#endif

#if defined(flagDEBUG)
#ifndef _DEBUG
#define _DEBUG
#endif
#ifndef TESTLEAKS
#define TESTLEAKS
#endif
#ifndef HEAPDBG
#define HEAPDBG
#endif
#else
#ifndef _RELEASE
#define _RELEASE
#endif
#endif

#include "config.h"

#include <typeinfo>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>





#if defined(COMPILER_MSC) && defined(CPU_X86)
#pragma warning(disable: 4035)


#define DIR_SEP  '\\'
#define DIR_SEPS "\\"
#define PLATFORM_PATH_HAS_CASE 0
#include <io.h>
#ifndef PLATFORM_MFC // just mini Windows headers
#ifdef COMPILER_MSC
#ifndef CPU_ARM
#ifndef CPU_AMD64
#ifndef _X86_
#define _X86_
#endif
#else
#ifndef _AMD64_
#define _AMD64_
#endif
#ifndef __NOASSEMBLY__
#define __NOASSEMBLY__
#endif
#ifndef WIN64
#define WIN64
#endif
#endif
#endif
#ifndef _WINDOWS_
#define _WINDOWS_
#endif
#ifndef _INC_WINDOWS
#define _INC_WINDOWS
#endif
#ifndef _STRUCT_NAME
#define _STRUCT_NAME(x)
#define DUMMYSTRUCTNAME
#define DUMMYSTRUCTNAME2
#define DUMMYSTRUCTNAME3
#endif
#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <wingdi.h>
#include <winuser.h>
#define byte win32_byte_ // RpcNdr defines byte -> class with Upp::byte
#define CY win32_CY_
#include <objidl.h>
#include <winnetwk.h>
#undef byte
#undef CY
typedef DWORD LCTYPE;
#else
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
#include <windows.h>
#include <stdint.h>
#endif
#include <process.h>
#endif

#ifdef RGBA
#undef RGBA
#endif
#endif

#include <algorithm>
#include <string>

// fix MSC8 beta problem....
#ifdef COMPILER_MSC
namespace std {
inline void __cdecl _Debug_message(const wchar_t *, const wchar_t *, unsigned int line) {}
};
#endif

namespace Upp {};

#ifdef flagNONAMESPACE
#define NAMESPACE_UPP
#define END_UPP_NAMESPACE
#define UPP
#else
#define NAMESPACE_UPP     namespace Upp {
#define END_UPP_NAMESPACE };
#define UPP               Upp
#endif

NAMESPACE_UPP

#include <Core/Defs.h>

END_UPP_NAMESPACE

#ifdef UPP_HEAP
#include <new>

inline void *operator new(size_t size) throw(std::bad_alloc) {
    void *ptr = UPP::MemoryAlloc(size);
    return ptr;
}
inline void operator  delete(void *ptr) throw() {
    UPP::MemoryFree(ptr);
}

inline void *operator new[](size_t size) throw(std::bad_alloc) {
    void *ptr = UPP::MemoryAlloc(size);
    return ptr;
}
inline void operator  delete[](void *ptr) throw() {
    UPP::MemoryFree(ptr);
}

inline void *operator new(size_t size, const std::nothrow_t&) throw() {
    void *ptr = UPP::MemoryAlloc(size);
    return ptr;
}
inline void operator  delete(void *ptr, const std::nothrow_t&) throw() {
    UPP::MemoryFree(ptr);
}

inline void *operator new[](size_t size, const std::nothrow_t&) throw() {
    void *ptr = UPP::MemoryAlloc(size);
    return ptr;
}
inline void operator  delete[](void *ptr, const std::nothrow_t&) throw() {
    UPP::MemoryFree(ptr);
}

#endif

NAMESPACE_UPP

#include "Mt.h"
#include "Global.h"
#include "Topt.h"
#include "Profile.h"
#include "String.h"

#include "CharSet.h"
#include "TimeDate.h"
#include "Path.h"
#include "Stream.h"
#include "Diag.h"

#include "Vcont.h"
#include "BiCont.h"
#include "Index.h"
#include "Map.h"
#include "Tuple.h"
#include "Other.h"
#include "Algo.h"
#include "Vcont.hpp"
#include "Index.hpp"

#include "Value.h"
#include "Gtypes.h"
#include "Color.h"

#include "Uuid.h"
#include "Ptr.h"

#include "Callback.h"
#include "Util.h"

#include "Format.h"
#include "Convert.h"

#include "z.h"
#include "Hash.h"

#include "Parser.h"
#include "XML.h"
#include "Lang.h"
#include "i18n.h"
#include "Topic.h"

#include "App.h"

#include "Xmlize.h"

#include "CoWork.h"

#include "LocalProcess.h"

#include "Win32Util.h"



NTL_MOVEABLE(POINT)
NTL_MOVEABLE(SIZE)
NTL_MOVEABLE(RECT)

END_UPP_NAMESPACE

#if (defined(TESTLEAKS) || defined(HEAPDBG)) && defined(PLATFORM_POSIX) && !defined(PLATFORM_OSX11) && defined(UPP_HEAP)

//Place it to the begining of each file to be the first function called in whole executable...

//$-
struct MemDiagCls {
    MemDiagCls() {
        if(!UPP::sMemDiagInitCount++) UPP::MemoryInitDiagnostics();
    }
    ~MemDiagCls() {
        if(!--UPP::sMemDiagInitCount) UPP::MemoryDumpLeaks();
    }
};
static const MemDiagCls sMemDiagHelper__upp__;
//$+


#endif

//some global definitions

#if !defined(STLPORT) && _MSC_VER < 1600
inline UPP::int64  abs(UPP::int64 x) {
    return x < 0 ? -x : x;
}
#endif

void      RegisterTopic__(const char *topicfile, const char *topic, const char *title, const UPP::byte *data, int len);

typedef HMODULE DLLHANDLE;


DLLHANDLE LoadDll__(UPP::String& fn, const char *const *names, void *const *procs);
void      FreeDll__(DLLHANDLE dllhandle);

#ifndef flagNONAMESPACE
using Upp::byte; // Dirty solution to Windows.h typedef byte...
#endif

#define DLLFILENAME "Kernel32.dll"
#define DLIMODULE   UnicodeWin32
#define DLIHEADER   <Core/Kernel32W.dli>
#include <Core/dli_header.h>

#define DLLFILENAME "Mpr.dll"
#define DLIMODULE   UnicodeWin32Net
#define DLIHEADER   <Core/Mpr32W.dli>
#include <Core/dli_header.h>

#endif //CORE_H
