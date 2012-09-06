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

void GetAppResourceDir(LPTSTR szWorkingFolder)
{
	if (IsEmptyString(g_szWorkingFolder))
	{
		GetAppResourceDir();
	}

	_tcscpy_safe(szWorkingFolder, MAX_PATH, g_szWorkingFolder);
}

LPCTSTR GetAppResourceDir()
{
	if (IsEmptyString(g_szWorkingFolder))
	{
		assert(g_hInst);

		GetModulePath(g_szWorkingFolder, g_hInst);

		DirStringFormat(g_szWorkingFolder);
	}

	return g_szWorkingFolder;
}

TCHAR			g_szAppDataDir[MAX_PATH] = _T("");

void GetAppDataDir(LPTSTR szAppDataDir, LPCTSTR szDefAppName)
{
	OSVERSIONINFO	osvi;
	bool			bInUserDir = false;

	TCHAR szAppExeName[MAX_PATH];
	if (szDefAppName == NULL)
	{
		TCHAR szAppExe[MAX_PATH];
		GetModuleFileName(NULL, szAppExe, CountOf(szAppExe));
		_tcscpy_safe(szAppExeName, CountOf(szAppExeName), FileGetTitle(szAppExe).c_str());
	}

	osvi.dwOSVersionInfoSize = sizeof(osvi);
	if (GetVersionEx(&osvi))
	{
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 5)
			bInUserDir = true;
	}
	if (bInUserDir)
	{
		tstring		strFile;
		
		//
		// PerUserSettings?
		//
		strFile = GetAppResourceDir();
		strFile += _T("install.ini");
		if (IsFileExist(strFile.c_str()))
		{
			CProfile	file;
			file.Init(strFile.c_str(), _T("install"));
			bInUserDir = file.GetProfileBool(_T("PerUserSettings"), true);
			if (!bInUserDir)
			{
				// To verify that it is writable.
				if (!CanWriteInDir(GetAppResourceDir()))
					bInUserDir = true;
			}
		}
	}
	
	if (!bInUserDir)
	{
		GetAppResourceDir(szAppDataDir);
	}
	else
	{
		if (SUCCEEDED(SHGetSpecialFolderPath(NULL, szAppDataDir, CSIDL_APPDATA, FALSE)))
		{
			DirStringFormat(szAppDataDir);
			_tcscat_safe(szAppDataDir, CountOf(szAppDataDir), szDefAppName);
			if (!IsDirExist(szAppDataDir))
			{
				CreateDirectory(szAppDataDir, NULL);
				DirStringAddSlash(szAppDataDir);
			}
		}
		else
			GetAppResourceDir(szAppDataDir);
	}
}

LPCTSTR GetAppDataDir()
{
	if (IsEmptyString(g_szAppDataDir))
	{
		GetAppDataDir(g_szAppDataDir, NULL);
	}

	return g_szAppDataDir;
}

void GetAppDataDir(TCHAR *szAppDir)
{
	_tcscpy_safe(szAppDir, MAX_PATH, GetAppDataDir());
}

#if defined (_DEBUG) && defined (_WIN32)
tstring GetInstallShareDir()
{
	tstring strShareDir = GetAppResourceDir();

	TCHAR		szFile[MAX_PATH];
	if (GetModuleFileName(GetAppInstance(), szFile, CountOf(szFile)))
	{
		if (_tcsicmp(FileGetName(szFile), _T("MiniLyrics.exe")) == 0)
			strShareDir += _T("..\\Install_ML\\Share\\");
		else if (_tcsicmp(FileGetName(szFile), _T("iPodLyricsDownloader.exe")) == 0)
			strShareDir += _T("..\\Install_iPodLyricsDownloader\\Share\\");
		else if (_tcsicmp(FileGetName(szFile), _T("WPOnline.exe")) == 0)
			strShareDir += _T("..\\Install_WP\\Share\\");
		else
			strShareDir += _T("..\\Install_ZP\\Share\\");
	}

	return strShareDir;
}
#endif

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

static void _invalid_parameter(
						const wchar_t * expression,
						const wchar_t * function, 
						const wchar_t * file, 
						unsigned int line,
						uintptr_t pReserved
						)
{
}

BOOL InitBaseFrameWork(HINSTANCE hInst, LPCTSTR szLogFile, LPCTSTR szProfileName, LPCTSTR szDefAppName)
{
	g_hInst = hInst;

	g_Profile.Init(szProfileName, szDefAppName);
	g_Log.Init(szLogFile);

	GetAppDataDir(g_szAppDataDir, szDefAppName);

	 _set_invalid_parameter_handler(_invalid_parameter);

	return TRUE;
}
