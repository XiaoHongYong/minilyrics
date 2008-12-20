/********************************************************************
	Created  :	2003年11月2日 23:24:29
	FileName :	BaseFrmWork.cpp
	Author   :	xhy
	
	Purpose  :	应用程序的基本框架结构定义
*********************************************************************/

#include "stdafx.h"
#include "Utils.h"
#include "App.h"

// 配置文件
class CProfile		g_Profile;

// 日志
class CLog		g_Log;

HINSTANCE		g_hInst = NULL;			// Instance handle of the application

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
	if (IsEmptyString(g_szWorkingFolder))
	{
		assert(g_hInst);

		GetModulePath(g_szWorkingFolder, g_hInst);

		DirStringFormat(g_szWorkingFolder);
	}

	return g_szWorkingFolder;
}

HINSTANCE GetAppInstance()
{
	return g_hInst;
}

void SetAppInstance(HINSTANCE hInst)
{
	g_hInst = hInst;
}
/*
HINSTANCE GetResourceHandle()
{
}

HINSTANCE SetResourceHandle(HINSTANCE hResHandle)
{
}*/

BOOL InitBaseFrameWork(HINSTANCE hInst, LPCTSTR szLogFile, LPCTSTR szProfileName, LPCTSTR szDefAppName)
{
	g_hInst = hInst;

	g_Profile.Init(szProfileName, szDefAppName);
	g_Log.Init(szLogFile, TRUE);

	return TRUE;
}
