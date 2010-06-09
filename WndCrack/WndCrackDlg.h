// WndCrackDlg.h : header file
//

#if !defined(AFX_WNDCRACKDLG_H__8407EAC6_B4A2_11D4_9F4C_52544C98EE4B__INCLUDED_)
#define AFX_WNDCRACKDLG_H__8407EAC6_B4A2_11D4_9F4C_52544C98EE4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CWndCrackDlg dialog

class CWndCrackDlg : public CDialog
{
// Construction
public:
	HWND GetCapturedWnd();
	void SetCapturedWnd(HWND hWndCap);

	CWndCrackDlg(CWnd* pParent = NULL);	// standard constructor

	static BOOL CALLBACK EnumWindowProc(
	  HWND hwnd,      // handle to parent window
	  LPARAM lParam   // application-defined value
	);

// Dialog Data
	//{{AFX_DATA(CWndCrackDlg)
	enum { IDD = IDD_WNDCRACK_DIALOG };
	CTreeCtrl	m_wndTree;
	CString	m_strWndCaptured;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWndCrackDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	HWND		m_hWndMouseSelect;
	CRect		m_rcSelectingBt;
	HCURSOR		m_hcursorSelecting;
	BOOL		m_bSelectingWnd;
	HPEN		m_hPenWhite;

	DWORD		m_dwProcessID;

	HBRUSH		m_hbrColor;
	void SetMaskColor(COLORREF clrMask);

	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CWndCrackDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTopmost();
	afx_msg void OnShowwindow();
	afx_msg void OnMovewindow();
	afx_msg void OnClosewnd();
	afx_msg void OnUpdate();
	afx_msg void OnSetWndCaption();
	afx_msg void OnMinimizeWnd();
	afx_msg void OnSetWndOpaque();
	afx_msg void OnEnableWnd();
	afx_msg void OnEnum();
	afx_msg void OnSelchangedTreeWnd(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedMaximizeWnd();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WNDCRACKDLG_H__8407EAC6_B4A2_11D4_9F4C_52544C98EE4B__INCLUDED_)
