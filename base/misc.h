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

bool GetMonitorRestrictRect(const RECT &rcIn, RECT &rcRestrict);


#ifndef _WIN32

// return system elapsed time in ms.
uint32 GetTickCount();

void Sleep(DWORD dwMilliseconds);

#endif

#ifdef WIN32

BOOL ExecuteCmdAndWait(LPCTSTR szCmd, DWORD dwTimeOut = INFINITE, DWORD *pExitCode = NULL);

BOOL GetModulePath(TCHAR szPath[], HINSTANCE hInstance = NULL);

void CenterWindowToMonitor(HWND hWnd);

bool CopyTextToClipboard(LPCTSTR szText);

bool GetClipBoardText(tstringex &str);

#else

bool CopyTextToClipboard(LPCTSTR szText);

bool GetClipBoardText(tstringex &str);

#endif // WIN32

#endif // !defined(_ML_MISC_INC_)
