// SkinToolTip.cpp: implementation of the CSkinToolTip class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkinToolTip.h"

void FillInToolInfo(TOOLINFO &ti, HWND hWnd, UINT nIDTool)
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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinToolTip::CSkinToolTip()
{
	m_hWnd = NULL;
	m_hWndParent = NULL;
}

CSkinToolTip::~CSkinToolTip()
{

}

BOOL CSkinToolTip::Create(CBaseWnd *pWndParent)
{
	m_hWnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
		WS_POPUP, // force WS_POPUP
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		pWndParent->GetHandle(), NULL, GetAppInstance(), NULL);

	m_hWndParent = pWndParent->GetHandle();

	return (m_hWnd != NULL);
}

BOOL CSkinToolTip::AddTool(LPCTSTR szText, LPCRECT lpRectTool, UINT nIDTool)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(szText != NULL);
	// the toolrect and toolid must both be zero or both valid
	ASSERT((lpRectTool != NULL && nIDTool != 0) ||
		   (lpRectTool == NULL) && (nIDTool == 0));

	TOOLINFO	ti;
	FillInToolInfo(ti, m_hWndParent, nIDTool);
	if (lpRectTool != NULL)
		memcpy(&ti.rect, lpRectTool, sizeof(RECT));
	ti.lpszText = (LPTSTR)szText;

	return (BOOL) ::SendMessage(m_hWnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void CSkinToolTip::DelTool(UINT nIDTool)
{
	ASSERT(::IsWindow(m_hWnd));

	TOOLINFO ti;
	FillInToolInfo(ti, m_hWndParent, nIDTool);

	::SendMessage(m_hWnd, TTM_DELTOOL, 0, (LPARAM)&ti);
}

void CSkinToolTip::RelayEvent(LPMSG lpMsg)
{
	assert(::IsWindow(m_hWnd));
	::SendMessage(m_hWnd, TTM_RELAYEVENT, 0, (LPARAM)lpMsg);
}
