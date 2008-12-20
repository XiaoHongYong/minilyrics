// ChildFrm.h : interface of the CDynCtrlFrame class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ChildView.h"

class CDynCtrlFrame : public CMDIChildWnd, public ISESkinDataItem
{
	DECLARE_DYNCREATE(CDynCtrlFrame)
public:
	CDynCtrlFrame();

// Attributes
public:

// Operations
public:
	virtual void OnSave();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDynCtrlFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	// view for the client area of the frame.
	CEdit		m_wndEdit;
	virtual ~CDynCtrlFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CDynCtrlFrame)
	afx_msg void OnFileClose();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChildActivate();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
