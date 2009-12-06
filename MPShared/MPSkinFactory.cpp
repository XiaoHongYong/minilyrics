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
#include "SkinTextButton.h"
#include "SkinStaticText.h"
#include "SkinSeekCtrl.h"
#include "SkinListCtrl.h"
#include "SkinListCtrlEx.h"
#include "SkinEditCtrl.h"
#include "SkinRateCtrl.h"
#include "SkinToolBar.h"
#include "SkinTabHeader.h"
#include "SkinBarFrame.h"
#include "SkinContainerBar.h"
#include "SkinBarFrameClientArea.h"
#include "SkinPicText.h"
#include "MPSkinTimeCtrl.h"
#include "MPSkinInfoTextCtrl.h"
#include "MediaInfoTextCtrl.h"
#include "MediaAlbumArtCtrl.h"
#include "MPMsgWnd.h"
#include "SkinFilterCtrl.h"

#ifdef _MPLAYER
#include "SkinDroplistCtrl.h"
#include "SkinTreeCtrl.h"
#include "MPSkinVis.h"
#include "MPlaylistCtrl.h"
#include "MPSkinMediaNumInfoCtrl.h"
#include "MPPPCWorkArea.h"
#include "MPlaylistSearchObj.h"
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

CMPSkinFactory::CMPSkinFactory()
{
	m_strMainSkinWndClassName = SZ_APP_NAME;

#ifdef _WIN32_DESKTOP
	m_bClickThrough = false;
#endif
}

CMPSkinFactory::~CMPSkinFactory()
{

}

int CMPSkinFactory::LoadDefaultSkin()
{
	tstring		strSkin;

	strSkin = g_Profile.GetProfileString(_T("DefaultSkin"), _T(""));
	if (strSkin.empty())
	{
		strSkin = SZ_DEFSKIN;
		g_Profile.WriteProfileString(_T("DefaultSkin"), strSkin.c_str());
	}

	return ChangeSkinEx(strSkin.c_str());
}

void CMPSkinFactory::Quit()
{
	Close();

	LIST_SKINWNDS::iterator	it, itEnd;
	LIST_SKINWNDS	listSkinWnds;
	listSkinWnds = m_listSkinWnds;

	itEnd = listSkinWnds.end();
	for (it = listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pSkin = *it;
		pSkin->Destroy();
	}
	m_listSkinWnds.clear();
}

CUIObject *CMPSkinFactory::CreateUIObject(CSkinWnd *pSkin, LPCTSTR szClassName)
{
	CUIObject		*pObj;

	pObj = CSkinFactory::CreateUIObject(pSkin, szClassName);
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
	else if (_tcsicmp(szClassName, CSkinToolbar::ClassName()) == 0)
		pObj = new CSkinToolbar;
	else if (_tcsicmp(szClassName, CSkinStaticText::ClassName()) == 0)
		pObj = new CSkinStaticText;
	else if (_tcsicmp(szClassName, CSkinTextButton::ClassName()) == 0)
		pObj = new CSkinTextButton;
	else if (_tcsicmp(szClassName, CSkinSeekCtrl::ClassName()) == 0)
		pObj = new CSkinSeekCtrl;
	else if (_tcsicmp(szClassName, CSkinListCtrl::ClassName()) == 0)
		pObj = new CSkinListCtrl;
	else if (_tcsicmp(szClassName, CSkinEditCtrl::ClassName()) == 0)
		pObj = new CSkinEditCtrl;
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
	else if (_tcsicmp(szClassName, CSkinTabHeader::ClassName()) == 0)
		pObj = new CSkinTabHeader;
	else if (_tcsicmp(szClassName, CSkinFilterCtrl::ClassName()) == 0)
		pObj = new CSkinFilterCtrl;
#ifdef _MPLAYER
	else if (_tcsicmp(szClassName, CSkinDroplistCtrl::ClassName()) == 0)
		pObj = new CSkinDroplistCtrl;
	else if (_tcsicmp(szClassName, CSkinTreeCtrl::ClassName()) == 0)
		pObj = new CSkinTreeCtrl;
	else if (_tcsicmp(szClassName, CMPSkinVis::ClassName()) == 0)
		pObj = new CMPSkinVis;
	else if (_tcsicmp(szClassName, CMPPPCWorkArea::ClassName()) == 0)
		pObj = new CMPPPCWorkArea;
	else if (_tcsicmp(szClassName, CSkinListCtrlEx::ClassName()) == 0)
		pObj = new CSkinListCtrlEx;
	else if (_tcsicmp(szClassName, CMPSkinMediaNumInfoCtrl::ClassName()) == 0)
		pObj = new CMPSkinMediaNumInfoCtrl;
	else if (_tcsicmp(szClassName, CMPlaylistSearchObj::ClassName()) == 0)
		pObj = new CMPlaylistSearchObj;
#endif
	else if (_tcsicmp(szClassName, CMediaInfoTextCtrl::ClassName()) == 0)
		pObj = new CMediaInfoTextCtrl;
	else if (_tcsicmp(szClassName, CMediaAlbumArtCtrl::ClassName()) == 0)
		pObj = new CMediaAlbumArtCtrl;
	else if (_tcsicmp(szClassName, CSkinHScrollBar::ClassName()) == 0)
		pObj = new CSkinHScrollBar;
	else
		return NULL;

	pObj->m_pSkin = pSkin;
	pObj->m_pContainer = pSkin->GetRootContainer();

	return pObj;
}

CSkinWnd *CMPSkinFactory::NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd)
{
	if (bMainWnd)
		return new CMPSkinMainWnd;
	else
		return new CMPSkinWnd;
}

SXNode *CMPSkinFactory::GetMenuOfMenus(SXNode *pNodeMenus, LPCTSTR szMenuName)
{
	SXNode::iterator it, itEnd;
	itEnd = pNodeMenus->listChildren.end();
	for (it = pNodeMenus->listChildren.begin(); it != itEnd; ++it)
	{
		SXNode		*pNode = *it;

		// OK, found the contorl
		if (_tcsicmp(pNode->strName.c_str(), _T("Menu")) == 0)
		{
			if (_tcsicmp(pNode->GetPropertySafe(_T("Name")), szMenuName) == 0)
				return pNode;
		}
	}

	return NULL;
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
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadPopupMenu(IDM_MENU, 0);
#endif
	}
	else if (_tcsicmp(szMenu, _T("LyricsMenu")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadPopupMenu(IDM_MENU, 1);
	}
	else if (_tcsicmp(szMenu, _T("LyrEditorMenu")) == 0)
	{
		pSkinMenu = new CLyrEditorMenu();
		pSkinMenu->LoadMenu(IDM_LYR_EDITOR);
	}
#ifdef _MPLAYER
	else if (_tcsicmp(szMenu, _T("MainWndMenu")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadMenu(IDM_MAIN_WND);
	}
	else if (_tcsicmp(szMenu, _T("PlaylistMenu")) == 0)
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadPopupMenu(IDM_MENU, 2);
	}
	else if (_tcsicmp(szMenu, _T("MediaLibMenu")) == 0)
	{
		pSkinMenu = new CSkinMenu();
		pSkinMenu->LoadPopupMenu(IDM_PLAYER_MENU, 1);
	}
	else if (_tcsicmp(szMenu, _T("MediaGuideTreeMenu")) == 0)
	{
		pSkinMenu = new CSkinMenu();
		pSkinMenu->LoadPopupMenu(IDM_PLAYER_MENU, 1);
	}
	else if (_tcsicmp(szMenu, _T("MediaGuideSongListMenu")) == 0)
	{
		pSkinMenu = new CSkinMenu();
		pSkinMenu->LoadPopupMenu(IDM_PLAYER_MENU, 1);
	}
#endif
	else
	{
		CMPSkinMenu	*pMenu = new CMPSkinMenu();
		pSkinMenu = pMenu;
		pMenu->AddUICheckStatusIf(pWnd);
		pSkinMenu->LoadPopupMenu(IDM_MENU, 0);
	}

	return pSkinMenu;
}

bool CMPSkinFactory::LoadMenu(CSkinWnd *pWnd, CMLMenu **ppMenu, LPCTSTR szMenu)
{
	CSkinMenu	*pSkinMenu = NULL;
	SXNode		*pNodeMenus = NULL, *pNode = NULL;

	*ppMenu = NULL;

	// Load menu from xml
	pNodeMenus = m_skinFile.GetMenusNode();
	if (pNodeMenus)
	{
		pNode = GetMenuOfMenus(pNodeMenus, szMenu);
		if (pNode)
		{
			LPCTSTR szFromMenu = pNode->GetProperty(_T("From"));
			if (szFromMenu)
				pSkinMenu = LoadPresetMenu(pWnd, szFromMenu);

			if (!pSkinMenu)
				pSkinMenu = new CSkinMenu;

			pSkinMenu->FromXML(pNode, pSkinMenu->GetOrgAppendPos(), this);
		}
	}

	if (!pSkinMenu)
	{
		// is this a preset menu?
		pSkinMenu = LoadPresetMenu(pWnd, szMenu);
		if (pSkinMenu)
		{
			*ppMenu = pSkinMenu;
			return true;
		}
	}

	*ppMenu = pSkinMenu;

	return pSkinMenu != NULL;
}

void CMPSkinFactory::OnLanguageChanged()
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();
	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pWnd = *it;
		pWnd->OnLanguageChanged();
	}
}

//
// Bring all the skin windows to the top
//
void CMPSkinFactory::OnSkinWndActivate(CSkinWnd *pWndActive)
{
#ifdef _WIN32
	LIST_SKINWNDS::iterator	it, itEnd;

	HWND		hWndPrev, hWndNext, hWndActive;
	HDWP		hWindowPosInfo;
	vector<HWND>	vToBring;

	if (pWndActive)
		hWndActive = pWndActive->GetHandle();
	else
		hWndActive = GetForegroundWindow();

	if (IsTopmostWindow(hWndActive))
		vToBring.push_back(HWND_TOP);
	else
		vToBring.push_back(hWndActive);

	hWndPrev = hWndNext = hWndActive;
	hWndNext = GetWindow(hWndNext, GW_HWNDNEXT);
	while (hWndNext)
	{
		itEnd = m_listSkinWnds.end();
		for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
		{
			if ((*it)->GetHandle() == hWndNext && hWndNext != hWndActive)
			{
				if (!IsTopmostWindow(hWndNext) && !::IsIconic(hWndNext))
				{
					vToBring.push_back(hWndNext);
					hWndPrev = hWndNext;
				}
				break;
			}
		}
		hWndNext = GetWindow(hWndNext, GW_HWNDNEXT);
	}
	hWindowPosInfo = BeginDeferWindowPos(vToBring.size());
	for (int i = 1; i < (int)vToBring.size(); i++)
		DeferWindowPos(hWindowPosInfo, vToBring[i], vToBring[i - 1], 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	EndDeferWindowPos(hWindowPosInfo);

	// DBGLOG1(_T("OnSkinWndActivate: %s"), pWndActive->GetSkinWndName());
#endif
}


void CMPSkinFactory::OnMainSkinWndDestory(CSkinWnd *pSkinWnd)
{
// #ifdef _MPLAYER
// 	CSkinFactory::OnMainSkinWndDestory(pSkinWnd);
// #endif
	g_mpMsgWnd.PostQuitMPMessage();
}

void CMPSkinFactory::TopmostAll(bool bTopmost)
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();

	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pWnd = *it;
		if (!pWnd->IsIndividualProperties())
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
	if (nID == SKIN_UIOBJ_ID_UNDEFINE)
		return nID;

	tstring		strHotkey;

	if (g_mpApp.GetHotkey().GetHotkeyText(nID, strHotkey))
	{
		strToolTip += _T(" (");
		strToolTip += strHotkey;
		strToolTip += _T(")");
	}

	return nID;
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

	nRet = CreateMainSkinWnd(SZ_APP_NAME, _T(""), _T(""));
	if (nRet != ERR_OK)
		return nRet;

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
	if (::GetParent(g_mpApp.GetMainWnd()->GetHandle()))
		return;
#endif

	COLORREF		clrTrans = RGB(0, 0, 0);
	BYTE			byAlpha = 255;
	bool			bTransparentClr = false;

	if (!g_Profile.GetProfileBool(SZ_SECT_UI, _T("WndOpacity"), true))
	{
		// °ëÍ¸Ã÷Öµ
		int		nOpaque = g_Profile.GetProfileInt(SZ_SECT_UI, _T("WndOpaquePercent"), 70);
		if (nOpaque < 10)
			nOpaque = 10;
		else if (nOpaque > 100)
			nOpaque = 100;
		byAlpha = nOpaque * 255 / 100;
	}

	// bTransparentClr = g_Profile.GetProfileBool(SZ_SECT_UI, _T("TransparentBkColor"), FALSE);

	LIST_SKINWNDS::iterator	it, itEnd;

	itEnd = m_listSkinWnds.end();
	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pWnd = *it;
		if (!pWnd->IsIndividualProperties())
			pWnd->SetTransparent(byAlpha, bTransparentClr, pWnd->GetBgColor().get(), m_bClickThrough);
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

		nShowPos = g_Profile.GetProfileInt(SZ_SECT_UI, _T("ShowIconOn"), SHOW_ICON_ON_SYSTRAY);
		if (nShowPos == SHOW_ICON_ON_NONE || nShowPos == SHOW_ICON_ON_TASKBAR)
		{
			g_Profile.WriteProfileInt(SZ_SECT_UI, _T("ShowIconOn"), SHOW_ICON_ON_BAR_TRAY);
			g_mpApp.GetMainWnd()->GetTrayIcon().UpdateShowIconPos();
		}
	}
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
