#include "../stdafx.h"
#include "../stringex_t.h"
#include "../CharEncoding.h"
#include "../misc.h"

int GetOperationSystemType()
{
#ifdef WIN32
	OSVERSIONINFO		version;

	memset(&version, 0, sizeof(version));
	version.dwOSVersionInfoSize = sizeof(version);

	if (!GetVersionEx(&version))
	{
		// LOG1(LOG_LVL_ERROR, _T("GetVersionEx FAILED! Error Id: %d"), GetLastError);
		return OPS_UNKNOWN;
	}

	// Major version: 5, windows 2000
	if (version.dwMajorVersion == 3)
	{
		if (version.dwMinorVersion == 51)
			return OPS_WINNT351;
	}
	else if (version.dwMajorVersion == 4)
	{
		if (version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			if (version.dwMinorVersion == 0)
				return OPS_WIN95;
			else if (version.dwMinorVersion == 10)
				return OPS_WIN98;
			else if (version.dwMinorVersion == 90)
				return OPS_WINME;
			else
				return OPS_WIN9XMORE;
		}
		else if (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			if (version.dwMinorVersion == 0)
				return OPS_WINNT4;
		}
	}
	else if (version.dwMajorVersion == 5)
	{
		if (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			if (version.dwMinorVersion == 0)
				return OPS_WIN2000;
			else if (version.dwMinorVersion == 1)
				return OPS_WINXP;
			else
				return OPS_WINXPMORE;
		}
	}
	else
		return OPS_WINXPMORE;
#endif

#ifdef _LINUX
	return OPS_LINUX;
#endif

#ifdef _MAC_OSX
	return OPS_MACOSX;
#endif

	return OPS_UNKNOWN;
}

bool IsWin9xSystem()
{
	return (GetOperationSystemType() <= OPS_WIN9XMORE);
}

#ifndef _WIN32

#if defined(_MAC)

#include <sys/time.h>

uint32 GetTickCount()
{
	timeval tim;
	gettimeofday(&tim, NULL);
	return (uint32)(tim.tv_sec * 1000 + tim.tv_usec / 1000);
}

#else // defined(_MAC)

uint32 GetTickCount()
{
	tms tm;
	return times(&tm);
}

bool CopyTextToClipboard(LPCTSTR szText)
{
	return false;
}

bool GetClipBoardText(tstringex &str)
{
	return false;
}

#endif

bool GetMonitorRestrictRect(const RECT &rcIn, RECT &rcRestrict)
{
	return false;
}

void Sleep(DWORD dwMilliseconds)
{
	usleep((unsigned int)dwMilliseconds);
}

#endif

/*

BOOL EnumIPAddr(vector<string> &vIps)
{
	//////////////////
	// Get host name.
	//
	char hostname[256];
	int res = gethostname(hostname, sizeof(hostname));
	if (res != 0)
	{
		ERRLOG1(_T("gethostname FAILED! error: %d"), WSAGetLastError());
		return FALSE;
	}

	////////////////
	// Get host info for hostname. 
	//
	hostent* pHostent = gethostbyname(hostname);
	if (pHostent==NULL)
	{
		ERRLOG1(_T("gethostbyname FAILED! error: %d"), WSAGetLastError());
		return FALSE;
	}

	//////////////////
	// Parse the hostent information returned
	//
	hostent& he = *pHostent;
	//printf("name=%s\naliases=%s\naddrtype=%d\nlength=%d\n",
	//	he.h_name, he.h_aliases, he.h_addrtype, he.h_length);
	
	sockaddr_in sa;
	for (int nAdapter=0; he.h_addr_list[nAdapter]; nAdapter++) {
		memcpy ( &sa.sin_addr.s_addr, he.h_addr_list[nAdapter],he.h_length);
		// printf("Address: %s\n", inet_ntoa(sa.sin_addr)); // display as string
		USES_CONVERSION;
		vIps.push_back(A2T(inet_ntoa(sa.sin_addr)));
		DBGLOG1(_T("Ip of the host: %s"), vIps.back().c_str());
	}
	
	return TRUE;
}
*/
/*

void DrawBlueCtrl(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	char		szString[256];

	GetWindowText(lpDrawItemStruct->hwndItem, szString, 256);

	SetTextColor(lpDrawItemStruct->hDC, RGB(0, 0, 255));
	SetBkMode(lpDrawItemStruct->hDC, TRANSPARENT);
	DrawText(lpDrawItemStruct->hDC, szString, strlen(szString), &lpDrawItemStruct->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}
*/

#ifdef WIN32

BOOL ExecuteCmdAndWait(LPCTSTR szCmd, DWORD dwTimeOut, DWORD *pExitCode)
{
	STARTUPINFO			startInfo;
	PROCESS_INFORMATION	procInfo;
	BOOL				bRet = TRUE;

	memset(&startInfo, 0, sizeof(startInfo));
	memset(&procInfo, 0, sizeof(procInfo));

	if (!CreateProcess(NULL, (LPTSTR)szCmd, NULL, NULL, false, 0, NULL, NULL, &startInfo, &procInfo))
		return FALSE;

	if (dwTimeOut != 0 && WaitForSingleObject(procInfo.hProcess, dwTimeOut) == WAIT_TIMEOUT)
	{
		TerminateProcess(procInfo.hProcess, 0);
		bRet = FALSE;
	}

	if (pExitCode && bRet)
		GetExitCodeProcess(procInfo.hProcess, pExitCode);

	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);

	return bRet;
}

// PURPOSE   : Get full task save file name by task name.
//
// PARAMETER: 
//      szPath is the path that returned: format is like : "c:\\antivirus\\"
BOOL GetModulePath(TCHAR szPath[], HINSTANCE hInstance/* = NULL*/)
{
    __try
    {
        int nLen;

		EmptyStr(szPath);

        nLen = GetModuleFileName(hInstance, szPath, MAX_PATH);
        nLen --;
        while (nLen >= 0 && szPath[nLen] != DIR_SLASH)
            nLen --;
        if (nLen < 0)
            return FALSE;
        
        szPath[nLen + 1] = '\0';
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }
    
    return TRUE;
}

bool GetMonitorRestrictRect(const RECT &rcIn, RECT &rcRestrict)
{
	if (GetOperationSystemType() >= OPS_WIN98)
	{
typedef struct tagMONITORINFO {
  DWORD  cbSize; 
  RECT   rcMonitor; 
  RECT   rcWork; 
  DWORD  dwFlags; 
} MONITORINFO, *LPMONITORINFO;
typedef BOOL (WINAPI *FUNGetMonitorInfo)(HANDLE hMonitor, LPMONITORINFO lpmi);
typedef HANDLE (WINAPI *FUNMonitorFromRect)(LPCRECT lprc, DWORD dwFlags);
#ifndef MONITOR_DEFAULTTONULL
#define MONITOR_DEFAULTTONULL	0
#endif
		static FUNMonitorFromRect	funMonitorFromRect = NULL;
		static FUNGetMonitorInfo	funGetMonitorInfo = NULL;

		if (!funMonitorFromRect || !funGetMonitorInfo)
		{
			static HMODULE		hModule = NULL;
			if (!hModule)
				hModule = LoadLibrary(_T("user32.dll"));
			if (hModule)
			{
#ifdef _WINCE_
				funMonitorFromRect = (FUNMonitorFromRect)GetProcAddress(hModule, _T("MonitorFromRect"));
				funGetMonitorInfo = (FUNGetMonitorInfo)GetProcAddress(hModule, _T("GetMonitorInfoA"));
#else
				funMonitorFromRect = (FUNMonitorFromRect)GetProcAddress(hModule, "MonitorFromRect");
				funGetMonitorInfo = (FUNGetMonitorInfo)GetProcAddress(hModule, "GetMonitorInfoA");
#endif
			}
		}
		if (funMonitorFromRect && funGetMonitorInfo)
		{
			HANDLE		hMonitor;
			MONITORINFO	monitorInfo;
			monitorInfo.cbSize = sizeof(monitorInfo);
			hMonitor = funMonitorFromRect(&rcIn, MONITOR_DEFAULTTONULL);
			if (hMonitor)
			{
				funGetMonitorInfo(hMonitor, &monitorInfo);
				rcRestrict = monitorInfo.rcMonitor;
				return;
			}
		}
	}

	rcRestrict.top = rcRestrict.left = 0;
	rcRestrict.bottom = GetSystemMetrics(SM_CYSCREEN);
	rcRestrict.right = GetSystemMetrics(SM_CXSCREEN);
	
	return true;
}

void CenterWindowToMonitor(HWND hWnd)
{
	RECT	rc, rcRestrict;
	int		w, h;

	GetWindowRect(hWnd, &rc);
	GetMonitorRestrictRect(rc, rcRestrict);

	w = rc.right - rc.left;
	h = rc.bottom - rc.top;

	rc.left = rcRestrict.left + (rcRestrict.right - rcRestrict.left - w) / 2;
	rc.top = rcRestrict.top + (rcRestrict.bottom - rcRestrict.top - h) / 2;

	SetWindowPos(hWnd, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

bool CopyTextToClipboard(LPCTSTR szText)
{
	int		nLen;
    HGLOBAL hglbCopy;
	LPTSTR	lptstrCopy;
	
    if (!OpenClipboard(NULL))
        return FALSE;
    EmptyClipboard();
	
	nLen = _tcslen(szText);
	
    hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (nLen + 1) * sizeof(TCHAR));
	
	if (hglbCopy == NULL) 
    {
        CloseClipboard();
        return FALSE;
    }
	
	lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	memcpy(lptstrCopy, szText,
		nLen * sizeof(TCHAR));
	lptstrCopy[nLen] = (TCHAR) 0;    // null character
	GlobalUnlock(hglbCopy);
	
	// Place the handle on the clipboard. 
	
#ifdef _UNICODE
	SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
	SetClipboardData(CF_TEXT, hglbCopy);
#endif

	CloseClipboard();

	return TRUE;
}

bool GetClipBoardText(tstringex &str)
{
	HANDLE		handle;
	OpenClipboard(NULL);
	handle = GetClipboardData(CF_UNICODETEXT);
	if (handle)
	{
#ifdef _UNICODE
		str = (LPCWSTR)GlobalLock(handle);
#else
		UCS2ToTString((LPCWSTR)GlobalLock(handle), -1, str);
#endif
		GlobalUnlock(handle);
	}
	else
	{
		handle = GetClipboardData(CF_TEXT);
		if (handle)
		{
			ConvertStr((LPCSTR)GlobalLock(handle), -1, str);
			GlobalUnlock(handle);
		}
	}
	CloseClipboard();

	return handle != NULL;
}

#elif (defined(_LINUX))// _WIN32

bool CopyTextToClipboard(LPCTSTR szText)
{
	GtkClipboard	*clipBoard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	if (!clipBoard)
		return false;

	gtk_clipboard_set_text(clipBoard, szText, _tcslen(szText));

	return true;
}

bool GetClipBoardText(tstringex &str)
{
	return false;
}

#endif

/*

BOOL EnumIPAddr(vector<string> &vIps)
{
	//////////////////
	// Get host name.
	//
	char hostname[256];
	int res = gethostname(hostname, sizeof(hostname));
	if (res != 0)
	{
		ERRLOG1(_T("gethostname FAILED! error: %d"), WSAGetLastError());
		return FALSE;
	}

	////////////////
	// Get host info for hostname. 
	//
	hostent* pHostent = gethostbyname(hostname);
	if (pHostent==NULL)
	{
		ERRLOG1(_T("gethostbyname FAILED! error: %d"), WSAGetLastError());
		return FALSE;
	}

	//////////////////
	// Parse the hostent information returned
	//
	hostent& he = *pHostent;
	//printf("name=%s\naliases=%s\naddrtype=%d\nlength=%d\n",
	//	he.h_name, he.h_aliases, he.h_addrtype, he.h_length);
	
	sockaddr_in sa;
	for (int nAdapter=0; he.h_addr_list[nAdapter]; nAdapter++) {
		memcpy ( &sa.sin_addr.s_addr, he.h_addr_list[nAdapter],he.h_length);
		// printf("Address: %s\n", inet_ntoa(sa.sin_addr)); // display as string
		USES_CONVERSION;
		vIps.push_back(A2T(inet_ntoa(sa.sin_addr)));
		DBGLOG1(_T("Ip of the host: %s"), vIps.back().c_str());
	}
	
	return TRUE;
}
*/

/*

#define BMPWIDTHBYTES(dwWidth, dwBitCount)	( ( dwWidth * dwBitCount + 7 ) / 8 + 3 ) & ~3

HBITMAP LoadBmpFile(LPCTSTR szFile)
{
	DWORD				dwQUADSize = 0;
	LPBITMAPINFO		lpbi = NULL;				//BITMAPINFO (include infoheader and RGBQUAD)
	LPBITMAPINFOHEADER	lpbih = NULL;
	HANDLE				hFile;
	DWORD				dwFileLen, dwBytesRead;
	BYTE				*lpbBmpData = NULL;
	DWORD				dwBiSize, dwDataSize;
	HDC					hdc = NULL;
	HBITMAP				hBmp = NULL;

	//
	// first Load bitmap from file to memory
	// 

	lpbi = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
	lpbih = &lpbi->bmiHeader;

	hFile = CreateFile(szFile, GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
		OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		ERRLOG1(_T("Open File:%s FAILED!"), szFile);
		return NULL;
	}

	dwFileLen = GetFileSize(hFile, NULL);

	if (dwFileLen <= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
	{
		ERRLOG1(_T("File:%s is not a Bitmap file. File Size is less than BMP File Header size!"), szFile);
		goto FAILED;
	}

	// Read BITMAPFILEHEADER and judge whether it is a bmp file?
	BITMAPFILEHEADER bfh;

	if (!ReadFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &dwBytesRead, NULL))
		goto READ_FILE_FAILED;

	WORD	wFlagBM;
	wFlagBM = MAKEWORD((BYTE)'B',(BYTE)'M');
	if (bfh.bfType != wFlagBM)
	{
		ERRLOG1(_T("File:%s is not a Bitmap file. File Header does not match TAG:BM!"), szFile);
		goto FAILED;
	}

	if (! ReadFile(hFile, lpbih, sizeof(BITMAPINFOHEADER), &dwBytesRead, NULL))
		goto READ_FILE_FAILED;

	switch (lpbih->biBitCount)
	{
	case 32:
	case 24:
	case 16:
		dwQUADSize = 0;//sizeof(RGBQUAD);
		break;
	case 8:								
		dwQUADSize = sizeof(RGBQUAD) * (lpbih->biClrUsed ? lpbih->biClrUsed : 256);
		break;
	case 4:
		dwQUADSize = sizeof(RGBQUAD) * (lpbih->biClrUsed ? lpbih->biClrUsed : 16);
		break;
	case 2:
		dwQUADSize = sizeof(RGBQUAD) * (lpbih->biClrUsed ? lpbih->biClrUsed : 4);
		break;
	case 1:
		dwQUADSize = sizeof(RGBQUAD) * (lpbih->biClrUsed ? lpbih->biClrUsed : 2);
		break;
	}
	dwBiSize = sizeof(BITMAPINFOHEADER) + dwQUADSize;
	dwDataSize = dwFileLen - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER) - dwQUADSize;

	lpbBmpData = new BYTE[dwDataSize];
	if (lpbBmpData == NULL)
	{
		ERRLOG1(_T("Alloc %d size of Memory FAILED!"), dwDataSize);
		goto FAILED;
	}

	if (! ReadFile(hFile, lpbi->bmiColors, dwQUADSize, &dwBytesRead, NULL))
		goto READ_FILE_FAILED;
	if (! ReadFile(hFile, lpbBmpData, dwDataSize, &dwBytesRead, NULL))
		goto READ_FILE_FAILED;

	//
	// load bitmap from memory to HBITMAP
	//
	hdc = GetDC(NULL);

	hBmp = CreateCompatibleBitmap(hdc, lpbih->biWidth, lpbih->biHeight);
	if (hBmp == NULL)
	{
		ERRLOG0(_T("CreateCompatibleBitmap FAILED!"));
		goto FAILED;
	}
	
	if (!SetDIBits(hdc, hBmp, 0, lpbih->biHeight, lpbBmpData, lpbi, DIB_RGB_COLORS))
	{
		ERRLOG0(_T("SetDIBits FAILED!"));
		goto FAILED;
	}

	CloseHandle(hFile);

	ReleaseDC(NULL, hdc);

	delete [](BYTE *)lpbi;

	delete [] lpbBmpData;

	return hBmp;

READ_FILE_FAILED:
	ERRLOG1(_T("Read File:%s FAILED!"), szFile);
FAILED:
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	if (lpbi)
		delete [](BYTE *)lpbi;

	if (lpbBmpData)
		delete [] lpbBmpData;

	if (hdc)
		ReleaseDC(NULL, hdc);

	if (hBmp)
		DeleteObject(hBmp);

	return NULL;
}

*/
