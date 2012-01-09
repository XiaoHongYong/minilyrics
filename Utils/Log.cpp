// Log.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#ifdef _WIN32
#include <wtypes.h>
#include <oleauto.h>
#include <crtdbg.h>
#include <atldef.h>
#include <atlconv.h>
#include <stdio.h>
#endif
#include <time.h>

#include "log.h"
#include "App.h"

/*

static HWND _hDebugWnd;
#include <tchar.h>

#define SENDDEBUGMESSAGE	19771212
#define	DEBUGCLASSNAME		_T("Debug Tracer Class")
#define	DEBUGWNDNAME		_T("Debug Tracer")

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
	nLen = wsprintf(szBuffer, _T("|T%d-%d %d:%d:%d|M%s"), LocalTime.wMonth, LocalTime.wDay,
	LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, szData);

	char		szBuff[1024];
	WideCharToMultiByte(CP_ACP, 0, szBuffer, nLen, szBuff, CountOf(szBuff), 0, NULL);

	LogTrace((LPVOID)szBuff, strlen(szBuff));
}

	DBGLOG0("|MCWinamp2Player::Quit()");
*/

// determine number of elements in an array (not bytes)

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
	g_print("%s\n", (const char*)lpData);
}
#endif

#ifdef _IPHONE
void LogTrace(LPVOID lpData, int nSize)
{
	printf("%s\n", (const char*)lpData);
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

size_t fwriteWT(const TCHAR *szBuffer, size_t n, size_t nLen, FILE *fp)
{
	int		nLenWrite;
#if defined(_WIN32)
	USES_CONVERSION;
	LPCSTR	szMsgAssii = T2CA((TCHAR *)szBuffer);
	nLenWrite = strlen(szMsgAssii);
#else
	LPCSTR	szMsgAssii = (LPCSTR)szBuffer;
	nLenWrite = sizeof(TCHAR) * nLen;
#endif
	return fwrite(szMsgAssii, 1, nLenWrite, fp);
}


//////////////////////////////////////////////////////////////////////


static LPCTSTR IgnoreRootDir(LPCTSTR szFile, LPCTSTR szRootDir)
{
	LPCTSTR		szRet = szFile;

	while (*szRet && *szRet == *szRootDir)
	{
		szRootDir++;
		szRet++;
	}

	if (*szRootDir == '\0')
		return szRet;
	else
		return szFile;
}


CLog::CLog()
{
	m_fpLog = NULL;
	EmptyStr(m_szLogFile);
	EmptyStr(m_szSrcRootDir);
}

CLog::~CLog()
{
	if (m_fpLog)
		fclose(m_fpLog);
}

void CLog::LogDumpStr(BYTE byLevel, LPCTSTR szFile, int nLine, LPCSTR szStr, int nStrLen)
{
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
	// 将发行版本中的调试信息去掉
	if (byLevel == LOG_LVL_DEBUG)
		return;
#endif	// _DEBUG

	szFile = IgnoreRootDir(szFile, m_szSrcRootDir);

	va_list		args;

	int			nBuf;
	TCHAR		szBuffer[MAX_MSG_LEN] = _T("");
	int			nLen = 0;

	CBaseDate time = CBaseDate::GetCurrentDate();

#if defined(_WIN32_DESKTOP)
	__try
#endif
	{
		nLen = _stprintf_s(szBuffer, CountOf(szBuffer), _T("|T%d-%d %d:%d:%d|F%s|L%d|E%d|M"), 
			time.month, time.day, time.hour, time.minute, time.second,
			szFile, nLine, byLevel);

		// 格式化日志
		TCHAR		*szMsg = NULL;
		va_start(args, szFormat);
		szMsg = szBuffer + nLen;
		nBuf = _vstprintf_s(szMsg, CountOf(szBuffer) - nLen, szFormat, args);
		va_end(args);

		// was there an error? was the expanded string too long?
		if (nBuf < 0)
			_tcscpy_safe(szMsg, CountOf(szBuffer) - nLen, _T("Insufficient log buffer space."));
	}
#if defined(_WIN32_DESKTOP)
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		// 有异常发生！
		_tcscpy_safe(szBuffer + nLen, CountOf(szBuffer) - nLen, _T("Exception error in format."));
	}
#endif

	nLen = (int)_tcslen(szBuffer) + 1;

#ifdef _DEBUG
	LogTrace(szBuffer, nLen);
#endif

	// 写入日志文件
	m_mutex.Acquire();
	if (m_fpLog)
	{
		if (ftell(m_fpLog) >= MAX_LOG_FILE_LEN)
		{
			fclose(m_fpLog);
			m_fpLog = NULL;
			OpenLogFile();
		}
	}
	else
	{
		//
		// 没有打开日志文件，试着重新打开日志文件
		if (OpenLogFile())
		{
			WriteLog(LOG_LVL_INFO, _T(__FILE__), __LINE__, _T("Log File is NOT opened, Open it At %d-%d-%d %d:%d:%d. LOG STARTED:)\n"), 
				time.year, time.month, time.day, time.hour, time.minute, time.second);
		}
	}

	if (m_fpLog)
	{
		fwriteWT(szBuffer, sizeof(TCHAR), (nLen - 1), m_fpLog);
		fwriteWT(SZ_RETURN, sizeof(TCHAR), _tcslen(SZ_RETURN), m_fpLog);
		fflush(m_fpLog);
	}
	else
		_tprintf(_T("%s\n"), szBuffer);

	m_mutex.Release();
}

void CLog::Init(LPCTSTR szLogFileName)
{
	if (!szLogFileName)
		return;

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

	if (!OpenLogFile())
		return;

	CBaseDate time = CBaseDate::GetCurrentDate();

	WriteLog(LOG_LVL_INFO, _T(__FILE__), __LINE__, _T("Started Log : %s At %d-%d-%d %d:%d:%d. LOG STARTED:)"), szLogFileName, 
		time.year, time.month, time.day, time.hour, time.minute, time.second);
}

void CLog::Close()
{
	if (m_fpLog)
	{
		fclose(m_fpLog);
		m_fpLog = NULL;
	}
}

void CLog::SetSrcRootDir(LPCTSTR szSrcFile, int nCurFileDeep)
{
	LPCTSTR		szDirEndPos = NULL;

	szDirEndPos = szSrcFile + _tcslen(szSrcFile);

	while (szDirEndPos > szSrcFile)
	{
		if (*szDirEndPos == DIR_SLASH)
		{
			if (nCurFileDeep > 0)
				nCurFileDeep--;
			else
				break;
		}
		szDirEndPos--;
	}

	_tcsncpysz_safe(m_szSrcRootDir, CountOf(m_szSrcRootDir), szSrcFile, int(szDirEndPos - szSrcFile));
}

BOOL CLog::OpenLogFile()
{
	if (IsEmptyString(m_szLogFile))
		return false;

	if (m_fpLog)
	{
		fclose(m_fpLog);
		m_fpLog = NULL;
	}

	uint64		nFileLength;
	if (GetFileLength(m_szLogFile, nFileLength) && nFileLength >= MAX_LOG_FILE_LEN)
		DeleteFile(m_szLogFile);

	m_fpLog = _tfopen(m_szLogFile, _T("a+"));

	return (m_fpLog != NULL);
}
