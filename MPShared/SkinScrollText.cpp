#include "stdafx.h"
#include "SkinScrollText.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define TIMER_SPAN_SCROLL		30
#define WIDTH_TXT_STILL			(2000 / TIMER_SPAN_SCROLL)

UIOBJECT_CLASS_NAME_IMP(CSkinScrollText, _T("ScrollText"))

CSkinScrollText::CSkinScrollText()
{
	m_nTimerIDScroll = 0;
	m_nPosScroll = WIDTH_TXT_STILL;
	m_nWidthText = 0;
	m_bToLeft = true;
}

CSkinScrollText::~CSkinScrollText()
{

}
// 
// bool CSkinScrollText::OnLButtonUp(UINT nFlags, CPoint point)
// {
// 	if (!m_bEnable)
// 		return false;
// 
// 	if (m_nId != UID_INVALID)
// 		m_pSkin->OnCustomCommand(m_nId);
// 
// 	return true;
// }

void CSkinScrollText::Draw(CRawGraph *pGraph)
{
	if (m_imgBk.IsValid())
	{
		if (m_BltMode == BLT_COPY)
		{
			m_imgBk.Blt(pGraph, m_rcObj.left, m_rcObj.top);
		}
		else if (m_BltMode == BLT_TILE)
		{
			m_imgBk.TileBlt(pGraph, m_rcObj.left, m_rcObj.top,
				m_rcObj.Width(), m_rcObj.Height());
		}
		else
		{
			m_imgBk.StretchBlt(pGraph, m_rcObj.left, m_rcObj.top,
						m_rcObj.Width(), m_rcObj.Height());
		}
	}
	else
	{
		if (m_bPaintBg && !m_bUseSkinBg)
			pGraph->FillRect(m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), m_clrBg);
	}

	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	CRect		rc = m_rcObj;
	SIZE		size;

	rc.left += m_nLeftMargin;

	pGraph->GetTextExtentPoint32(m_strText.c_str(), m_strText.size(), &size);
	size.cx += 3;
	if (m_nWidthText != size.cx)
	{
		m_nWidthText = size.cx;
		m_nPosScroll = 0;
		m_bToLeft = true;
	}

	// set timer to scroll
	if (m_nWidthText > rc.Width() && m_nTimerIDScroll == 0)
	{
		m_nTimerIDScroll = m_pSkin->RegisterTimerObject(this, TIMER_SPAN_SCROLL);
	}

	// kill timer
	if (m_nWidthText <= rc.Width() && m_nTimerIDScroll != 0)
	{
		m_pSkin->UnregisterTimerObject(this, m_nTimerIDScroll);
		m_nTimerIDScroll = 0;
		m_nPosScroll = -WIDTH_TXT_STILL;
	}

	int			nLeftClip = 0;
	if (m_nTimerIDScroll != 0 && m_nPosScroll > 0)
	{
		if (m_nPosScroll + rc.Width() >= m_nWidthText)
		{
			nLeftClip = m_nWidthText - rc.Width();
			rc.left -= m_nWidthText - rc.Width();
		}
		else
		{
			rc.left -= m_nPosScroll;
			nLeftClip = m_nPosScroll;
		}
	}

	if (m_font.IsOutlined())
	{
		pGraph->DrawTextClipOutlined(m_strText.c_str(), (int)m_strText.size(), &rc, m_font.GetTextColor(m_bEnable), m_font.GetColorOutlined(), nLeftClip);
	}
	else
	{
		pGraph->SetTextColor(m_font.GetTextColor(m_bEnable));
		pGraph->DrawTextClip(m_strText.c_str(), (int)m_strText.size(), &rc, nLeftClip);
	}
	// pGraph->DrawText(m_strText.c_str(), (int)m_strText.size(), &rc, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
}
// 
// BOOL CSkinScrollText::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
// {
// 	if (CSkinStaticText::SetProperty(szProperty, szValue))
// 		return TRUE;
// 	else
// 		return false;
// 
// 	return TRUE;
// }
// 
// void CSkinScrollText::EnumProperties(CUIObjProperties &listProperties)
// {
// 	CSkinStaticText::EnumProperties(listProperties);
// }
// 
// void CSkinScrollText::OnCreate()
// {
// // 	m_nTimerIDScroll = m_pSkin->RegisterTimerObject(this, TIMER_SPAN_SCROLL);
// }

bool CSkinScrollText::OnLButtonUp(UINT nFlags, CPoint point)
{
	return true;
}

bool CSkinScrollText::OnLButtonDown(UINT nFlags, CPoint point)
{
	return true;
}

void CSkinScrollText::OnTimer(int nId)
{
	if (nId == m_nTimerIDScroll)
	{
		if (m_bToLeft)
		{
			m_nPosScroll--;
			if (m_nPosScroll <= -WIDTH_TXT_STILL)
			{
				m_bToLeft = false;
				m_nPosScroll = 0;
			}
		}
		else
		{
			m_nPosScroll++;
			if (m_nPosScroll >= (m_nWidthText - m_rcObj.Width() - m_nLeftMargin) + WIDTH_TXT_STILL)
			{
				m_bToLeft = true;
				m_nPosScroll = (m_nWidthText - m_rcObj.Width() - m_nLeftMargin);
			}
		}

		Invalidate();
	}
}
