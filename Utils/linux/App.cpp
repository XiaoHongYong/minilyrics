/********************************************************************
	Created  :	2003年11月2日 23:24:29
	FileName :	BaseFrmWork.cpp
	Author   :	xhy
	
	Purpose  :	应用程序的基本框架结构定义
*********************************************************************/

#include "../stdafx.h"
#include "../Utils.h"
#include "../App.h"

// 配置文件
class CProfile	g_Profile;

// 日志
class CLog		g_Log;

TCHAR			g_szWorkingFolder[MAX_PATH] = _T("");
TCHAR			g_szAppDataDir[MAX_PATH] = _T("");

void SetWorkingFolder(LPCTSTR szWorkingFolder)
{
	// assert(szWorkingFolder && !IsEmptyString(szWorkingFolder));
	_tcscpy_safe(g_szWorkingFolder, MAX_PATH, szWorkingFolder);
	DirStringFormat(g_szWorkingFolder);
}

void GetAppResourceDir(LPTSTR szWorkingFolder)
{
	if (IsEmptyString(g_szWorkingFolder))
	{
		GetAppResourceDir();
	}

	_tcscpy(szWorkingFolder, g_szWorkingFolder);
}

LPCTSTR GetAppResourceDir()
{
	return g_szWorkingFolder;
}

LPCTSTR GetAppDataDir()
{
	if (IsEmptyString(g_szAppDataDir))
	{
		GetAppResourceDir(g_szAppDataDir);
	}
	
	return g_szAppDataDir;
}

void GetAppDataDir(TCHAR *szAppDir)
{
	_tcscpy_safe(szAppDir, MAX_PATH, GetAppDataDir());
}

void InitSetDefaultCharEncoding();

#ifdef _ANDROID

BOOL InitBaseFrameWork(LPCTSTR szWorkingFolder, LPCTSTR szLogTag, LPCTSTR szProfileName, LPCTSTR szDefAppName)
{
	assert(sizeof(byte) == 1);
	assert(sizeof(WORD) == 2);
	assert(sizeof(DWORD) == 4);
	assert(sizeof(int16) == 2);
	assert(sizeof(int32) == 4);
	assert(sizeof(int64) == 8);
	assert(sizeof(WCHAR) == 2);
	assert(sizeof(WCHAR) == 2);

	_tcscpy_safe(g_szAppDataDir, CountOf(g_szAppDataDir), szWorkingFolder);
	_tcscpy_safe(g_szWorkingFolder, CountOf(g_szWorkingFolder), szWorkingFolder);
	DirStringAddSlash(g_szWorkingFolder);
	DirStringAddSlash(g_szAppDataDir);

	g_Profile.Init(szProfileName, szDefAppName);

	g_Log.Init(szLogTag);

	return TRUE;
}

#else

BOOL InitBaseFrameWork(int argc, char *argv[], LPCTSTR szLogFile, LPCTSTR szProfileName, LPCTSTR szDefAppName)
{
	assert(sizeof(byte) == 1);
	assert(sizeof(WORD) == 2);
	assert(sizeof(DWORD) == 4);
	assert(sizeof(int16) == 2);
	assert(sizeof(int32) == 4);
	assert(sizeof(int64) == 8);
	assert(sizeof(WCHAR) == 2);
	assert(sizeof(WCHAR) == 2);

	{
		int nLen;

		_tcscpy(g_szWorkingFolder, argv[0]);
		nLen = _tcslen(g_szWorkingFolder);
		nLen --;
		while (nLen >= 0 && g_szWorkingFolder[nLen] != DIR_SLASH)
			nLen --;
		if (nLen < 0)
			return FALSE;

		g_szWorkingFolder[nLen + 1] = '\0';
	}

	g_Profile.Init(szProfileName, szDefAppName);

	g_Log.Init(szLogFile);

	return TRUE;
}

#endif
