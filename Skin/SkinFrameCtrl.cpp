#include "StdAfx.h"
#include "Skin.h"
#include "SkinFrameCtrl.h"


UIOBJECT_CLASS_NAME_IMP(CSkinFrameCtrl, _T("Frame"))

CSkinFrameCtrl::CSkinFrameCtrl()
{
	m_nRoundWidthTop = m_nRoundWidthBottom = 0;
	m_xBorder = m_yBorder = 0;
	m_nThickWidth = 0;
	m_pObjFocusIndicator = NULL;
	m_bpm = BPM_BLEND;
}

CSkinFrameCtrl::~CSkinFrameCtrl()
{
}

void CSkinFrameCtrl::OnCreate()
{
	CUIObject::OnCreate();

	m_font.OnCreate(m_pSkin);
}

void CSkinFrameCtrl::OnAdjustHue(float hue, float saturation, float luminance)
{
	m_font.OnAdjustHue(m_pSkin, hue, saturation, luminance);
}

void CSkinFrameCtrl::Draw(CRawGraph *pGraph)
{
	int xTextStart = 10;
	const int xTextBorder = 10;
	SIZE sizeText = {0, 0};
	CRect rcFrame = m_rcObj;

	rcFrame.DeflateRect(m_xBorder, 0);
	rcFrame.top += m_yBorder;

	pGraph->GetTextExtentPoint32(m_strText.c_str(), m_strText.size(), &sizeText);
	if (sizeText.cx > 0)
		sizeText.cx += xTextBorder * 2;

	if (m_nRoundWidthTop > xTextStart)
		xTextStart = m_nRoundWidthTop + rcFrame.left;
	else
		xTextStart += rcFrame.left;

	CSFImage *pImage = &m_image;
	if (m_pObjFocusIndicator)
	{
		if (m_imageFocus.IsValid() && m_pSkin->GetFocusUIObj() == m_pObjFocusIndicator)
			pImage = &m_imageFocus;
	}
	else if (m_imageFocus.IsValid() && m_pSkin->IsWndActive())
		pImage = &m_imageFocus;

	// Draw top left corner.
	pImage->Blt(pGraph, rcFrame.left, rcFrame.top, m_nRoundWidthTop, m_nRoundWidthTop, 0, 0, m_bpm);

	if (sizeText.cx > 0)
	{
		// Draw top border left
		pImage->XTileBlt(pGraph, rcFrame.left + m_nRoundWidthTop, rcFrame.top, 
			xTextStart - rcFrame.left, m_nThickWidth, m_nRoundWidthTop, 0,
			pImage->m_cx - m_nRoundWidthTop * 2, m_nThickWidth, m_bpm);


		if (m_font.IsGood())
			pGraph->SetFont(m_font.GetFont());

		// Draw text
		CRect	rcText(xTextStart, m_rcObj.top, 
			xTextStart + sizeText.cx, m_rcObj.top + m_yBorder * 2 + m_nThickWidth);
		pGraph->SetTextColor(m_font.GetTextColor(m_bEnable));
		pGraph->DrawText(m_strText.c_str(), m_strText.size(), &rcText, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

		// Draw top border right
		pImage->XTileBlt(pGraph, rcText.right, rcFrame.top, 
			rcFrame.right - rcText.right - m_nRoundWidthTop, m_nThickWidth, m_nRoundWidthTop, 0,
			pImage->m_cx - m_nRoundWidthTop * 2, m_nThickWidth, m_bpm);
	}
	else
	{
		// Draw top border
		pImage->XTileBlt(pGraph, rcFrame.left + m_nRoundWidthTop, rcFrame.top, 
			rcFrame.Width() - m_nRoundWidthTop * 2, m_nThickWidth, m_nRoundWidthTop, 0,
			pImage->m_cx - m_nRoundWidthTop * 2, m_nThickWidth, m_bpm);
	}

	// Draw top right corner
	pImage->Blt(pGraph, rcFrame.right - m_nRoundWidthTop, rcFrame.top,
		m_nRoundWidthTop, m_nRoundWidthTop, pImage->m_cx - m_nRoundWidthTop, 0, m_bpm);

	// Draw left border
	pImage->YTileBlt(pGraph, rcFrame.left, rcFrame.top + m_nRoundWidthTop,
		m_nThickWidth, rcFrame.Height() - m_nRoundWidthTop - m_nRoundWidthBottom,
		0, m_nRoundWidthTop, m_nThickWidth, pImage->m_cy - m_nRoundWidthTop - m_nRoundWidthBottom, m_bpm);

	// Draw right border
	pImage->YTileBlt(pGraph, rcFrame.right - m_nThickWidth, rcFrame.top + m_nRoundWidthTop,
		m_nThickWidth, rcFrame.Height() - m_nRoundWidthTop - m_nRoundWidthBottom,
		pImage->m_cx - m_nThickWidth, m_nRoundWidthTop, m_nThickWidth, pImage->m_cy - m_nRoundWidthTop - m_nRoundWidthBottom, m_bpm);

	// Draw left bottom corner.
	pImage->Blt(pGraph, rcFrame.left, rcFrame.bottom - m_nRoundWidthBottom,
		m_nRoundWidthBottom, m_nRoundWidthBottom, 0, pImage->m_cy - m_nRoundWidthBottom, m_bpm);

	// Draw bottom border
	pImage->XTileBlt(pGraph, rcFrame.left + m_nRoundWidthBottom, rcFrame.bottom - m_nThickWidth, 
		rcFrame.Width() - m_nRoundWidthBottom * 2, m_nThickWidth, m_nRoundWidthBottom, pImage->m_cy - m_nThickWidth,
		pImage->m_cx - m_nRoundWidthBottom * 2, m_nThickWidth, m_bpm);

	// Draw right bottom corner
	pImage->Blt(pGraph, rcFrame.right - m_nRoundWidthBottom, rcFrame.bottom - m_nRoundWidthBottom,
		m_nRoundWidthBottom, m_nRoundWidthBottom, pImage->m_cx - m_nRoundWidthBottom, pImage->m_cy - m_nRoundWidthBottom, m_bpm);
}

BOOL CSkinFrameCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, SZ_PN_IMAGE))
		m_image.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	else if (IsPropertyName(szProperty, _T("ImageFocus")))
		m_imageFocus.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	else if (IsPropertyName(szProperty, _T("BlendPixMode")))
		m_bpm = BlendPixModeFromStr(szValue);
	else if (IsPropertyName(szProperty, _T("RoundWidth")))
		m_nRoundWidthTop = m_nRoundWidthBottom = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("RoundWidthTop")))
		m_nRoundWidthTop = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("RoundWidthBottom")))
		m_nRoundWidthBottom = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("ThickWidth")))
		m_nThickWidth = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("XBorder")))
		m_xBorder = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("YBorder")))
		m_yBorder = _ttoi(szValue);
	else if (m_font.SetProperty(szProperty, szValue))
		return TRUE;
	else
		return FALSE;

	return TRUE;
}
