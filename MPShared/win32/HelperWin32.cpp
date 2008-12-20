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
		SetCustomErrorDesc(CWin32Error().Description());
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

void OpenUrl(CBaseWnd *pWnd, LPCTSTR szUrl)
{
	ShellExecute(pWnd->GetHandle(), _T("open"), szUrl, NULL, NULL, SW_SHOWNORMAL);
}

void Execute(CBaseWnd *pWnd, LPCTSTR szExe, LPCTSTR szParam)
{
	ShellExecute(pWnd->GetHandle(), _T("open"), 
		szExe,
		szParam, NULL, SW_SHOWNORMAL);
}

void MLHtmlHelper(CBaseWnd *pWnd, LPCTSTR szFile)
{
	TCHAR		szHelp[MAX_PATH];

	GetWorkingFolder(szHelp);
	_tcscat(szHelp, szFile);

	if (!HtmlHelpOur(GetDesktopWindow(), szHelp, HH_DISPLAY_TOPIC, NULL))
	{
		GetWorkingFolder(szHelp);
		_tcscat(szHelp, _T("MiniLyrics.chm"));
		ShellExecute(pWnd->GetHandle(), _T("open"), szHelp, NULL, NULL, SW_SHOWMAXIMIZED);
	}
}

bool SetClipboardText(CBaseWnd *pWnd, LPCTSTR szText)
{
	return Tobool(CopyTextToClipboard(szText, pWnd->GetHandle()));
}

class CMsgOutDlg : public CBaseDialog
{
public:
	CMsgOutDlg() : CBaseDialog(IDD_MSG)
	{
		m_dwStyle = MB_OK;
	}
	~CMsgOutDlg()
	{
	}

	BOOL OnInitDialog()
	{
		if (!m_strSaveDefault.empty())
		{
			if (g_Profile.GetProfileInt(m_strSaveDefault.c_str(), FALSE))
				CheckButton(IDC_NO_PROMPT, true);
			else
				CheckButton(IDC_NO_PROMPT, false);
		}

		if (!m_strCaption.empty())
			SetWindowText(m_strCaption.c_str());

		SetDlgItemText(IDC_MSG, m_strText.c_str());

		if (IsFlagSet(m_dwStyle, MB_YESNO))
		{
			ShowDlgItem(IDYES, SW_SHOW);
			ShowDlgItem(IDNO, SW_SHOW);
		}
		else if (IsFlagSet(m_dwStyle, MB_OKCANCEL))
		{
			ShowDlgItem(IDOK, SW_SHOW);
			ShowDlgItem(IDCANCEL, SW_SHOW);
		}
		else
		{
			ShowDlgItem(IDOK, SW_SHOW);
		}

		if (!CBaseDialog::OnInitDialog())
			return FALSE;

		return TRUE;
	}

	void OnCommand(UINT uID, UINT nNotifyCode)
	{
		CBaseDialog::OnCommand(uID, nNotifyCode);

		switch (uID)
		{
		case IDYES:
		case IDNO:
#ifdef _WIN32
			EndDialog(m_hWnd, uID);
#endif
			break;
		case IDC_NO_PROMPT:
			if (!m_strSaveDefault.empty())
				g_Profile.WriteProfileInt(m_strSaveDefault.c_str(), IsButtonChecked(IDC_NO_PROMPT));
			break;
		default:
			break;
		}
	}

	tstring		m_strSaveDefault;
	tstring		m_strText;
	tstring		m_strCaption;
	DWORD		m_dwStyle;

};

// 向用户显示消息框
int MessageOut(LPCTSTR lpText, LPCTSTR szSaveDefault,CBaseWnd *pWnd/* = NULL*/, UINT uType/* = MB_ICONINFORMATION*/, LPCTSTR lpCaption/* = NULL*/)
{
	assert(lpText);

	if (pWnd == NULL)
		pWnd = g_mpApp.GetMainWnd();

	if (lpCaption == NULL)
		lpCaption = SZ_APP_NAME;

	if (g_Profile.GetProfileInt(szSaveDefault, FALSE))
	{
		// no prompt
		TCHAR		szKey[256];
		_stprintf(szKey, _T("%sValue"), szSaveDefault);
		return g_Profile.GetProfileInt(szKey, IDOK);
	}

	CMsgOutDlg		dlg;

	dlg.m_dwStyle = uType;
	dlg.m_strText = lpText;
	dlg.m_strCaption = lpCaption;
	dlg.m_strSaveDefault = szSaveDefault;

	return dlg.DoModal(pWnd);
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

void DeleteAllFiles(LPCTSTR szDir, BOOL bIncSubDir, BOOL bDelReadOnlyFile)
{
	HANDLE				hFileFind;
	WIN32_FIND_DATA		FindData;
	TCHAR				szFileName[MAX_PATH];

	_tcscpy(szFileName, szDir);
	DirStringFormat(szFileName);
	_tcscat(szFileName, _T("*.*"));

	hFileFind = FindFirstFile(szFileName, &FindData);
	if (hFileFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (IsFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) && 
			(bIncSubDir) &&
			(_tcscmp(FindData.cFileName, _T(".")) != 0) && 
			(_tcscmp(FindData.cFileName, _T("..")) != 0))
		{
			// search in sub directory
			TCHAR	szSubDir[MAX_PATH];

			_tcscpy(szSubDir, szDir);
			DirStringFormat(szSubDir);
			_tcscat(szSubDir, FindData.cFileName);
			_tcscat(szSubDir, _T(SZ_DIR_SLASH));

			DeleteAllFiles(szSubDir, bIncSubDir, bDelReadOnlyFile);
			RemoveDirectory(szSubDir);
		}
		else if (!IsFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
		{
			TCHAR				szFile[MAX_PATH];
			_tcscpy(szFile, szDir);
			DirStringFormat(szFile);
			_tcscat(szFile, FindData.cFileName);
			if (!DeleteFile(szFile) && bDelReadOnlyFile)
			{
				SetFileNoReadOnly(szFile);
				DeleteFile(szFile);
			}
		}
	}
	while (FindNextFile(hFileFind, &FindData));

	FindClose(hFileFind);
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
					_tcsncpysz_safe(szHttp, CountOf(szBuff), szBeg, (int)(szEnd - szBeg));
				else
					_tcscpy(szHttp, szBeg);
			}
			else
				_tcscpy(szHttp, szBuff);

			AnalysisProxySetting(szHttp, szServer, CountOf(szServer), nPort);
			strSvr = szServer;
		}

		RegCloseKey(hKeyInet);
	}
	else
		bRet = false;

	return bRet;
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
