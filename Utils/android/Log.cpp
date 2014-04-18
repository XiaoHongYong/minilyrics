// Log.cpp: implementation of the CLog class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"

#include "log.h"
#include "../App.h"

#define MAX_MSG_LEN 1024

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
	_tcscpy(m_szTag, _T("Log"));
	EmptyStr(m_szSrcRootDir);
}

CLog::~CLog()
{
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

	TCHAR		szBuffer[MAX_MSG_LEN] = _T("");
	int			nLen = 0;

	nLen = _stprintf_s(szBuffer, CountOf(szBuffer) - 1, _T("%s %d "), 
		szFile, nLine);

	// 格式化日志
	TCHAR		*szMsg = NULL;
	va_start(args, szFormat);
	szMsg = szBuffer + nLen;
	int nBuf = _vstprintf_s(szMsg, CountOf(szBuffer) - nLen, szFormat, args);
	va_end(args);

	// was there an error? was the expanded string too long?
	if (nBuf < 0 || nBuf >= CountOf(szBuffer) - nLen)
		_tcscpy_safe(szMsg, CountOf(szBuffer) - nLen, _T("Insufficient log buffer space."));
	else
		szMsg[nBuf] = '\0';

	nLen = (int)_tcslen(szBuffer) + 1;

	__android_log_write(byLevel, "Tag", szBuffer);
}

void CLog::LogDumpStr(BYTE byLevel, LPCTSTR szFile, int nLine, LPCSTR szStr, int nStrLen)
{
}

void CLog::Init(LPCTSTR szLogFileName)
{
	if (!szLogFileName)
		return;

	_tcscpy_safe(m_szTag, CountOf(m_szTag), FileGetTitle(szLogFileName).c_str());
}

void CLog::Close()
{
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
