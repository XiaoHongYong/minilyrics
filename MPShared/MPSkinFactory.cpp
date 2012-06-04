// MPSkinFactory.cpp: implementation of the CMPSkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MPSkinFactory.h"
#include "MPSkinWnd.h"
#include "LyricShowMultiRowObj.h"
#include "LyricShowTxtObj.h"
#include "LyricShowTwoRowObj.h"
#include "LyricShowHorzScrollObj.h"
#include "LyricShowSingleRowObj.h"
#include "LyricShowVobSub.h"
#include "LyricShowAgentObj.h"
#include "LyricShowTextEditObj.h"
#include "LyricShowTxtContainer.h"
#include "SkinListCtrlEx.h"
#include "SkinRateCtrl.h"
#include "SkinPicText.h"
#include "SkinActivateCtrl.h"
#include "MPSkinTimeCtrl.h"
#include "MPSkinInfoTextCtrl.h"
#include "MediaInfoTextCtrl.h"
#include "MediaAlbumArtCtrl.h"
#include "SkinFilterCtrl.h"
#include "MPSkinInfoTextCtrlEx.h"
#include "DlgAbout.h"
#include "DlgSearchLyrics.h"
#include "PreferenceDlg.h"
#include "DlgUpload.h"
#include "DlgAdjustHue.h"
#include "MPlaylistCtrl.h"
#include "MPSkinMediaNumInfoCtrl.h"
#include "MPlaylistSearchObj.h"

#ifdef _MPLAYER
#include "SkinTreeCtrl.h"
#include "MPSkinVis.h"
#ifdef _WIN32_WCE
#include "win32/wce/WCEMainCmdBar.h"
#endif
#endif

#ifdef _WIN32
#include "win32/LyricsDownloader.h"
#endif

#include "MPSkinMenu.h"

#ifdef _WIN32_DESKTOP
#include "win32/MPFloatingLyrWnd.h"
#else
#include "MPFloatingLyrWnd.h"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPSkinFactory::CMPSkinFactory(CSkinApp *pApp, UIObjectIDDefinition uidDefinition[])
	: CSkinFactory(pApp, uidDefinition)
{
#ifdef _WIN32_DESKTOP
	m_bClickThrough = false;
#endif

	SetMenuIDByUID(CMD_QUIT, IDC_EXIT);
}

CMPSkinFactory::~CMPSkinFactory()
{

}

int CMPSkinFactory::Init()
{
	AddUIObjNewer(CLyricShowMultiRowObj);
	AddUIObjNewer(CLyricShowTxtObj);
	AddUIObjNewer(CLyricShowTwoRowObj);
	AddUIObjNewer(CLyricShowHorzScrollObj);
	AddUIObjNewer(CLyricShowSingleRowObj);
	AddUIObjNewer(CLyricShowVobSub);
	AddUIObjNewer(CLyricShowTextEditObj);
	AddUIObjNewer(CLyricShowTxtContainer);
	AddUIObjNewer(CLyricShowAgentObj);
	AddUIObjNewer(CSkinRateCtrl);
	AddUIObjNewer(CSkinPicText);
	AddUIObjNewer(CMPSkinTimeCtrl<CSkinPicText>);
	AddUIObjNewer(CMPSkinTimeCtrl<CSkinStaticText>);
	AddUIObjNewer(CMPSkinInfoTextCtrl);
	AddUIObjNewer(CMPSkinInfoTextCtrlEx);
	AddUIObjNewer(CSkinFilterCtrl);
#ifdef _MPLAYER
	AddUIObjNewer(CSkinTreeCtrl);
	AddUIObjNewer(CMPSkinVis);
#endif
	AddUIObjNewer(CSkinListCtrlEx);
	AddUIObjNewer(CMPSkinMediaNumInfoCtrl);
	AddUIObjNewer(CMPlaylistSearchObj);
	AddUIObjNewer(CMediaInfoTextCtrl);
	AddUIObjNewer(CMediaAlbumArtCtrl);
	AddUIObjNewer(CSkinActivateCtrl);
	AddUIObjNewer(CMPlaylistCtrl);

	RegisterAboutPage(this);
	RegisterSearchLyricsPage(this);
	RegisterPreferencePage(this);
	RegisterUploadLyrPage(this);
	RegisterAdjustHuePage(this);
	
#ifndef _MAC_OSX
	RegisterLyricsToolPage(this);
#endif

	return CSkinFactory::Init();
}

CSkinWnd *CMPSkinFactory::NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd)
{
	if (bMainWnd)
		return new CMPSkinMainWnd;
	else
		return new CMPSkinWnd;
}

CSkinMenu *CMPSkinFactory::LoadPresetMenu(CSkinWnd *pWnd, LPCTSTR szMenu)
{
	CSkinMenu	*pSkinMenu = NULL;

	if (_tcsicmp(szMenu, _T("MainMenu")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadMenu(IDM_NORMAL_STAT);
	}
	else if (_tcsicmp(szMenu, _T("LyricsMenu")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadMenu(IDM_FLOATING_LYRICS);
	}
	else if (_tcsicmp(szMenu, _T("LyrEditorMenu")) == 0)
	{
		pSkinMenu = new CLyrEditorMenu();
		// pSkinMenu->LoadMenu(IDM_LYR_EDITOR);
		pSkinMenu->LoadMenu(IDM_LYR_EDITOR);
	}
	else if (_tcsicmp(szMenu, _T("MainWndMenu")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		// pSkinMenu->LoadMenu(IDM_MAIN_WND);
		pSkinMenu->LoadMenu(IDM_MAIN_WND);
	}
	else if (_tcsicmp(szMenu, _T("LyrDlCtxMenu")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadMenu(IDM_NORMAL_STAT);
		pMenu->RemoveItem(7); // 7 is Lyrics Downloader for iPod
	}
	else if (_tcsicmp(szMenu, _T("iPodLyricsDownloader")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pSkinMenu->LoadMenu(IDM_IPOD_LYR_DOWNLOADER);
	}
#ifdef _MPLAYER
	else if (_tcsicmp(szMenu, _T("PlaylistMenu")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadPopupMenu(IDM_MENU, 2);
	}
	else if (_tcsicmp(szMenu, _T("MediaLibMenu")) == 0)
	{
		pSkinMenu = new CSkinMenu(this);
		pSkinMenu->LoadPopupMenu(IDM_PLAYER_MENU, 1);
	}
	else if (_tcsicmp(szMenu, _T("MediaGuideTreeMenu")) == 0)
	{
		pSkinMenu = new CSkinMenu(this);
		pSkinMenu->LoadPopupMenu(IDM_PLAYER_MENU, 1);
	}
	else if (_tcsicmp(szMenu, _T("MediaGuideSongListMenu")) == 0)
	{
		pSkinMenu = new CSkinMenu(this);
		pSkinMenu->LoadPopupMenu(IDM_PLAYER_MENU, 1);
	}
#endif
	else
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadMenu(IDM_NORMAL_STAT);
	}

	return pSkinMenu;
}

void CMPSkinFactory::TopmostAll(bool bTopmost)
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();

	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pWnd = *it;
		pWnd->SetTopmost(bTopmost);
	}
}

void CMPSkinFactory::MinizeAll(bool bSilently)
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();

	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pWnd = *it;
#ifdef _WIN32
		if (::GetParent(pWnd->GetHandle()) != NULL)
			continue;
#endif
		if (pWnd->IsToolWindow())
		{
			pWnd->ShowWindow(SW_SHOWMINNOACTIVE);
			pWnd->ShowWindow(SW_HIDE);
		}
		else
			pWnd->ShowWindow(bSilently ? SW_SHOWMINNOACTIVE : SW_MINIMIZE);
	}
}

void CMPSkinFactory::RestoreAll()
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();

	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pWnd = *it;

		if (pWnd->IsIconic())
			pWnd->ShowWindow(SW_SHOWNOACTIVATE);

		if (pWnd->IsToolWindow())
			pWnd->ShowWindow(SW_SHOW);
	}
}

int CMPSkinFactory::GetIDByNameEx(LPCTSTR szId, tstring &strToolTip)
{
	int		nID = CSkinFactory::GetIDByNameEx(szId, strToolTip);
	if (nID == UID_INVALID)
		return nID;

	tstring		strHotkey;

	if (CMPlayerAppBase::GetHotkey().GetHotkeyText(nID, strHotkey))
	{
		strToolTip = _TL(strToolTip.c_str());
		strToolTip += _T(" (");
		strToolTip += strHotkey;
		strToolTip += _T(")");
	}

	return nID;
}

void CMPSkinFactory::GetTooltip(int nId, tstring &strToolTip)
{
	CSkinFactory::GetTooltip(nId, strToolTip);

	if (strToolTip.size())
	{
		tstring		strHotkey;

		if (CMPlayerAppBase::GetHotkey().GetHotkeyText(nId, strHotkey))
		{
			strToolTip = _TL(strToolTip.c_str());
			strToolTip += _T(" (");
			strToolTip += strHotkey;
			strToolTip += _T(")");
		}
	}
}

int CMPSkinFactory::OpenSimpliestSkin()
{
	int			nRet;
	tstring		strSkinFile;

	strSkinFile = GetWorkingFolder();
	strSkinFile += _T("sskin.xml");

	nRet = OpenSkinFile(strSkinFile.c_str());
	if (nRet != ERR_OK)
		return nRet;

	ASSERT(m_listSkinWnds.size() > 0);
	if (m_listSkinWnds.size() > 0)
	{
		CSkinWnd *pWnd = m_listSkinWnds.front();
		pWnd->OpenSkin(_T("MiniLyrics"));
	}

	return ERR_OK;
}

void CMPSkinFactory::AdjustHue(float hue, float saturation, float luminance)
{
	CSkinFactory::AdjustHue(hue, saturation, luminance);

	g_wndFloatingLyr.OnAdjustHue(hue, saturation, luminance);
	g_wndFloatingLyr.InvalidateRect();
}

#ifndef _MPLAYER
void CMPSkinFactory::BeforeTrackMoveWith(CBaseWnd *pWndChain[], int nCount, CBaseWnd *pWndToTrack)
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();

	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pWnd = *it;
		pWnd->m_WndDrag.BeforeTrackMoveWith(pWndChain, nCount, pWndToTrack);
	}
}

void CMPSkinFactory::TrackMoveWith(CBaseWnd *pWnd, int x, int y)
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();

	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pWnd = *it;
		pWnd->m_WndDrag.TrackMoveWith(pWnd, x, y);
	}
}

void CMPSkinFactory::AddWndCloseto(CBaseWnd *pWnd, LPCTSTR szWndName, LPCTSTR szClass)
{
	m_WndCloseToPlayers.AddWndCloseto(pWnd, szClass, szWndName);
}

// ISkinWndDragHost
void CMPSkinFactory::GetWndDragAutoCloseTo(vector<CBaseWnd *> &vWnd)
{
	CSkinFactory::GetWndDragAutoCloseTo(vWnd);

	V_WNDCLOSETO::iterator	it, itEnd;
	itEnd = m_WndCloseToPlayers.m_vWndCloseTo.end();
	for (it = m_WndCloseToPlayers.m_vWndCloseTo.begin(); it != itEnd; ++it)
	{
		CBaseWnd		*pWnd;
		CWndDrag::WndCloseTo		&item = *it;

		if (item.pWnd)
			pWnd = item.pWnd;
		else
		{
			const TCHAR	*szClass = item.strClass.c_str();
			if (IsEmptyString(szClass))
				szClass = NULL;
			const TCHAR	*szWnd = item.strWndName.c_str();
			if (IsEmptyString(szWnd))
				szWnd = NULL;
			pWnd = MLWidget::FindWindow(szClass, szWnd);
		}

		if (pWnd)
			vWnd.push_back(pWnd);
	}
}
#endif

#ifdef _WIN32_DESKTOP

void CMPSkinFactory::AllUpdateTransparent()
{
	if (!IsLayeredWndSupported())
		return;

	int		nOpaque = g_Profile.GetProfileInt(SZ_SECT_UI, _T("WindowOpaquePercent"), 100);
	byte	byAlpha = OpaquePercentToAlpha(nOpaque);

#ifndef _MPLAYER
	if (g_wndFloatingLyr.IsValid())
		g_wndFloatingLyr.SetTransparent(byAlpha, false);

	if (::GetParent(CMPlayerAppBase::GetMainWnd()->GetHandle()))
		return;
#endif

	LIST_SKINWNDS::iterator	it, itEnd;

	itEnd = m_listSkinWnds.end();
	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pWnd = *it;
		pWnd->SetTransparent(byAlpha, m_bClickThrough);
	}
}

void CMPSkinFactory::SetClickThrough(bool bClickThrough)
{
	m_bClickThrough = bClickThrough;

	// Save ClickThrough options
	g_Profile.WriteProfileInt(SZ_SECT_UI, _T("ClickThrough"), m_bClickThrough);

	if (m_bClickThrough)
	{
		// Show MiniLyrics system tray icon.
		int			nShowPos;

		nShowPos = g_Profile.GetProfileInt(SZ_SECT_UI, _T("ShowIconOn"), SHOW_ICON_ON_TASKBAR);
		if (nShowPos == SHOW_ICON_ON_NONE || nShowPos == SHOW_ICON_ON_TASKBAR)
		{
			g_Profile.WriteProfileInt(SZ_SECT_UI, _T("ShowIconOn"), SHOW_ICON_ON_TASKBAR);
			CMPlayerAppBase::GetMainWnd()->GetTrayIcon().UpdateShowIconPos();
		}
	}

	AllUpdateTransparent();
}
#else

void CMPSkinFactory::AllUpdateTransparent()
{
}

void CMPSkinFactory::SetClickThrough(bool bClickThrough)
{
	m_bClickThrough = bClickThrough;
	
	// Save ClickThrough options
	g_Profile.WriteProfileInt(SZ_SECT_UI, _T("ClickThrough"), m_bClickThrough);
}

#endif // _WIN32_DESKTOP
