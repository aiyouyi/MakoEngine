#ifndef __COMMON_H__
#define __COMMON_H__

#include "win/win32.h"
#include <DShow.h>
#include "qedit.h"
#include <strsafe.h>
#include <assert.h>
#pragma comment(lib,"Strmiids.lib")

#undef IsMaximized
#undef IsMinimized

//define release maco
#define ReleaseInterface(x) \
	if ( NULL != x ) \
{ \
	x->Release( ); \
	x = NULL; \
}
// Application-defined message to notify app of filter graph events
#define WM_GRAPHNOTIFY  WM_APP+100

void Msg(HWND hwnd,TCHAR *szFormat, ...);
bool Bstr_Compare(BSTR bstrFilter,BSTR bstrDevice);


#ifdef WINDOWS_PLATFORM_TYPES_GUARD
#include "Windows/HideWindowsPlatformTypes.h"
#endif


#endif// __COMMON_H__