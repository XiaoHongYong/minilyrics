/********************************************************************
	Created  :	2002/02/04	23:08
	FileName :	FolderDialog.cpp
	Author   :	xhy
	
	Purpose  :	浏览选择文件夹的类
*********************************************************************/

#include "stdafx.h"
#include "shlobj.h"
#include "BaseWnd.h"
#include "FolderDialog.h"


int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
	TCHAR szDir[MAX_PATH];
	
	switch(uMsg)
	{
	case BFFM_INITIALIZED:
		{
#ifdef _WINCE_
#else
			CFolderDialog		*pDlg;

			pDlg = (CFolderDialog *)pData;
			assert(pDlg);

			if (pDlg && (!pDlg->m_strInitFolder.empty()))
			{
				// WParam is TRUE since you are passing a path.
				// It would be FALSE if you were passing a pidl.
				SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)pDlg->m_strInitFolder.c_str());
			}
#endif
			break;
		}
	case BFFM_SELCHANGED:
		{
			// Set the status window to the currently selected path.
			if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
			{
				SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
			}
			break;
		}
	default:
		break;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

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
	LPMALLOC	pMalloc;

	if (SHGetMalloc(&pMalloc) != NOERROR)
	{
		return FALSE;
	}

	BROWSEINFO		bInfo;
	LPITEMIDLIST	pidl;

	ZeroMemory((PVOID)&bInfo, sizeof(bInfo));

	if (!m_strRootFolder.empty())
	{
		OLECHAR		olePath[MAX_PATH];
		ULONG		chEaten;
		ULONG		dwAttributes;
		HRESULT		hr;
		LPSHELLFOLDER		pDesktopFolder;

		// Get a pointer to the Desktop's IShellFolder interface.
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
		{
			// IShellFolder::ParseDisplayName requires the file name be in Unicode
#ifndef _UNICODE
			MultiByteToWideChar(CP_ACP,
				MB_PRECOMPOSED,
				m_strRootFolder.c_str(),
				-1,
				olePath,
				MAX_PATH
			);
#else
			wcscpy_safe(olePath, CountOf(olePath), m_strRootFolder.c_str());
#endif

			// Convert the path to ITEMIDLIST
			hr = pDesktopFolder->ParseDisplayName(NULL,
				NULL,
				olePath,
				&chEaten,
				&pidl,
				&dwAttributes
			);
			if (FAILED(hr))
				goto BR_FAILED;
			bInfo.pidlRoot = pidl;
		}
	}

	bInfo.hwndOwner = pWndParent->GetHandle();

#ifdef _WINCE_
	static char		__szPath[MAX_PATH];
	bInfo.pszDisplayName = __szPath;
	bInfo.lpszTitle = "Choose Folder";	// STRING
#else
	bInfo.pszDisplayName = m_szPath;
	if (m_strTitle.empty())
		bInfo.lpszTitle = _TLT("Choose Folder");	// STRING
#endif

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE 0x0040
#endif
	bInfo.ulFlags = /*BIF_EDITBOX | */BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;//BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;

	bInfo.lpfn = BrowseCallbackProc;	// address of callback function.
	bInfo.lParam = (LPARAM)this;		// pass address of object to callback function

	pidl = ::SHBrowseForFolder(&bInfo);
	if (pidl == NULL)
		goto BR_FAILED;

//	m_iImageIndex = bInfo.iImage;

	if (::SHGetPathFromIDList(pidl, m_szPath) == FALSE)
		goto BR_FAILED;

	pMalloc->Free(pidl);
	pMalloc->Release();
	return TRUE;

BR_FAILED:
	if (pMalloc)
	{
		pMalloc->Free(pidl);
		pMalloc->Release();
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
