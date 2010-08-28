// FileOpenDlg.h: interface for the CFileOpenDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEOPENDLG_H__ACA8E71F_C3AE_48EC_BE68_10498B2F5270__INCLUDED_)
#define AFX_FILEOPENDLG_H__ACA8E71F_C3AE_48EC_BE68_10498B2F5270__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFileOpenDlg  
{
public:
	CFileOpenDlg(LPCTSTR szTitle, LPCTSTR szFile, LPCTSTR szFilter, int nDefFileType, bool bAllowMultiSel = false);
	virtual ~CFileOpenDlg();

#ifdef _NO_WIGET_LIB
	int DoModal(HWND hWndParent);
#else
	int DoModal(CBaseWnd *pWndParent);
#endif

	LPCTSTR GetOpenFile();

	void GetOpenFile(vector<tstring> &vFiles);

public:
	OPENFILENAME	m_openfile;
	LPTSTR			m_szFile;
	int				m_nBufLen;
	bool			m_bAllowMultiSel;

};

#endif // !defined(AFX_FILEOPENDLG_H__ACA8E71F_C3AE_48EC_BE68_10498B2F5270__INCLUDED_)
