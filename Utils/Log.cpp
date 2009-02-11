// Log.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#if defined(_WIN32) && !defined(_WINCE_)
#include <wtypes.h>
#include <oleauto.h>
#include <crtdbg.h>
#include <atldef.h>
#include <atlconv.h>
#include <stdio.h>
#endif
#include <time.h>

#include "log.h"
#include "app.h"

/*

static HWND _hDebugWnd;

#define SENDDEBUGMESSAGE	19771212
#define	DEBUGCLASSNAME		"Debug Tracer Class"
#define	DEBUGWNDNAME		"Debug Tracer"

void LogTrace(LPVOID lpData, int nSize)
{
	COPYDATASTRUCT data;

	data.dwData = SENDDEBUGMESSAGE;
	data.lpData = (LPVOID)lpData;
	data.cbData = nSize;

	if (_hDebugWnd == NULL)
		_hDebugWnd = FindWindow(DEBUGCLASSNAME, DEBUGWNDNAME);

	if (_hDebugWnd)
		::SendMessage(_hDebugWnd, WM_COPYDATA, NULL, (LPARAM)&data);
}


void DBGLOG0(LPCTSTR szData)
{
	TCHAR		szBuffer[1024];
	SYSTEMTIME	LocalTime;
	int		nLen;
	GetLocalTime(&LocalTime);
	nLen = wsprintf(szBuffer, "|T%d-%d %d:%d:%d|M%s", LocalTime.wMonth, LocalTime.wDay,
			LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, szData);

	LogTrace((LPVOID)szBuffer, nLen);
}
	DBGLOG0("|MCWinamp2Player::Quit()");
*/

// determine number of elements in an array (not bytes)


#define SZ_RETURN			_T("\r\n")

// 日志文件的最大长度为1MB
#ifdef _WIN32_WCE
#define MAX_LOG_FILE_LEN	(1024 * 50)
#define	MAX_MSG_LEN			(1024 * 1)
#else
#define	MAX_MSG_LEN			(1024 * 10)
#define MAX_LOG_FILE_LEN	(1024 * 1024)
#endif

#ifdef _WIN32_DESKTOP
#define SENDDEBUGMESSAGE	19771212
#define	DEBUGCLASSNAME		_T("Debug Tracer Class")
#define	DEBUGWNDNAME		_T("Debug Tracer")

static HWND _hDebugWnd;

void LogTrace(LPVOID lpData, int nSize)
{
	COPYDATASTRUCT data;
#if defined(_WIN32) && !defined(_WINCE_)
	USES_CONVERSION;
	LPCSTR	szMsgAssii = T2CA((TCHAR *)lpData);
#else
	LPCSTR	szMsgAssii = (LPCSTR)lpData;
#endif

	data.dwData = SENDDEBUGMESSAGE;
	data.lpData = (LPVOID)szMsgAssii;
	data.cbData = strlen(szMsgAssii) + 1;

	if (_hDebugWnd == NULL)
		_hDebugWnd = FindWindow(DEBUGCLASSNAME, DEBUGWNDNAME);

	if (_hDebugWnd)
		::SendMessage(_hDebugWnd, WM_COPYDATA, NULL, (LPARAM)&data);
}
#endif

#ifdef _LINUX
void LogTrace(LPVOID lpData, int nSize)
{
	g_print("%s\n", lpData);
}
#endif
/*
|F%s|L%d|E%d|M%s
D: Date
F: File
L: Line
E: lEvel  (Error)
M: Message
*/

// #define fwriteWT		fwrite

size_t fwriteWT(TCHAR *szBuffer, size_t n, size_t nLen, FILE *fp)
{
	int		nLenWrite;
#if defined(_WIN32)
#if defined(_WINCE_)
	stringex	str;
	ConvertStr(szBuffer, nLen, str);
	LPCSTR	szMsgAssii = str.c_str();
	nLenWrite = str.size();
#else
	USES_CONVERSION;
	LPCSTR	szMsgAssii = T2CA((TCHAR *)szBuffer);
	nLenWrite = strlen(szMsgAssii);
#endif
#else
	// g_print("%s", szBuffer);
	LPCSTR	szMsgAssii = (LPCSTR)szBuffer;
//	stringex		str;
//	ConvertStr(szBuffer, -1, str);
//	LPCSTR	szMsgAssii = str.c_str();
	nLenWrite = sizeof(TCHAR) * nLen;
#endif
	return fwrite(szMsgAssii, 1, nLenWrite, fp);
}


//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLog::CLog()
{
#if defined(_WIN32) && !defined(_WINCE_)
	m_hWndMessage = NULL;
#endif
	m_fpLog = NULL;
	m_bTryOpenAgain = TRUE;
	m_bAlwaysLogToFile = FALSE;
	m_bEnableLog = TRUE;
	EmptyStr(m_szLogFile);
}

CLog::~CLog()
{
	if (m_fpLog)
		fclose(m_fpLog);
}

void CLog::LogDumpStr(BYTE byLevel, LPCTSTR szFile, int nLine, LPCSTR szStr, int nStrLen)
{
#ifndef _DEBUG
	if (!m_bEnableLog)
		return;
#endif

	char		*szMsgAssii = new char[nStrLen + 1];
	assert(szMsgAssii);

	strncpy_safe(szMsgAssii, nStrLen + 1, szStr, nStrLen);
	szMsgAssii[nStrLen] = '\0';
#if defined(_WIN32) && !defined(_WINCE_)
	USES_CONVERSION;
	const TCHAR	*szMsg = A2CT(szMsgAssii);
#else
	const TCHAR	*szMsg = LPCTSTR(szMsgAssii);
#endif

	WriteLog(byLevel, szFile, nLine, _T("%s"), szMsg);

	delete[] szMsgAssii;
}

void CLog::WriteLog(BYTE byLevel, LPCTSTR szFile, int nLine, LPCTSTR szFormat, ...)
{
#ifndef _DEBUG
	if (!m_bEnableLog)
		return;

	// 将发行版本中的调试信息去掉
	if (byLevel == LOG_LVL_DEBUG)
		return;
#endif	// _DEBUG

	va_list		args;

	int			nBuf;
	TCHAR		szBuffer[MAX_MSG_LEN] = _T("");
	TCHAR		*szMsg = _T("");
	int			nLen = 0;

#ifdef _WIN32_WCE
	SYSTEMTIME	time;

	GetLocalTime(&time);
#else
	time_t		ltime;
	tm			*ptm;

	time(&ltime);
	ptm = localtime(&ltime);
#endif

#if defined(_WIN32) && !defined(_WINCE_)
	__try
#endif
	{
		nLen = _stprintf_s(szBuffer, CountOf(szBuffer), _T("|T%d-%d %d:%d:%d|F%s|L%d|E%d|M"), 
#ifdef _WINCE_
			time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond,
#else
			ptm->tm_mon + 1, ptm->tm_mday,ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
#endif
			szFile, nLine, byLevel);

		// 格式化日志
		va_start(args, szFormat);
		szMsg = szBuffer + nLen;
		nBuf = _vsntprintf_s(szMsg, CountOf(szBuffer) - nLen, CountOf(szBuffer) - nLen, szFormat, args);
		va_end(args);

		// was there an error? was the expanded string too long?
		if (nBuf < 0)
			_tcscpy_safe(szMsg, CountOf(szBuffer) - nLen, _T("Insufficient log buffer space."));
	}
#if defined(_WIN32) && !defined(_WINCE_)
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// 有异常发生！
		_tcscpy_safe(szBuffer + nLen, CountOf(szBuffer) - nLen, _T("Exception error in format."));
	}
#endif

	nLen = (int)_tcslen(szBuffer) + 1;

#ifdef _DEBUG
#ifndef _WIN32_WCE
	// 调试版本则输出日志到日志窗口中
/*	USES_CONVERSION;
	LPCSTR	szMsgAssii = T2CA(szBuffer);
	LogTrace((void *)szMsgAssii, nLen * sizeof(TCHAR));*/
	LogTrace(szBuffer, nLen);
	if (!m_bAlwaysLogToFile)
		return;
#endif
#endif
	// 写入日志文件
	m_mutex.Acquire();
	if (m_fpLog)
	{
		if (ftell(m_fpLog) >= MAX_LOG_FILE_LEN)
		{
			//
			// 设置这次开始写日志的位置
			//
			// fseek(m_fpLog, 0, SEEK_SET);
			fclose(m_fpLog);
			m_fpLog = NULL;
			OpenLogFile();
		}
	}
	else
	{
		//
		// 没有打开日志文件，试着重新打开日志文件
		if (m_bTryOpenAgain)
		{
			m_bTryOpenAgain = FALSE;
			if (OpenLogFile())
			{
				WriteLog(LOG_LVL_EVENT_IMP, _T(__FILE__), __LINE__, _T("Log File is NOT opened, Open it At %d-%d-%d %d:%d:%d. LOG STARTED:)\n"), 
#ifdef _WIN32_WCE
					time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
#else
					ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
#endif
			}
		}
	}

	if (m_fpLog)
	{
//		fwrite(szBuffer, 1, nLen - 1, m_fpLog);
//		fwrite("\r\n", 1, 2, m_fpLog);
		fwriteWT(szBuffer, sizeof(TCHAR), (nLen - 1), m_fpLog);
		fwriteWT(SZ_RETURN, sizeof(TCHAR), _tcslen(SZ_RETURN), m_fpLog);
		fflush(m_fpLog);
	}

	m_mutex.Release();
}

void CLog::LogOut(BYTE byLevel, LPCTSTR szFile, int nLine, LPCTSTR szFormat, ...)
{
#ifndef _DEBUG
	if (!m_bEnableLog)
		return;

	// 将发行版本中的调试信息去掉
	if (byLevel == LOG_LVL_DEBUG)
		return;
#endif	// _DEBUG

	va_list		args;

	int			nBuf;
	TCHAR		szBuffer[MAX_MSG_LEN] = _T("");
	TCHAR		*szMsg = _T("");
	int			nLen = 0;
#ifdef _WIN32_WCE
	SYSTEMTIME	time;

	GetLocalTime(&time);
#else
	time_t		ltime;
	tm			*ptm;

	time(&ltime);
	ptm = localtime(&ltime);
#endif

#if defined(_WIN32) && !defined(_WINCE_)
	__try
#endif
	{
		nLen = _stprintf_s(szBuffer, CountOf(szBuffer), _T("|T%d-%d %d:%d:%d|F%s|L%d|E%d|M"),
#ifdef _WINCE_
			time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond,
#else
			ptm->tm_mon + 1, ptm->tm_mday,ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
#endif
			szFile, nLine, byLevel);

		// 格式化日志
		va_start(args, szFormat);
		szMsg = szBuffer + nLen;
		nBuf = _vsntprintf_s(szMsg, CountOf(szBuffer) - nLen, CountOf(szBuffer) - nLen, szFormat, args);
		va_end(args);

		// was there an error? was the expanded string too long?
		if (nBuf < 0)
			_tcscpy_safe(szMsg, CountOf(szBuffer) - nLen, _T("Insufficient log buffer space."));
	}
#if defined(_WIN32) && !defined(_WINCE_)
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// 有异常发生！
		_tcscpy_safe(szBuffer + nLen, CountOf(szBuffer) - nLen, _T("Exception error in format."));
	}
#endif

	nLen = (int)_tcslen(szBuffer) + 1;

#ifdef _DEBUG
#ifndef _WIN32_WCE
	// 调试版本则输出日志到日志窗口中
/*	USES_CONVERSION;
	LPCSTR	szMsgAssii = T2CA(szBuffer);
	LogTrace((void *)szMsgAssii, nLen * sizeof(TCHAR));*/
	LogTrace(szBuffer, nLen);
	if (byLevel > LOG_LVL_EVENT_IMP)
		return;
	if (!m_bAlwaysLogToFile)
		return;
#endif
#endif
	// 写入日志文件
	m_mutex.Acquire();
	if (m_fpLog)
	{
		if (ftell(m_fpLog) >= MAX_LOG_FILE_LEN)
		{
			//
			// 设置这次开始写日志的位置
			//
			// fseek(m_fpLog, 0, SEEK_SET);
			fclose(m_fpLog);
			m_fpLog = NULL;
			OpenLogFile();
		}
	}
	else
	{
		//
		// 没有打开日志文件，试着重新打开日志文件
		if (m_bTryOpenAgain)
		{
			m_bTryOpenAgain = FALSE;
			if (OpenLogFile())
			{
				WriteLog(LOG_LVL_EVENT_IMP, _T(__FILE__), __LINE__, _T("Log File is NOT opened, Open it At %d-%d-%d %d:%d:%d. LOG STARTED:)\n"), 
#ifdef _WIN32_WCE
					time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
#else
					ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
#endif
			}
		}
	}

	if (m_fpLog)
	{
		fwriteWT(szBuffer, sizeof(TCHAR), (nLen - 1), m_fpLog);
		fwriteWT(SZ_RETURN, sizeof(TCHAR), _tcslen(SZ_RETURN), m_fpLog);
//		fwrite(szBuffer, 1, nLen - 1, m_fpLog);
//		fwrite(_T("\r\n"), 1, 2, m_fpLog);
		fflush(m_fpLog);
	}
	m_mutex.Release();
#if defined(_WIN32) && !defined(_WINCE_)
	::MessageBox(m_hWndMessage, szMsg, _T("Log Message"), MB_ICONWARNING);
#endif
}

void CLog::Init(LPCTSTR szLogFileName, BOOL bAlwaysLogToFile)
{
	m_bAlwaysLogToFile = Tobool(bAlwaysLogToFile);

	if (szLogFileName == NULL)
	{
		m_bEnableLog = false;
		return;
	}

	if ((szLogFileName[0] != '\0' && szLogFileName[1] == ':' && szLogFileName[2] == DIR_SLASH) ||
		(szLogFileName[0] == DIR_SLASH))
	{
		// 如果已经是全路径，则不管
		_tcscpy_safe(m_szLogFile, CountOf(m_szLogFile), szLogFileName);
	}
	else
	{
		// 如果不是全路径，则
		GetWorkingFolder(m_szLogFile);
		_tcscat_safe(m_szLogFile, CountOf(m_szLogFile), szLogFileName);
	}

#ifndef _DEBUG
	if (!OpenLogFile())
		return;
#else
	//
	// 在调试状态也写Log：）
	if (m_bAlwaysLogToFile)
		if (!OpenLogFile())
			return;
#endif

#ifdef _WIN32
	SYSTEMTIME	time;

	GetLocalTime(&time);

	WriteLog(LOG_LVL_EVENT_IMP, _T(__FILE__), __LINE__, _T("Started Log : %s At %d-%d-%d %d:%d:%d. LOG STARTED:)"), szLogFileName, 
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
#else
	time_t		ltime;
	tm			*ptm;

	time(&ltime);
	ptm = localtime(&ltime);

	WriteLog(LOG_LVL_EVENT_IMP, _T(__FILE__), __LINE__, _T("Started Log : %s At %d-%d-%d %d:%d:%d. LOG STARTED:)"), szLogFileName, 
		ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
#endif
}

void CLog::Close()
{
	// DeleteCriticalSection(&m_csLog);
	if (m_fpLog)
	{
		fclose(m_fpLog);
		m_fpLog = NULL;
	}
}

BOOL CLog::OpenLogFile()
{
	if (!m_bEnableLog)
		return TRUE;

	// 写入日志文件log.txt
	if (m_fpLog)
	{
		fclose(m_fpLog);
		m_fpLog = NULL;
	}

	if (IsEmptyString(m_szLogFile))
	{
		GetWorkingFolder(m_szLogFile);
		_tcscat_safe(m_szLogFile, CountOf(m_szLogFile), _T("log.txt"));
	}

	uint64		nFileLength;
	if (GetFileLength(m_szLogFile, nFileLength) && nFileLength >= MAX_LOG_FILE_LEN)
	{
		DeleteFile(m_szLogFile);
	}

	m_fpLog = _tfopen(m_szLogFile, _T("a+"));

	return (m_fpLog != NULL);
}

#ifdef _WIN32_DESKTOP
void CLog::SetMessageOutWnd(HWND hWnd)
{
	m_hWndMessage = hWnd;
}

void CLog::SetTargetWnd(HWND hWnd)
{
	_hDebugWnd = hWnd;
}
#endif
