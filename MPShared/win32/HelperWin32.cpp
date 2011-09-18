// Helper.cpp: implementation of the Helper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "Helper.h"
#include <time.h>
#include <olectl.h>

void AnalysisProxySetting(LPCTSTR szProxySetting, TCHAR szServer[], int nMaxSize, int &nPort);

int CanOpenFileForWrite(LPCTSTR szFile, bool bMustExist)
{
	if (bMustExist && !IsFileExist(szFile))
		return ERR_FILE_NOT_EXIST;

	HANDLE		hFile;

	hFile = CreateFile(szFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		SetCustomErrorDesc(OSError().Description());
		return ERR_CUSTOM_ERROR;
	}
	CloseHandle(hFile);

/*	FILE		*fp;

	fp = _tfopen(szFile, _T("r+b"));
	if (!fp)
		return false;

	fclose(fp);*/

	return ERR_OK;
}

void Execute(CBaseWnd *pWnd, LPCTSTR szExe, LPCTSTR szParam)
{
	ShellExecute(pWnd != NULL ? pWnd->GetHandle() : NULL, _T("open"), 
		szExe,
		szParam, NULL, SW_SHOWNORMAL);
}

bool SetClipboardText(CBaseWnd *pWnd, LPCTSTR szText)
{
	return Tobool(CopyTextToClipboard(szText));
}

BOOL SHDeleteFile(LPCTSTR szFile, CBaseWnd *pWndParent)
{
#ifndef _WIN32_WCE
	if (IsWin9xSystem())
	{
		SHFILEOPSTRUCTA	sop;
		string			strFile;

		ConvertStr(szFile, -1, strFile);
		strFile += '\0';

		sop.hwnd = pWndParent->GetHandle();
		sop.wFunc = FO_DELETE;
		sop.pFrom = strFile.c_str();
		sop.pTo = NULL;
		sop.fFlags = FOF_ALLOWUNDO;
		sop.fAnyOperationsAborted = FALSE;
		sop.hNameMappings = NULL;
		sop.lpszProgressTitle = "";

		return (SHFileOperationA(&sop) == 0 && !sop.fAnyOperationsAborted);
	}
	else
#endif
	{
		SHFILEOPSTRUCT	sop;
		tstring			strFile;

		strFile = szFile;
		strFile += _T('\0');

		sop.hwnd = pWndParent->GetHandle();
		sop.wFunc = FO_DELETE;
		sop.pFrom = strFile.c_str();
		sop.pTo = NULL;
		sop.fFlags = FOF_ALLOWUNDO;
		sop.fAnyOperationsAborted = FALSE;
		sop.hNameMappings = NULL;
		sop.lpszProgressTitle = _T("");

		return (SHFileOperation(&sop) == 0 && !sop.fAnyOperationsAborted);
	}
}

BOOL SHCopyFile(LPCTSTR szSrcFile, LPCTSTR szTargFile, CBaseWnd *pWndParent)
{
#ifndef _WIN32_WCE
	if (IsWin9xSystem())
	{
		SHFILEOPSTRUCTA	sop;
		string			strSrc, strTarg;

		ConvertStr(szSrcFile, -1, strSrc);
		ConvertStr(szTargFile, -1, strTarg);
		strSrc += '\0';
		strTarg += '\0';

		sop.hwnd = pWndParent->GetHandle();
		sop.wFunc = FO_COPY;
		sop.pFrom = strSrc.c_str();
		sop.pTo = strTarg.c_str();
		sop.fFlags = FOF_ALLOWUNDO;
		sop.fAnyOperationsAborted = FALSE;
		sop.hNameMappings = NULL;
		sop.lpszProgressTitle = "";

		return (SHFileOperationA(&sop) == 0 && !sop.fAnyOperationsAborted);
	}
	else
#endif
	{
		SHFILEOPSTRUCT	sop;
		tstring			strSrc, strTarg;

		strSrc = szSrcFile;
		strTarg = szTargFile;
		szSrcFile += _T('\0');
		szTargFile += _T('\0');

		sop.hwnd = pWndParent->GetHandle();
		sop.wFunc = FO_COPY;
		sop.pFrom = strSrc.c_str();
		sop.pTo = strTarg.c_str();
		sop.fFlags = FOF_ALLOWUNDO;
		sop.fAnyOperationsAborted = FALSE;
		sop.hNameMappings = NULL;
		sop.lpszProgressTitle = _T("");

		return (SHFileOperation(&sop) == 0 && !sop.fAnyOperationsAborted);
	}
}

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES	0xFFFFFFFF
#endif

BOOL SetFileNoReadOnly(LPCTSTR szFile)
{
	DWORD	dwAttr;
	dwAttr = GetFileAttributes(szFile);

	if (dwAttr != INVALID_FILE_ATTRIBUTES)
	{
		if (IsFlagSet(dwAttr, FILE_ATTRIBUTE_READONLY))
			return SetFileAttributes(szFile, dwAttr & ~FILE_ATTRIBUTE_READONLY);
		else
			return TRUE;
	}
	else
		return FALSE;
}

bool LoadProxySvrFromIE(bool &bUseProxy, tstring &strSvr, int &nPort)
{
	// 取得IE 的代理设置
	// [HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings]
	// REG_SZ: "ProxyServer"="127.1.1.1:80"
	// ftp=192.168.1.12:80;gopher=192.168.1.13:80;http=192.168.1.1:80;https=192.168.1.11:80
	HKEY	hKeyInet;
	bool	bRet = true;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"),
		0, KEY_QUERY_VALUE, &hKeyInet) == ERROR_SUCCESS)
	{
		DWORD		nSize, dwType, dwEnable;
		TCHAR		szBuff[256];
		nSize = sizeof(szBuff);

		// ProxyEnable?
		dwType = REG_DWORD;
		nSize = sizeof(dwEnable);
		if (RegQueryValueEx(hKeyInet, _T("ProxyEnable"), NULL, &dwType, (BYTE *)&dwEnable, &nSize) != ERROR_SUCCESS)
		{
			bRet = false;
			bUseProxy = false;
		}
		else
		{
			bUseProxy = dwEnable != 0;
		}

		// ProxyServer ?
		dwType = REG_SZ;
		nSize = sizeof(szBuff);
		if (RegQueryValueEx(hKeyInet, _T("ProxyServer"), NULL, &dwType, (BYTE *)szBuff, &nSize) != ERROR_SUCCESS)
		{
			bRet = false;
		}
		else
		{
			TCHAR	szServer[256] = _T("");
			TCHAR	szHttp[256];
			LPCTSTR	szBeg, szEnd;
			szBeg = _tcsstr(szBuff, _T("http="));
			if (szBeg)
			{
				szBeg += 5;
				szEnd = _tcschr(szBeg, ';');
				if (szEnd)
					_tcsncpysz_safe(szHttp, CountOf(szHttp), szBeg, (int)(szEnd - szBeg));
				else
					_tcscpy_safe(szHttp, CountOf(szHttp), szBeg);
			}
			else
				_tcscpy_safe(szHttp, CountOf(szHttp), szBuff);

			AnalysisProxySetting(szHttp, szServer, CountOf(szServer), nPort);
			strSvr = szServer;
		}

		RegCloseKey(hKeyInet);
	}
	else
		bRet = false;

	return bRet;
}


static bool FindStringValue(LPCTSTR szBuff, LPCTSTR szName, LPCTSTR szEndTag, tstring &strValue)
{
	LPCTSTR		szBeg, szEnd;

	strValue.resize(0);

	szBeg = _tcsstr(szBuff, szName);
	if (!szBeg)
		return false;

	szBeg += _tcslen(szName);

	szEnd = _tcsstr(szBeg, szEndTag);
	if (!szEnd)
		return false;

	strValue.append(szBeg, szEnd);

	return true;
}

bool LoadProxySvrFromFireFox(bool &bUseProxy, tstring &strSvr, int &nPort)
{
	TCHAR		szFireFoxDir[MAX_PATH];

	if (!SUCCEEDED(SHGetSpecialFolderPath(NULL, szFireFoxDir, CSIDL_APPDATA, FALSE)))
		return false;

	DirStringAddSlash(szFireFoxDir);
	_tcscat_safe(szFireFoxDir, CountOf(szFireFoxDir), _T("Mozilla\\Firefox\\"));
	// C:\Documents and Settings\username\Application Data\Mozilla\Firefox

	tstring		strProfileFile, strProfilePath;
	CProfile	profile;

	strProfileFile = szFireFoxDir;
	strProfileFile += _T("profiles.ini");
	if (!IsFileExist(strProfileFile.c_str()))
		return false;

	profile.Init(strProfileFile.c_str());

	{
		//
		// Get current profile path.
		//

		int		nDefault = 0;
		// Get current profile folder.
		for (int i = 0; i < 10; i++)
		{
			if (1 == profile.GetProfileInt(CStrPrintf(_T("Profile%d"), i).c_str(), _T("Default"), 0))
			{
				// Find the default profile.
				nDefault = i;
				break;
			}
		}

		tstring		strPath = profile.GetProfileString(CStrPrintf(_T("Profile%d"), nDefault).c_str(), _T("Path"), _T(""));
		if (strPath.empty())
			return false;

		if (strPath.size() > 2 && strPath[1] == ':')
			strProfilePath = strPath;
		else
		{
			strProfilePath = szFireFoxDir;
			strProfilePath += strPath;
		}
		DirStringAddSlash(strProfilePath);
	}

	// Get proxy settings.
	tstring		strPrefJSFile;
	tstringex	strPrefJS;

	strPrefJSFile = strProfilePath;
	strPrefJSFile += _T("prefs.js");

	if (!OpenFileToTString(strPrefJSFile.c_str(), strPrefJS))
		return false;

	tstring		strProxy, strPort, strType;

	// user_pref("network.proxy.http", "192.168.1.112");
	// user_pref("network.proxy.http_port", 80);
	// user_pref("network.proxy.type", 1);
	FindStringValue(strPrefJS.c_str(), _T("user_pref(\"network.proxy.http\", \""), _T("\");"), strSvr);
	FindStringValue(strPrefJS.c_str(), _T("user_pref(\"network.proxy.http_port\", "), _T(");"), strPort);
	FindStringValue(strPrefJS.c_str(), _T("user_pref(\"network.proxy.type\", "), _T(");"), strType);

	if (_ttoi(strType.c_str()) == 1)
	{
		nPort = _ttoi(strPort.c_str());
		return true;
	}

	return false;
}


#ifdef _WIN32_WCE
void GetNotepadEditor(tstring &strEditor)
{
	strEditor = _T("\\windows\\pword.exe");
}
#else
void GetNotepadEditor(tstring &strEditor)
{
	TCHAR		szBuffer[MAX_PATH];

	GetWindowsDirectory(szBuffer, MAX_PATH);
	DirStringFormat(szBuffer);

	strEditor = szBuffer;
	strEditor += _T("notepad.exe");
	if (IsFileExist(strEditor.c_str()))
		return;

	strEditor = szBuffer;
	strEditor += _T("system32\\notepad.exe");
	if (IsFileExist(strEditor.c_str()))
		return;

	strEditor = szBuffer;
	strEditor += _T("system\\notepad.exe");
	if (IsFileExist(strEditor.c_str()))
		return;
}
#endif

uint32 GetSecCount()
{
	return GetTickCount() / 1000;
}
