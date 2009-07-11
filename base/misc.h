// xstr.h: interface for the Cxstr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_ML_MISC_INC_)
#define _ML_MISC_INC_

//
// 操作系统的类型定义
//
enum OPERATION_SYSTEM_TYPE
{
	OPS_WIN95,
	OPS_WINNT4,
	OPS_WIN98,
	OPS_WINME,
	OPS_WIN9XMORE,
	OPS_WINNT351,
	OPS_WIN2000,
	OPS_WINXP,
	OPS_WINXPMORE,
	OPS_UNKNOWN,

	OPS_LINUX,
	OPS_MACOSX,
};

// 取得操作系统的类型
int GetOperationSystemType();

bool IsWin9xSystem();

#ifndef _WIN32

// return system elapsed time in ms.
uint32 GetTickCount();

void Sleep(DWORD dwMilliseconds);

#endif

#ifdef WIN32

BOOL GetModulePath(TCHAR szPath[], HINSTANCE hInstance = NULL);

HWND
WINAPI
HtmlHelpOurA(
    HWND hwndCaller,
    LPCSTR pszFile,
    UINT uCommand,
    LPVOID dwData
    );

HWND
WINAPI
HtmlHelpOurW(
    HWND hwndCaller,
    LPWSTR pszFile,
    UINT uCommand,
    LPVOID dwData
    );

#ifdef _UNICODE
#define HtmlHelpOur		HtmlHelpOurW
#else
#define HtmlHelpOur		HtmlHelpOurA
#endif

BOOL InitRichEdit();

void GetMonitorRestrictRect(const RECT &rcIn, RECT &rcRestrict);

void CenterWindowToMonitor(HWND hWnd);

BOOL CopyTextToClipboard(LPCTSTR szText, HWND hWnd);

bool GetClipBoardText(wstringex &str, HWND hWnd);

#endif // WIN32

#endif // !defined(_ML_MISC_INC_)
