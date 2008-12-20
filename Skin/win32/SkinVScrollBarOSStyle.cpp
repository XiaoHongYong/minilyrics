// SkinVScrollBarOSStyle.cpp: implementation of the CSkinScrollBarOSStyleBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLSkin.h"
#include "SkinVScrollBarOSStyle.h"

//////////////////////////////////////////////////////////////////////////

CSkinScrollBarOSStyleBase::CSkinScrollBarOSStyleBase()
{
	m_pScrollNotify = NULL;
	m_formWidth.SetFormula(CStrPrintf(_T("%d"), GetSystemMetrics(SM_CXVSCROLL)).c_str());
}

CSkinScrollBarOSStyleBase::~CSkinScrollBarOSStyleBase()
{
	m_scrollbar.Destroy();
}

void CSkinScrollBarOSStyleBase::OnSize(int cx, int cy)
{
	CUIObject::OnSize(cx, cy);
	m_scrollbar.MoveWindow(m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), TRUE);
}

void CSkinScrollBarOSStyleBase::OnSetFocus()
{
	m_scrollbar.SetFocus();
}

void CSkinScrollBarOSStyleBase::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	if (nMkeys == 0)
	{
		if (nWheelDistance > 0)
			m_scrollbar.HandleScrollCode(SB_LINEUP, 0);
		else
			m_scrollbar.HandleScrollCode(SB_LINEDOWN, 0);
	}
}

void CSkinScrollBarOSStyleBase::OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	if (m_pScrollNotify)
		m_pScrollNotify->OnVScroll(nSBCode, GetScrollPos(), this);
	else
		m_pSkin->OnVScroll(nSBCode, GetScrollPos(), this);
}

void CSkinScrollBarOSStyleBase::OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	if (m_pScrollNotify)
		m_pScrollNotify->OnVScroll(nSBCode, GetScrollPos(), this);
	else
		m_pSkin->OnVScroll(nSBCode, GetScrollPos(), this);
}


UIOBJECT_CLASS_NAME_IMP(CSkinVScrollBarOSStyle, _T("VScrollBarOSStyle"))

void CSkinVScrollBarOSStyle::OnCreate()
{
	m_scrollbar.Create(m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), true, m_pSkin, m_nId);
	m_scrollbar.SetScrollNotify(this);

	m_scrollbar.ShowWindow(SW_SHOW);
}

UIOBJECT_CLASS_NAME_IMP(CSkinHScrollBarOSStyle, _T("HScrollBarOSStyle"))

void CSkinHScrollBarOSStyle::OnCreate()
{
	m_scrollbar.Create(m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), false, m_pSkin, m_nId);
	m_scrollbar.SetScrollNotify(this);

	m_scrollbar.ShowWindow(SW_SHOW);
}
