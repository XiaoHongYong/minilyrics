// SESkinFactory.cpp: implementation of the CSESkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPSkinEditor.h"
#include "SESkinFactory.h"
#include "../MPShared/SkinTextButton.h"
#include "../MPShared/SkinStaticText.h"
#include "../MPShared/SkinSeekCtrl.h"
#include "../MPShared/SkinListCtrl.h"
#include "../MPShared/SkinRateCtrl.h"
#include "../MPShared/SkinDroplistCtrl.h"
#include "../MPlayer/MPSkinVis.h"
#include "../MPShared/SkinBarFrame.h"
#include "../MPShared/SkinBarFrameClientArea.h"
#include "../MPShared/SkinContainerBar.h"
#include "../MPShared/SkinToolbar.h"
#include "../MPShared/SkinTreeCtrl.h"
#include "MPSkinTimeCtrl.h"
#include "MPSkinMediaNumInfoCtrl.h"
#include "MPSkinInfoTextCtrl.h"
#include "MPPPCWorkArea.h"
#include "UIEditObject.h"
#include "SkinUIObjAny.h"
#include "SESkinWnd.h"
#include "SESkinContainer.h"
#include "SEUIEditObject.h"
#include "MediaInfoTextCtrl.h"
#include "MediaAlbumArtCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSESkinFactory::CSESkinFactory()
{

}

CSESkinFactory::~CSESkinFactory()
{

}

int CSESkinFactory::OpenSkin(LPCTSTR szSkin)
{
	int			nRet;
	tstring		strSkin, strSkinWnd;
	tstring		strOpenedSkinWnds;
	vector<tstring>	vSkinWnds;

	nRet = CSkinFactory::OpenSkin(szSkin);
	if (nRet != ERR_OK)
		return nRet;
/*
	strSkinWnd = g_Profile.GetProfileString(szSkin, _T("DefaultSkinWnd"), _T(""));
	nRet = CreateMainSkinWnd(_T("MiniLyrics"), _T("MiniLyrics"), strSkinWnd.c_str());
	if (nRet != ERR_OK)
		return nRet;

	strOpenedSkinWnds = g_Profile.GetProfileString(_T("LatestOpenedSkinWnds"), _T(""));
	StrBreak(strOpenedSkinWnds.c_str(), ',', vSkinWnds);
	for (i = 0; i < vSkinWnds.size(); i++)
	{
		if (_tcsicmp(vSkinWnds[i].c_str(), GetMainWnd()->GetSkinWndName()) == 0)
		{
			vSkinWnds.erase(vSkinWnds.begin() + i);
			break;
		}
	}

	for (i = 0; i < vSkinWnds.size(); i++)
	{
		CreateSkinWnd(false, _T("CMPSkin"), _T("CMPSkin"), vSkinWnds[i].c_str());
	}*/

	return nRet;
}

void CSESkinFactory::Quit()
{
	LIST_SKINWNDS::iterator	it, itEnd;
	tstring		strOpenedSkinWnds;

	itEnd = m_listSkinWnds.end();
	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkin	*pSkin = *it;
		if (!strOpenedSkinWnds.empty())
			strOpenedSkinWnds += _T(",");
		strOpenedSkinWnds += pSkin->GetSkinWndName();
	}

	g_Profile.WriteProfileString(_T("LatestOpenedSkinWnds"), strOpenedSkinWnds.c_str());

	Close();
}

CUIObject *CSESkinFactory::CreateUIObject(CSkin *pSkin, LPCTSTR szClassName)
{
	CUIObject		*pObj = NULL;
	CSESkinWnd		*pSESkinWnd;

	pSESkinWnd = (CSESkinWnd *)pSkin;
	ISESkinNotify	*pNotify = pSESkinWnd->GetNotify();

	if (_tcsicmp(szClassName, CSkinButton::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinButton>(pNotify);
	else if (_tcsicmp(szClassName, CSkinActiveButton::ClassName()) == 0 ||
		_tcsicmp(szClassName, _T("FocusButton")) == 0)
		pObj = new CSEUIEditObject<CSkinActiveButton>(pNotify);
	else if (_tcsicmp(szClassName, CSkinImage::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinImage>(pNotify);
	else if (_tcsicmp(szClassName, CSkinNStatusImage::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinNStatusImage>(pNotify);
	else if (_tcsicmp(szClassName, CSkinActiveImage::ClassName()) == 0|| 
		_tcsicmp(szClassName, _T("FocusImage")) == 0)
		pObj = new CSEUIEditObject<CSkinActiveImage>(pNotify);
	else if (_tcsicmp(szClassName, CSkinXScaleImage::ClassName()) == 0||
		_tcsicmp(szClassName, _T("XScaleImage")) == 0)
		pObj = new CSEUIEditObject<CSkinXScaleImage>(pNotify);
	else if (_tcsicmp(szClassName, CSkinYScaleImage::ClassName()) == 0 ||
		_tcsicmp(szClassName, _T("YScaleImage")) == 0)
		pObj = new CSEUIEditObject<CSkinYScaleImage>(pNotify);
	else if (_tcsicmp(szClassName, CSkinVScrollBar::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinVScrollBar>(pNotify);
	else if (_tcsicmp(szClassName, CSkinVScrollBarOSStyle::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinVScrollBarOSStyle>(pNotify);
	else if (_tcsicmp(szClassName, CSkinHScrollBarOSStyle::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinHScrollBarOSStyle>(pNotify);
	else if (_tcsicmp(szClassName, CSkinCaption::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinCaption>(pNotify);
	else if (_tcsicmp(szClassName, CSkinNStatusButton::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinNStatusButton>(pNotify);
	else if (_tcsicmp(szClassName, CSkinRateCtrl::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinRateCtrl>(pNotify);
	else if (_tcsicmp(szClassName, CSkinStaticText::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinStaticText>(pNotify);
	else if (_tcsicmp(szClassName, CSkinToolbar::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinToolbar>(pNotify);
	else if (_tcsicmp(szClassName, CSkinTreeCtrl::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinTreeCtrl>(pNotify);
	else if (_tcsicmp(szClassName, CSkinTextButton::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinTextButton>(pNotify);
	else if (_tcsicmp(szClassName, CSkinSeekCtrl::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinSeekCtrl>(pNotify);
	else if (_tcsicmp(szClassName, CSkinListCtrl::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinListCtrl>(pNotify);
	else if (_tcsicmp(szClassName, CSkinDroplistCtrl::ClassName()) == 0)
		pObj = new CSEUIEditObject<CSkinDroplistCtrl>(pNotify);
	else if (_tcsicmp(szClassName, CMPSkinVis::ClassName()) == 0)
		pObj = new CSEUIEditObject<CMPSkinVis>(pNotify);
	else if (_tcsicmp(szClassName, CMPSkinTimeCtrl::ClassName()) == 0)
		pObj = new CSEUIEditObject<CMPSkinTimeCtrl>(pNotify);
	else if (_tcsicmp(szClassName, CMPSkinMediaNumInfoCtrl::ClassName()) == 0)
		pObj = new CSEUIEditObject<CMPSkinMediaNumInfoCtrl>(pNotify);
	else if (_tcsicmp(szClassName, CMPSkinInfoTextCtrl::ClassName()) == 0)
		pObj = new CSEUIEditObject<CMPSkinInfoTextCtrl>(pNotify);
	else if (_tcsicmp(szClassName, CMPPPCWorkArea::ClassName()) == 0)
		pObj = new CSEUIEditObject<CMPPPCWorkArea>(pNotify);
	else if (_tcsicmp(szClassName, CMediaInfoTextCtrl::ClassName()) == 0)
		pObj = new CSEUIEditObject<CMediaInfoTextCtrl>(pNotify);
	else if (_tcsicmp(szClassName, CMediaAlbumArtCtrl::ClassName()) == 0)
		pObj = new CSEUIEditObject<CMediaAlbumArtCtrl>(pNotify);
	// container
	else if (_tcsicmp(szClassName, CSkinContainer::ClassName()) == 0)
		pObj = new CSESkinContainer<CSkinContainer>(pNotify);
	else if (_tcsicmp(szClassName, CSkinBarFrame::ClassName()) == 0)
		pObj = new CSESkinContainer<CSkinBarFrame>(pNotify);
	else if (_tcsicmp(szClassName, CSkinBarFrameClientArea::ClassName()) == 0)
		pObj = new CSESkinContainer<CSkinBarFrameClientArea>(pNotify);
	else if (_tcsicmp(szClassName, CSkinContainerBar::ClassName()) == 0)
		pObj = new CSESkinContainer<CSkinContainerBar>(pNotify);
	else
	{
		// other ui object
		CSEUIEditObject<CSkinUIObjAny>		*pObjAny = new CSEUIEditObject<CSkinUIObjAny>(pNotify);

		pObjAny->SetClassName(szClassName);
		pObj = pObjAny;
	}

	if (pObj)
		pObj->m_pSkin = pSkin;

	return pObj;
/*
	if (pObj == NULL)
	{
	}

	if (!pObj)
		pObj = new CSkinUIObjAny(szClassName);

	pObj->m_pSkin = pSkin;

	pObjEdit = new CUIEditObject;
	pObjEdit->m_pSkin = pSkin;

	pObjEdit->SetContainedUIObject(pObj);

	// Set property changed notify
	CSESkinWnd	*pSESkinWnd;
	pSESkinWnd = (CSESkinWnd *)pSkin;
	pObjEdit->SetNotify(pSESkinWnd->GetNotify());

	return pObjEdit;*/
}
/*

CUIObject *CSESkinFactory::CreateUIObject(CSkin *pSkin, LPCTSTR szClassName)
{
	CUIObject		*pObj;
	CUIEditObject	*pObjEdit = NULL;

	if (_tcsicmp(szClassName, CSkinContainer::ClassName()) == 0)
	{
		pObj = new CSESkinContainer;

		pObj->m_pSkin = pSkin;

		// Set property changed notify
		CSESkinWnd	*pSESkinWnd;
		pSESkinWnd = (CSESkinWnd *)pSkin;
		((CSESkinContainer*)pObj)->SetNotify(pSESkinWnd->GetNotify());

		return pObj;
	}
	else
		pObj = CSkinFactory::CreateUIObject(pSkin, szClassName);

	if (pObj == NULL)
	{
		if (_tcsicmp(szClassName, CSkinRateCtrl::ClassName()) == 0)
			pObj = new CSkinRateCtrl;
		else if (_tcsicmp(szClassName, CSkinStaticText::ClassName()) == 0)
			pObj = new CSkinStaticText;
		else if (_tcsicmp(szClassName, CSkinTextButton::ClassName()) == 0)
			pObj = new CSkinTextButton;
		else if (_tcsicmp(szClassName, CSkinSeekCtrl::ClassName()) == 0)
			pObj = new CSkinSeekCtrl;
		else if (_tcsicmp(szClassName, CSkinListCtrl::ClassName()) == 0)
			pObj = new CSkinListCtrl;
		else if (_tcsicmp(szClassName, CSkinDroplistCtrl::ClassName()) == 0)
			pObj = new CSkinDroplistCtrl;
		else if (_tcsicmp(szClassName, CMPSkinVis::ClassName()) == 0)
			pObj = new CMPSkinVis;
		else if (_tcsicmp(szClassName, CMPSkinTimeCtrl::ClassName()) == 0)
			pObj = new CMPSkinTimeCtrl;
		else if (_tcsicmp(szClassName, CMPSkinMediaNumInfoCtrl::ClassName()) == 0)
			pObj = new CMPSkinMediaNumInfoCtrl;
		else if (_tcsicmp(szClassName, CMPSkinInfoTextCtrl::ClassName()) == 0)
			pObj = new CMPSkinInfoTextCtrl;
		else if (_tcsicmp(szClassName, CMPPPCWorkArea::ClassName()) == 0)
			pObj = new CMPPPCWorkArea;
		else if (_tcsicmp(szClassName, CSkinBarFrame::ClassName()) == 0)
			pObj = new CSkinBarFrame;
		else if (_tcsicmp(szClassName, CSkinBarFrameClientArea::ClassName()) == 0)
			pObj = new CSkinBarFrameClientArea;
		else if (_tcsicmp(szClassName, CSkinContainerBar::ClassName()) == 0)
			pObj = new CSkinContainerBar;
	}

	if (!pObj)
		pObj = new CSkinUIObjAny(szClassName);

	pObj->m_pSkin = pSkin;

	pObjEdit = new CUIEditObject;
	pObjEdit->m_pSkin = pSkin;

	pObjEdit->SetContainedUIObject(pObj);

	// Set property changed notify
	CSESkinWnd	*pSESkinWnd;
	pSESkinWnd = (CSESkinWnd *)pSkin;
	pObjEdit->SetNotify(pSESkinWnd->GetNotify());

	return pObjEdit;
}
*/

CSkin *CSESkinFactory::NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd)
{
	return new CSESkinWnd();
}

bool CSESkinFactory::LoadMenu(CMenu **ppMenu, LPCTSTR szMenu)
{
/*	if (_tcsicmp(szMenu, _T("MainMenu")) == 0)
	{
		*ppMenu = new CMainMenu();
		return (*ppMenu)->LoadPopupMenu(IDM_MENU, 0);
	}
	else if (_tcsicmp(szMenu, _T("LyricsMenu")) == 0)
	{
		*ppMenu = new CMainMenu();
		return (*ppMenu)->LoadPopupMenu(IDM_MENU, 1);
	}
	else if (_tcsicmp(szMenu, _T("PlaylistMenu")) == 0)
	{
		*ppMenu = new CMainMenu();
		return (*ppMenu)->LoadPopupMenu(IDM_MENU, 2);
	}
	else*/
		return false;
}

void CSESkinFactory::OnLanguageChanged()
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();
	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkin	*pWnd = *it;
		pWnd->OnLanguageChanged();
	}
}
