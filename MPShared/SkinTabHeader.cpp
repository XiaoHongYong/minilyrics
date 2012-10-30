// SkinTabHeader.cpp: implementation of the CSkinTabHeader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkinTabHeader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinTabHeader, _T("TabHeader"))

CSkinTabHeader::CSkinTabHeader()
{
	m_nMsgWant = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON;

	m_bLBtDown = FALSE;
	m_nActiveButton = 0;
	m_nHoverButton = -1;

	m_nButtonFaceSize = 0;
	m_nButtonBorderSize = 0;
	m_nTabHeight = 0;

	m_nActiveButtonIncSize = 0;

	m_bEnableHover = false;
	m_bHover = false;
}

CSkinTabHeader::~CSkinTabHeader()
{
	g_Profile.WriteInt((tstring(_T("TabActiveBt_")) + m_strName).c_str(), m_nActiveButton);
}


CSkinTabHeader::Button::Button()
{
	nID = UID_INVALID;
	nAutoUniID = UID_INVALID;
	nIDAssociatedObj = UID_INVALID;
	bTempToolTip = true;
	nImageStartPos = 0;
	nImageWidth = 0;
	nWidth = 0;
}


void CSkinTabHeader::Button::FromXML(CSkinTabHeader *pTabHeader, SXNode *pNode)
{
	nAutoUniID = pTabHeader->m_pSkin->AllocAutoUniID();

	SXNode::iterProperties	it, itEnd;

	itEnd = pNode->listProperties.end();
	for (it = pNode->listProperties.begin(); it != itEnd; ++it)
	{
		SXNode::Property	&prop = *it;
		LPCTSTR		szProperty = prop.strName.c_str();
		LPCTSTR		szValue = prop.strValue.c_str();

		if (IsPropertyName(szProperty, _T("width")))
			nWidth = _ttoi(szValue);
		else if (IsPropertyName(szProperty, SZ_PN_ID))
		{
			if (strTooltip.empty())
			{
				nID = pTabHeader->m_pSkin->GetSkinFactory()->GetIDByNameEx(szValue, strTooltip);
				if (!strTooltip.empty())
					bTempToolTip = TRUE;
			}
			else
				nID = pTabHeader->m_pSkin->GetSkinFactory()->GetIDByName(szValue);
		}
		else if (IsPropertyName(szProperty, _T("ToolTip")))
		{
			if (_tcscmp(strTooltip.c_str(), szValue) != 0)
			{
				strTooltip = szValue;
				bTempToolTip = false;
			}
		}
		else if (IsPropertyName(szProperty, _T("Text")))
		{
			strText = szValue;
		}
		else if (IsPropertyName(szProperty, _T("IDAssociatedObj")))
		{
			strIDAssociatedObj = szValue;
			nIDAssociatedObj = pTabHeader->m_pSkin->GetSkinFactory()->GetIDByName(szValue);
		}
	}
}


#ifdef _SKIN_EDITOR_
void CSkinTabHeader::Button::ToXML(CSkinTabHeader *pTabHeader, CXMLWriter &xml)
{
	xml.WriteStartElement(_T("button"));

	if (nID != UID_INVALID)
		xml.WriteAttribute(SZ_PN_ID, pTabHeader->m_pSkin->GetSkinFactory()->GetStringOfID(nID).c_str());

	xml.WriteAttribute(_T("width"), nWidth);
	if (!strTooltip.empty() && !bTempToolTip)
		xml.WriteAttribute(_T("ToolTip"), strTooltip.c_str());
	if (!strText.empty())
		xml.WriteAttribute(_T("Text"), strText.c_str());
	if (!strIDAssociatedObj.empty())
		xml.WriteAttribute(_T("AssociatedObjID"), strIDAssociatedObj.c_str());

	xml.WriteEndElement();
}
#endif // _SKIN_EDITOR_


void CSkinTabHeader::OnCreate()
{
	CUIObject::OnCreate();

	m_font.OnCreate(m_pSkin);

	m_nActiveButton = g_Profile.GetInt((tstring(_T("TabActiveBt_")) + m_strName).c_str(), m_nActiveButton);
	if (m_nActiveButton < 0 || m_nActiveButton >= (int)m_vButtons.size())
		m_nActiveButton = 0;

	//
	// Hide/show all controls associated with TabHeader.
	//
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];

		CUIObject	*pObj = m_pSkin->GetUIObjectById(bt.nIDAssociatedObj);
		if (pObj)
		{
			if (i == m_nActiveButton)
				pObj->SetVisible(true);
			else
				pObj->SetVisible(false);
		}
	}
}


bool CSkinTabHeader::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_nHoverButton = ButtonHitTest(point);
	if (m_nHoverButton == -1)
		return false;

	m_bLBtDown = TRUE;

	if (m_bHover)
		m_bHover = false;
	else
	{
		// 捕捉鼠标输入
		m_pSkin->SetCaptureMouse(this);
	}

	Invalidate();

	return true;
}


bool CSkinTabHeader::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	return false;
}

void CSkinTabHeader::OnSize()
{
	// The whole toolbar can't have the tooltip
	if (m_strTooltip.size())
		m_strTooltip.resize(0);

	CUIObject::OnSize();

	CRect	rcBt;
	int		i;

	// Add tooltip of all the buttons.
	rcBt.left = m_rcObj.left;
	rcBt.top = m_rcObj.top;
	rcBt.bottom = m_rcObj.bottom;

	for (i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		if (bt.strTooltip.size())
		{
			if (rcBt.left + bt.nWidth < m_rcObj.right)
				rcBt.right = rcBt.left + bt.nWidth;
			else
				rcBt.right = m_rcObj.right;

			m_pSkin->DelTool(bt.nAutoUniID);
			m_pSkin->AddTool(TOLOCAL(bt.strTooltip.c_str()), &rcBt, bt.nAutoUniID);
		}

		rcBt.left += bt.nWidth;
		if (rcBt.left >= m_rcObj.right)
			break;
	}
}

bool CSkinTabHeader::OnLButtonUp(UINT nFlags, CPoint point)
{
	int			nNewCurButton = ButtonHitTest(point);
	if (nNewCurButton == -1)
		return false;

	m_bLBtDown = FALSE;

	// 释放鼠标输入
	m_pSkin->ReleaseCaptureMouse(this);

	// Execute...
	if (nNewCurButton >= 0 && nNewCurButton < (int)m_vButtons.size())
	{
		Button		&bt = m_vButtons[nNewCurButton];

		if (bt.nID != UID_INVALID)
			m_pSkin->PostCustomCommandMsg(bt.nID);

		// Hide old tab, show this tab.
		if (m_nActiveButton >= 0 && m_nActiveButton < (int)m_vButtons.size())
		{
			CUIObject	*pObj = m_pSkin->GetUIObjectById(m_vButtons[m_nActiveButton].nIDAssociatedObj);
			if (pObj)
				pObj->SetVisible(false);
		}

		CUIObject	*pObj = m_pSkin->GetUIObjectById(m_vButtons[nNewCurButton].nIDAssociatedObj);
		if (pObj)
			pObj->SetVisible(true);

		m_nActiveButton = nNewCurButton;

		m_pSkin->InvalidateRect();
	}

	return true;
}


bool CSkinTabHeader::OnMouseMove(UINT nFlags, CPoint point)
{
	assert(m_pSkin);

	if (IsPtIn(point))
	{
		int		nOldCurButton = m_nHoverButton;

		m_nHoverButton = ButtonHitTest(point);

		if (!m_bLBtDown && (nFlags & MK_LBUTTON))
		{
			m_bLBtDown = TRUE;

			if (m_bHover)
				m_bHover = false;
			else
			{
				// 捕捉鼠标输入
				m_pSkin->SetCaptureMouse(this);
			}

			Invalidate();
		}
		else if (!m_bLBtDown && m_bEnableHover)
		{
			m_bHover = true;

			// 捕捉鼠标输入
			m_pSkin->SetCaptureMouse(this);

			Invalidate();
		}
		else if (m_nHoverButton != nOldCurButton)
			Invalidate();
	}
	else
	{
		int		nOldCurButton = m_nHoverButton;

		m_nHoverButton = ButtonHitTest(point);

		if (m_bLBtDown)
		{
			m_bLBtDown = FALSE;

			// 释放鼠标输入
			m_pSkin->ReleaseCaptureMouse(this);

			Invalidate();
		}
		else if (m_bHover)
		{
			m_bHover = false;

			// 释放鼠标输入
			m_pSkin->ReleaseCaptureMouse(this);

			Invalidate();
		}
		else if (m_nHoverButton != nOldCurButton)
			Invalidate();
	}

	return true;
}


void CSkinTabHeader::Draw(CRawGraph *pGraph)
{
	int			x, xActiveTabButton = 0;
	CRect		rcButton;

	x = m_rcObj.left;

	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	pGraph->SetTextColor(m_font.GetTextColor(m_bEnable));

	rcButton.top = m_rcObj.top;
	rcButton.bottom = m_rcObj.top + m_nTabHeight;

	//
	// Draw every table button
	//
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button			&bt = m_vButtons[i];

		if (x + bt.nWidth >= m_rcObj.right)
			break;

		if (i == m_nActiveButton)
			xActiveTabButton = x;
		else
		{
			rcButton.left = x;
			rcButton.right = x + bt.nWidth;
			DrawTabButton(pGraph, bt, rcButton, false);
		}

		x += bt.nWidth;
	}

	if (m_nActiveButton >= 0 && m_nActiveButton < (int)m_vButtons.size())
	{
		rcButton.left = xActiveTabButton;
		rcButton.right = xActiveTabButton + m_vButtons[m_nActiveButton].nWidth;
		DrawTabButton(pGraph, m_vButtons[m_nActiveButton], rcButton, true);
	}
}

BOOL CSkinTabHeader::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, SZ_PN_IMAGE))
	{
		m_strImageFile = szValue;
		m_img.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	}
	else if (IsPropertyName(szProperty, SZ_PN_IMAGERECT))
	{
		if (!GetRectValue(szValue, m_img))
			ERRLOG2(_T("Analysis Value: %s = %s FAILED."), szProperty, szValue);
	}
	else if (IsPropertyName(szProperty, _T("ButtonFaceSize")))
		m_nButtonFaceSize = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("ButtonBorderSize")))
		m_nButtonBorderSize = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("TabHeight")))
		m_nTabHeight = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("ActiveButton")))
		m_nActiveButton = _ttoi(szValue);
	else if (m_font.SetProperty(szProperty, szValue))
		return TRUE;
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinTabHeader::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strImageFile.c_str(), m_img);
	listProperties.AddPropInt(_T("ButtonFaceSize"), m_nButtonFaceSize, true);
	listProperties.AddPropInt(_T("BorderSize"), m_nButtonBorderSize, true);
	listProperties.AddPropInt(_T("TabHeight"), m_nTabHeight, true);
	listProperties.AddPropInt(_T("ActiveButton"), m_nActiveButton, m_nActiveButton != 0);
	m_font.EnumProperties(listProperties);
}
#endif // _SKIN_EDITOR_


int CSkinTabHeader::FromXML(SXNode *pXmlNode)
{
	int		nRet;

	nRet = CUIObject::FromXML(pXmlNode);
	if (nRet != ERR_OK)
		return nRet;

	for (SXNode::iterator it = pXmlNode->listChildren.begin(); 
		it != pXmlNode->listChildren.end(); ++it)
	{
		SXNode		*pChild = *it;

		if (_tcsicmp(pChild->strName.c_str(), _T("button")) == 0)
		{
			Button	bt;
			bt.FromXML(this, pChild);
			m_vButtons.push_back(bt);
		}
	}

	return ERR_OK;
}


#ifdef _SKIN_EDITOR_
void CSkinTabHeader::OnToXMLChild(CXMLWriter &xmlStream)
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button	&bt = m_vButtons[i];

		bt.ToXML(this, xmlStream);
	}
}
#endif // _SKIN_EDITOR_


void CSkinTabHeader::InvalidateButton(int nButton)
{
	Invalidate();
}


int CSkinTabHeader::ButtonHitTest(POINT pt)
{
	if (!m_rcObj.PtInRect(pt))
		return -1;

	pt.x -= m_rcObj.left;
	pt.y -= m_rcObj.top;

	int		x = 0;

	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];

		x += bt.nWidth;
		if (x > m_rcObj.Width())
			return -1;

		if (pt.x < x)
			return i;
	}

	return -1;
}

void TillFill(CRawGraph *pGraph, RECT *rcFill, PRAW_IMAGE_DATA rawImage, RECT *rcMask, const MLColor &clrFill)
{
	CRect		rc = *rcFill;
	int			nWidthMask = rcMask->right - rcMask->left;

	rc.right = rc.left + nWidthMask;
	if (rc.right > rcFill->right)
		rc.right = rcFill->right;
	for (; rc.right <= rcFill->right; rc.left += nWidthMask, rc.right += nWidthMask)
	{
		pGraph->FillRect(&rc, rawImage, rcMask, clrFill);
	}

	if (rc.right > rcFill->right)
	{
		rc.right = rcFill->right;
		pGraph->FillRect(&rc, rawImage, rcMask, clrFill);
	}
}

void CSkinTabHeader::DrawTabButton(CRawGraph *pGraph, Button &bt, CRect &rcButton, bool bActiveTabButton)
{
	CRect			rcMask;
	int				nYPosTabButton;
	int				x = rcButton.left;
	int				y = rcButton.top;

	ASSERT(m_img.IsValid());
	if (!m_img.IsValid())
		return;

	// Draw active button bg
	if (bActiveTabButton)
	{
		CRect		rc;

		nYPosTabButton = TIP_ACTIVE * m_nTabHeight;

		rcMask.left = 0;
		rcMask.top = TIP_ACTIVE_MASK * m_nTabHeight;
		rcMask.right = m_img.width();
		rcMask.bottom = rcMask.top + m_nTabHeight;

		MLColor clrBg = m_pContainer->GetBgColor();

		// Fill left border
		rc = rcButton;
		rc.right = rc.left + m_nButtonBorderSize;
		pGraph->FillRect(&rc, m_img.GetHandle(), &rcMask, clrBg);

		// Button face
		rc.left = rc.right;
		rc.right = rcButton.right;
		rcMask.left += m_nButtonBorderSize;
		rcMask.right -= m_nButtonBorderSize;
		TillFill(pGraph, &rc, m_img.GetHandle(), &rcMask, clrBg);

		// Fill right border
		rc.left = rc.right;
		rc.right = rc.left + m_nButtonBorderSize;
		rcMask.left = rcMask.right;
		rcMask.right += m_nButtonBorderSize;
		pGraph->FillRect(&rc, m_img.GetHandle(), &rcMask, clrBg);
	}
	else
		nYPosTabButton = TIP_INACTIVE * m_nTabHeight;

	// Draw left border
	m_img.Blt(pGraph, x, y, m_nButtonBorderSize, m_nTabHeight, 0, nYPosTabButton, BPM_BLEND);

	// Draw button face
	m_img.XTileBlt(pGraph, x + m_nButtonBorderSize, y, bt.nWidth - m_nButtonBorderSize, m_nTabHeight, 
		m_nButtonBorderSize, nYPosTabButton, m_nButtonFaceSize, m_nTabHeight, BPM_BLEND);

	// Draw Right border
	m_img.Blt(pGraph, rcButton.right, y, m_nButtonBorderSize, m_nTabHeight, 
		m_nButtonFaceSize + m_nButtonBorderSize, nYPosTabButton, BPM_BLEND);

	// Draw Text
	if (!bt.strText.empty())
	{
		CRect	rcText = rcButton;
		rcText.left += m_nButtonBorderSize;
		if (m_font.IsOutlined())
			pGraph->DrawTextOutlined(bt.strText.c_str(), bt.strText.size(), &rcText, m_font.GetTextColor(m_bEnable), m_font.GetColorOutlined(), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		else
		{
			pGraph->SetTextColor(m_font.GetTextColor(m_bEnable));
			pGraph->DrawText(bt.strText.c_str(), bt.strText.size(), &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
	}
}
