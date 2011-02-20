// SkinTextButton.cpp: implementation of the CSkinTextButton class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinTextButton.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinTextButton,  _T("TextButton"))

CSkinTextButton::CSkinTextButton()
{
	m_dwAlignTextFlags = AT_VCENTER | AT_CENTER;
	m_bResizeToContent = false;
	m_nTextLeftMargin = m_nTextRightMargin = 0;
	m_chMenuKey = 0;
}

CSkinTextButton::~CSkinTextButton()
{

}

#define TEXT_MARGIN		4

void CSkinTextButton::Draw(CRawGraph *pGraph)
{
	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	if (m_bResizeToContent)
	{
		// Resize button by content size.

		m_rcObj.right = m_rcObj.left + m_nTextLeftMargin + m_nTextRightMargin;

		SIZE size;
		if (m_strText.size() && pGraph->GetTextExtentPoint32(m_strText.c_str(), m_strText.size(), &size))
			m_rcObj.right += size.cx + TEXT_MARGIN * 2;

		if (m_imgContent.IsValid())
		{
			m_rcObj.right += m_imgContent.m_cx;
			if (m_strText.size())
				m_rcObj.right += TEXT_MARGIN;
		}
	}

	CSkinButton::Draw(pGraph);

	CRect		rc = m_rcObj;
	rc.left += m_nTextLeftMargin;
	rc.right -= m_nTextRightMargin;

	// Draw content image
	if (m_imgContent.IsValid())
	{
		m_imgContent.Blt(pGraph, rc.left, m_rcObj.top + (m_rcObj.Height() - m_imgContent.m_cy) / 2);
		rc.left += TEXT_MARGIN + m_imgContent.m_cx;
	}

	// Draw button text
	if (m_strText.size())
	{
		MLColor clrText = m_font.GetTextColor(m_bEnable);

		if (m_font.IsOutlined())
		{
			pGraph->DrawTextClipOutlined(m_strText.c_str(), (int)m_strText.size(), &rc, 
				clrText, m_font.GetColorOutlined(), m_dwAlignTextFlags | DT_PREFIX_TEXT | DT_SINGLELINE | DT_VCENTER);
		}
		else
		{
			pGraph->SetTextColor(clrText);
			pGraph->DrawText(m_strText.c_str(), m_strText.size(), &rc, m_dwAlignTextFlags | DT_PREFIX_TEXT | DT_SINGLELINE | DT_VCENTER);
		}
	}
}

void CSkinTextButton::OnCreate()
{
	m_font.OnCreate(m_pSkin);

	CSkinButton::OnCreate();
}

bool CSkinTextButton::OnMenuKey(UINT nChar, UINT nFlags)
{
	if (CharUpr(m_chMenuKey) == CharUpr(nChar))
	{
		ButtonUpAction();
		return true;
	}

	return false;
}

BOOL CSkinTextButton::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinButton::SetProperty(szProperty, szValue))
	{
		if (IsPropertyName(szProperty, _T("CheckBox"))
			|| IsPropertyName(szProperty, _T("Radio")))
			m_bResizeToContent = true;

		return TRUE;
	}

	if (IsPropertyName(szProperty, _T("AlignText")))
		m_dwAlignTextFlags = AlignTextFromStr(szValue);
	else if (IsPropertyName(szProperty, _T("ResizeToContent")))
		m_bResizeToContent = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("ImageContent")))
		m_imgContent.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	else if (IsPropertyName(szProperty, _T("TextLeftMargin")))
		m_nTextLeftMargin = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("TextRightMargin")))
		m_nTextRightMargin = _ttoi(szValue);
	else if (!m_font.SetProperty(szProperty, szValue))
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinTextButton::EnumProperties(CUIObjProperties &listProperties)
{
	CSkinButton::EnumProperties(listProperties);

	tstring		str;
	AlignTextToStr(m_dwAlignTextFlags, str);
	listProperties.AddPropStr(_T("AlignText"), str.c_str());
	listProperties.AddPropInt(_T("TextLeftMargin"), m_nTextLeft);
	m_font.EnumProperties(listProperties);
	listProperties.AddPropStr(_T("Text"), m_strText.c_str());
}
#endif // _SKIN_EDITOR_

void CSkinTextButton::SetText(LPCTSTR szText)
{
	CUIObject::SetText(szText);

	m_chMenuKey = GetMenuKey(m_strText.c_str());
	if (m_chMenuKey != 0)
		m_nMsgWant |= UO_MSG_WANT_MENU_KEY;
	else
		m_nMsgWant &= ~UO_MSG_WANT_MENU_KEY;
}

void CSkinTextButton::OnLanguageChanged()
{
	CSkinButton::OnLanguageChanged();

	m_chMenuKey = GetMenuKey(m_strText.c_str());
	if (m_chMenuKey != 0)
		m_nMsgWant |= UO_MSG_WANT_MENU_KEY;
	else
		m_nMsgWant &= ~UO_MSG_WANT_MENU_KEY;
}