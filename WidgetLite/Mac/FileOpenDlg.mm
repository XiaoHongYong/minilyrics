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
	if (bAllowMultiSel)
		m_nBufLen = 1024 * 10;
	else
		m_nBufLen = MAX_PATH;

	m_bAllowMultiSel = bAllowMultiSel;
	m_szFile = new TCHAR[m_nBufLen];
	EmptyStr(m_szFile);

	if (szFile)
		_tcscpy_safe(m_szFile, m_nBufLen, szFile);
	
	if (szFilter)
		ExtFiltersToSet(szFilter, m_setExt);
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

	[openDlg setAllowsMultipleSelection:NO];
	
	[openDlg setDirectoryURL:[NSURL URLWithString:[NSString stringWithUTF8String:m_szFile]]];

	[openDlg setAllowedFileTypes:SetStrToExtArray(m_setExt)];

	if ([openDlg runModal] == NSOKButton)
	{
		NSArray* files = [openDlg filenames];

		// Loop through all the files and process them.
		for(int i = 0; i < [files count]; i++ )
		{
			NSString* fileName = [files objectAtIndex:i];
			
			_tcscpy_safe(m_szFile, m_nBufLen, [fileName UTF8String]);
			break;
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
