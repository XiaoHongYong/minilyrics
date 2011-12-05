// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__57A8F822_10B6_4939_B51A_4E1E84678795__INCLUDED_)
#define AFX_STDAFX_H__57A8F822_10B6_4939_B51A_4E1E84678795__INCLUDED_

#pragma once

#ifdef _WIN32_WCE

#pragma comment(linker, "/nodefaultlib:libc.lib")
#pragma comment(linker, "/nodefaultlib:libcd.lib")

// NOTE - this value is not strongly correlated to the Windows CE OS version being targeted
#define WINVER _WIN32_WCE

#include <ceconfig.h>
#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#define SHELL_AYGSHELL
#endif

#ifdef _CE_DCOM
#define _ATL_APARTMENT_THREADED
#endif

#include <aygshell.h>
#pragma comment(lib, "aygshell.lib") 


// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#ifndef _DEVICE_RESOLUTION_AWARE
#define _DEVICE_RESOLUTION_AWARE
#endif
#endif

#ifdef _DEVICE_RESOLUTION_AWARE
#include "DeviceResolutionAware.h"
#endif

#if _WIN32_WCE < 0x500 && ( defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP) )
#pragma comment(lib, "ccrtrtti.lib")
#ifdef _X86_	
#if defined(_DEBUG)
#pragma comment(lib, "libcmtx86d.lib")
#else
#pragma comment(lib, "libcmtx86.lib")
#endif
#endif
#endif

#include <altcecrt.h>

#endif // _WIN32_WCE

#include "../base/base.h"
#include "../Utils/Utils.h"
#include "../OurMFCLib/OurMFClib.h"
#include "../GfxLite/GfxLite.h"
#include "../WidgetLite/WidgetLite.h"
#include "../Skin/Skin.h"
#include "../MLLib/MLLib.h"
#include "../MPlayerLib/IMPlayer.h"

#ifdef _WIN32
#include "../widget/widget.h"
#endif

// Local Header Files

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__57A8F822_10B6_4939_B51A_4E1E84678795__INCLUDED_)
