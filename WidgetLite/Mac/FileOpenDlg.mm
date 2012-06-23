#include "stdafx.h"
#ifndef _NO_WIGET_LIB
#include "WidgetLite.h"
#endif
#include "FileOpenDlg.h"


void CFileDlgExtFilter::AddExtention(LPCTSTR szDesc, LPCTSTR szExt)
{
	append(szDesc);
	append(1, TCHAR('\0'));

	append(szExt);
	append(1, TCHAR('\0'));
}

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
}

CFileOpenDlg::~CFileOpenDlg()
{
	delete[] m_szFile;
}

int CFileOpenDlg::DoModal(CBaseWnd *pWndParent)
{
	NSOpenPanel* openDlg = [NSOpenPanel openPanel];
	
	// Enable the selection of files in the dialog.
	[openDlg setCanChooseFiles:YES];
	
	// Enable the selection of directories in the dialog.
	[openDlg setCanChooseDirectories:NO];

	if ([openDlg runModal] == NSOKButton)
	{
		NSArray* files = [openDlg filenames];

		// Loop through all the files and process them.
		for( i = 0; i < [files count]; i++ )
		{
			NSString* fileName = [files objectAtIndex:i];
			
			// Do something with the filename.
		}
		
		return IDOK;
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
