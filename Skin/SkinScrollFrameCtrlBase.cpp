#include "StdAfx.h"
#include "Skin.h"
#include "SkinScrollFrameCtrlBase.h"
#include "SkinFrameCtrl.h"
#include "SkinListCtrl.h"

CSkinScrollFrameCtrlBase::CSkinScrollFrameCtrlBase()
{
	m_bHorzScrollBar = m_bVertScrollBar = false;
	m_bEnableBorder = false;

	m_pObjVertScrollBar = NULL;
	m_pObjHorzScrollBar = NULL;
	m_nHorzScrollBarId = m_nVertScrollBarId = SKIN_UIOBJ_ID_UNDEFINE;
	m_pVertScrollBar = NULL;
	m_pHorzScrollBar = NULL;
	m_pObjFrame = NULL;

	m_nLeftMargin = m_nTopMargin = m_nRightMargin = m_nBottomMargin = 0;
	m_rcCtrl.SetRect(0, 0, 0, 0);
}

CSkinScrollFrameCtrlBase::~CSkinScrollFrameCtrlBase()
{
	if (m_nVertScrollBarId == SKIN_UIOBJ_ID_UNDEFINE && m_pObjVertScrollBar)
	{
		m_pSkin->RemoveUIObject(m_pObjVertScrollBar, true);
		m_pObjVertScrollBar = NULL;
	}
	if (m_nHorzScrollBarId == SKIN_UIOBJ_ID_UNDEFINE && m_pObjHorzScrollBar)
	{
		m_pSkin->RemoveUIObject(m_pObjHorzScrollBar, true);
		m_pObjHorzScrollBar = NULL;
	}
}

void CSkinScrollFrameCtrlBase::OnCreate()
{
	CSkinContainer::OnCreate();

	if (m_bEnableBorder)
		CreateFrameCtrl();

	if (m_bVertScrollBar)
		CreateVertScrollbar();
	if (m_bHorzScrollBar)
		CreateHorzScrollbar();
}

void CSkinScrollFrameCtrlBase::OnSize(int cx, int cy)
{
	if (m_pObjFrame)
	{
		CSkinFrameCtrl *pFrameCtrl = (CSkinFrameCtrl *)m_pObjFrame;
		int nBorderThick = pFrameCtrl->GetBorderThick();
		m_rcObj.OffsetRect(-nBorderThick, -nBorderThick);
	}

	CSkinContainer::OnSize(cx, cy);

	m_rcCtrl.left = m_rcObj.left + m_nLeftMargin;
	m_rcCtrl.top = m_rcObj.top + m_nTopMargin;
	m_rcCtrl.right = m_rcObj.right - m_nRightMargin;
	m_rcCtrl.bottom = m_rcObj.bottom - m_nBottomMargin;
}

void CSkinScrollFrameCtrlBase::OnKillFocus()
{
	if (m_pObjFrame)
		m_pSkin->InvalidateRect();
}

void CSkinScrollFrameCtrlBase::OnSetFocus()
{
	if (m_pObjFrame)
		m_pSkin->InvalidateRect();
}

void CSkinScrollFrameCtrlBase::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	if (IsFlagSet(nMkeys, MK_SHIFT))
	{
		if (m_pObjHorzScrollBar)
			m_pObjHorzScrollBar->OnMouseWheel(nWheelDistance, nMkeys, pt);
	}
	else if (m_pObjVertScrollBar)
		m_pObjVertScrollBar->OnMouseWheel(nWheelDistance, nMkeys, pt);
}

BOOL CSkinScrollFrameCtrlBase::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinContainer::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("EnableBorder")))
		m_bEnableBorder = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("EnableHorzScrollBar")))
		m_bHorzScrollBar = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("EnableVertScrollBar")))
		m_bVertScrollBar = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("ScrollbarId")))
	{
		V_Str	vStr;
		StrSplit(szValue, ',', vStr);
		TrimStr(vStr);
		if (vStr.size() > 0)
			m_nVertScrollBarId = m_pSkin->GetSkinFactory()->GetIDByName(vStr[0].c_str());
		if (vStr.size() > 1)
			m_nHorzScrollBarId = m_pSkin->GetSkinFactory()->GetIDByName(vStr[1].c_str());
	}
	else
		return FALSE;

	return TRUE;
}


void CSkinScrollFrameCtrlBase::CreateFrameCtrl()
{
	m_pObjFrame = m_pSkin->GetSkinFactory()->CreateDynamicCtrl(this,
		IsKindOf(CSkinListCtrl::ClassName()) ? _T("ListCtrlFrame") : _T("EditCtrlFrame"),
		ID_UNDEFINE, _T("0"), _T("0"), _T("w"), _T("h"));

	ASSERT(m_pObjFrame);
	if (m_pObjFrame)
	{
		CSkinFrameCtrl *pFrameCtrl = (CSkinFrameCtrl *)m_pObjFrame;
		int nBorderThick = pFrameCtrl->GetBorderThick();
// 		m_formLeft.Increase(-nBorderThick);
// 		m_formTop.Increase(-nBorderThick);
// 		m_formWidth.Increase(nBorderThick * 2);
// 		m_formHeight.Increase(nBorderThick * 2);

		pFrameCtrl->SetFocusIndicator(this);

		m_nLeftMargin = m_nTopMargin = m_nRightMargin = m_nBottomMargin = nBorderThick;
	}
}

void CSkinScrollFrameCtrlBase::CreateVertScrollbar()
{
	if (m_nVertScrollBarId != SKIN_UIOBJ_ID_UNDEFINE)
	{
		m_pObjVertScrollBar = m_pSkin->GetUIObjectById(m_nVertScrollBarId, CSkinVScrollBar::ClassName());
		if (m_pObjVertScrollBar)
		{
			m_pVertScrollBar = (CSkinVScrollBar*)m_pObjVertScrollBar;
			m_pVertScrollBar->SetScrollNotify(this);
			return;
		}
		else
			ERRLOG0(_T("Can NOT get the vertical scroll bar of list control."));
	}

	// Create Scrollbar
	m_pObjVertScrollBar = m_pSkin->GetSkinFactory()->CreateDynamicCtrl(this, CSkinVScrollBar::ClassName(), SKIN_UIOBJ_ID_UNDEFINE, NULL, NULL, NULL, NULL);
	ASSERT(m_pObjVertScrollBar);
	if (!m_pObjVertScrollBar)
		return;

	int nWidth = 14;
	m_pObjVertScrollBar->m_formWidth.CalCualteValue(nWidth);
	m_nRightMargin += nWidth;

	m_pObjVertScrollBar->m_formLeft.SetFormula(CStrPrintf(_T("w-%d"), m_nRightMargin).c_str());
	m_pObjVertScrollBar->m_formTop.SetFormula(m_nTopMargin);
	m_pObjVertScrollBar->m_formHeight.SetFormula(CStrPrintf(_T("h-%d"), m_nTopMargin + m_nBottomMargin).c_str());
	if (m_pObjHorzScrollBar)
	{
		// Horizontal scroll bar is created first, decrease its width now.
		m_pObjHorzScrollBar->m_formWidth.Increase(-nWidth);
	}

	m_pContainer->RecalculateUIObjSizePos(m_pObjVertScrollBar);

	m_pVertScrollBar = (CSkinVScrollBar*)m_pObjVertScrollBar;
	m_pVertScrollBar->SetScrollNotify(this);
}

void CSkinScrollFrameCtrlBase::CreateHorzScrollbar()
{
	if (m_nHorzScrollBarId != SKIN_UIOBJ_ID_UNDEFINE)
	{
		m_pObjHorzScrollBar = m_pSkin->GetUIObjectById(m_nHorzScrollBarId, CSkinHScrollBar::ClassName());
		if (m_pObjHorzScrollBar)
		{
			m_pHorzScrollBar = (CSkinHScrollBar*)m_pObjHorzScrollBar;
			m_pHorzScrollBar->SetScrollNotify(this);
			return;
		}
		else
			ERRLOG0(_T("Can NOT get the Horzical scroll bar of list control."));
	}

	// Create Scrollbar
	m_pObjHorzScrollBar = m_pSkin->GetSkinFactory()->CreateDynamicCtrl(this, CSkinHScrollBar::ClassName(), SKIN_UIOBJ_ID_UNDEFINE, NULL, NULL, NULL, NULL);
	ASSERT(m_pObjHorzScrollBar);
	if (!m_pObjHorzScrollBar)
		return;

	int nHeight = 14;
	m_pObjHorzScrollBar->m_formHeight.CalCualteValue(nHeight);
	m_nBottomMargin += nHeight;

	m_pObjHorzScrollBar->m_formLeft.SetFormula(m_nLeftMargin);
	m_pObjHorzScrollBar->m_formTop.SetFormula(CStrPrintf(_T("h-%d"), m_nBottomMargin).c_str());
	m_pObjHorzScrollBar->m_formWidth.SetFormula(CStrPrintf(_T("w-%d"), m_nLeftMargin + m_nRightMargin).c_str());
	if (m_pObjVertScrollBar)
	{
		// Vertical scroll bar is created first, decrease its height now.
		m_pObjVertScrollBar->m_formHeight.Increase(-nHeight);
	}

	m_pContainer->RecalculateUIObjSizePos(m_pObjHorzScrollBar);

	m_pHorzScrollBar = (CSkinVScrollBar*)m_pObjHorzScrollBar;
	m_pHorzScrollBar->SetScrollNotify(this);
}
