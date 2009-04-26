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
}

CLyricShowAgentObj::~CLyricShowAgentObj()
{
	g_eventDispather.UnregisterHanlder(ET_UI_SETTINGS_CHANGED, this);
	g_eventDispather.UnregisterHanlder(ET_LYRICS_CHANGED, this);
	if (m_pLyricsShow)
		m_pSkin->RemoveUIObject(m_pLyricsShow, true);
}

void CLyricShowAgentObj::OnCreate()
{
	CUIObject::OnCreate();

	if (!m_pSkin->GetUnprocessedProperty(_T("LyrDisplayStylePropName"), m_strLyrDisplayStylePropName))
		m_strLyrDisplayStylePropName = _T("LyrDisplayStyle");

	g_eventDispather.RegisterHanlder(ET_UI_SETTINGS_CHANGED, this);
	g_eventDispather.RegisterHanlder(ET_LYRICS_CHANGED, this);

	m_strLyrDisplayStyle = g_Profile.GetProfileString(SZ_SECT_LYR_DISPLAY, m_strLyrDisplayStylePropName.c_str(), m_strLyrDisplayStyle.c_str());

	if (m_bEnableToolbar && !g_Profile.GetProfileBool(_T("HideToolbar"), false))
	{
		// create tool bar
		LoadToolbar();
	}

	ChangeLyricsDisplayStyle(m_strLyrDisplayStyle.c_str());
}

void CLyricShowAgentObj::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_UI_SETTINGS_CHANGED)
	{
		if (_tcsicmp(pEvent->strName.c_str(), m_strLyrDisplayStylePropName.c_str()) == 0)
		{
			m_strLyrDisplayStyle = pEvent->strValue;
			ChangeLyricsDisplayStyle(m_strLyrDisplayStyle.c_str());
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
		if (g_LyricData.GetLyrContentType() != LCT_TXT)
		{
			// Change to default lyrics display style
			ChangeLyricsDisplayStyle(m_strLyrDisplayStyle.c_str());
			m_pSkin->InvalidateRect();
		}
		else if (m_pLyricsShow && _tcscmp(CLyricShowMultiRowObj::ClassName(), m_pLyricsShow->GetClassName()) == 0)
		{
			// To static text style, if the original is MultiLine style.
			ChangeLyricsDisplayStyle(CLyricShowTxtObj::ClassName());
			m_pSkin->InvalidateRect();
		}
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
		m_strLyrDisplayStyle = szValue;
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

void CLyricShowAgentObj::ChangeLyricsDisplayStyle(LPCTSTR szLyrDispalyStyle)
{
	if (m_pLyricsShow)
	{
		if (_tcsicmp(szLyrDispalyStyle, m_pLyricsShow->GetClassName()) == 0)
			return;

		m_pSkin->RemoveUIObject(m_pLyricsShow, true);
		m_pLyricsShow = NULL;
	}

	if ((_tcsicmp(szLyrDispalyStyle, CLyricShowMultiRowObj::ClassName()) != 0
		&& _tcsicmp(szLyrDispalyStyle, CLyricShowTxtObj::ClassName()) != 0
		&& _tcsicmp(szLyrDispalyStyle, CLyricShowTwoRowObj::ClassName()) != 0
		&& _tcsicmp(szLyrDispalyStyle, CLyricShowHorzScrollObj::ClassName()) != 0
		&& _tcsicmp(szLyrDispalyStyle, CLyricShowSingleRowObj::ClassName()) != 0
		&& _tcsicmp(szLyrDispalyStyle, CLyricShowVobSub::ClassName()) != 0)
		|| (_tcsicmp(szLyrDispalyStyle, CLyricShowTxtObj::ClassName()) == 0 && !m_bEnableStaticTextStyle))
	{
		m_strLyrDisplayStyle = CLyricShowMultiRowObj::ClassName();
		szLyrDispalyStyle = m_strLyrDisplayStyle.c_str();
	}

	m_pLyricsShow = m_pSkin->GetSkinFactory()->CreateUIObject(m_pSkin, szLyrDispalyStyle);
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
	m_pContainer->RecalculateUIObjSizePos(m_pLyricsShow);
}

void CLyricShowAgentObj::LoadToolbar()
{
	if (!m_bEnableToolbar)
		return;

	if (m_pToolbar)
		m_pContainer->RemoveUIObject(m_pToolbar, true);

	m_pToolbar = m_pSkin->GetSkinFactory()->CreateDynamicCtrl(m_pContainer, CSkinToolbar::ClassName(), _T("normal"), ID_TOOLBAR, m_formLeft.GetFormula(), m_formTop.GetFormula(), m_formWidth.GetFormula(), NULL);
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
