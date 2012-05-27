// HyperLinkCtrl.cpp: implementation of the CHyperLinkCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Widget.h"
#include "WidgetCtrlBase.h"
#include "HyperLinkCtrl.h"
#include "../MPShared/resource.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHyperLinkCtrl::CHyperLinkCtrl()
{
	m_sLinkColor = RGB(0, 0 ,255);
	m_sHoverColor = RGB(255, 0, 0);
	m_sVisitedColor = RGB(84, 0, 84);

	m_bVisited = FALSE;
}

CHyperLinkCtrl::~CHyperLinkCtrl()
{
}

BOOL CHyperLinkCtrl::Attach(CBaseWnd *pParentWnd, UINT uChildId, LPCTSTR szUrl)
{
	m_hWnd = ::GetDlgItem(pParentWnd->GetHandle(), uChildId);
	if (!m_hWnd)
		return FALSE;

	// stash pointer to self
	SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);

	m_pWndProcOrg = (WNDPROC)GetWindowLong(m_hWnd, GWL_WNDPROC);
	SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)BaseWndProc);

	m_cursor.LoadCursorFromRes(IDC_MLHAND);

	m_strUrl = szUrl;

	return TRUE;
}

void CHyperLinkCtrl::OnPaint(CMLGraphics *pGraph, CRect &rcClip)
{
	CRect		rc;
	TCHAR		szText[256];
	HWND		hWnd;
	HBRUSH		hbrBk, hbrOld;
	HFONT		hFont, hFontOld;
	HDC			hdc = pGraph->GetHandle();

#ifdef _WINCE_
	hbrBk = CreateSolidBrush(GetSysColor(COLOR_STATIC));
#else
	hbrBk = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
#endif
	hWnd = ::GetParent(m_hWnd);

	hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);

	hFontOld = (HFONT)::SelectObject(hdc, hFont);
	hbrOld = (HBRUSH)::SelectObject(hdc, hbrBk);

	SetBkMode(hdc, TRANSPARENT);

	if (m_bVisited)
		SetTextColor(hdc, m_sVisitedColor);
	else
		SetTextColor(hdc, m_sLinkColor);

	GetClientRect(&rc);
	FillRect(hdc, &rc, hbrBk);
	GetWindowText(szText, CountOf(szText));
	DrawText(hdc, szText, _tcslen(szText), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	// DrawText(hdc, szText, _tcslen(szText), &rc, 0);

	::SelectObject(hdc, hFontOld);
	::SelectObject(hdc, hbrOld);

	DeleteObject(hbrBk);
}

LRESULT CHyperLinkCtrl::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_PAINT || message == WM_LBUTTONUP || message == WM_LBUTTONDOWN || message == WM_MOUSEMOVE)
		return CBaseWnd::WndProc(message, wParam, lParam);
	else if (message == WM_DESTROY)
	{
		SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)m_pWndProcOrg);
	}

	return CallWindowProc(m_pWndProcOrg, m_hWnd, message, wParam, lParam);
}

void CHyperLinkCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	MLWidget::SetCursor(m_cursor);
}

void CHyperLinkCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	MLWidget::SetCursor(m_cursor);
}

void CHyperLinkCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	MLWidget::SetCursor(m_cursor);
	m_bVisited = TRUE;

	ShellExecute(m_hWnd, _T("open"), m_strUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
