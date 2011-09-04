#include "StdAfx.h"
#include "MPlayerApp.h"
#include "LyricShowTxtContainer.h"
#include "LyricShowTxtObj.h"

UIOBJECT_CLASS_NAME_IMP(CLyricShowTxtContainer, _T("LyricShowTxtContainer"))

CLyricShowTxtContainer::CLyricShowTxtContainer()
{
	m_bVertical = false;
	m_pLyricsShow = NULL;
	m_pObjScrollBar = NULL;
}

CLyricShowTxtContainer::~CLyricShowTxtContainer()
{
}

void CLyricShowTxtContainer::OnCreate()
{
	CSkinLinearContainer::OnCreate();

	//
	// Create Scrollbar
	//
	m_pObjScrollBar = m_pSkin->GetSkinFactory()->CreateUIObject(m_pSkin, CSkinVScrollBar::ClassName(), m_pContainer);
	if (!m_pObjScrollBar)
		return;

	m_pObjScrollBar->SetProperty(_T("LinesPerWheel"), _T("1"));
	m_pObjScrollBar->SetProperty(SZ_PN_LAYOUT_PARAMS, _T("match_parent_height"));

	AddUIObject(m_pObjScrollBar);


	m_pLyricsShow = m_pSkin->GetSkinFactory()->CreateUIObject(m_pSkin, CLyricShowTxtObj::ClassName(), this);
	if (!m_pLyricsShow)
		return;

	m_pLyricsShow->SetProperty(SZ_PN_RECT, _T("0,0,0,h"));
	m_pLyricsShow->SetProperty(SZ_PN_WEIGHT, _T("1"));
	m_pLyricsShow->SetProperties(m_vProperties);

	InsertUIObjectAt(m_pObjScrollBar, m_pLyricsShow);
}

BOOL CLyricShowTxtContainer::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (m_pLyricsShow)
		m_pLyricsShow->SetProperty(szProperty, szValue);

	if (CSkinLinearContainer::SetProperty(szProperty, szValue))
		return TRUE;

	m_vProperties.push_back(szProperty);
	m_vProperties.push_back(szValue);

	return TRUE;
}
