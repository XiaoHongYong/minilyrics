/********************************************************************
	Created  :	2002/02/01	20:08
	FileName :	Log.h
	Author   :	xhy
	
	Purpose  :	»’÷æŒƒº˛µƒ≤Ÿ◊˜“‘º∞∂®“Â

	Comment  :  

	1£©»’÷æ–≈œ¢∑÷Œ™œ¬√Êº∏∏ˆµ»º∂£®0 £≠ 255£©£∫
		ID		√Ë ˆ
		1		÷¬√¸¥ÌŒÛªÚ“Ï≥£
		2		“ª∞„¥ÌŒÛªÚ“Ï≥£
		3		Ã· æ¥ÌŒÛªÚ“Ï≥£
		4		÷ÿ“™≤Ÿ◊˜ ¬º˛
		5		“ª∞„ ¬º˛
		6		µ˜ ‘–≈œ¢
		µ»º∂ 0 £≠ 16 ±£¡Ù£¨16 “‘∫Ûµƒø…“‘◊‘”… π”√£¨≤¢«“÷ª”–”√ªß…Ë÷√¡Àƒ≥¿‡–Õ≤≈ª· π”√

	3£©ƒ‹πªº«¬ºµ±«∞≥Ã–Úµƒ◊¥Ã¨

	4£©

	 π”√µƒ◊¢“‚ ¬œÓ£∫
	1°¢œ∞πﬂ π”√CString£¨stringµ»¿‡µƒ”√ªß¿¥Àµ£¨◊¢“‚¿‡–Õµƒ◊™ªª£¨“™◊™ªªŒ™(LPCTSTR)¿‡–Õ£¨
		º¥const char *¿‡–Õ°£∑Ò‘Úª·∑¢…˙“Ï≥£°£
*********************************************************************/
#if !defined(AFX_LOG_H__2003_02_09__INCLUDED_)
#define AFX_LOG_H__2003_02_09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// »’÷æµƒµ»º∂∂®“Â
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
