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
#include "SkinListCtrlEx.h"
#include "SkinRateCtrl.h"
#include "SkinTabHeader.h"
#include "SkinBarFrame.h"
#include "SkinContainerBar.h"
#include "SkinBarFrameClientArea.h"
#include "SkinPicText.h"
#include "MPSkinTimeCtrl.h"
#include "MPSkinInfoTextCtrl.h"
#include "MediaInfoTextCtrl.h"
#include "MediaAlbumArtCtrl.h"
#include "SkinFilterCtrl.h"
#include "MPSkinInfoTextCtrlEx.h"

#ifdef _MPLAYER
#include "SkinTreeCtrl.h"
#include "MPSkinVis.h"
#include "MPlaylistCtrl.h"
#include "MPSkinMediaNumInfoCtrl.h"
#include "MPlaylistSearchObj.h"
#include "../MPlayer/SkinVideo.h"
#ifdef _WIN32_WCE
#include "win32/wce/WCEMainCmdBar.h"
#endif
#endif

#ifdef _WIN32
#include "win32/MPSkinMenu.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/MPSkinMenu.h"
#endif

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
}

CMPSkinFactory::~CMPSkinFactory()
{

}

CUIObject *CMPSkinFactory::NewUIObject(CSkinWnd *pSkin, CSkinContainer *pContainer, LPCTSTR szClassName)
{
	CUIObject		*pObj;

	pObj = CSkinFactory::NewUIObject(pSkin, pContainer, szClassName);
	if (pObj)
		return pObj;

	if (_tcsicmp(szClassName, CLyricShowMultiRowObj::ClassName()) == 0)
		pObj = new CLyricShowMultiRowObj;
	else if (_tcsicmp(szClassName, CLyricShowTxtObj::ClassName()) == 0)
		pObj = new CLyricShowTxtObj;
	else if (_tcsicmp(szClassName, CLyricShowTwoRowObj::ClassName()) == 0)
		pObj = new CLyricShowTwoRowObj;
	else if (_tcsicmp(szClassName, CLyricShowHorzScrollObj::ClassName()) == 0)
		pObj = new CLyricShowHorzScrollObj;
	else if (_tcsicmp(szClassName, CLyricShowSingleRowObj::ClassName()) == 0)
		pObj = new CLyricShowSingleRowObj;
	else if (_tcsicmp(szClassName, CLyricShowVobSub::ClassName()) == 0)
		pObj = new CLyricShowVobSub;
	else if (_tcsicmp(szClassName, CLyricShowTextEditObj::ClassName()) == 0)
		pObj = new CLyricShowTextEditObj;
	else if (_tcsicmp(szClassName, CLyricShowAgentObj::ClassName()) == 0)
		pObj = new CLyricShowAgentObj;
//	else if (_tcsicmp(szClassName, CLyricShowPreviewObj::ClassName()) == 0)
//		pObj = new CLyricShowPreviewObj;
	else if (_tcsicmp(szClassName, CSkinRateCtrl::ClassName()) == 0)
		pObj = new CSkinRateCtrl;
	else if (_tcsicmp(szClassName, CSkinBarFrame::ClassName()) == 0)
		pObj = new CSkinBarFrame;
	else if (_tcsicmp(szClassName, CSkinContainerBar::ClassName()) == 0)
		pObj = new CSkinContainerBar;
	else if (_tcsicmp(szClassName, CSkinBarFrameClientArea::ClassName()) == 0)
		pObj = new CSkinBarFrameClientArea;
	else if (_tcsicmp(szClassName, CSkinPicText::ClassName()) == 0)
		pObj = new CSkinPicText;
	else if (_tcsicmp(szClassName, CMPSkinTimeCtrl<CSkinPicText>::ClassName()) == 0)
		pObj = new CMPSkinTimeCtrl<CSkinPicText>;
	else if (_tcsicmp(szClassName, CMPSkinTimeCtrl<CSkinStaticText>::ClassName()) == 0)
		pObj = new CMPSkinTimeCtrl<CSkinStaticText>;
	else if (_tcsicmp(szClassName, CMPSkinInfoTextCtrl::ClassName()) == 0)
		pObj = new CMPSkinInfoTextCtrl;
	else if (_tcsicmp(szClassName, CMPSkinInfoTextCtrlEx::ClassName()) == 0)
		pObj = new CMPSkinInfoTextCtrlEx;
	else if (_tcsicmp(szClassName, CSkinTabHeader::ClassName()) == 0)
		pObj = new CSkinTabHeader;
	else if (_tcsicmp(szClassName, CSkinFilterCtrl::ClassName()) == 0)
		pObj = new CSkinFilterCtrl;
#ifdef _MPLAYER
	else if (_tcsicmp(szClassName, CSkinTreeCtrl::ClassName()) == 0)
		pObj = new CSkinTreeCtrl;
	else if (_tcsicmp(szClassName, CMPSkinVis::ClassName()) == 0)
		pObj = new CMPSkinVis;
	else if (_tcsicmp(szClassName, CSkinListCtrlEx::ClassName()) == 0)
		pObj = new CSkinListCtrlEx;
	else if (_tcsicmp(szClassName, CMPSkinMediaNumInfoCtrl::ClassName()) == 0)
		pObj = new CMPSkinMediaNumInfoCtrl;
	else if (_tcsicmp(szClassName, CMPlaylistSearchObj::ClassName()) == 0)
		pObj = new CMPlaylistSearchObj;
	else if (_tcsicmp(szClassName, CSkinVideo::ClassName()) == 0)
		pObj = new CSkinVideo;
#endif
	else if (_tcsicmp(szClassName, CMediaInfoTextCtrl::ClassName()) == 0)
		pObj = new CMediaInfoTextCtrl;
	else if (_tcsicmp(szClassName, CMediaAlbumArtCtrl::ClassName()) == 0)
		pObj = new CMediaAlbumArtCtrl;
	else
		return NULL;

	pObj->SetParent(pSkin, pContainer);

	return pObj;
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
	CSkinMenu		*pSkinMenu = NULL;

	if (_tcsicmp(szMenu, _T("MainMenu")) == 0)
	{
#ifdef _WIN32_WCE
		CWCEMainCmdBar *pMenu = new CWCEMainCmdBar;
		pMenu->Create(pWnd, IDM_MENU);
#else
		CMPSkinMenu	*pMenu = new CMPSkinMenu(this);
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadPopupMenu(IDM_MENU, 0);
#endif
	}
	else if (_tcsicmp(szMenu, _T("LyricsMenu")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu(this);
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadPopupMenu(IDM_MENU, 1);
	}
	else if (_tcsicmp(szMenu, _T("LyrEditorMenu")) == 0)
	{
		pSkinMenu = new CLyrEditorMenu(this);
		pSkinMenu->LoadMenu(IDM_LYR_EDITOR);
	}
	else if (_tcsicmp(szMenu, _T("MainWndMenu")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu(this);
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadMenu(IDM_MAIN_WND);
	}
	else if (_tcsicmp(szMenu, _T("LyricsTool")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu(this);
		pSkinMenu = pMenu;
		pSkinMenu->LoadMenu(IDM_LYRICS_TOOL);
	}
#ifdef _MPLAYER
	else if (_tcsicmp(szMenu, _T("PlaylistMenu")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu(this);
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
		CMPSkinMenu	*pMenu = new CMPSkinMenu(this);
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadPopupMenu(IDM_MENU, 0);
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
			pWnd->ShowWindow(SW_RESTORE);

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

#ifndef _MPLAYER
	if (::GetParent(CMPlayerAppBase::GetMainWnd()->GetHandle()))
		return;
#endif

	// °ëÍ¸Ã÷Öµ
	int		nOpaque = g_Profile.GetProfileInt(SZ_SECT_UI, _T("WindowOpaquePercent"), 100);
	byte	byAlpha = OpaquePercentToAlpha(nOpaque);

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

		nShowPos = g_Profile.GetProfileInt(SZ_SECT_UI, _T("ShowIconOn"), SHOW_ICON_ON_BAR_TRAY);
		if (nShowPos == SHOW_ICON_ON_NONE || nShowPos == SHOW_ICON_ON_TASKBAR)
		{
			g_Profile.WriteProfileInt(SZ_SECT_UI, _T("ShowIconOn"), SHOW_ICON_ON_BAR_TRAY);
			CMPlayerAppBase::GetMainWnd()->GetTrayIcon().UpdateShowIconPos();
		}
	}

	AllUpdateTransparent();
}

#endif // _WIN32_DESKTOP

int CMPSkinFactory::ExeDynamicCmd(const CDynamicCmds::DynamicCmd &cmd, CSkinWnd *pSkinWnd)
{
	if (cmd.strFunction == _T("openclose_framebar_bar"))
	{
		int		nIDFrame, nIDBar;
		V_Str	vStrParam;

		StrBreak(cmd.strParam.c_str(), ',', vStrParam);
		if (vStrParam.size() != 2)
			return ERR_FALSE;
		for (int i = 0; i < (int)vStrParam.size(); i++)
			TrimStr(vStrParam[i]);

		nIDFrame = GetIDByName(vStrParam[0].c_str());
		nIDBar = GetIDByName(vStrParam[1].c_str());

		CSkinBarFrame	*pFrame = (CSkinBarFrame*)pSkinWnd->GetUIObjectById(nIDFrame, CSkinBarFrame::ClassName());

		if (pFrame)
			pFrame->ShowBar(nIDBar, !pFrame->IsBarVisible(nIDBar));
	}
	else if (cmd.strFunction == _T("is_framebar_bar_visible"))
	{
		int		nIDFrame, nIDBar;
		V_Str	vStrParam;

		StrBreak(cmd.strParam.c_str(), ',', vStrParam);
		if (vStrParam.size() != 2)
			return ERR_FALSE;
		for (int i = 0; i < (int)vStrParam.size(); i++)
			TrimStr(vStrParam[i]);

		nIDFrame = GetIDByName(vStrParam[0].c_str());
		nIDBar = GetIDByName(vStrParam[1].c_str());

		CSkinBarFrame	*pFrame = (CSkinBarFrame*)pSkinWnd->GetUIObjectById(nIDFrame, CSkinBarFrame::ClassName());

		if (pFrame && pFrame->IsBarVisible(nIDBar))
			return ERR_OK;
		else
			return ERR_FALSE;
	}
	else
		return CSkinFactory::ExeDynamicCmd(cmd, pSkinWnd);

	return ERR_OK;
}
