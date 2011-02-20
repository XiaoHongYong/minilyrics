#include "StdAfx.h"
#include "Skin.h"
#include "SkinTabButton.h"

UIOBJECT_CLASS_NAME_IMP(CSkinTabButton,  _T("TabButton"))

CSkinTabButton::CSkinTabButton()
{
	m_bDrawBtText = true;

	m_nSperatorLineWidth = 1;
	m_nButtonFaceWidth = 1;
	m_nButtonBorderWidth = 1;
}

CSkinTabButton::~CSkinTabButton()
{
}

void CSkinTabButton::Draw(CRawGraph *pGraph)
{
	CSkinToolbar::Draw(pGraph);

	int		nWidth = 0;
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		nWidth += m_vButtons[i].nWidth;
	}
	m_rcObj.right = m_rcObj.left + nWidth;
}

void CSkinTabButton::DrawButton(CRawGraph *pGraph, int nButton, BT_DRAW_STATE btDrawState, int x, int y, int nHeight, int nBtImageLeft)
{
	Button &bt = m_vButtons[nButton];
	int		xOrg = x;

	CalculateBtWidth(pGraph, bt);

	int		nBtImageTop = m_nImageHeight * btDrawState;
	int		nButtonWidth = bt.nWidth;

	if (nButton != m_vButtons.size() - 1)
		nButtonWidth--;

	if (bt.nCurStatus == Button::STATUS_CHECKED)
		nBtImageLeft = m_image.m_cx / 2;
	else
		nBtImageLeft = 0;

	// Draw button left border
	int		nBorderLeft = m_nMarginX + nBtImageLeft;
	if (nButton != 0)
		nBorderLeft += m_nButtonBorderWidth * 2 + m_nButtonFaceWidth + m_nSperatorLineWidth;
	m_image.Blt(pGraph, x, y, m_nButtonBorderWidth, nHeight, nBorderLeft, nBtImageTop);
	x += m_nButtonBorderWidth;

	// Draw button face
	int		nFaceLeft = nBorderLeft + m_nButtonBorderWidth;
	int		nFaceWidth = nButtonWidth - m_nButtonBorderWidth * 2;
	m_image.XTileBlt(pGraph, x, y, nFaceWidth, nHeight, nFaceLeft, nBtImageTop,
		m_nButtonFaceWidth, m_nImageHeight);
	x += nFaceWidth;

	// Draw button right border
	int		nRightBorderLeft = nBtImageLeft;
	if (nButton == m_vButtons.size() - 1)
		nRightBorderLeft += m_image.m_cx / 2 - m_nMarginX - m_nButtonBorderWidth;
	else
		nRightBorderLeft += m_nMarginX + m_nButtonBorderWidth + m_nButtonFaceWidth;
	m_image.Blt(pGraph, x, y, m_nButtonBorderWidth, nHeight, nRightBorderLeft, nBtImageTop);
	x += m_nButtonBorderWidth;

	// Draw button sepeartor
	if (nButton != m_vButtons.size() - 1)
	{
		// If next button is checked, draw seperator highlight.
		if (m_vButtons[nButton + 1].nCurStatus == Button::STATUS_CHECKED)
			nBtImageLeft = m_image.m_cx / 2;
		int		nSeperatorLeft = nBtImageLeft + m_nMarginX
			+ m_nButtonBorderWidth * 2 + m_nButtonFaceWidth;
		m_image.Blt(pGraph, x, y, m_nSperatorLineWidth, nHeight, nSeperatorLeft, nBtImageTop);
	}

	// Draw Button text
	if (bt.strText.size())
	{
		CRect	rcText(xOrg, y, xOrg + bt.nWidth, y + nHeight);

		pGraph->DrawText(bt.strText.c_str(), bt.strText.size(), &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
}

void CSkinTabButton::OnCreate()
{
	CSkinToolbar::OnCreate();

	m_nBlankX = -1;

	m_image.GetOrginalSize(m_image.m_cx, m_image.m_cy);

	m_nImageHeight = m_image.m_cy / 3;

	// 2 * ( <MarginX> <ButtonBorder> <ButtonFace> <ButtonBorder> <SperatorLine> <ButtonBorder> <ButtonFace> <ButtonBorder> <MarginX>)
	m_nButtonFaceWidth = (m_image.m_cx / 2 - m_nMarginX * 2 - m_nSperatorLineWidth) / 2
		- m_nButtonBorderWidth * 2;

	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		m_vButtons[i].nWidthImage = m_vButtons[i].nWidth = m_nButtonBorderWidth * 2 + m_nButtonFaceWidth;
		m_vButtons[i].nGroup = 1;
		m_vButtons[i].nStatusMax = Button::STATUS_CHECKED;
	}
}

bool CSkinTabButton::OnMenuKey(UINT nChar, UINT nFlags)
{
// 	if (CharUpr(m_chMenuKey) == CharUpr(nChar))
// 	{
// 		if (m_nId != SKIN_UIOBJ_ID_UNDEFINE)
// 			m_pSkin->PostCustomCommandMsg(m_nId);
// 
// 		if (m_vBtStatImg[m_nCurStatus]->nIDCmd != SKIN_UIOBJ_ID_UNDEFINE)
// 			m_pSkin->PostCustomCommandMsg(m_vBtStatImg[m_nCurStatus]->nIDCmd);
// 		return true;
// 	}

	return false;
}

BOOL CSkinTabButton::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinToolbar::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("SperatorLineWidth")))
		m_nSperatorLineWidth = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("ButtunBorderWidth")))
		m_nButtonBorderWidth = _ttoi(szValue);

	return TRUE;
}
