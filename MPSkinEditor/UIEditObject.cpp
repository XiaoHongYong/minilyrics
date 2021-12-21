// UIEditObject.cpp: implementation of the CUIEditObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "UIEditObject.h"
#include "SESkinWnd.h"
#include "SEPropertyListCtrl.h"

#define SIZE_MARGIN		0

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CUIEditObject, _T("SkinEditObject"))

CUIEditObject::CUIEditObject()
{
	// m_nMsgMask = UO_MSG_MASK_KEY | UO_MSG_MASK_MOUSEWHEEL | UO_MSG_MASK_RBUTTON;
	m_pUIObject = NULL;
	m_bLBtDown = FALSE;
	static BYTE	r = 0, g = 0, b = 0;
	r += 75;
	g += 50;
	b += 25;
	m_hbrBk = CreateSolidBrush(RGB(r, g, b));
	m_nMsgWant = UO_MSG_WANT_ALL;
	m_pNotify = NULL;
}

CUIEditObject::~CUIEditObject()
{
	if (m_pUIObject)
		delete m_pUIObject;
	if (m_hbrBk)
		DeleteObject(m_hbrBk);
}

void CUIEditObject::OnCreate()
{
	CUIObject::OnCreate();

	m_pUIObject->m_pContainer = m_pContainer;
	m_pUIObject->OnCreate();
}

bool CUIEditObject::OnLButtonDown(UINT nFlags, CPoint point)
{
	assert(m_pSkin);

	m_bLBtDown = TRUE;
	m_ptDragOld = point;

	// 捕捉鼠标输入
	m_pSkin->SetCaptureMouse(this);

//	Invalidate();
	return true;
}

bool CUIEditObject::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bLBtDown = FALSE;

	// 释放鼠标输入
	m_pSkin->ReleaseCaptureMouse(this);

	//Invalidate();
	
	//m_pSkin->OnCustomCommand(m_nId);
	return true;
}

bool CUIEditObject::OnMouseMove(UINT nFlags, CPoint point)
{
	assert(m_pSkin);

	if (m_bLBtDown)
	{
		//
		// 移动此控件
		CString		str;
		// int			nNewPos;
		int			nOffset;

		nOffset = point.x - m_ptDragOld.x;
		// nNewPos = m_rcObj.left + point.x - m_ptDragOld.x + SIZE_MARGIN;
		if (nOffset != 0)
		{
			if (nOffset > 0)
				str.Format(_T("%s + %d"), m_formLeft.GetFormula(), nOffset);
			else
				str.Format(_T("%s%d"), m_formLeft.GetFormula(), nOffset);
			SetProperty(_T("left"), str);

			if (m_pNotify)
				m_pNotify->OnFocusUIObjPropertyChanged(_T("left"), m_formLeft.GetFormula());
		}

		nOffset = point.y - m_ptDragOld.y;
		// nNewPos = m_rcObj.top + point.y - m_ptDragOld.y + SIZE_MARGIN;
		if (nOffset != 0)
		{
			if (nOffset > 0)
				str.Format(_T("%s + %d"), m_formTop.GetFormula(), nOffset);
			else
				str.Format(_T("%s%d"), m_formTop.GetFormula(), nOffset);
			SetProperty(_T("top"), str);
			if (m_pNotify)
				m_pNotify->OnFocusUIObjPropertyChanged(_T("top"), m_formTop.GetFormula());
		}

		m_pSkin->RecalculateSizePos();

		m_pSkin->InvalidateRect(NULL, TRUE);

		m_ptDragOld = point;
	}
	return true;
}

void CUIEditObject::Draw(CRawGraph *pGraph)
{
	HDC		hdc = pGraph->GetHandle();

	CRect	rc;
	rc = m_rcObj;
	rc.left += SIZE_MARGIN;
	rc.top += SIZE_MARGIN;
	rc.right -= SIZE_MARGIN;
	rc.bottom -= SIZE_MARGIN;
	FillRect(hdc, &rc, m_hbrBk);

	m_pUIObject->Draw(pGraph);

	if (IsOnFocus())
	{
	//	BitBlt(hdc, m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), NULL, 0, 0, WHITENESS);
		HPEN	hPen, hPenOld;
		COLORREF	clrOld;
		int			nBkModeOld;

		hPen = CreatePen(PS_DOT, 1, RGB(0x31, 0x69, 0xc6));
		hPenOld = (HPEN)SelectObject(hdc, hPen);
		clrOld = SetBkColor(hdc, RGB(255, 255, 255));
		nBkModeOld = SetBkMode(hdc, OPAQUE);

		// 画边缘
		// for (int i = 0; i < SIZE_MARGIN; i+=2)
		int	i = 0;
		{
			MoveToEx(hdc, m_rcObj.left + i, m_rcObj.top + i, NULL);
			LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.top + i);

			MoveToEx(hdc, m_rcObj.left + i, m_rcObj.bottom - i - 1, NULL);
			LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.bottom - i - 1);

			MoveToEx(hdc, m_rcObj.left + i, m_rcObj.top + i, NULL);
			LineTo(hdc, m_rcObj.left + i, m_rcObj.bottom - i - 1);

			MoveToEx(hdc, m_rcObj.right - i - 1, m_rcObj.top + i, NULL);
			LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.bottom - i - 1);
		}

		SetBkMode(hdc, nBkModeOld);
		SetBkColor(hdc, clrOld);
		SelectObject(hdc, hPenOld);

		DeleteObject(hPen);
	}
}

BOOL CUIEditObject::ReCalculatePos(FORMULA_VAR vars[])
{
	if (!CUIObject::ReCalculatePos(vars))
		return FALSE;

	m_pUIObject->ReCalculatePos(vars);

	m_rcObj.left -= SIZE_MARGIN;
	m_rcObj.top -= SIZE_MARGIN;
	m_rcObj.right += SIZE_MARGIN;
	m_rcObj.bottom += SIZE_MARGIN;

	return TRUE;
}


void CUIEditObject::EnumProperties(CUIObjProperties &listProperties)
{
	m_pUIObject->EnumProperties(listProperties);
}

BOOL CUIEditObject::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (_tcsicmp(szProperty, _T("visible")) != 0)
		CUIObject::SetProperty(szProperty, szValue);

	if (_tcsicmp(szProperty, SZ_PN_NAME) == 0)
	{
		m_pUIObject->SetProperty(szProperty, szValue);

		if (m_pNotify)
			m_pNotify->OnUIObjNameChanged(this);

		return true;
	}

	return m_pUIObject->SetProperty(szProperty, szValue);
}

/*
int CUIEditObject::FromXML(SXNode *pXmlNode)
{
	for (UINT i = 0; i < pXmlNode->vProperties.size(); i += 2)
	{
		SetProperty(pXmlNode->vProperties[i], pXmlNode->vProperties[i + 1]);
	}

	return ERR_OK
	// return m_pUIObject->FromXML(pXmlNode);
}*/

int CUIEditObject::FromXML(CSkinContainer *pContainer, SXNode *pXmlNode)
{
	CUIObject::FromXML(pContainer, pXmlNode);
	return m_pUIObject->FromXML(pContainer, pXmlNode);
}

void CUIEditObject::ToXML(CXMLWriter &xmlStream)
{
	m_pUIObject->ToXML(xmlStream);
}

void CUIEditObject::OnKillFocus()
{
	m_pSkin->InvalidateRect(NULL, TRUE);
}

void CUIEditObject::OnSetFocus()
{
	Invalidate();
}
