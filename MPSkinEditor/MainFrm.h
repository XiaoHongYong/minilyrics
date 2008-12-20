// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__A9635B17_F5B1_4834_AD13_226B09620008__INCLUDED_)
#define AFX_MAINFRM_H__A9635B17_F5B1_4834_AD13_226B09620008__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropertyBar.h"
#include "ObjListBar.h"
#include "MDITabs.h"

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:
	bool m_bTop;
	bool m_bImages;

	CMDITabs		*GetMDITab() { return &m_wndMDITabs; }

protected:
	CMDITabs			m_wndMDITabs;
	CPropertyBar		m_wndPropertyBar;
	CObjListBar			m_wndObjListBar;

// Operations
public:
	void OnUpdateFrameTitle(BOOL bAddToTitle);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CStatusBar *GetStatusBar() { return &m_wndStatusBar; }

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSaveSkin();
	afx_msg void OnViewObjlistBar();
	afx_msg void OnViewPropertyBar();
	afx_msg void OnUpdatePaneMousePointer(CCmdUI *pCmdUI)  { pCmdUI->Enable(); }
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__A9635B17_F5B1_4834_AD13_226B09620008__INCLUDED_)
