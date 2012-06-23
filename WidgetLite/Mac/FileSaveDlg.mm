#include "StdAfx.h"
#include "WidgetLite.h"
#include "FileSaveDlg.h"

NSArray *SetStrToExtArray(SetString &setExt);

void ExtFiltersToSet(LPCTSTR szFilter, SetString &setExts);

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

CFileSaveDlg::CFileSaveDlg(LPCTSTR szTitle, LPCTSTR szFile, LPCTSTR szFilter, int nDefFileType)
{
	_tcscpy_safe(m_szFile, CountOf(m_szFile), szFile);
	
	if (szFilter)
		ExtFiltersToSet(szFilter, m_setExt);
}

CFileSaveDlg::~CFileSaveDlg(void)
{
}

int CFileSaveDlg::DoModal(CBaseWnd *pWndParent)
{
	NSSavePanel* saveDlg = [NSSavePanel savePanel];
	
	// Enable the selection of files in the dialog.
	[saveDlg setDirectoryURL:[NSURL URLWithString:[NSString stringWithUTF8String:m_szFile]]];
	
	[saveDlg setAllowedFileTypes:SetStrToExtArray(m_setExt)];

	//[saveDlg set:YES];
	
	// Enable the selection of directories in the dialog.
	//[saveDlg setCanChooseDirectories:NO];
	
	if ([saveDlg runModal] == NSOKButton)
	{
		NSString *fileName = [saveDlg filename];

		_tcscpy_safe(m_szFile, CountOf(m_szFile), [fileName UTF8String]);
		
		return IDOK;
	}
	
	return IDCANCEL;
}

LPCTSTR CFileSaveDlg::GetSaveFile()
{
	return m_szFile;
}

LPCTSTR CFileSaveDlg::GetSelectedExt()
{
	return _T("");
}
