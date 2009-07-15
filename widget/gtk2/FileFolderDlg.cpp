// FileOpenDlg.cpp: implementation of the CFileOpenDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BaseWnd.h"
#include "FileFolderDlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileOpenDlg::CFileOpenDlg(LPCTSTR szTitle, LPCTSTR szFile, LPCTSTR szFilter, int nDefFileType, bool bAllowMultiSel)
{
	m_bAllowMultiSel = bAllowMultiSel;

	m_strTitle = szTitle;
	if (szFile)
		m_strInitFile = szFile;
	m_window = NULL;
}

CFileOpenDlg::~CFileOpenDlg()
{
}

int CFileOpenDlg::DoModal(CBaseWnd *pWndParent)
{
	int		nRet;

	m_vSelectedFiles.clear();

	m_window = gtk_file_chooser_dialog_new(m_strTitle.c_str(),
		GTK_WINDOW(pWndParent->GetHandle()),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_OK,
		NULL);
	if (!m_window)
		return GTK_RESPONSE_CANCEL;

	if (!m_strInitFile.empty())
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(m_window), m_strInitFile.c_str());

	if (m_bAllowMultiSel)
		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(m_window), true);

	nRet = gtk_dialog_run(GTK_DIALOG(m_window));

	// Save selected files in list.
	if (m_bAllowMultiSel)
	{
		GSList*		listFiles;

		listFiles = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(m_window));
		GSList*		p = listFiles;
		while (p)
		{
			m_vSelectedFiles.push_back((const char *)p->data);
			g_free(p->data);
			p = p->next;
		}
		g_slist_free(listFiles);
	}
	else
	{
		char *szFile = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_window));
		if (szFile)
		{
			m_vSelectedFiles.push_back(szFile);
			g_free(szFile);
		}
	}

	gtk_widget_destroy(m_window);

	return nRet;
}

LPCTSTR CFileOpenDlg::GetOpenFile()
{
	if (m_vSelectedFiles.size())
		return m_vSelectedFiles[0].c_str();
	else
		return "";
}

void CFileOpenDlg::GetOpenFile(vector<tstring> &vFiles)
{
	vFiles = m_vSelectedFiles;
}

LPCTSTR CFileOpenDlg::GetSaveFile()
{
	return GetOpenFile();
}

LPCTSTR CFileOpenDlg::GetSelectedExt()
{
	return FileGetExt(GetOpenFile());
}

//////////////////////////////////////////////////////////////////////////
CFolderDialog::CFolderDialog()
{
}

CFolderDialog::~CFolderDialog()
{

}

void CFolderDialog::SetRootFolder(LPCTSTR szRootFolder)
{

}

void CFolderDialog::SetTitle(LPCTSTR szTitle)
{
	m_strTitle = szTitle;
}

void CFolderDialog::SetInitFolder(LPCTSTR szInitDir)
{
	m_strInitFolder = szInitDir;
}

LPCTSTR CFolderDialog::GetFolder()
{
	if (!m_szFile)
		m_szFile = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_window));

	if (m_szFile)
		return m_szFile;
	else
		return "";
}

BOOL CFolderDialog::DoBrowse(CBaseWnd *pWndParent)
{
	int		nRet;

	m_window = gtk_file_chooser_dialog_new(m_strTitle.c_str(),
		GTK_WINDOW(pWndParent->GetHandle()),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);

	if (!m_strInitFolder.empty())
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(m_window), m_strInitFolder.c_str());

	nRet = gtk_dialog_run(GTK_DIALOG(m_window));

	gtk_widget_destroy(m_window);

	return nRet;
}
