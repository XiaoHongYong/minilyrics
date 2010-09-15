// ID3LibTestDlg.h : header file
//

#if !defined(AFX_ID3LIBTESTDLG_H__218E2EC8_287F_477E_9583_0E679BF52649__INCLUDED_)
#define AFX_ID3LIBTESTDLG_H__218E2EC8_287F_477E_9583_0E679BF52649__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CID3LibTestDlg dialog

class CID3LibTestDlg : public CDialog
{
// Construction
public:
	CID3LibTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CID3LibTestDlg)
	enum { IDD = IDD_ID3LIBTEST_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CID3LibTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	void GetDlgItemText(int nID, tstring &str);

	tstring  artist,  title,  album,  comment;
	tstring  track,  year,  genre,  composer;
	tstring  encoded,  url,  copyRight,  origArtist,  publisher,  bpm;

	tstring		m_strMp3File;


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CID3LibTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnOpenMp3();
	afx_msg void OnModify();
	afx_msg void OnAddImg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ID3LIBTESTDLG_H__218E2EC8_287F_477E_9583_0E679BF52649__INCLUDED_)
