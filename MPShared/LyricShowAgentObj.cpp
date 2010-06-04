// LyricShowAgentObj.cpp: implementation of the CLyricShowAgentObj class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowAgentObj.h"
#include "SkinToolBar.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CLyricShowAgentObj, _T("LyricsShow"))

CLyricShowAgentObj::CLyricShowAgentObj()
{
	m_pLyricsShow = NULL;
	m_bEnableStaticTextStyle = true;

	m_strLyrDisplayStylePropName = _T("LyrDisplayStyle");
	m_bEnableToolbar = true;
	m_pToolbar = NULL;
	m_bFloatingLyr = false;
}

CLyricShowAgentObj::~CLyricShowAgentObj()
{
	g_eventDispather.UnregisterHanlder(ET_UI_SETTINGS_CHANGED, this);
	g_eventDispather.UnregisterHanlder(ET_LYRICS_CHANGED, this);
	if (m_pLyricsShow)
		m_pSkin->RemoveUIObject(m_pLyricsShow, true);
}

void CLyricShowAgentObj::GetLyrDispStylePropName(CSkinWnd *pSkinWnd, bool &bFloatingLyr, tstring &strLyrStylePropName)
{
	bFloatingLyr = false;

	if (!pSkinWnd->GetUnprocessedProperty(_T("LyrDisplayStylePropName"), strLyrStylePropName))
		strLyrStylePropName = _T("LyrDisplayStyle");

	if (_tcsicmp(_T("FloatingLyrDispStyle"), strLyrStylePropName.c_str()) == 0)
	{
		bFloatingLyr = true;
		strLyrStylePropName = _T("LyrDisplayStyle");
	}
}

void CLyricShowAgentObj::GetLyrDispStyleSettings(CSkinWnd *pSkinWnd, tstring &strLyrStyle)
{
	tstring			strLyrStylePropName;
	bool			bFloatingLyr;

	GetLyrDispStylePropName(pSkinWnd, bFloatingLyr, strLyrStylePropName);

	strLyrStyle = g_Profile.GetProfileString(
		bFloatingLyr ? SZ_SECT_FLOATING_LYR : SZ_SECT_LYR_DISPLAY,
		strLyrStylePropName.c_str(),
		strLyrStyle.c_str());
}

void CLyricShowAgentObj::SetLyrDispStyleSettings(CSkinWnd *pSkinWnd, LPCTSTR szLyrStyle)
{
	tstring			strLyrStylePropName;
	bool			bFloatingLyr;

	GetLyrDispStylePropName(pSkinWnd, bFloatingLyr, strLyrStylePropName);

	CMPlayerSettings::SetSettings(ET_UI_SETTINGS_CHANGED, 
		bFloatingLyr ? SZ_SECT_FLOATING_LYR : SZ_SECT_LYR_DISPLAY, 
		strLyrStylePropName.c_str(), szLyrStyle);
}

void CLyricShowAgentObj::OnCreate()
{
	CUIObject::OnCreate();

	GetLyrDispStylePropName(m_pSkin, m_bFloatingLyr, m_strLyrDisplayStylePropName);

	m_strLyrDisplayStyleDefault = g_Profile.GetProfileString(
		m_bFloatingLyr ? SZ_SECT_FLOATING_LYR : SZ_SECT_LYR_DISPLAY,
		m_strLyrDisplayStylePropName.c_str(),
		m_strLyrDisplayStyleDefault.c_str());

	g_eventDispather.RegisterHanlder(ET_UI_SETTINGS_CHANGED, this);
	g_eventDispather.RegisterHanlder(ET_LYRICS_CHANGED, this);

	if (m_bEnableToolbar && !g_Profile.GetProfileBool(_T("HideToolbar"), false))
	{
		// create tool bar
		LoadToolbar();
	}

	ChangeLyricsDisplayStyle(m_strLyrDisplayStyleDefault.c_str(), false);
}

void CLyricShowAgentObj::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_UI_SETTINGS_CHANGED)
	{
		if (_tcsicmp(pEvent->strName.c_str(), m_strLyrDisplayStylePropName.c_str()) == 0)
		{
			GetLyrDispStyleSettings(m_pSkin, m_strLyrDisplayStyleDefault);
			ChangeLyricsDisplayStyle(m_strLyrDisplayStyleDefault.c_str());
			m_pSkin->InvalidateRect();
		}
		else if (IsPropertyName(pEvent->strName.c_str(), _T("HideToolbar")))
		{
			bool		bHideToolbar;

			bHideToolbar = IsTRUE(pEvent->strValue.c_str());
			if (bHideToolbar)
			{
				if (m_pToolbar)
				{
					// destroy toolbar
					m_pContainer->RemoveUIObject(m_pToolbar, true);
					m_pToolbar = NULL;

					// reposition lyrics show obj
					if (m_pLyricsShow)
					{
						m_pLyricsShow->m_formTop = m_formTop;
						m_pLyricsShow->m_formHeight = m_formHeight;
						m_pContainer->RecalculateUIObjSizePos(m_pLyricsShow);
						m_pLyricsShow->Invalidate();
					}
				}
			}
			else
				LoadToolbar();
		}
	}
	else if (pEvent->eventType == ET_LYRICS_CHANGED)
	{
		tstring		strNewStyle;
		if (g_LyricData.GetLyrContentType() != LCT_TXT)
			strNewStyle = m_strLyrDisplayStyleDefault;
		else
			strNewStyle = SZ_TXT_LYR_CONTAINER;

		ChangeLyricsDisplayStyle(strNewStyle.c_str());
	}
}

BOOL CLyricShowAgentObj::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (m_pLyricsShow)
		m_pLyricsShow->SetProperty(szProperty, szValue);

	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("LyrDisplayStyle"))
		|| IsPropertyName(szProperty, m_strLyrDisplayStylePropName.c_str()))
		m_strLyrDisplayStyleDefault = szValue;
	else if (IsPropertyName(szProperty, _T("EnableStaticTextStyle")))
		m_bEnableStaticTextStyle = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("EnableToolbar")))
		m_bEnableToolbar = IsTRUE(szValue);
	else
	{
		m_vProperties.push_back(szProperty);
		m_vProperties.push_back(szValue);
	}

	return TRUE;
}

void CLyricShowAgentObj::ChangeLyricsDisplayStyle(LPCTSTR szLyrDispalyStyle, bool bRedraw)
{
	if (_tcsicmp(m_strLyrDisplayStyleCurrent.c_str(), szLyrDispalyStyle) == 0
		&& !IsEmptyString(szLyrDispalyStyle))
		return;

	if (m_pLyricsShow)
	{
		m_pSkin->RemoveUIObject(m_pLyricsShow, true);
		m_pLyricsShow = NULL;
	}

	if ((_tcsicmp(szLyrDispalyStyle, CLyricShowMultiRowObj::ClassName()) != 0
		&& _tcsicmp(szLyrDispalyStyle, SZ_TXT_LYR_CONTAINER) != 0
		&& _tcsicmp(szLyrDispalyStyle, CLyricShowTwoRowObj::ClassName()) != 0
		&& _tcsicmp(szLyrDispalyStyle, CLyricShowHorzScrollObj::ClassName()) != 0
		&& _tcsicmp(szLyrDispalyStyle, CLyricShowSingleRowObj::ClassName()) != 0
		&& _tcsicmp(szLyrDispalyStyle, CLyricShowVobSub::ClassName()) != 0)
		|| (_tcsicmp(szLyrDispalyStyle, SZ_TXT_LYR_CONTAINER) == 0 && !m_bEnableStaticTextStyle))
	{
		m_strLyrDisplayStyleDefault = CLyricShowMultiRowObj::ClassName();
		szLyrDispalyStyle = m_strLyrDisplayStyleDefault.c_str();
	}

	m_strLyrDisplayStyleCurrent = szLyrDispalyStyle;

	m_pLyricsShow = m_pSkin->GetSkinFactory()->CreateUIObject(m_pSkin, szLyrDispalyStyle, m_pContainer);
	if (!m_pLyricsShow)
		return;

	tstring		strLyrTop, strLyrHeight;

	if (m_pToolbar)
	{
		strLyrTop = m_formTop.GetFormula();
		strLyrTop += _T("+");
		strLyrTop += m_strTBHeight;
		strLyrHeight = m_formHeight.GetFormula();
		strLyrHeight += _T("-");
		strLyrHeight += m_strTBHeight;
	}
	else
	{
		strLyrTop = m_formTop.GetFormula();
		strLyrHeight = m_formHeight.GetFormula();
	}

	m_pLyricsShow->m_formLeft = m_formLeft;
	m_pLyricsShow->m_formTop.SetFormula(strLyrTop.c_str());
	m_pLyricsShow->m_formWidth = m_formWidth;
	m_pLyricsShow->m_formHeight.SetFormula(strLyrHeight.c_str());
	m_pLyricsShow->SetProperties(m_vProperties);

	m_pLyricsShow->m_pContainer = m_pContainer;
	m_pContainer->InsertUIObjectAt(this, m_pLyricsShow);

	if (bRedraw)
		m_pContainer->RecalculateUIObjSizePos(m_pLyricsShow);

	if (_tcsicmp(m_strLyrDisplayStyleCurrent.c_str(), SZ_TXT_LYR_CONTAINER) == 0)
	{
		// Set the stat of record scrolling lyrics buttons
		CLyricShowTxtObj *pObj = (CLyricShowTxtObj*)m_pSkin->GetUIObjectByClassName(CLyricShowTxtObj::ClassName());
		if (pObj)
		{
			m_pSkin->CheckButton(CMD_LYR_SCROLL_ENABLE_RECORD,
				pObj->IsRecordScrollingActionsEnabled() && g_LyricData.GetLyrContentType() == LCT_TXT);

			m_pSkin->CheckButton(CMD_LYR_SCROLL_ENABLE_REPLAY,
				pObj->IsReplayScrollingActionsEnabled());
		}
	}
}

void CLyricShowAgentObj::LoadToolbar()
{
	if (!m_bEnableToolbar)
		return;

	if (m_pToolbar)
		m_pContainer->RemoveUIObject(m_pToolbar, true);

	m_pToolbar = m_pSkin->GetSkinFactory()->CreateDynamicCtrl(m_pContainer, _T("NormalToolbar"), ID_TOOLBAR, m_formLeft.GetFormula(), m_formTop.GetFormula(), m_formWidth.GetFormula(), NULL);
	if (!m_pToolbar)
	{
		m_strTBHeight = _T("0");
		return;
	}
	else
		m_pContainer->RecalculateUIObjSizePos(m_pToolbar);

	m_pToolbar->Invalidate();

	m_strTBHeight = m_pToolbar->m_formHeight.GetFormula();

	// reposition lyrics show object
	if (m_pLyricsShow)
	{
		tstring		strLyrTop, strLyrHeight;

		strLyrTop = m_formTop.GetFormula();
		strLyrTop += _T("+");
		strLyrTop += m_strTBHeight;
		strLyrHeight = m_formHeight.GetFormula();
		strLyrHeight += _T("-");
		strLyrHeight += m_strTBHeight;

		m_pLyricsShow->m_formTop.SetFormula(strLyrTop.c_str());
		m_pLyricsShow->m_formHeight.SetFormula(strLyrHeight.c_str());

		m_pContainer->RecalculateUIObjSizePos(m_pLyricsShow);
		m_pLyricsShow->Invalidate();
	}
}
