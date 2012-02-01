
// ID3v2TagRemoverDlg.h : header file
//

#pragma once


// CID3v2TagRemoverDlg dialog
class CID3v2TagRemoverDlg : public CDialog
{
// Construction
public:
	CID3v2TagRemoverDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ID3V2TAGREMOVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_strDir;
	afx_msg void OnBnClickedBrFolder();
	afx_msg void OnBnClickedOk();
};
