/********************************************************************
	Created  :	2002/02/01	20:08
	FileName :	Log.h
	Author   :	xhy
*********************************************************************/
#pragma once

// Log level
enum
{
	LOG_LVL_ERROR				= 2,
	LOG_LVL_WARNING				= 3,
	LOG_LVL_EVENT_IMP			= 4,
	LOG_LVL_DEBUG				= 5,
};

class CLog  
{
protected:
	FILE		*m_fpLog;
	TCHAR		m_szLogFile[MAX_PATH];
	TCHAR		m_szSrcRootDir[128];

	Mutex		m_mutex;

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
	BOOL OpenLogFile();

};
