#include "StdAfx.h"
#include "BaseWnd.h"
#include "FileSaveDlg.h"
#include <Dlgs.h>


// COMMENT:
//		取得 GetSaveFileName 中lpstrFilter 的第nIndex个扩展名。
// INPUT:
//		szFilters	:	_T("All supported files (*.lrc; *.txt; *.snc)\0*.lrc;*.txt;*.snc\0LRC Lyrics File (*.lrc)\0*.LRC\0Text File (*.txt)\0*.TXT\0Snc File (*.snc)\0*.snc\0\0")
LPCTSTR GetFileFilterExtByIndex(LPCTSTR szFilters, int nIndex)
{
	assert(nIndex >= 1);
	LPCTSTR		szReturn;

	szReturn = szFilters;

	//
	// 找到第nIndex个Filter
	for (int i = 0; i < nIndex * 2 - 1; i++)
	{
		while (*szReturn != '\0')
			szReturn++;
		szReturn++;
		if (*szReturn == '\0')
			return NULL;
	}

	// 移动直到扩展名
	while (*szReturn != '.' && *szReturn != '\0')
		szReturn++;

	return szReturn;
}

UINT_PTR CALLBACK MyOFNHookProc(
								HWND hdlg,      // handle to dialog box
								UINT uiMsg,      // message identifier
								WPARAM wParam,  // message parameter
								LPARAM lParam   // message parameter
								)
{
	switch (uiMsg)
	{
	case WM_NOTIFY:
		LPOFNOTIFY		lpOfNotify;

		lpOfNotify = (LPOFNOTIFY)lParam;
		if (lpOfNotify)
		{
			//DBGLOG2(_T("hdr.code: %d, hdr.idFrom: %d"), lpOfNotify->hdr.code, lpOfNotify->hdr.idFrom);

			if (lpOfNotify->hdr.code == CDN_TYPECHANGE)
			{
				//if (lpOfNotify->pszFile)
				//_tcscpy(lpOfNotify->pszFile, _T("aa.snc"));
				//DBGLOG3(_T("HWND: %x, hWndFrom: %x, FilterIndex: %d"), hdlg, lpOfNotify->hdr.hwndFrom, lpOfNotify->lpOFN->nFilterIndex);
				TCHAR	szFileName[MAX_PATH];
				LPCTSTR	szNewExt;

				szNewExt = GetFileFilterExtByIndex(lpOfNotify->lpOFN->lpstrFilter, lpOfNotify->lpOFN->nFilterIndex);
				if (lpOfNotify->lpOFN->nFilterIndex > 1 && szNewExt)
				{

					GetDlgItemText(lpOfNotify->hdr.hwndFrom, cmb13, szFileName, MAX_PATH);
					FileSetExt(szFileName, CountOf(szFileName), szNewExt);
					SetDlgItemText(lpOfNotify->hdr.hwndFrom, cmb13, szFileName);
				}
			}
			return 1;
		}
		break;
	}
	return 0;
}

CFileSaveDlg::CFileSaveDlg(LPCTSTR szTitle, LPCTSTR szFile, LPCTSTR szFilter, int nDefFileType)
{
	_tcscpy_safe(m_szFile, CountOf(m_szFile), szFile);

	memset(&m_openfile, 0, sizeof(m_openfile));
	m_openfile.lpstrTitle = szTitle;
	m_openfile.lpstrFilter = szFilter;
	m_openfile.lpstrFile = m_szFile;
	m_openfile.nMaxFile = CountOf(m_szFile);
	m_openfile.nFilterIndex = nDefFileType;

	m_openfile.lStructSize = sizeof(m_openfile);
	m_openfile.hInstance = GetAppInstance();
	m_openfile.Flags = OFN_OVERWRITEPROMPT | OFN_ENABLEHOOK | OFN_EXPLORER;
	m_openfile.lpfnHook = (LPOFNHOOKPROC)MyOFNHookProc;
}

CFileSaveDlg::~CFileSaveDlg(void)
{
}

int CFileSaveDlg::DoModal(CBaseWnd *pWndParent)
{
	m_openfile.hwndOwner = pWndParent->GetHandle();
	if (GetSaveFileName(&m_openfile))
		return IDOK;
	else
		return IDCANCEL;
}

LPCTSTR CFileSaveDlg::GetSaveFile()
{
	return m_szFile;
}

LPCTSTR CFileSaveDlg::GetSelectedExt()
{
	LPCTSTR		szNewExt;
	szNewExt = GetFileFilterExtByIndex(m_openfile.lpstrFilter, m_openfile.nFilterIndex);
	if (szNewExt)
		return szNewExt;
	else
		return _T("");
}
