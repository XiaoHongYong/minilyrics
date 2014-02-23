/********************************************************************
	Created  :	2003/11/2	23:28
	FileName :	App.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#pragma once

#include "Utils.h"
#include "LogAlias.h"

//
// golobal variable
//

// Configuration file
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
