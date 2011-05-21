// SkinTxtLink.cpp: implementation of the CSkinTxtLink class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinTxtLink.h"

LPCTSTR CSkinTxtLink::ms_szClassName = _T("TxtLink");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinTxtLink::CSkinTxtLink()
{
	m_nMsgWant = UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MOUSEMOVE;
	m_dwAlignText = 0;
}

CSkinTxtLink::~CSkinTxtLink()
{
}

LPCTSTR CSkinTxtLink::GetClassName()
{
	return ms_szClassName;
}

void CSkinTxtLink::Draw(CRawGraph *pGraph)
{
	CRect				rc;

	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	// Resize control by content size.
	SIZE size;
	if (pGraph->GetTextExtentPoint32(m_strText.c_str(), m_strText.size(), &size))
	{
		const int TEXT_MARGIN	= 8;
		if (IsFlagSet(m_dwAlignText, DT_RIGHT))
			m_rcObj.left = m_rcObj.right - size.cx - TEXT_MARGIN;
		else if (IsFlagSet(m_dwAlignText, DT_CENTER))
		{
			m_rcObj.left = (m_rcObj.left + m_rcObj.right - size.cx - TEXT_MARGIN) / 2;
			m_rcObj.right = m_rcObj.left + size.cx + TEXT_MARGIN;
		}
		else
			m_rcObj.right = m_rcObj.left + size.cx + TEXT_MARGIN;
		m_rcObj.bottom = m_rcObj.top + size.cy + TEXT_MARGIN / 2;
	}

	pGraph->SetTextColor(m_font.GetTextColor(m_bEnable));

	rc = m_rcObj;
	rc.left += 4;
	rc.right -= 2;
	pGraph->DrawText(m_strText.c_str(), m_strText.size(), &rc, m_dwAlignText | DT_SINGLELINE);
}

BOOL CSkinTxtLink::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("Link")) == 0)
	{
		m_strLink = szValue;
	}
	else if (m_font.SetProperty(szProperty, szValue))
		return TRUE;
	else if (_tcsicmp(szProperty, _T("AlignText")) == 0)
	{
		m_dwAlignText = AlignTextFromStr(szValue);
	}
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinTxtLink::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropStr(_T("Link"), m_strLink.c_str());
	m_font.EnumProperties(listProperties);
}
#endif // _SKIN_EDITOR_

void CSkinTxtLink::OnCreate()
{
	CUIObject::OnCreate();

	m_font.OnCreate(m_pSkin);
	m_font.SetProperty(_T("FontUnderLine"), _T("1"));

	m_Cursor.LoadStdCursor(CCursor::C_HAND);
}

bool CSkinTxtLink::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_strLink.size())
		OpenUrl(m_pSkin, m_strLink.c_str());

	if (m_nId != UID_INVALID)
		m_pSkin->PostCustomCommandMsg(m_nId);

	MLWidget::SetCursor(m_Cursor);

	return true;
}

bool CSkinTxtLink::OnLButtonDown(UINT nFlags, CPoint point)
{
	MLWidget::SetCursor(m_Cursor);

	return true;
}

bool CSkinTxtLink::OnMouseMove(UINT nFlags, CPoint point)
{
	MLWidget::SetCursor(m_Cursor);

	return true;
}
