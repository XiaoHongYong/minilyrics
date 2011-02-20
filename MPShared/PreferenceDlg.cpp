/********************************************************************
	Created  :	2001Äê12ÔÂ6ÈÕ 21:52:18
	FileName :	PreferenceDlg.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferenceDlg.h"
#include "PreferPageSystem.h"
#include "PreferPageUI.h"
#include "PreferPageLyrics.h"
#include "PreferPageTheme.h"
#include "PreferPageAdvanced.h"

#ifdef _MPLAYER
#include "PreferPageAssociation.h"
#endif

CPreferenceDlg::CPreferenceDlg(bool bFloatingLyrMode, PreferPageID defaultPage)
	: CMPSkinDialog(_T("Preferences.xml"))
{
	m_bFloatingLyrMode = bFloatingLyrMode;
	m_defaultPage = defaultPage;
}

void CPreferenceDlg::OnSkinLoaded()
{
	m_pageHandler.AddPage(new CPagePfUIRoot());
	m_pageHandler.AddPage(new CPagePfThemeRoot(m_bFloatingLyrMode));
	m_pageHandler.AddPage(new CPagePfSystemRoot());
	m_pageHandler.AddPage(new CPagePfLyricsRoot());
	m_pageHandler.AddPage(new CPagePfAdvanced());

	int				nDefaultRootPage = UID_INVALID;

	// find default page.
	CSkinPageHandler::ListPageHandler::iterator it, itEnd = m_pageHandler.ChildEnd();
	for (it = m_pageHandler.ChildBegin(); it != itEnd; ++it)
	{
		CPagePfBase	*pPageRoot = (CPagePfBase *)*it;
		if (pPageRoot->SetDefaultPreferPage(m_defaultPage))
		{
			nDefaultRootPage = pPageRoot->GetPageId();
			break;
		}
	}

	m_pageHandler.SetDefaultPage(nDefaultRootPage);

	CMPSkinDialog::OnSkinLoaded();

	CSkinToolbar*pToolbar = (CSkinToolbar *)GetUIObjectById(GetIDByName(_T("CID_TB_PREFERENCE")),
		CSkinToolbar::ClassName());
	if (pToolbar)
	{
		CPagePfBase	*pPageRoot = (CPagePfBase *)m_pageHandler.GetPage(nDefaultRootPage);
		ASSERT(pPageRoot);
		if (pPageRoot)
			pToolbar->SetCheck(pPageRoot->GetAssociateTabButtonId(), true, false);
	}
}

bool CPreferenceDlg::OnCustomCommand(int nId)
{
	if (CMPSkinDialog::OnCustomCommand(nId))
		return true;

	CSkinPageHandler::ListPageHandler::iterator it, itEnd = m_pageHandler.ChildEnd();
	for (it = m_pageHandler.ChildBegin(); it != itEnd; ++it)
	{
		CPagePfBase	*pPageRoot = (CPagePfBase *)*it;
		if (pPageRoot->GetAssociateTabButtonId() == nId)
		{
			m_pageHandler.SwitchToPage(pPageRoot->GetPageId());
			return true;
		}
	}

	return false;
}

