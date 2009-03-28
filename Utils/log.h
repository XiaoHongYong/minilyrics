/********************************************************************
	Created  :	2002/02/01	20:08
	FileName :	Log.h
	Author   :	xhy
	
	Purpose  :	日志文件的操作以及定义

	Comment  :  

	1）日志信息分为下面几个等级（0 － 255）：
		ID		描述
		1		致命错误或异常
		2		一般错误或异常
		3		提示错误或异常
		4		重要操作事件
		5		一般事件
		6		调试信息
		等级 0 － 16 保留，16 以后的可以自由使用，并且只有用户设置了某类型才会使用

	3）能够记录当前程序的状态

	4）

	使用的注意事项：
	1、习惯使用CString，string等类的用户来说，注意类型的转换，要转换为(LPCTSTR)类型，
		即const char *类型。否则会发生异常。
*********************************************************************/
#if !defined(AFX_LOG_H__2003_02_09__INCLUDED_)
#define AFX_LOG_H__2003_02_09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// 日志的等级定义
enum
{
	LOG_LVL_ERROR_FATAL			= 1,
	LOG_LVL_ERROR				= 2,
	LOG_LVL_ERROR_MINOR			= 3,
	LOG_LVL_WARNING				= 4,
	LOG_LVL_EVENT_IMP			= 5,
	LOG_LVL_EVENT_NORMAL		= 6,
	LOG_LVL_DEBUG				= 7,
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

#endif // !defined(AFX_LOG_H__2003_02_09__INCLUDED_)
