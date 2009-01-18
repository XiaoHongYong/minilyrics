// CommonWidget.cpp: implementation of the CommonWidget class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommonWidget.h"

//////////////////////////////////////////////////////////////////////////

bool CWidgetLable::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_hWnd = CreateWindowEx(0, _T("Static"), szText, WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, pWndParent->GetHandle(), NULL, GetAppInstance(), this);
	if (!m_hWnd)
		return false;

	SetWindowLong(m_hWnd, GWL_ID, nID);

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CWidgetButton::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_hWnd = CreateWindowEx(0, _T("Button"), szText, WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, pWndParent->GetHandle(), NULL, GetAppInstance(), this);
	if (!m_hWnd)
		return false;

	SetWindowLong(m_hWnd, GWL_ID, nID);

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CWidgetToggleButton::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_hWnd = CreateWindowEx(0, _T("Button"), szText, BS_AUTOCHECKBOX | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, pWndParent->GetHandle(), NULL, GetAppInstance(), this);
	if (!m_hWnd)
		return false;

	SetWindowLong(m_hWnd, GWL_ID, nID);

	return true;
}

bool CWidgetToggleButton::IsChecked()
{
	return true;
}

void CWidgetToggleButton::SetCheck(bool bChecked)
{
}

//////////////////////////////////////////////////////////////////////////

bool CWidgetCheckButton::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_hWnd = CreateWindowEx(0, _T("Button"), szText, BS_AUTOCHECKBOX | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, pWndParent->GetHandle(), NULL, GetAppInstance(), this);
	if (!m_hWnd)
		return false;

	SetWindowLong(m_hWnd, GWL_ID, nID);

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CWidgetRadioButton::Create(LPCTSTR szText, CBaseWnd *pWndParent, CWidgetRadioButton *pGroupStart, int nID)
{
	m_hWnd = CreateWindowEx(0, _T("Button"), szText, BS_AUTORADIOBUTTON | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, pWndParent->GetHandle(), NULL, GetAppInstance(), this);
	if (!m_hWnd)
		return false;

	SetWindowLong(m_hWnd, GWL_ID, nID);

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CWidgetFrame::Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID)
{
	m_hWnd = CreateWindowEx(0, _T("Button"), szText, BS_GROUPBOX | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, pWndParent->GetHandle(), NULL, GetAppInstance(), this);
	if (!m_hWnd)
		return false;

	SetWindowLong(m_hWnd, GWL_ID, nID);

	return true;
}

//////////////////////////////////////////////////////////////////////////

CWidgetBox::CWidgetBox()
{
	m_bHorz = true;
	m_bHomogeneous = true;
	m_nSpacing = 0;
}

bool CWidgetBox::Create(bool bHorz, bool bHomogeneous, int nSpacing, CBaseWnd *pWndParent, int nID)
{
	m_bHorz = bHorz;
	m_bHomogeneous = bHomogeneous;
	m_nSpacing = Tobool(nSpacing);

	return true;
}

// pack 
void CWidgetBox::PackStart(CBaseWnd *pWnd, bool bExpand, bool bFill, uint nPadding)
{
}

void CWidgetBox::PackEnd(CBaseWnd *pWnd, bool bExpand, bool bFill, uint nPadding)
{
}

//////////////////////////////////////////////////////////////////////////

CWidgetScrollBar::CWidgetScrollBar()
{
	m_pScrollNotify = NULL;
}

CWidgetScrollBar::~CWidgetScrollBar()
{

}

BOOL CWidgetScrollBar::Create(int x, int y, int nWidth, int nHeight, bool bVert, CBaseWnd *pWndParent, int nID)
{
	BOOL		bRet;
	DWORD		dwStyle = WS_CHILD;

	ASSERT(m_hWnd != NULL);

	dwStyle |= bVert ? SBS_VERT : SBS_HORZ;
	bRet = CBaseWnd::CreateEx(_T("SCROLLBAR"), NULL, x, y, nWidth, nHeight, pWndParent, dwStyle, nID);
	if (!bRet)
		return FALSE;

	SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)(IScrollNotify*)this);

	m_sbHandler.Init(this, SB_CTL);

	return bRet;
}

void CWidgetScrollBar::SetScrollInfo(int nMin, int nMax, int nPage, int nPos, int nLine, bool bRedraw)
{
	m_sbHandler.SetScrollInfo(nMin, nMax, nPage, nPos, nLine, bRedraw);
}

int CWidgetScrollBar::SetScrollPos(int nPos, bool bRedraw)
{
	if (m_sbHandler.IsEnabled())
		return m_sbHandler.SetScrollPos(nPos, bRedraw);
	else
		return nPos;
}

int CWidgetScrollBar::GetScrollPos() const
{
	return m_sbHandler.GetScrollPos();
}

bool CWidgetScrollBar::HandleScrollCode(UINT nSBCode, int nPos)
{
	if (!m_sbHandler.HandleScrollCode(nSBCode, nPos))
		return false;

	if (m_pScrollNotify)
	{
		if (m_sbHandler.IsVertBar())
			m_pScrollNotify->OnVScroll(nSBCode, nPos, this);
		else
			m_pScrollNotify->OnHScroll(nSBCode, nPos, this);
	}
	return true;
}

void CWidgetScrollBar::OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	m_sbHandler.HandleScrollCode(nSBCode, nPos);

	if (m_pScrollNotify)
		m_pScrollNotify->OnVScroll(nSBCode, nPos, this);
}

void CWidgetScrollBar::OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	m_sbHandler.HandleScrollCode(nSBCode, nPos);

	if (m_pScrollNotify)
		m_pScrollNotify->OnHScroll(nSBCode, nPos, this);
}
