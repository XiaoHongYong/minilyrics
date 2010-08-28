#pragma once

class CFileSaveDlg
{
public:
	CFileSaveDlg(LPCTSTR szTitle, LPCTSTR szFile, LPCTSTR szFilter, int nDefFileType);
	virtual ~CFileSaveDlg(void);

	int DoModal(CBaseWnd *pWndParent);

	LPCTSTR GetSaveFile();

	LPCTSTR GetSelectedExt();

public:
	OPENFILENAME	m_openfile;
	TCHAR			m_szFile[MAX_PATH];

};
