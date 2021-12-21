/********************************************************************
	Created  :	2001年12月6日 21:52:18
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

class CPagePfRoot : public CPagePfBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
	CPagePfRoot();

	void OnInitialUpdate();

};

UIOBJECT_CLASS_NAME_IMP(CPagePfRoot, _T("PreferPage.Root"))

CPagePfRoot::CPagePfRoot() : CPagePfBase(PAGE_UNKNOWN, _T("CMD_ROOT_UI"))
{
}

void CPagePfRoot::OnInitialUpdate()
{
	CPagePfBase::OnInitialUpdate();

	SetDefaultPreferPage(this, (PreferPageID)GetExPoolInt(SZ_EX_POOL_PF_DEFAULT_PAGE));
	CheckToolbarDefaultPage(_T("CID_TB_PREFERENCE"));
}

void ShowPreferenceDialog(CSkinWnd *pParent, bool bFloatingLyr, PreferPageID preferPageId)
{
	ASSERT(preferPageId != PAGE_UNKNOWN);
	SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, _SZ_SKINWND_CLASS_NAME, 
		_T("Preferences.xml"), pParent);

	skinWndStartupInfo.mapExchangePool[SZ_EX_POOL_PF_FLOATING_LYR] = BOOLTOSTR(bFloatingLyr);
	skinWndStartupInfo.mapExchangePool[SZ_EX_POOL_PF_DEFAULT_PAGE] = CStrPrintf(_T("%d"), preferPageId).c_str();

	CSkinApp::GetInstance()->GetSkinFactory()->ActiveOrCreateSkinWnd(skinWndStartupInfo);
}

void RegisterPreferencePage(CSkinFactory *pSkinFactory)
{
	RegisterPfUIPages(pSkinFactory);
	RegisterPfThemePages(pSkinFactory);
	RegisterPfSystemPages(pSkinFactory);
	RegisterPfLyricsPages(pSkinFactory);
	AddUIObjNewer2(pSkinFactory, CPagePfAdvanced);
	AddUIObjNewer2(pSkinFactory, CPagePfRoot);
}
