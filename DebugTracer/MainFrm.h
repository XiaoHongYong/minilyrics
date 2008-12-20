// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__83D58DAA_4E4E_4BB6_BF3F_07C4BE87370D__INCLUDED_)
#define AFX_MAINFRM_H__83D58DAA_4E4E_4BB6_BF3F_07C4BE87370D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChildView.h"
#include "DlgTree.h"

// using STL
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#include <string>
#include <vector>
using namespace std;

class CMainFrame : public CFrameWnd
{
	enum PAUSE_STATE
	{
		PS_NORMAL,
		PS_PAUSE,
		PS_PAUSE_DROP
	};
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:
	CDlgTree		m_dlgTree;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	void AnanysisLogMsg(LPTSTR szBuffer);
	void InsertLog(LPCTSTR szTime, LPCTSTR szSrcFile, int nLine, int nLevel, LPCTSTR szMsg);
	void ViewColumnChanged(int nColumn);
	// virtual BOOL LoadFrame1 (UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	BOOL RegisterWindowClass();
	void AddTree(DWORD dwTreeId, DWORD dwParentId, DWORD CurId, LPCTSTR szName);
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CListCtrl    m_wndView;
	vector<string>	m_vLogMsg;
	BOOL		m_bTopmost;
	PAUSE_STATE	m_PauseState;
	BOOL		m_bScroll;
	bool		m_bShowOnlyFileName;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
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
	afx_msg void OnEditCopy();
	afx_msg void OnViewFilenameOnly();
	//}}AFX_MSG
	afx_msg LRESULT OnLogBuffer(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__83D58DAA_4E4E_4BB6_BF3F_07C4BE87370D__INCLUDED_)
