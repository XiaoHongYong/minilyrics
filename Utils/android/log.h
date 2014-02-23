/********************************************************************
	Created  :	2002/02/01	20:08
	FileName :	Log.h
	Author   :	xhy
*********************************************************************/
#pragma once

#include <android/log.h>

// Log level
enum
{
	LOG_LVL_ERROR				= ANDROID_LOG_ERROR,
	LOG_LVL_WARNING				= ANDROID_LOG_WARN,
	LOG_LVL_INFO				= ANDROID_LOG_INFO,
	LOG_LVL_DEBUG				= ANDROID_LOG_DEBUG,
};

class CLog  
{
public:
	CLog();
	virtual ~CLog();

public:
	void Init(LPCTSTR szLogFileName);
	void Close();

	void SetSrcRootDir(LPCTSTR szSrcFile, int nCurFileDeep = 1);

	void WriteLog(BYTE byLevel, LPCTSTR szFile, int nLine, LPCTSTR szFormat, ...);
	void LogDumpStr(BYTE byLevel, LPCTSTR szFile, int nLine, LPCSTR szStr, int nStrLen);

protected:
	TCHAR		m_szTag[32];
	TCHAR		m_szSrcRootDir[128];

};
