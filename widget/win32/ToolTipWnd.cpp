// ToolTipWnd.cpp: implementation of the CToolTipWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolTipWnd.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CToolTipWnd::CToolTipWnd()
{

}

CToolTipWnd::~CToolTipWnd()
{

}

BOOL CToolTipWnd::Create(HWND hWndParent, DWORD dwStyle)
{
	m_hWnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
		WS_POPUP | dwStyle, // force WS_POPUP
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hWndParent, NULL, GetAppInstance(), NULL);

	return (m_hWnd != NULL);
}

BOOL CToolTipWnd::AddTool(HWND hWnd, LPCTSTR szText, LPCRECT lpRectTool, UINT nIDTool)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(hWnd != NULL);
	ASSERT(szText != NULL);
	// the toolrect and toolid must both be zero or both valid
	ASSERT((lpRectTool != NULL && nIDTool != 0) ||
		   (lpRectTool == NULL) && (nIDTool == 0));

	TOOLINFO	ti;
	FillInToolInfo(ti, hWnd, nIDTool);
	if (lpRectTool != NULL)
		memcpy(&ti.rect, lpRectTool, sizeof(RECT));
	ti.lpszText = (LPTSTR)szText;

	return (BOOL) ::SendMessage(m_hWnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void CToolTipWnd::DelTool(HWND hWnd, UINT nIDTool)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(hWnd != NULL);

	TOOLINFO ti;
	FillInToolInfo(ti, hWnd, nIDTool);

	::SendMessage(m_hWnd, TTM_DELTOOL, 0, (LPARAM)&ti);
}

void CToolTipWnd::FillInToolInfo(TOOLINFO &ti, HWND hWnd, UINT nIDTool) const
{
	memset(&ti, 0, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);

	if (nIDTool == 0)
	{
		ti.hwnd = ::GetParent(hWnd);
		ti.uFlags = TTF_IDISHWND;
		ti.uId = (UINT)hWnd;
	}
	else
	{
		ti.hwnd = hWnd;
		ti.uFlags = 0;
		ti.uId = nIDTool;
	}
}

void CToolTipWnd::RelayEvent(LPMSG lpMsg)
{
	assert(::IsWindow(m_hWnd));
	::SendMessage(m_hWnd, TTM_RELAYEVENT, 0, (LPARAM)lpMsg);
}
