#include "StdAfx.h"
#include "iPodLyricsDownloaderApp.h"
#include "LDSkinWnd.h"
#include "LDSkinWndMain.h"

CLDSkinWndMain::CLDSkinWndMain(void)
{
	m_pFileListCtrl = NULL;
}

CLDSkinWndMain::~CLDSkinWndMain(void)
{
}

void CLDSkinWndMain::OnSkinLoaded()
{
	CLDSkinWnd::OnSkinLoaded();

	m_pFileListCtrl = (CSkinListCtrl*)GetUIObjectById(CMD_FILE_LIST, CSkinListCtrl::ClassName());
	ASSERT(m_pFileListCtrl);
	if (m_pFileListCtrl)
	{
		m_pFileListCtrl->AddColumn(_T(""), 60);
		m_pFileListCtrl->AddColumn(_T("Artist"), 250);
		m_pFileListCtrl->AddColumn(_T("Title"), 300);
		m_pFileListCtrl->AddColumn(_T("Lyrics"), 300);
		m_pFileListCtrl->InsertItem(0, _T(""));
		m_pFileListCtrl->SetItemText(0, 1, _T("Just a test artist"));
		m_pFileListCtrl->SetItemText(0, 1, _T("Just a test title"));
	}
}

void CLDSkinWndMain::OnDestroy()
{
	CiPodLyricsDownloaderApp::GetInstance()->PostQuitMessage();

	CLDSkinWnd::OnDestroy();
}
