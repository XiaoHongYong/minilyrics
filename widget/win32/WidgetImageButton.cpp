// WidgetImageButton.cpp: implementation of the CWidgetImageButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WidgetCtrlBase.h"
#include "WidgetImageButton.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWidgetImageButton::CWidgetImageButton()
{
	m_pWndProcOrg = NULL;
	m_nBmpCx = m_nBmpCy = 0;
	m_pImage = NULL;
}

CWidgetImageButton::~CWidgetImageButton()
{
	if (m_pImage)
		delete m_pImage;
}

BOOL CWidgetImageButton::Attach(CBaseWnd *pWndParent, int nCtrlID)
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

void CWidgetImageButton::SetImage(CMLImage *pImage)
{
	if (m_pImage)
	{
		delete m_pImage;
		m_hBmp = NULL;
		m_pImage = NULL;
	}

	if (!pImage)
		return;

	m_pImage = pImage;
	m_hBmp = pImage->GetHandle();

	if (m_hBmp)
	{
		BITMAP		bitmap;
		memset(&bitmap, 0, sizeof(bitmap));
		GetObject(m_hBmp, sizeof(bitmap), &bitmap);
		m_nBmpCx = bitmap.bmWidth;
		m_nBmpCy = bitmap.bmHeight;
	}
}


void CWidgetImageButton::OnPaint(IMLGraphicsBase *pGraph)
{
	if (m_hBmp)
	{
		HDC		hdcBk, hdc;
		HBITMAP	hBmpOld;
		CRect	rc;

		GetClientRect(&rc);

		hdc = pGraph->GetHandle();
		hdcBk = CreateCompatibleDC(hdc);
		hBmpOld = (HBITMAP)SelectObject(hdcBk, m_hBmp);
		int nOldMode = SetStretchBltMode(pGraph->GetHandle(), COLORONCOLOR);

		StretchBlt(hdc, 0, 0, rc.Width(), rc.Height(), hdcBk, 0, 0, m_nBmpCx, m_nBmpCy, SRCCOPY);

		SetStretchBltMode(pGraph->GetHandle(), nOldMode);
		SelectObject(hdcBk, hBmpOld);
		DeleteDC(hdcBk);
	}
}

LRESULT CWidgetImageButton::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_PAINT)
		return CBaseWnd::WndProc(message, wParam, lParam);
	else if (message == WM_DESTROY)
	{
		SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)m_pWndProcOrg);
	}

	return CallWindowProc(m_pWndProcOrg, m_hWnd, message, wParam, lParam);
}

void CWidgetImageButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	::PostMessage(::GetParent(m_hWnd), WM_COMMAND, GetDlgCtrlID(m_hWnd), (LPARAM)m_hWnd);
}
