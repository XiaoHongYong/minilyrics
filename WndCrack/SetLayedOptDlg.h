#if !defined(AFX_SETLAYEDOPTDLG_H__2D465158_5E5A_40F1_92BC_8B1D28DAB248__INCLUDED_)
#define AFX_SETLAYEDOPTDLG_H__2D465158_5E5A_40F1_92BC_8B1D28DAB248__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetLayedOptDlg.h : header file
//

BOOL IsLayeredWndSupported();

/////////////////////////////////////////////////////////////////////////////
// CSetLayedOptDlg dialog

class CSetLayedOptDlg : public CDialog
{
// Construction
public:
	void SetMaskColor(COLORREF clrMask);
	CSetLayedOptDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetLayedOptDlg)
	enum { IDD = IDD_SET_LAYEREDWND_DLG };
	BOOL	m_bUseLayeredOpt;
	CString	m_strMaskColor;
	BYTE	m_byOpaque;
	BOOL	m_bTransColor;
	BOOL	m_bOpaque;
	BOOL	m_bClickThrough;
	//}}AFX_DATA

	CRect		m_rcSelectingBt;
	HCURSOR		m_hcursorSelecting;
	BOOL		m_bSelectingColor;
	HBRUSH		m_hbrColor;

	HWND		m_hWndModify;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetLayedOptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetLayedOptDlg)
	virtual void OnOK();
	afx_msg void OnLayeredOpaque();
	afx_msg void OnEnableLayered();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSelColor();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETLAYEDOPTDLG_H__2D465158_5E5A_40F1_92BC_8B1D28DAB248__INCLUDED_)
