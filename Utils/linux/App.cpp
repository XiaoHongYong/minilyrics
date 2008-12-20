/********************************************************************
	Created  :	2003年11月2日 23:24:29
	FileName :	BaseFrmWork.cpp
	Author   :	xhy
	
	Purpose  :	应用程序的基本框架结构定义
*********************************************************************/

#include "../stdafx.h"
#include "Utils.h"
#include "App.h"

// 配置文件
class CProfile	g_Profile;

// 日志
class CLog		g_Log;

TCHAR			g_szWorkingFolder[MAX_PATH] = _T("");

void SetWorkingFolder(LPCTSTR szWorkingFolder)
{
	// assert(szWorkingFolder && !IsEmptyString(szWorkingFolder));
	_tcscpy_safe(g_szWorkingFolder, MAX_PATH, szWorkingFolder);
	DirStringFormat(g_szWorkingFolder);
}

void GetWorkingFolder(LPTSTR szWorkingFolder)
{
	if (IsEmptyString(g_szWorkingFolder))
	{
		GetWorkingFolder();
	}

	_tcscpy(szWorkingFolder, g_szWorkingFolder);
}

LPCTSTR GetWorkingFolder()
{
	return g_szWorkingFolder;
}

BOOL InitBaseFrameWork(int argc, char *argv[], LPCTSTR szLogFile, LPCTSTR szProfileName, LPCTSTR szDefAppName)
{
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

	g_Log.Init(szLogFile, TRUE);

	return TRUE;
}
