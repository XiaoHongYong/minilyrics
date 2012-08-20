/********************************************************************
	Created  :	2003/11/2	23:28
	FileName :	App.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/


#ifndef _FRAME_WORK_2003_11_2_23_28INCLUDED_
#define _FRAME_WORK_2003_11_2_23_28INCLUDED_

#include "log.h"
#include "LogAlias.h"

//
// golobal variable
//

// ≈‰÷√Œƒº˛
extern class CProfile			g_Profile;

void GetAppResourceDir(LPTSTR szWorkingFolder);
LPCTSTR GetAppResourceDir();

void GetAppDataDir(TCHAR *szAppDir);
LPCTSTR GetAppDataDir();

void SetWorkingFolder(LPCTSTR szWorkingFolder);

#ifdef _WIN32

#if defined (_DEBUG) && defined (_WIN32)
tstring GetInstallShareDir();
#endif

HINSTANCE GetAppInstance();
void SetAppInstance(HINSTANCE hInst);

HINSTANCE GetResourceHandle();
HINSTANCE SetResourceHandle(HINSTANCE hResHandle);

BOOL InitBaseFrameWork(HINSTANCE hInst, LPCTSTR szLogFile, LPCTSTR szProfileName, LPCTSTR szDefAppName = NULL);
#else
BOOL InitBaseFrameWork(int argc, char *argv[], LPCTSTR szLogFile, LPCTSTR szProfileName, LPCTSTR szDefAppName = NULL);
#endif

#endif // _FRAME_WORK_2003_11_2_23_28INCLUDED_
