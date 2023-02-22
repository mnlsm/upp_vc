// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define _ATL_FREE_THREADED
#define _ATL_NO_VARIANT_THROW

#pragma warning( disable:4244)
#pragma warning( disable:4018)

#if defined(_MSC_VER) && (_MSC_VER <= 1200 )
#define _ATL_STATIC_REGISTRY
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#define STRSAFE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS


#define WIN32_LEAN_AND_MEAN

//set socket api select function maxinum sockets for work
//#define FD_SETSIZE 1024


// Change these values to use different versions
#define WINVER          0x0501
#define _WIN32_WINNT    0x0501
#define _WIN32_WINDOWS  0x0500
#define _WIN32_IE       0x0501
#define _RICHEDIT_VER   0x0200


#define _WTL_NO_WTYPES
#define _WTL_NO_CSTRING

#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <Windows.h>
#include <assert.h>

#include <atlbase.h>
#include <atlstr.h>

//#include "wtl\\atlapp.h"

#include <atlcom.h>
#include <atlwin.h>
#include <atlctl.h>
#include <atlhost.h>
#include <atlcoll.h>
#include <atlimage.h>
#include <atlutil.h>
#include <atlenc.h>
#include <atlconv.h>
#include <atlfile.h>
#include <atltime.h>
#include <atlsync.h>
#include <atlimage.h>
#include <ATLComTime.h>

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <algorithm>
#include <memory>

#include <hash_map>
#include <hash_set>



#include <shlobj.h>
#include <shellapi.h>
#pragma comment( lib , "Shell32.lib" )

#define _WSPIAPI_COUNTOF
#include <ws2tcpip.h>
#include <winsock2.h>
#pragma comment( lib , "Ws2_32.lib" )
#include <wininet.h>
#pragma comment( lib , "Wininet.lib" )
#include <Gdiplus.h>
#pragma comment( lib, "gdiplus.lib" )

#pragma comment( lib, "Version.lib" )
#pragma comment( lib, "strsafe.lib" )
#pragma comment( lib, "Wintrust.lib" )
#pragma comment( lib, "Rpcrt4.lib" )
#pragma comment( lib, "Winmm.lib" )




// TODO: reference additional headers your program requires here
