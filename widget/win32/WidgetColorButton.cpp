// HyperLinkCtrl.cpp: implementation of the CWidgetColorButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WidgetColorButton.h"
#ifdef _WIN32_DESKTOP
#include "DlgChooseColor.h"
#else
#include "../MiniLyrics6/win32/wce/DlgChooseColor.h"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWidgetColorButton::CWidgetColorButton()
{
	m_pWndProcOrg = NULL;
	m_clrBg.set(GetSysColor(COLOR_BTNFACE));
}

CWidgetColorButton::~CWidgetColorButton()
{
}

BOOL CWidgetColorButton::Attach(CBaseWnd *pWndParent, int nCtrlID)
{
	m_hWnd = ::GetDlgItem(pWndParent->GetHandle(), nCtrlID);
	if (!m_hWnd)
		return FALSE;

	// stash pointer to self
	SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);

	m_pWndProcOrg = (WNDPROC)GetWindowLong(m_hWnd, GWL_WNDPROC);
	SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)BaseWndProc);

	return TRUE;
}

void CWidgetColorButton::SetColor(const MLColor &clr)
{
	m_clrBg = clr;
}

void CWidgetColorButton::GetColor(MLColor &clr)
{
	clr = m_clrBg;
}

void CWidgetColorButton::OnPaint(IMLGraphicsBase *pGraph)
{
	CRect		rc;
	HBRUSH		hbrBg;

	hbrBg = CreateSolidBrush(m_clrBg.get());

	GetClientRect(&rc);

	FillRect(pGraph->GetHandle(), &rc, hbrBg);

	DeleteObject(hbrBg);
}

LRESULT CWidgetColorButton::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_PAINT || message == WM_LBUTTONUP)
		return CBaseWnd::WndProc(message, wParam, lParam);
	else if (message == WM_DESTROY)
	{
		SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)m_pWndProcOrg);
	}

	return CallWindowProc(m_pWndProcOrg, m_hWnd, message, wParam, lParam);
}

void CWidgetColorButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDlgChooseColor		dlg;

	if (dlg.DoModal(this, m_clrBg) == IDOK)
	{
		m_clrBg = dlg.GetColor();
		InvalidateRect();
		::PostMessage(::GetParent(m_hWnd), WM_COMMAND, GetDlgCtrlID(m_hWnd), (LPARAM)m_hWnd);
	}
}
