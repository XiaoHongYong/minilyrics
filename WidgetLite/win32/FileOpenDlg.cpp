// FileOpenDlg.cpp: implementation of the CFileOpenDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#ifndef _NO_WIGET_LIB
#include "BaseWnd.h"
#endif
#include "FileOpenDlg.h"
#include <CdErr.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileOpenDlg::CFileOpenDlg(LPCTSTR szTitle, LPCTSTR szFile, LPCTSTR szFilter, int nDefFileType, bool bAllowMultiSel)
{
	if (bAllowMultiSel)
		m_nBufLen = 1024 * 10;
	else
		m_nBufLen = MAX_PATH;

	m_bAllowMultiSel = bAllowMultiSel;
	m_szFile = new TCHAR[m_nBufLen];
	EmptyStr(m_szFile);

	if (szFile)
		_tcscpy_safe(m_szFile, m_nBufLen, szFile);

	memset(&m_openfile, 0, sizeof(m_openfile));
	m_openfile.lpstrTitle = szTitle;
	m_openfile.lpstrFilter = szFilter;
	m_openfile.lpstrFile = m_szFile;
	m_openfile.nMaxFile = m_nBufLen;
	m_openfile.nFilterIndex = nDefFileType;

	m_openfile.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	if (bAllowMultiSel)
		m_openfile.Flags |= OFN_ALLOWMULTISELECT;
	m_openfile.lStructSize = sizeof(m_openfile);
	m_openfile.hInstance = GetAppInstance();
}

CFileOpenDlg::~CFileOpenDlg()
{
	delete[] m_szFile;
}

#ifdef _NO_WIGET_LIB
int CFileOpenDlg::DoModal(HWND hWndParent) {
	m_openfile.hwndOwner = hWndParent;
#else
int CFileOpenDlg::DoModal(CBaseWnd *pWndParent) {
	m_openfile.hwndOwner = pWndParent->GetHandle();
#endif

	if (GetOpenFileName(&m_openfile))
		return IDOK;
	else
	{
		DWORD	dwErr = CommDlgExtendedError();
		if (dwErr == FNERR_INVALIDFILENAME)
		{
			EmptyStr(m_openfile.lpstrFile);
			if (GetOpenFileName(&m_openfile))
				return IDOK;
		}
	}
	return IDCANCEL;
}

LPCTSTR CFileOpenDlg::GetOpenFile()
{
	return m_szFile;
}

void CFileOpenDlg::GetOpenFile(vector<tstring> &vFiles)
{
	if (m_bAllowMultiSel)
	{
		LPCTSTR		szText = m_szFile;
		tstring		strDir;

		strDir = m_szFile;
		DirStringFormat(strDir);

		while (*szText != '\0')
			szText++;
		szText++;

		while (*szText)
		{
			vFiles.push_back(strDir + szText);
			while (*szText != '\0')
				szText++;
			szText++;
		}

		if (vFiles.empty())
			vFiles.push_back(m_szFile);
	}
	else
		vFiles.push_back(m_szFile);
}
