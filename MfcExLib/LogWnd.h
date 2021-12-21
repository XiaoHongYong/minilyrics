#if !defined(AFX_LOGWND_H__4E5D2F7E_8D9E_4FFD_86DB_914C3DC0C228__INCLUDED_)
#define AFX_LOGWND_H__4E5D2F7E_8D9E_4FFD_86DB_914C3DC0C228__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogWnd.h : header file
//

/************************************************************************
通过INI文件控制日志的显示
[LogWnd]
ShowLogLevel=5
AutoScroll=1
序号=1
时间=1
源文件=1
代码行=1
日志级别=1

/************************************************************************/

// using STL
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#include <string>
#include <vector>
using namespace std;

#define SENDDEBUGMESSAGE 19771212

#define MYWM_ADD_MSG		(WM_USER + 12)

#define SENDDEBUGMESSAGE	19771212
#define	DEBUGCLASSNAME		"Debug Tracer Class"
#define	DEBUGWNDNAME		"Debug Tracer"

/////////////////////////////////////////////////////////////////////////////
// CLogWnd window

class CLogWnd : public CListCtrl
{
// Construction
public:
	enum PAUSE_STATE
	{
		PS_NORMAL,
		PS_PAUSE,
		PS_PAUSE_DROP
	};
	CLogWnd();

// Attributes
public:
	vector<string>	m_vLogMsg;
	BOOL		m_bTopmost;
	PAUSE_STATE	m_PauseState;
	BOOL		m_bScroll;
	int			m_nShowLogLvl;		// 显示调试日志的级别

	BOOL		m_bMaxLogLimit;

// Operations
public:
	void AnanysisLogMsg(LPTSTR szBuffer);
	void InsertLog(LPCTSTR szTime, LPCTSTR szSrcFile, int nLine, int nLevel, LPCTSTR szMsg);

	void Init();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLogWnd();

	// Generated message map functions
protected:
	void ViewColumnChanged(int nColumn);

	//{{AFX_MSG(CLogWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnViewLogLvl();
	afx_msg void OnViewSrcFile();
	afx_msg void OnViewSrcLine();
	afx_msg void OnViewTime();
	afx_msg void OnOpen();
	afx_msg void OnAlwaysOntop();
	afx_msg void OnPause();
	afx_msg void OnPauseDropLog();
	afx_msg void OnClearLog();
	afx_msg void OnScrollLog();
	afx_msg LRESULT OnLogBuffer(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGWND_H__4E5D2F7E_8D9E_4FFD_86DB_914C3DC0C228__INCLUDED_)
