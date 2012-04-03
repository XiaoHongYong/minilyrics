// SkinToolBar.cpp: implementation of the CSkinToolbar class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinToolBar.h"

#define INVALID_POS		(-1)

#define FADE_BUTTON_TIME_OUT		300

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// <Toolbar name="selfedit" image="toolbar.bmp" enablehover="1" height="22"
//     units_x="22" blank_x="638">
//   <buttons ID="ID_NEW" left="1" width="1" CanCheck="0" Checked="0"/>
//	 <buttons ID="ID_OPEN" left="3" width="1" CanCheck="0" Checked="0"/>
//	 <buttons ID="ID_EDIT_MODE" left="6" width="1" CanCheck="1" Checked="0" 
//     checked_left="7" disabled_left="8"/>
//   <seperator left="1" width="1"/>
// </Toolbar>

#define SZ_LEFT_OF_STATUS _T("left_of_status_")
#define LEN_LEFT_OF_STATUS	(CountOf(SZ_LEFT_OF_STATUS) - 1)

UIOBJECT_CLASS_NAME_IMP(CSkinToolbar, _T("Toolbar"))

CSkinToolbar::CSkinToolbar()
{
	m_nMsgWant = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON;

	m_nUnitsOfX = 1;

	m_bLBtDown = FALSE;
	m_bHover = false;
	m_bEnableHover = false;

	m_nBlankX = 0;
	m_nBlankCX = 0;

	m_nSeperatorX = -1;
	m_nSeperatorCX = 0;

	m_nBtSpacesCX = 0;

	m_nMarginX = 1;
	m_nMarginY = 1;

	m_nCurButton = -1;

	m_nImageHeight = 0;

	m_bFullStatusImage = false;
	m_bContinuousBegin = false;

	m_bFadein = true;
	m_dwBeginFadeinTime = 0;
}

CSkinToolbar::~CSkinToolbar()
{
	DelTooltip();
}

CSkinToolbar::Button::Button()
{
	nID = UID_INVALID;
	nWidth = 1;
	nWidthImage = 1;
	bSeperator = false;
	nGroup = -1;
	nStatusMax = STATUS_NORMAL;
	nCurStatus = STATUS_NORMAL;
	bEnabled = true;
	bTempToolTip = true;
	nLeftOfDiabled = -1;		// Image position of disabled
	bContinuousCmd = false;
	nAutoUniID = 0;
}

void CSkinToolbar::Button::FromXML(CSkinToolbar *pToolbar, SXNode *pNode)
{
	nAutoUniID = pToolbar->m_pSkin->AllocAutoUniID();

	SXNode::iterProperties	it, itEnd;

	itEnd = pNode->listProperties.end();
	for (it = pNode->listProperties.begin(); it != itEnd; ++it)
	{
		SXNode::Property	&prop = *it;
		LPCTSTR		szProperty = prop.strName.c_str();
		LPCTSTR		szValue = prop.strValue.c_str();

		if (_tcsicmp(szProperty, _T("CanCheck")) == 0)
			nStatusMax = IsTRUE(szValue);
		else if (_tcsicmp(szProperty, _T("Checked")) == 0)
			nCurStatus = IsTRUE(szValue);
		else if (_tcsicmp(szProperty, _T("Enabled")) == 0)
			bEnabled = IsTRUE(szValue);
		else if (IsPropertyName(szProperty, _T("Text")))
		{
			strTextEnglish = szValue;
			strText = _TL(szValue);
		}
		else if (IsPropertyName(szProperty, _T("RadioGroup")))
			nGroup = _ttoi(szValue);
		else if (_tcsicmp(szProperty, _T("left")) == 0)
			nLeftPositon[STATUS_NORMAL] = _ttoi(szValue) * pToolbar->m_nUnitsOfX;
		else if (_tcsicmp(szProperty, _T("width")) == 0)
			nWidthImage = nWidth = _ttoi(szValue) * pToolbar->m_nUnitsOfX;
		else if (_tcsicmp(szProperty, _T("checked_left")) == 0)
		{
			nLeftPositon[STATUS_CHECKED] = _ttoi(szValue) * pToolbar->m_nUnitsOfX;
			if (nStatusMax < STATUS_CHECKED)
				nStatusMax = STATUS_CHECKED;
		}
		else if (_tcsicmp(szProperty, _T("disabled_left")) == 0)
			nLeftOfDiabled = _ttoi(szValue) * pToolbar->m_nUnitsOfX;
		else if (_tcsnicmp(szProperty, SZ_LEFT_OF_STATUS, LEN_LEFT_OF_STATUS) == 0)
		{
			// "left_of_status_" property
			int		n = _ttoi(szProperty + LEN_LEFT_OF_STATUS);
			if (n > Button::STATUS_MAX)
				return;
			if (n > nStatusMax)
				nStatusMax = n;
			nLeftPositon[n] = _ttoi(szValue) * pToolbar->m_nUnitsOfX;
		}
		else if (_tcsicmp(szProperty, _T("Continuous")) == 0)
			bContinuousCmd = IsTRUE(szValue);
		else if (_tcsicmp(szProperty, SZ_PN_ID) == 0)
		{
			if (strTooltip.empty())
			{
				nID = pToolbar->m_pSkin->GetSkinFactory()->GetIDByNameEx(szValue, strTooltip);
				if (!strTooltip.empty())
					bTempToolTip = TRUE;
			}
			else
				nID = pToolbar->m_pSkin->GetSkinFactory()->GetIDByName(szValue);
		}
		else if (_tcsicmp(szProperty, _T("ToolTip")) == 0)
		{
			tstring str = _TL(szValue);
			if (_tcscmp(strTooltip.c_str(), str.c_str()) != 0)
			{
				strTooltip = str.c_str();
				bTempToolTip = false;
			}
		}
	}
}

#ifdef _SKIN_EDITOR_
void CSkinToolbar::Button::ToXML(CSkinToolbar *pToolbar, CXMLWriter &xml)
{
	if (bSeperator)
	{
		xml.WriteStartElement(_T("seperator"));
		xml.WriteEndElement();
	}
	else
	{
		xml.WriteStartElement(_T("button"));

		if (nID != UID_INVALID)
			xml.WriteAttribute(SZ_PN_ID, pToolbar->m_pSkin->GetSkinFactory()->GetStringOfID(nID).c_str());

		if (bCanCheck)
			xml.WriteAttribute(_T("CanCheck"), bCanCheck);
		if (bCanCheck)
			xml.WriteAttribute(_T("Checked"), bChecked);
		if (!bEnabled)
			xml.WriteAttribute(_T("Enabled"), bEnabled);

		if (pToolbar->m_nUnitsOfX <= 0)
			pToolbar->m_nUnitsOfX = 1;
		xml.WriteAttribute(_T("left"), nLeft / pToolbar->m_nUnitsOfX);
		if (nWidth != pToolbar->m_nUnitsOfX)
			xml.WriteAttribute(_T("width"), nWidth / pToolbar->m_nUnitsOfX);
		if (nLeftOfChecked != -1)
			xml.WriteAttribute(_T("checked_left"), nLeftOfChecked / pToolbar->m_nUnitsOfX);
		if (nLeftOfDiabled != -1)
			xml.WriteAttribute(_T("disabled_left"), nLeftOfDiabled / pToolbar->m_nUnitsOfX);
		if (bContinuousCmd)
			xml.WriteAttribute(_T("Continuous"), bContinuousCmd);
		if (!strTooltip.empty() && !bTempToolTip)
			xml.WriteAttribute(_T("ToolTip"), strTooltip.c_str());

		xml.WriteEndElement();
	}
}
#endif // _SKIN_EDITOR_

void CSkinToolbar::OnCreate()
{
	CUIObject::OnCreate();

	m_font.OnCreate(m_pSkin);
	if (m_nImageHeight == 0)
		m_nImageHeight = _ttoi(m_formHeight.GetFormula());
}

void CSkinToolbar::OnAdjustHue(float hue, float saturation, float luminance)
{
	m_font.OnAdjustHue(m_pSkin, hue, saturation, luminance);
}

bool CSkinToolbar::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_nCurButton = GetCurButton(point);
	if (m_nCurButton == -1)
		return false;

	if (m_bFadein && m_bFullStatusImage)
	{
		m_pSkin->RegisterTimerObject(this, 50);
		m_dwBeginFadeinTime = GetTickCount();
		Button &bt = m_vButtons[m_nCurButton];
		m_nLastImageLeft = bt.nLeftPositon[bt.nCurStatus];
		m_nLastImageDrawState = ROW_HOVER;
	}
	else if (m_vButtons[m_nCurButton].bContinuousCmd)
	{
		m_bContinuousBegin = true;
		m_pSkin->RegisterTimerObject(this, 500);
	}

	assert(m_pSkin);

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

bool CSkinToolbar::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_pSkin->UnregisterTimerObject(this);

	if (m_bFadein && m_bFullStatusImage && m_nCurButton != -1)
	{
		m_pSkin->RegisterTimerObject(this, 50);
		m_dwBeginFadeinTime = GetTickCount();
		Button &bt = m_vButtons[m_nCurButton];
		m_nLastImageLeft = bt.nLeftPositon[bt.nCurStatus];
		m_nLastImageDrawState = ROW_DOWN;
	}

	m_bLBtDown = FALSE;

	// 释放鼠标输入
	m_pSkin->ReleaseCaptureMouse(this);

	// 转换按钮的状态
	if (m_nCurButton != -1)
	{
		ASSERT(m_nCurButton >= 0 && m_nCurButton <= (int)m_vButtons.size());
		Button		&bt = m_vButtons[m_nCurButton];
		if (!(bt.nGroup != -1 && bt.nCurStatus == Button::STATUS_CHECKED))
		{
			if (bt.nCurStatus < bt.nStatusMax)
				bt.nCurStatus++;
			else if (bt.nCurStatus > 0)
				bt.nCurStatus = 0;

			if (bt.nGroup != -1)
				GroupButtonUncheckOld(bt.nGroup, m_nCurButton);

			InvalidateButton(m_nCurButton);

			if (bt.nID != UID_INVALID)
				m_pSkin->PostCustomCommandMsg(bt.nID);
		}
	}

	return true;
}

void CSkinToolbar::OnSize()
{
	// The whole toolbar can't have the tooltip
	if (m_strTooltip.size())
		m_strTooltip.resize(0);

	CUIObject::OnSize();

	if (m_bVisible)
		AddTooltip();
}

void CSkinToolbar::OnTimer(int nId)
{
	if (m_dwBeginFadeinTime != 0)
	{
		// In fade in, just redraw
		Invalidate();
		return;
	}

	if (IsKeyPressed(VK_LBUTTON))
	{
		POINT	pt;

		MLWidget::GetCursorPos(&pt);

		if (m_nCurButton != -1)
		{
			if (m_vButtons[m_nCurButton].nID != UID_INVALID)
				m_pSkin->PostCustomCommandMsg(m_vButtons[m_nCurButton].nID);
		}

		if (m_bContinuousBegin)
		{
			m_pSkin->UnregisterTimerObject(this);
			m_pSkin->RegisterTimerObject(this, 100);
			m_bContinuousBegin = false;
		}
	}
	else
		m_pSkin->UnregisterTimerObject(this);
}

bool CSkinToolbar::OnMouseDrag(CPoint point)
{
	int		nOldCurButton = m_nCurButton;
	bool	bUpdate = false;

	m_nCurButton = GetCurButton(point);

	if (IsPtIn(point))
	{
		if (!m_bLBtDown)
		{
			m_bLBtDown = TRUE;
			bUpdate = true;

			if (m_bHover)
				m_bHover = false;
			else
				m_pSkin->SetCaptureMouse(this);
		}
	}
	else
	{
		if (m_bLBtDown)
		{
			m_bLBtDown = FALSE;
			bUpdate = true;
		}
	}

	if (m_nCurButton != nOldCurButton || bUpdate)
		Invalidate();

	return true;
}

bool CSkinToolbar::OnMouseMove(CPoint point)
{
	int		nOldCurButton = m_nCurButton;
	bool	bHoverOld = m_bHover;
	m_nCurButton = GetCurButton(point);

	if (IsPtIn(point))
	{
		if (m_bEnableHover)
		{
			m_bHover = true;
			m_pSkin->SetCaptureMouse(this);
		}
	}
	else
	{
		if (m_bHover)
		{
			m_bHover = false;
			m_pSkin->ReleaseCaptureMouse(this);
		}
	}

	if (m_nCurButton != nOldCurButton || bHoverOld != m_bHover)
		Invalidate();

	return true;
}

void CSkinToolbar::OnLanguageChanged()
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		m_vButtons[i].strText = _TL(m_vButtons[i].strTextEnglish.c_str());
		if (m_vButtons[i].strTooltip.size())
			m_pSkin->GetSkinFactory()->GetTooltip(m_vButtons[i].nID, m_vButtons[i].strTooltip);
	}
}

/*
void Draw3DFrame(CRawGraph *pGraph, CRect &rc, COLORREF clrHigh, COLORREF clrLow)
{
	HDC		hdc = pGraph->GetHandle();
	HPEN	hpenLow, hpenHigh, hpenOld;

	hpenLow = CreatePen(PS_SOLID, 1, clrLow);
	hpenHigh = CreatePen(PS_SOLID, 1, clrHigh);

	hpenOld = (HPEN)SelectObject(hdc, hpenHigh);

	MoveToEx(hdc, rc.left, rc.bottom, NULL);
	LineTo(hdc, rc.left, rc.top);
	LineTo(hdc, rc.right, rc.top);

	SelectObject(hdc, hpenLow);
	LineTo(hdc, rc.right, rc.bottom);
	LineTo(hdc, rc.left, rc.bottom);

	SelectObject(hdc, hpenOld);

	DeleteObject(hpenLow);
	DeleteObject(hpenHigh);
}
*/

void Draw3DFrame(CRawGraph *pGraph, CRect &rc, MLColor &clrHigh, MLColor &clrLow)
{
	CRawPen	penLow, penHigh;

	penLow.CreateSolidPen(1, clrLow);
	penHigh.CreateSolidPen(1, clrHigh);

	pGraph->SetPen(penHigh);
	pGraph->Line(rc.left, rc.bottom, rc.left, rc.top);
	pGraph->Line(rc.left, rc.top, rc.right, rc.top);

	pGraph->SetPen(penLow);
	pGraph->Line(rc.right, rc.top, rc.right, rc.bottom);
	pGraph->Line(rc.right, rc.bottom, rc.left, rc.bottom);
}

void DrawSeperator(CRawGraph *pGraph, CRect &rc, MLColor &clrHigh, MLColor &clrLow)
{
	CRawPen	penLow, penHigh;
	int		x;

	penLow.CreateSolidPen(1, clrLow);
	penHigh.CreateSolidPen(1, clrHigh);

	x = (rc.right + rc.left) / 2;

	pGraph->SetPen(penLow);
	pGraph->Line(x, rc.bottom, x, rc.top);

	x++;
	pGraph->SetPen(penHigh);
	pGraph->Line(x, rc.top, x, rc.bottom);
}

/*
void DrawSeperator(CRawGraph *pGraph, CRect &rc, MLColor &clrHigh, MLColor &clrLow)
{
	HDC		hdc = pGraph->GetHandle();
	HPEN	hpenLow, hpenHigh, hpenOld;
	int		x;

	hpenLow = CreatePen(PS_SOLID, 1, clrLow);
	hpenHigh = CreatePen(PS_SOLID, 1, clrHigh);

	hpenOld = (HPEN)SelectObject(hdc, hpenLow);

	x = (rc.right + rc.left) / 2;

	MoveToEx(hdc, x, rc.bottom, NULL);
	LineTo(hdc, x, rc.top);

	x++;
	SelectObject(hdc, hpenHigh);
	MoveToEx(hdc, x, rc.top, NULL);
	LineTo(hdc, x, rc.bottom);

	SelectObject(hdc, hpenOld);

	DeleteObject(hpenLow);
	DeleteObject(hpenHigh);
}*/

void CSkinToolbar::Draw(CRawGraph *pGraph)
{
	BT_DRAW_STATE		btDrawState = ROW_NORMAL;

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraph);
	pGraph->SetClipBoundBox(m_rcObj);

	int		x, y;
	int		nHeight = m_rcObj.Height();

	if (m_bDrawBtText && m_font.IsGood())
	{
		pGraph->SetFont(m_font.GetFont());
		pGraph->SetTextColor(m_font.GetTextColor(m_bEnable));
	}

	if (m_imageBg.IsValid())
		m_imageBg.Blt(pGraph, m_rcObj.left, m_rcObj.top, m_nMarginX, m_rcObj.Height(), 0, 0);

	if (m_nBlankX != INVALID_POS)
		m_image.Blt(pGraph, m_rcObj.left, m_rcObj.top, m_nMarginX, m_nImageHeight, m_nBlankX, 0);

	x = m_rcObj.left + m_nMarginX;
	y = m_rcObj.top;

	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		int			nBtImageLeft;

		if (bt.bEnabled || bt.nLeftOfDiabled == -1)
		{
			ASSERT(bt.nCurStatus <= bt.nStatusMax);
			nBtImageLeft = bt.nLeftPositon[bt.nCurStatus];

			if (i == m_nCurButton)
			{
				if (m_bLBtDown)
					btDrawState = ROW_DOWN;
				else if (m_bHover)
					btDrawState = ROW_HOVER;
				else
					btDrawState = ROW_NORMAL;
			}
			else
				btDrawState = ROW_NORMAL;
		}
		else
		{
			nBtImageLeft = bt.nLeftOfDiabled;
			btDrawState = ROW_NORMAL;
		}

		if (bt.bSeperator)
			btDrawState = ROW_NORMAL;

		if (i == m_nCurButton && m_dwBeginFadeinTime != 0)
			DrawCurButtonFadein(pGraph, i, btDrawState, x, y, nHeight, nBtImageLeft);
		else
			DrawButton(pGraph, i, btDrawState, x, y, nHeight, nBtImageLeft);

		x += bt.nWidth + m_nBtSpacesCX;
		if (x >= m_rcObj.right)
			break;

		// Draw button spaces?
		if (m_nBtSpacesCX > 0 && m_nBlankX != INVALID_POS)
			m_image.Blt(pGraph, x - m_nBtSpacesCX, y, m_nBtSpacesCX, m_nImageHeight, m_nBlankX, 0);
	}

	if (x < m_rcObj.right && m_nBlankX != INVALID_POS)
	{
		m_image.XTileBlt(pGraph, x, y, m_rcObj.right - x, nHeight, m_nBlankX, 0, m_nBlankCX, nHeight);
		if (m_imageBg.IsValid())
			m_imageBg.XTileBlt(pGraph, x, m_rcObj.top, m_rcObj.right - x, m_rcObj.Height());
	}
}

void CSkinToolbar::SetVisible(bool bVisible, bool bRedraw)
{
	CUIObject::SetVisible(bVisible, bRedraw);

	if (bVisible)
		AddTooltip();
	else
		DelTooltip();
}

BOOL CSkinToolbar::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
	{
		return TRUE;
	}

	if (IsPropertyName(szProperty, _T("DrawBtText")))
		m_bDrawBtText = IsTRUE(szValue);
	else if (m_font.SetProperty(szProperty, szValue))
		return TRUE;
	else if (IsPropertyName(szProperty, _T("BgImage")))
	{
		m_strImageBgFile = szValue;
		m_imageBg.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imageBg.GetOrginalSize(m_imageBg.m_cx, m_imageBg.m_cy);
	}
	else if (IsPropertyName(szProperty, _T("BgImageChecked")))
	{
		m_strImageBgCheckedFile = szValue;
		m_imageBgChecked.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imageBgChecked.GetOrginalSize(m_imageBgChecked.m_cx, m_imageBgChecked.m_cy);
	}
	else if (_tcsicmp(szProperty, SZ_PN_IMAGE) == 0)
	{
		m_strImageFile = szValue;
		m_image.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_image.GetOrginalSize(m_image.m_cx, m_image.m_cy);
	}
	else if (IsPropertyName(szProperty, _T("ImageHeight")))
		m_nImageHeight = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("units_x")) == 0)
		m_nUnitsOfX = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("blank_x")) == 0)
		m_nBlankX = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("blank_cx")) == 0)
		m_nBlankCX = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("seperator_x")) == 0)
		m_nSeperatorX = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("seperator_cx")) == 0)
		m_nSeperatorCX = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("enablehover")) == 0)
		m_bEnableHover = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("MarginX")) == 0)
		m_nMarginX = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("MarginY")) == 0)
		m_nMarginY = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("ButtonSpacesCX")) == 0)
		m_nBtSpacesCX = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("FullStatusImage")) == 0)
		m_bFullStatusImage = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("Fadein")))
	{
		m_bFadein = IsTRUE(szValue);
	}
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinToolbar::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropImageFile(SZ_PN_IMAGE, m_strImageFile.c_str());

	listProperties.AddPropInt( _T("units_x"), m_nUnitsOfX);
	listProperties.AddPropInt( _T("blank_x"), m_nBlankX);
	listProperties.AddPropInt( _T("blank_cx"), m_nBlankCX);
	listProperties.AddPropInt( _T("seperator_x"), m_nSeperatorX);
	listProperties.AddPropInt( _T("seperator_cx"), m_nSeperatorCX);
	listProperties.AddPropBoolStr(_T("enablehover"), m_bEnableHover);
	listProperties.AddPropInt( _T("MarginX"), m_nMarginX);
	listProperties.AddPropInt( _T("MarginY"), m_nMarginY);
}
#endif // _SKIN_EDITOR_

int CSkinToolbar::FromXML(SXNode *pXmlNode)
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
			bt.nWidthImage = bt.nWidth = m_nUnitsOfX;
			bt.FromXML(this, pChild);
			m_vButtons.push_back(bt);
		}
		else if (_tcsicmp(pChild->strName.c_str(), _T("seperator")) == 0)
		{
			Button	bt;
			bt.bSeperator = true;
			bt.nLeftPositon[0] = bt.nLeftOfDiabled = (m_nSeperatorX == -1 ? m_nBlankX : m_nSeperatorX);
			bt.nWidthImage = bt.nWidth = (m_nSeperatorX == -1 ? m_nBlankCX : m_nSeperatorCX);
			m_vButtons.push_back(bt);
		}
	}

	return ERR_OK;
}

#ifdef _SKIN_EDITOR_
void CSkinToolbar::OnToXMLChild(CXMLWriter &xmlStream)
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button	&bt = m_vButtons[i];

		bt.ToXML(this, xmlStream);
	}
}
#endif // _SKIN_EDITOR_

void CSkinToolbar::InvalidateButton(int nButton)
{
	Invalidate();
}

int CSkinToolbar::GetCurButton(POINT pt)
{
	if (pt.y <= m_rcObj.top || pt.y >= m_rcObj.bottom)
		return -1;
	if (pt.x <= m_rcObj.left + m_nMarginX || pt.x >= m_rcObj.right)
		return -1;

	pt.x -= m_rcObj.left + m_nMarginX;
	pt.y -= m_rcObj.top;

	int		x = 0;

	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		x += bt.nWidth;
		if (pt.x < x)
		{
			if (bt.bSeperator)
				return -1;
			return i;
		}
		x += m_nBtSpacesCX;
		if (pt.x < x)
			return -1;
	}

	return -1;
}

void CSkinToolbar::GroupButtonUncheckOld(int nGroup, int nCurButton)
{
	ASSERT(nGroup != -1);
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		if (bt.nGroup == nGroup && i != nCurButton)
		{
			bt.nCurStatus = 0;
		}
	}
}

bool CSkinToolbar::IsCheck(int nCmdID)
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		if (bt.nID == nCmdID)
		{
			return bt.nCurStatus == Button::STATUS_CHECKED;
		}
	}

	return false;
}

void CSkinToolbar::SetCheck(int nCmdID, bool bCheck, bool bRedraw)
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		if (bt.nID == nCmdID)
		{
			if (bCheck)
			{
				if (bt.nStatusMax >= Button::STATUS_CHECKED)
				{
					bt.nCurStatus = Button::STATUS_CHECKED;
					if (bt.nGroup != -1)
						GroupButtonUncheckOld(bt.nGroup, i);
					if (bRedraw)
						Invalidate();
				}
			}
			else
				bt.nCurStatus = Button::STATUS_NORMAL;
			return;
		}
	}
}

void CSkinToolbar::SetBtStatus(int nCmdID, int nStatus, bool bRedraw)
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		if (bt.nID == nCmdID)
		{
			if (nStatus <= bt.nStatusMax)
			{
				bt.nCurStatus = nStatus;
				if (bRedraw)
					Invalidate();
			}
			return;
		}
	}
}

int CSkinToolbar::GetBtStatus(int nCmdID)
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		if (bt.nID == nCmdID)
			return bt.nCurStatus;
	}

	return 0;
}

bool CSkinToolbar::IsBtEnabled(int nCmdID)
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		if (bt.nID == nCmdID)
		{
			return bt.bEnabled;
		}
	}

	return false;
}

void CSkinToolbar::EnableBt(int nCmdID, bool bEnable)
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		if (bt.nID == nCmdID)
		{
			bt.bEnabled = bEnable;
			return;
		}
	}
}

bool CSkinToolbar::IsButtonExist(int nCmdID)
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		if (bt.nID == nCmdID)
		{
			return true;
		}
	}

	return false;
}

void CSkinToolbar::CalculateBtWidth(CRawGraph *pGraph, Button &bt)
{
	if (m_bDrawBtText && bt.strText.size())
	{
		SIZE	size;

		const int TEXT_SEPERATOR_WIDTH = 20;
		if (pGraph->GetTextExtentPoint32(bt.strText.c_str(), bt.strText.size(), &size))
		{
			if (size.cx + TEXT_SEPERATOR_WIDTH > bt.nWidthImage)
				bt.nWidth = (int16)size.cx + TEXT_SEPERATOR_WIDTH;
			else
				bt.nWidthImage = bt.nWidth;
		}
	}
}

void CSkinToolbar::DrawButton(CRawGraph *pGraph, int nButton, BT_DRAW_STATE btDrawState, int x, int y, int nHeight, int nBtImageLeft)
{
	Button &bt = m_vButtons[nButton];

	CalculateBtWidth(pGraph, bt);

	int		xDrawImage = x + (bt.nWidth - bt.nWidthImage) / 2;

	CSFImage	*pImage = NULL;
	if (bt.nCurStatus == Button::STATUS_CHECKED)
		pImage = &m_imageBgChecked;
	else
		pImage = &m_imageBg;

	const int BUTTON_BG_BORDER = 5;
	if (pImage->IsValid())
		pImage->XScaleBlt(pGraph, x, y, bt.nWidth, m_rcObj.Height(),
			BUTTON_BG_BORDER, pImage->m_cx - BUTTON_BG_BORDER, true);

	// Draw Button text
	if (m_bDrawBtText && bt.strText.size())
	{
		CRect	rcText(x, y + m_nImageHeight, x + bt.nWidth, y + nHeight);

		pGraph->DrawText(bt.strText.c_str(), bt.strText.size(), &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	if (m_bFullStatusImage)
	{
		// 
		int		nBtImageTop;

		nBtImageTop = m_nImageHeight * btDrawState;

		m_image.Blt(pGraph, xDrawImage, y, bt.nWidthImage, m_nImageHeight, 
			nBtImageLeft, nBtImageTop);

		return;
	}

	MLColor				clrHigh(RGB(255, 255, 255)), clrLow(RGB(128, 128,128));

	if (btDrawState == ROW_DOWN)
		m_image.Blt(pGraph, xDrawImage + 1, y + 1, bt.nWidthImage - 1, m_nImageHeight - 1, 
		nBtImageLeft, 0);
	else
		m_image.Blt(pGraph, xDrawImage, y, bt.nWidthImage, m_nImageHeight, 
		nBtImageLeft, 0);

	if (btDrawState != ROW_NORMAL)
	{
		CRect	rc;
		rc.left = x;
		rc.top = y + m_nMarginY;
		rc.right = rc.left + bt.nWidth - 1;
		rc.bottom = y + nHeight - m_nMarginY - 1;
		if (btDrawState == ROW_HOVER)
			Draw3DFrame(pGraph, rc, clrHigh, clrLow);
		else if (btDrawState == ROW_DOWN)
			Draw3DFrame(pGraph, rc, clrLow, clrHigh);
	}

	if (bt.bSeperator)
	{
		CRect	rc;
		rc.left = x;
		rc.top = y + m_nMarginY;
		rc.right = rc.left + bt.nWidth - 1;
		rc.bottom = y + nHeight - m_nMarginY;
		DrawSeperator(pGraph, rc, clrHigh, clrLow);
	}
}

void CSkinToolbar::DrawCurButtonFadein(CRawGraph *pGraph, int nButton, BT_DRAW_STATE btDrawState, int x, int y, int nHeight, int nBtImageLeft)
{
	ASSERT(m_bFullStatusImage);

	DWORD		dwTimeCur = GetTickCount();

	if (dwTimeCur - m_dwBeginFadeinTime >= FADE_BUTTON_TIME_OUT)
	{
		m_dwBeginFadeinTime = 0;
		m_pSkin->UnregisterTimerObject(this);
		DrawButton(pGraph, nButton, btDrawState, x, y, nHeight, nBtImageLeft);
		return;
	}
	int		nAlpha = (dwTimeCur - m_dwBeginFadeinTime) * 255 / FADE_BUTTON_TIME_OUT;
	int		nAlphaOld = pGraph->GetOpacityPainting();

	pGraph->SetOpacityPainting((255 - nAlpha) * nAlphaOld / 255);

	DrawButton(pGraph, nButton, m_nLastImageDrawState, x, y, nHeight, nBtImageLeft);

	pGraph->SetOpacityPainting(nAlpha * nAlphaOld / 255);

	DrawButton(pGraph, nButton, btDrawState, x, y, nHeight, nBtImageLeft);

	pGraph->SetOpacityPainting(nAlphaOld);
}

void CSkinToolbar::AddTooltip()
{
	CRect	rcBt;
	int		i;

	// Add tooltip of all the buttons.
	rcBt.left = m_rcObj.left + m_nMarginX;
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
			m_pSkin->AddTool(_TL(bt.strTooltip.c_str()), &rcBt, bt.nAutoUniID);
		}

		rcBt.left += bt.nWidth + m_nBtSpacesCX;
		if (rcBt.left >= m_rcObj.right)
			break;
	}
}

void CSkinToolbar::DelTooltip()
{
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		Button		&bt = m_vButtons[i];
		if (bt.strTooltip.size())
		{
			m_pSkin->DelTool(bt.nAutoUniID);
		}
	}
}
