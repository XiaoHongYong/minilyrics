///////////////////////////////////////////////////
// the implementation of mylib

#include "stdafx.h"
#include <tchar.h>
#include <Shlobj.h>
#include "WndAbout.h"

//////////////////////////////////////////////////
// functions


BOOL IsTopmostWindow(HWND hWnd)
{
	DWORD	dwStyleEx;

	dwStyleEx = (DWORD)::GetWindowLong(hWnd, GWL_EXSTYLE);

	return ((dwStyleEx & WS_EX_TOPMOST) == WS_EX_TOPMOST);
}

void TopmostWindow(HWND hwnd, BOOL bTopmost)
{
	DWORD	dwStyleEx;
	if (bTopmost)
	{
		dwStyleEx = (DWORD)::GetWindowLong(hwnd, GWL_EXSTYLE);
		dwStyleEx |= WS_EX_TOPMOST;
		::SetWindowLong(hwnd, GWL_EXSTYLE, dwStyleEx);
		::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
	}
	else
	{
		dwStyleEx = (DWORD)::GetWindowLong(hwnd, GWL_EXSTYLE);
		dwStyleEx &= ~WS_EX_TOPMOST;
		::SetWindowLong(hwnd, GWL_EXSTYLE, dwStyleEx);
		::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
	}
}

// PURPOSE:
//      Activate the windows
void ActivateWindow(HWND hWnd)
{
#ifdef _WINCE_
	SetForegroundWindow(hWnd);
#else
    DWORD       dwTimeOutPrev = 0;

    SendMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    ShowWindow(hWnd, SW_SHOW);

	if (!IsTopmostWindow(hWnd))
	{
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
    // save old
    SystemParametersInfo(/*SPI_GETFOREGROUNDLOCKTIMEOUT*/0x2000, 0, (LPVOID)&dwTimeOutPrev, 0/* SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE*/);
    // set new
    SystemParametersInfo(/*SPI_SETFOREGROUNDLOCKTIMEOUT*/0x2001, 0, (LPVOID)0, SPIF_SENDWININICHANGE/* | SPIF_UPDATEINIFILE*/);

    if (::GetForegroundWindow() != hWnd)
        SetForegroundWindow(hWnd);
    if (IsWindowEnabled(hWnd))
        SetActiveWindow(hWnd);

    // restore old
    SystemParametersInfo(/*SPI_SETFOREGROUNDLOCKTIMEOUT*/0x2001, dwTimeOutPrev, (LPVOID)0, SPIF_SENDWININICHANGE/* | SPIF_UPDATEINIFILE*/);
#endif
//	SetActiveWindow(hWnd);
}

#ifdef _WINCE_

//
// remove it's WS_EX_APPWINDOW AND ADD WS_EX_TOOLWINDOW
BOOL ShowAsToolWindow(HWND hWnd)
{
	return false;
}

// add it's WS_EX_APPWINDOW AND remove WS_EX_TOOLWINDOW
BOOL ShowAsAppWindow(HWND hWnd)
{
	return false;
}

BOOL IsToolWindow(HWND hWnd)
{
	return false;
}

#else
//
// remove it's WS_EX_APPWINDOW AND ADD WS_EX_TOOLWINDOW
BOOL ShowAsToolWindow(HWND hWnd)
{
	DWORD dwStyleEx;
	dwStyleEx = (DWORD)GetWindowLong(hWnd, GWL_EXSTYLE);
	if (!IsFlagSet(dwStyleEx, WS_EX_TOOLWINDOW) && !IsFlagSet(dwStyleEx, WS_EX_PALETTEWINDOW))
	{
		// not showed in task bar, so show it
		ShowWindow(hWnd, SW_HIDE);
		dwStyleEx |= WS_EX_TOOLWINDOW;// | WS_EX_PALETTEWINDOW;
		dwStyleEx &= ~WS_EX_APPWINDOW;
		SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
//		SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		ShowWindow(hWnd, SW_SHOW);
		return TRUE;
	}
	return FALSE;
}

// add it's WS_EX_APPWINDOW AND remove WS_EX_TOOLWINDOW
BOOL ShowAsAppWindow(HWND hWnd)
{
	DWORD dwStyleEx;
	dwStyleEx = (DWORD)GetWindowLong(hWnd, GWL_EXSTYLE);
	if (IsFlagSet(dwStyleEx, WS_EX_TOOLWINDOW) || IsFlagSet(dwStyleEx, WS_EX_PALETTEWINDOW))
	{
		// not showed in task bar, so show it
		ShowWindow(hWnd, SW_HIDE);
		dwStyleEx &= ~(WS_EX_TOOLWINDOW | WS_EX_PALETTEWINDOW);
		dwStyleEx |= WS_EX_APPWINDOW;
		SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
//		SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		ShowWindow(hWnd, SW_SHOW);
		return TRUE;
	}
	return FALSE;
}

BOOL IsToolWindow(HWND hWnd)
{
	DWORD dwStyleEx;
	dwStyleEx = (DWORD)GetWindowLong(hWnd, GWL_EXSTYLE);
	
	if ((dwStyleEx & WS_EX_TOOLWINDOW) == WS_EX_TOOLWINDOW/* | WS_EX_PALETTEWINDOW*/)
		return TRUE;

	return FALSE;
}

static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
	TCHAR szDir[MAX_PATH];
	
	switch(uMsg)
	{
	case BFFM_INITIALIZED:
		{
			LPCTSTR		szInitFolder;

			szInitFolder = (LPCTSTR)pData;

			if (szInitFolder && !IsEmptyString(szInitFolder))
			{
				// WParam is TRUE since you are passing a path.
				// It would be FALSE if you were passing a pidl.
				SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)szInitFolder);
			}
			break;
		}
	case BFFM_SELCHANGED:
		{
			// Set the status window to the currently selected path.
			if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
			{
				SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
			}
			break;
		}
	default:
		break;
	}
	return 0;
}

BOOL BrowserForFolder(HWND hWnd, LPCTSTR szTitle, LPTSTR szPath, LPCTSTR szRootFoler)
{
	LPMALLOC	pMalloc;

	if (SHGetMalloc(&pMalloc) != NOERROR)
	{
		return FALSE;
	}

	BROWSEINFO		bInfo;
	LPITEMIDLIST	pidl;

	ZeroMemory((PVOID)&bInfo, sizeof(bInfo));

	if (szRootFoler != NULL && !IsEmptyString(szRootFoler))
	{
		OLECHAR		olePath[MAX_PATH];
		ULONG		chEaten;
		ULONG		dwAttributes;
		HRESULT		hr;
		LPSHELLFOLDER		pDesktopFolder;

		// Get a pointer to the Desktop's IShellFolder interface.
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
		{
#ifndef UNICODE
			// IShellFolder::ParseDisplayName requires the file name be in Unicode
			MultiByteToWideChar(CP_ACP,
				MB_PRECOMPOSED,
				szRootFoler,
				-1,
				olePath,
				MAX_PATH
			);
#else
			_tcscpy(olePath, szRootFoler);
#endif

			// Convert the path to ITEMIDLIST
			hr = pDesktopFolder->ParseDisplayName(NULL,
				NULL,
				olePath,
				&chEaten,
				&pidl,
				&dwAttributes
			);
			if (FAILED(hr))
				goto BR_FAILED;
			bInfo.pidlRoot = pidl;
		}
	}

	bInfo.hwndOwner = hWnd;
	bInfo.pszDisplayName = szPath;
	bInfo.lpszTitle = szTitle;
#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE 0x0040
#endif
	bInfo.ulFlags = /*BIF_EDITBOX | */BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;//BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;

	bInfo.lpfn = BrowseCallbackProc;	// address of callback function.
	bInfo.lParam = (LPARAM)szPath;		// pass address of object to callback function

	pidl = ::SHBrowseForFolder(&bInfo);
	if (pidl == NULL)
		goto BR_FAILED;

//	m_iImageIndex = bInfo.iImage;

	if (::SHGetPathFromIDList(pidl, szPath) == FALSE)
		goto BR_FAILED;

	pMalloc->Free(pidl);
	pMalloc->Release();
	return TRUE;

BR_FAILED:
	if (pMalloc)
	{
		pMalloc->Free(pidl);
		pMalloc->Release();
	}
	return FALSE;
}

// COMMENT:
//		判断系统是否支持半透明窗口
//		只有Windows2000或以上才支持
BOOL IsLayeredWndSupported()
{
	OSVERSIONINFO		version;

	memset(&version, 0, sizeof(version));
	version.dwOSVersionInfoSize = sizeof(version);

	if (!GetVersionEx(&version))
	{
//		LOGOUT1(LOG_LVL_ERROR, _T("GetVersionEx FAILED! Error Id: %d"), GetLastError);
		return FALSE;
	}

	// Major version: 5, windows 2000 系列
	if (version.dwMajorVersion < 5)
		return FALSE;

	return TRUE;
}

typedef BOOL (WINAPI *FNAlphaBlend)(
  HDC,            // handle to destination DC
  int,            // x-coord of upper-left corner
  int,            // y-coord of upper-left corner
  int,            // destination width
  int,            // destination height
  HDC,            // handle to source DC
  int,            // x-coord of upper-left corner
  int,            // y-coord of upper-left corner
  int,            // source width
  int,            // source height
  BLENDFUNCTION   // alpha-blending function
);

BOOL WinAlphaBlend(
  HDC hdcDest,                 // handle to destination DC
  int nXOriginDest,            // x-coord of upper-left corner
  int nYOriginDest,            // y-coord of upper-left corner
  int nWidthDest,              // destination width
  int nHeightDest,             // destination height
  HDC hdcSrc,                  // handle to source DC
  int nXOriginSrc,             // x-coord of upper-left corner
  int nYOriginSrc,             // y-coord of upper-left corner
  int nWidthSrc,               // source width
  int nHeightSrc,              // source height
  BLENDFUNCTION blendFunction  // alpha-blending function
)
{
	FNAlphaBlend		fnAlphaBlend;
	HMODULE				hModule;

	hModule = GetModuleHandle(_T("Msimg32.dll"));
	if (!hModule)
		hModule = LoadLibrary(_T("Msimg32.dll"));
	fnAlphaBlend = (FNAlphaBlend)GetProcAddress(hModule, "AlphaBlend");
	if (fnAlphaBlend)
	{
		return (*fnAlphaBlend)(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, 
			nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, blendFunction);
	}
	else
	{
		return StretchBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, 
			nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);
	}

	return TRUE;	
}

// 定义SetLayeredWindowAttributes函数
typedef BOOL (WINAPI *FNSetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);

typedef BOOL (WINAPI *FNUpdateLayeredWindow)(
  HWND,             // handle to layered window
  HDC,              // handle to screen DC
  POINT *,          // new screen position
  SIZE *,           // new size of the layered window
  HDC,              // handle to surface DC
  POINT *,          // layer position
  COLORREF,         // color key
  BLENDFUNCTION *,  // blend function
  DWORD             // options
);

//	COMMENT:
//		sets the opacity and transparency color key of a layered window
//	INPUT:
//		crKey,		specifies the color key
//		bAlpha,		value for the blend function, When bAlpha is 0, 
//					the window is completely transparent. 
//					When bAlpha is 255, the window is opaque. 
//		dwFlags		Specifies an action to take. This parameter can be one or 
//					more of the following values. Value Meaning 
//					LWA_COLORKEY Use crKey as the transparency color.  
//					LWA_ALPHA Use bAlpha to determine the opacity of the layered window 
BOOL SetLayeredWindow(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	FNSetLayeredWindowAttributes		fnSetLayeredWindowAttributes;
	HMODULE				hModule;
	DWORD				dwStyle;

	// 首先设置WS_EX_LAYERED属性
	dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	dwStyle |= WS_EX_LAYERED;
	SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle);

	// 调用SetLayeredWindowAttributes设置透明色
	hModule = GetModuleHandle(_T("user32.dll"));
	fnSetLayeredWindowAttributes = (FNSetLayeredWindowAttributes)GetProcAddress(hModule, "SetLayeredWindowAttributes");
	if (fnSetLayeredWindowAttributes)
	{
		return (*fnSetLayeredWindowAttributes)(hWnd, crKey, bAlpha, dwFlags);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL SetLayeredWindowAttributes(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	FNSetLayeredWindowAttributes		fnSetLayeredWindowAttributes;
	HMODULE				hModule;

	// 调用SetLayeredWindowAttributes设置透明色
	hModule = GetModuleHandle(_T("user32.dll"));
	fnSetLayeredWindowAttributes = (FNSetLayeredWindowAttributes)GetProcAddress(hModule, "SetLayeredWindowAttributes");
	if (fnSetLayeredWindowAttributes)
	{
		return (*fnSetLayeredWindowAttributes)(hWnd, crKey, bAlpha, dwFlags);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL UpdateLayeredWindow(
	HWND hwnd,             // handle to layered window
	HDC hdcDst,            // handle to screen DC
	POINT *pptDst,         // new screen position
	SIZE *psize,           // new size of the layered window
	HDC hdcSrc,            // handle to surface DC
	POINT *pptSrc,         // layer position
	COLORREF crKey,        // color key
	BLENDFUNCTION *pblend, // blend function
	DWORD dwFlags          // options
)
{
	FNUpdateLayeredWindow		fnUpdateLayeredWindow;

	HMODULE				hModule;

	// 调用SetLayeredWindowAttributes设置透明色
	hModule = GetModuleHandle(_T("user32.dll"));
	fnUpdateLayeredWindow = (FNUpdateLayeredWindow)GetProcAddress(hModule, "UpdateLayeredWindow");
	if (fnUpdateLayeredWindow)
	{
		return (*fnUpdateLayeredWindow)(hwnd, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend, dwFlags);
	}
	else
		return FALSE;

	return TRUE;
}

void UnSetLayeredWindow(HWND hWnd)
{
	DWORD				dwStyle;

	// 首先设置WS_EX_LAYERED属性
	dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	dwStyle &= ~WS_EX_LAYERED;
	SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle);
}
#endif

BOOL IsChildWnd(HWND hWnd)
{
	DWORD	dwStyleEx;

	dwStyleEx = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
	return (dwStyleEx & WS_CHILD) == WS_CHILD;
}

BOOL HasCaption(HWND hWnd)
{
	DWORD	dwStyleEx;

	dwStyleEx = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
	return (dwStyleEx & WS_CAPTION) == WS_CAPTION;
}

void RemoveCaption(HWND	hWnd)
{
	DWORD	dwStyleEx;

	dwStyleEx = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
	dwStyleEx &= ~WS_CAPTION;
	::SetWindowLong(hWnd, GWL_STYLE, dwStyleEx);
	::SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void AddCaption(HWND	hWnd)
{
	DWORD	dwStyleEx;

	dwStyleEx = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
	if ((dwStyleEx & WS_CAPTION) != WS_CAPTION)
	{
		dwStyleEx |= WS_CAPTION;
		::SetWindowLong(hWnd, GWL_STYLE, dwStyleEx);
		::SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}

BOOL MoveWindowSafely(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	if (IsChildWnd(hWnd))
	{
		POINT	pt;
		HWND	hWndParent;
		hWndParent = GetParent(hWnd);
		if (hWndParent)
		{
			pt.x = X;
			pt.y = Y;
			ScreenToClient(hWndParent, &pt);
			X = pt.x;
			Y = pt.y;
		}
	}

	BOOL		bHasCap;
	BOOL		bRet;

	bHasCap = HasCaption(hWnd);
	if (bHasCap)
		RemoveCaption(hWnd);

	bRet = MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);

	if (bHasCap)
		AddCaption(hWnd);

	return bRet;
}

bool EnableDlgItem(HWND hWnd, int nIDItem, bool bEnable)
{
	return Tobool(::EnableWindow(::GetDlgItem(hWnd, nIDItem), bEnable));
}

bool ShowDlgItem(HWND hWnd, int nIDItem, int nCmdShow)
{
	return Tobool(::ShowWindow(::GetDlgItem(hWnd, nIDItem), nCmdShow));
}

/*

// _T("All supported files (*.lrc; *.txt)\0*.lrc;*.txt\0Lyric File (*.lrc)\0*.LRC\0Text File (*.txt)\0*.TXT\0\0")
BOOL OpenFileDialog(HWND hWnd, HINSTANCE hInst, LPCTSTR szFilter, int nFilterIndex, LPTSTR szFile, int nFileNameBuffSize, LPCTSTR szTitle, DWORD dwFlag)
{
	OPENFILENAME openfile;

	if (!IsFileExist(szFile))
		EmptyStr(szFile);

	openfile.lStructSize = sizeof(openfile);
	openfile.hwndOwner = hWnd;
	openfile.hInstance = hInst;
	openfile.lpstrFilter = szFilter;
	openfile.lpstrCustomFilter = NULL;
	openfile.lpstrFile = szFile;
	openfile.nMaxFile = nFileNameBuffSize;
	openfile.lpstrFileTitle = NULL;
	openfile.nMaxFileTitle = 0;
	openfile.lpstrInitialDir = NULL;// g_szLyricInitDir;
	openfile.lpstrTitle = szTitle;
	openfile.Flags = dwFlag;
	openfile.nFileOffset = 0;
	openfile.nFileExtension = 0;
	openfile.lpstrDefExt = NULL;
	openfile.nFilterIndex = nFilterIndex;
	return GetOpenFileName(&openfile);
}
*/
