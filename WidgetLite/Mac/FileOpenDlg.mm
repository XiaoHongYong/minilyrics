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

void ExtFiltersToSet(LPCTSTR szFilter, SetString &setExts)
{
	VecString vStr;
	MultiStrToVStr(szFilter, vStr);
	
	tstring str;
	for (uint i = 0; i < vStr.size(); i++) {
		LPCTSTR p = vStr[i].c_str();
		while (p != NULL) {
			LPCTSTR beg = _tcschr(p, '.');
			if (beg == NULL)
				break;
			beg++;
			LPCTSTR end = beg;
			while (IsAlphaDigit(*end))
				end++;
			str.clear();
			str.append(beg, int(end - beg));
			setExts.insert(ToLower(str.c_str()));
			p = end;
		}
	}
}

NSArray *SetStrToExtArray(SetString &setExt)
{
	if (setExt.size() == 0)
		return nil;
	
	NSMutableArray *ext = [[NSMutableArray alloc] init];
	for (SetString::iterator it = setExt.begin(); it != setExt.end(); ++it) {
		NSString *s = [NSString stringWithUTF8String:(*it).c_str()];
		[ext addObject:s];
	}
	
	return ext;
}

//////////////////////////////////////////////////////////////////////

CFileOpenDlg::CFileOpenDlg(LPCTSTR szTitle, LPCTSTR szFile, LPCTSTR szFilter, int nDefFileType, bool bAllowMultiSel)
{
	m_bAllowMultiSel = bAllowMultiSel;

	if (szFile)
		m_vFiles.push_back(szFile);
	
	if (szFilter)
		ExtFiltersToSet(szFilter, m_setExt);
}

CFileOpenDlg::~CFileOpenDlg()
{
}

int CFileOpenDlg::DoModal(CBaseWnd *pWndParent)
{
	NSOpenPanel* openDlg = [NSOpenPanel openPanel];
	
	// Enable the selection of files in the dialog.
	[openDlg setCanChooseFiles:YES];
	
	// Enable the selection of directories in the dialog.
	[openDlg setCanChooseDirectories:NO];

	[openDlg setAllowsMultipleSelection: m_bAllowMultiSel ? YES : NO];
	
	// [openDlg setDirectoryURL:[NSURL URLWithString:[NSString stringWithUTF8String:m_szFile]]];

	[openDlg setAllowedFileTypes:SetStrToExtArray(m_setExt)];

	m_vFiles.clear();

	if ([openDlg runModal] == NSOKButton)
	{
		NSArray* files = [openDlg filenames];

		// Loop through all the files and process them.
		for(int i = 0; i < [files count]; i++ )
		{
			NSString* fileName = [files objectAtIndex:i];
			m_vFiles.push_back([fileName UTF8String]);
		}
		
		return IDOK;
	}

	return IDCANCEL;
}

LPCTSTR CFileOpenDlg::GetOpenFile()
{
	if (m_vFiles.size() > 0)
		return m_vFiles[0].c_str();
	else
		return _T("");
}

void CFileOpenDlg::GetOpenFile(vector<tstring> &vFiles)
{
	vFiles = m_vFiles;
}
