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

	int DoModal(CBaseWnd *pWndParent);

	LPCTSTR GetOpenFile();

	void GetOpenFile(vector<tstring> &vFiles);

protected:
	bool			m_bAllowMultiSel;
	GtkWidget		*m_window;
	char			*m_szFile;
	tstring			m_strTitle, m_strInitFile;

};


class CFileSaveDlg
{
public:
	CFileSaveDlg(LPCTSTR szTitle, LPCTSTR szFile, LPCTSTR szFilter, int nDefFileType);
	virtual ~CFileSaveDlg(void);

	int DoModal(CBaseWnd *pWndParent);

	LPCTSTR GetSaveFile();

	LPCTSTR GetSelectedExt();

protected:
	GtkWidget		*m_window;
	char			*m_szFile;
	tstring			m_strTitle, m_strInitFile;

};


class CFolderDialog  
{
public:
	CFolderDialog();
	virtual ~CFolderDialog();

	void SetRootFolder(LPCTSTR szRootFolder);
	void SetTitle(LPCTSTR szTitle);
	void SetInitFolder(LPCTSTR szInitDir);
	LPCTSTR GetFolder();
	BOOL DoBrowse(CBaseWnd *pWndParent);

protected:
	tstring		m_strInitFolder;
	tstring		m_strTitle;

	GtkWidget		*m_window;
	char			*m_szFile;

};

#endif // !defined(AFX_FILEOPENDLG_H__ACA8E71F_C3AE_48EC_BE68_10498B2F5270__INCLUDED_)
