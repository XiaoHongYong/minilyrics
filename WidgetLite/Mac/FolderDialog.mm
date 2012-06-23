/********************************************************************
	Created  :	2002/02/04	23:08
	FileName :	FolderDialog.cpp
	Author   :	xhy
	
	Purpose  :	浏览选择文件夹的类
*********************************************************************/

#include "stdafx.h"
#include "WidgetLite.h"
#include "FolderDialog.h"

CFolderDialog::CFolderDialog()
{
	m_strInitFolder = _T("");
	m_strRootFolder = _T("");
	m_strTitle = _T("");
	m_szPath[0] = '\0';
}

CFolderDialog::~CFolderDialog()
{
}

BOOL CFolderDialog::DoBrowse(CBaseWnd *pWndParent)
{
	NSOpenPanel* openDlg = [NSOpenPanel openPanel];
	
	// Enable the selection of files in the dialog.
	[openDlg setCanChooseFiles:NO];
	
	// Enable the selection of directories in the dialog.
	[openDlg setCanChooseDirectories:YES];
	
	if ([openDlg runModal] == NSOKButton)
	{
		NSArray* files = [openDlg filenames];
		
		// Loop through all the files and process them.
		for(int i = 0; i < [files count]; i++ )
		{
			NSString* fileName = [files objectAtIndex:i];
			
			_tcscpy_safe(m_szPath, CountOf(m_szPath), [fileName UTF8String]);
			break;
		}
		
		return TRUE;
	}

	return FALSE;
}

LPCTSTR CFolderDialog::GetFolder()
{
	return m_szPath;
}

void CFolderDialog::SetInitFolder(LPCTSTR szInitDir)
{
	m_strInitFolder = szInitDir;

	// 去掉目录后面的\ 
	if (m_strInitFolder.size() > 0 && m_strInitFolder[m_strInitFolder.size() - 1] == DIR_SLASH)
		m_strInitFolder[m_strInitFolder.size() - 1] = '\0';
}

void CFolderDialog::SetTitle(LPCTSTR szTitle)
{
	m_strTitle = szTitle;
}

void CFolderDialog::SetRootFolder(LPCTSTR szRootFolder)
{
	m_strRootFolder = szRootFolder;
}
