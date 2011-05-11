// LyricShowAgentObj.cpp: implementation of the CLyricShowAgentObj class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowAgentObj.h"
#include "SkinActivateCtrl.h"

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
	m_pActivateButton = NULL;
	m_nIdActivateNoticeOverlappedCtrl = UID_INVALID;
}

CLyricShowAgentObj::~CLyricShowAgentObj()
{
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(this);
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
	CSkinLinearContainer::OnCreate();

	GetLyrDispStylePropName(m_pSkin, m_bFloatingLyr, m_strLyrDisplayStylePropName);

	m_strLyrDisplayStyleDefault = g_Profile.GetProfileString(
		m_bFloatingLyr ? SZ_SECT_FLOATING_LYR : SZ_SECT_LYR_DISPLAY,
		m_strLyrDisplayStylePropName.c_str(),
		m_strLyrDisplayStyleDefault.c_str());

	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_UI_SETTINGS_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_LYRICS_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_UI_ACTIVATE_NOTICE, this);

	if (m_bEnableToolbar && !g_Profile.GetProfileBool(_T("HideToolbar"), false))
	{
		// create tool bar
		LoadToolbar();
	}

	ChangeLyricsDisplayStyle(m_strLyrDisplayStyleDefault.c_str(), false);

	ToggleActivateButton();
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
					this->RemoveUIObject(m_pToolbar, true);
					m_pToolbar = NULL;
				}
			}
			else
				LoadToolbar();

			m_pContainer->RecalculateUIObjSizePos(this);
		}
	}
	else if (pEvent->eventType == ET_LYRICS_CHANGED && m_bEnableStaticTextStyle)
	{
		tstring		strNewStyle;
		if (g_LyricData.GetLyrContentType() != LCT_TXT)
			strNewStyle = m_strLyrDisplayStyleDefault;
		else
			strNewStyle = SZ_TXT_LYR_CONTAINER;

		ChangeLyricsDisplayStyle(strNewStyle.c_str());

		m_pContainer->RecalculateUIObjSizePos(this);
	}
	else if (pEvent->eventType == ET_UI_ACTIVATE_NOTICE)
	{
		ToggleActivateButton();
		m_pContainer->RecalculateUIObjSizePos(this);
		Invalidate();
	}
}

BOOL CLyricShowAgentObj::SetProperty(LPCTSTR szProperty, SXNode *pXmlNode)
{
	if (IsPropertyName(szProperty, _T("ActivateNotice")))
	{
		m_activateButtonProperties = pXmlNode->listProperties;
		m_nIdActivateNoticeOverlappedCtrl = GetIDByName(pXmlNode->GetPropertySafe(_T("OverlappedCtrl")));
	}
	else
		return FALSE;

	return true;
}

BOOL CLyricShowAgentObj::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (m_pLyricsShow)
		m_pLyricsShow->SetProperty(szProperty, szValue);

	if (CSkinLinearContainer::SetProperty(szProperty, szValue))
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

tstring GetRandomActivateNoticeText()
{
	LPCTSTR	vNoticeText[] = {
		_T("These lyrics are very nice, right?"),
		_T("Do you think MiniLyrics is useful for you?"),
		_T("We love to make MiniLyrics better, how about you?"),
	};

	LPCTSTR	vNoticeText2[] = {
		_T("Buy Now"),
		_T("Support us"),
	};

	srand(GetTickCount());

	tstring str = vNoticeText[rand() % CountOf(vNoticeText)];
	str += _T("  -  ");
	str += vNoticeText2[rand() % CountOf(vNoticeText2)];

	return str;
}

void CLyricShowAgentObj::ToggleActivateButton()
{
	if (g_licenseManager.IsLicenseValid())
	{
		if (m_pActivateButton)
		{
			RemoveUIObject(m_pActivateButton, true);
			m_pActivateButton = NULL;
		}
	}
	else
	{
		if (!m_pActivateButton)
		{
			m_pActivateButton = m_pSkin->GetSkinFactory()->CreateUIObject(m_pSkin, 
				CSkinActivateCtrl::ClassName(), this);
			ASSERT(m_pActivateButton);
			m_pActivateButton->SetProperty(SZ_PN_RECT, _T("0,0,w,"));
			for (SXNode::iterProperties it = m_activateButtonProperties.begin();
				it != m_activateButtonProperties.end(); ++it)
			{
				m_pActivateButton->SetProperty((*it).strName.c_str(), (*it).strValue.c_str());
			}
			m_pActivateButton->SetProperty(SZ_PN_TEXT, GetRandomActivateNoticeText().c_str());

			if (m_nIdActivateNoticeOverlappedCtrl == UID_INVALID)
				this->AddUIObject(m_pActivateButton);
			else
			{
				this->InsertUIObjectAt(m_pLyricsShow, m_pActivateButton);
				m_pSkin->SetUIObjectVisible(m_nIdActivateNoticeOverlappedCtrl, false, false);
			}
		}
	}
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
		&& _tcsicmp(szLyrDispalyStyle, CLyricShowVobSub::ClassName()) != 0))
	{
		m_strLyrDisplayStyleDefault = CLyricShowMultiRowObj::ClassName();
		szLyrDispalyStyle = m_strLyrDisplayStyleDefault.c_str();
	}

	m_strLyrDisplayStyleCurrent = szLyrDispalyStyle;

	m_pLyricsShow = m_pSkin->GetSkinFactory()->CreateUIObject(m_pSkin, szLyrDispalyStyle, this);
	if (!m_pLyricsShow)
		return;

	m_pLyricsShow->SetProperty(SZ_PN_RECT, _T("0,0,w,0"));
	m_pLyricsShow->SetProperty(SZ_PN_WEIGHT, _T("1"));
	m_pLyricsShow->SetProperties(m_vProperties);

	this->AddUIObject(m_pLyricsShow);

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
		this->RemoveUIObject(m_pToolbar, true);

	m_pToolbar = m_pSkin->GetSkinFactory()->CreateUIObject(m_pSkin, _T("NormalToolbar"), this);
	if (m_pToolbar)
	{
		m_pToolbar->SetProperty(SZ_PN_RECT, _T("0,0,w,"));
		this->InsertUIObjectAt(m_pLyricsShow, m_pToolbar);
	}
}
