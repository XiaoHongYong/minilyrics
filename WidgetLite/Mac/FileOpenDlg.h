
#pragma once

class CFileDlgExtFilter : public tstring
{
public:
	void AddExtention(LPCTSTR szDesc, LPCTSTR szExt);

};

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
	LPTSTR			m_szFile;
	int				m_nBufLen;
	bool			m_bAllowMultiSel;
	SetString		m_setExt;

};
