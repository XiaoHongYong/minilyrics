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
	m_nHorzScrollBarId = m_nVertScrollBarId = UID_INVALID;
	m_pVertScrollBar = NULL;
	m_pHorzScrollBar = NULL;
	m_pObjFrame = NULL;

	m_nWidthVertScrollBar = m_nHeightHorzScrollBar = 0;
}

CSkinScrollFrameCtrlBase::~CSkinScrollFrameCtrlBase()
{
	if (m_nVertScrollBarId == UID_INVALID && m_pObjVertScrollBar)
	{
		m_pSkin->RemoveUIObject(m_pObjVertScrollBar, true);
		m_pObjVertScrollBar = NULL;
	}
	if (m_nHorzScrollBarId == UID_INVALID && m_pObjHorzScrollBar)
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

void CSkinScrollFrameCtrlBase::OnSize()
{
	CSkinContainer::OnSize();

	m_rcContent = m_rcObj;
	m_rcContent.left += m_rcPadding.left;
	m_rcContent.top += m_rcPadding.top;
	m_rcContent.right -= m_rcPadding.right;
	m_rcContent.bottom -= m_rcPadding.bottom;

	m_rcContent.right -= m_nWidthVertScrollBar;
	m_rcContent.bottom -= m_nHeightHorzScrollBar;
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

		m_rcPadding.left = m_rcPadding.top = m_rcPadding.right = m_rcPadding.bottom = nBorderThick;
	}
}

void CSkinScrollFrameCtrlBase::CreateVertScrollbar()
{
	if (m_nVertScrollBarId != UID_INVALID)
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
	m_pObjVertScrollBar = m_pSkin->GetSkinFactory()->CreateDynamicCtrl(this, CSkinVScrollBar::ClassName(), UID_INVALID, NULL, NULL, NULL, NULL);
	ASSERT(m_pObjVertScrollBar);
	if (!m_pObjVertScrollBar)
		return;

	int nWidth = 14;
	m_pObjVertScrollBar->m_formWidth.CalCualteValue(nWidth);
	m_nWidthVertScrollBar = nWidth;

	m_pObjVertScrollBar->m_formLeft.SetFormula(CStrPrintf(_T("w-%d"), m_nWidthVertScrollBar + m_rcPadding.right).c_str());
	m_pObjVertScrollBar->m_formTop.SetFormula(m_rcPadding.top);
	m_pObjVertScrollBar->m_formHeight.SetFormula(CStrPrintf(_T("h-%d"), m_rcPadding.top + m_rcPadding.bottom + m_nHeightHorzScrollBar).c_str());
	if (m_pObjHorzScrollBar)
	{
		// Horizontal scroll bar is created first, decrease its width now.
		m_pObjHorzScrollBar->m_formWidth.Increase(-nWidth);
	}

	m_pContainer->RecalculateUIObjSizePos(m_pObjVertScrollBar);

	m_pVertScrollBar = (CSkinVScrollBar*)m_pObjVertScrollBar;
	m_pVertScrollBar->SetScrollNotify(this);

	m_rcContent.right -= m_nWidthVertScrollBar;
}

void CSkinScrollFrameCtrlBase::CreateHorzScrollbar()
{
	if (m_nHorzScrollBarId != UID_INVALID)
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
	m_pObjHorzScrollBar = m_pSkin->GetSkinFactory()->CreateDynamicCtrl(this, CSkinHScrollBar::ClassName(), UID_INVALID, NULL, NULL, NULL, NULL);
	ASSERT(m_pObjHorzScrollBar);
	if (!m_pObjHorzScrollBar)
		return;

	int nHeight = 14;
	m_pObjHorzScrollBar->m_formHeight.CalCualteValue(nHeight);
	m_nHeightHorzScrollBar = nHeight;

	m_pObjHorzScrollBar->m_formLeft.SetFormula(m_rcPadding.left);
	m_pObjHorzScrollBar->m_formTop.SetFormula(CStrPrintf(_T("h-%d"), m_rcPadding.bottom + m_nHeightHorzScrollBar).c_str());
	m_pObjHorzScrollBar->m_formWidth.SetFormula(CStrPrintf(_T("w-%d"), m_rcPadding.left + m_rcPadding.right + m_nWidthVertScrollBar).c_str());
	if (m_pObjVertScrollBar)
	{
		// Vertical scroll bar is created first, decrease its height now.
		m_pObjVertScrollBar->m_formHeight.Increase(-nHeight);
	}

	m_pContainer->RecalculateUIObjSizePos(m_pObjHorzScrollBar);

	m_pHorzScrollBar = (CSkinVScrollBar*)m_pObjHorzScrollBar;
	m_pHorzScrollBar->SetScrollNotify(this);

	m_rcContent.bottom -= m_nHeightHorzScrollBar;
}
