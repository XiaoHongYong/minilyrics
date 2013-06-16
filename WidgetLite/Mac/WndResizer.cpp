/********************************************************************
Created  :	2001年7月21日 20:26:35
FileName :	UIWndSizing.cpp
Author   :	xhy

Purpose  :	调整窗口的大小
*********************************************************************/

#include "stdafx.h"
#include "WidgetLite.h"
#include "desktop.h"
#include "WndResizer.h"

#define SIZING_REGION		5		// 调整大小的范围

#ifdef _WIN32
void MyRectangle(HDC hdc, int x, int y, int x2, int y2)
{
	MoveToEx(hdc, x, y2, NULL);
	LineTo(hdc, x, y);
	LineTo(hdc, x2, y);
	LineTo(hdc, x2, y2);
	LineTo(hdc, x, y2);
}
#endif // #ifdef _WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWndResizer::CWndResizer()
{
	m_nMincx = 0;
	m_nMincy = 0;

	m_bFixedWidth = false;
	m_bFixedHeight = false;

	m_bResizing = false;
	m_ResizeDirection = 0;

	m_bInstanceResizing = true;

	m_bResizingAutoCloseto = true;

	m_pWnd = NULL;
	m_ptOld.x = m_ptOld.y = 0;
}

CWndResizer::~CWndResizer()
{

}

void CWndResizer::OnMouseMessage(UINT fwKeys, POINT pt)
{
	if (m_bFixedWidth && m_bFixedHeight)
		return;
	if (m_pWnd->IsZoomed())
		return;

	if (m_bResizing)
		SetCursor();
	else
	{
		// Need to do Hit Test.
		CRect		rc;
		m_pWnd->GetWindowRect(&rc);

		CPoint		point;
		MLWidget::GetCursorPos(&point);

		if (rc.PtInRect(point))
		{
			m_pWnd->ScreenToClient(rc);
			m_ResizeDirection = HitTestResizeArea(pt, rc);
			SetCursor();
		}
	}

	if (fwKeys & MK_LBUTTON)
	{
		if (m_bResizing)
			OnSizing(fwKeys, pt);
		else
			BeginSize(fwKeys, pt);
	}
	else if (m_bResizing)
	{
		EndSizing(fwKeys, pt);

		MLWidget::SetCursor(m_cursorArrow);
	}
}

void CWndResizer::Init(CBaseWnd *pWnd)
{
	m_pWnd = pWnd;

	m_cursorArrow.LoadStdCursor(CCursor::C_ARROW);
	m_cursorNWSE.LoadStdCursor(CCursor::C_SIZENWSE);
	m_cursorNESW.LoadStdCursor(CCursor::C_SIZENESW);
	m_cursorWE.LoadStdCursor(CCursor::C_SIZEWE);
	m_cursorNS.LoadStdCursor(CCursor::C_SIZENS);
}

bool CWndResizer::IsSizing()
{
	return m_bResizing;
}

void CWndResizer::BeginSize(UINT fwKeys, POINT &pt)
{
	// 记住鼠标位置
	MLWidget::GetCursorPos(&m_ptOld);

	// 记住窗口位置
	m_pWnd->GetWindowRect(&m_rcResizing);

	if (m_ResizeDirection != 0)
		m_bResizing = true;

	m_pWnd->SetCapture();

#ifdef _WIN32
	if (m_bResizing && !m_bInstanceResizing)
	{
		HDC			hdc;
		HBRUSH		hbrOld;
		HPEN		hpenOld;

		hdc = GetDC(NULL);
		hbrOld = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		hpenOld = (HPEN)SelectObject(hdc, GetStockObject(WHITE_PEN));
		SetROP2(hdc, R2_XORPEN);

		// 画出矩形框
		MyRectangle(hdc, m_rcResizing.left, m_rcResizing.top, m_rcResizing.right, m_rcResizing.bottom);

		SelectObject(hdc, hbrOld);
		SelectObject(hdc, hpenOld);
		ReleaseDC(NULL, hdc);

#if !defined(_WINCE_) && defined(_WIN32)
		LockWindowUpdate(m_pWnd->GetHandle());
#endif
	}
#endif // #ifdef _WIN32
}

void CWndResizer::OnSizing(UINT fwKeys, POINT &pt)
{
	// 正在调整大小
	int			nOffx;
	int			nOffy;

	MLWidget::GetCursorPos(&pt);

	// 只有鼠标移动了才调整大小
	if ((pt.x == m_ptOld.x) && (pt.y == m_ptOld.y))
		return;

	nOffx = pt.x - m_ptOld.x;	// x 偏移
	nOffy = pt.y - m_ptOld.y;	// y 偏移

	// 限制最小的宽度
	if (m_ResizeDirection & RD_LEFT)
	{
		if (m_rcResizing.right - (m_rcResizing.left + nOffx) <= m_nMincx)
			nOffx = m_rcResizing.Width() - m_nMincx;

		m_rcResizing.left += nOffx;
	}

	// 限制最小的高度
	if (m_ResizeDirection & RD_TOP)
	{
		if (m_rcResizing.bottom - (m_rcResizing.top + nOffy) <= m_nMincy)
			nOffy = m_rcResizing.Height() - m_nMincy;

		m_rcResizing.top += nOffy;
	}

	// 限制最小的宽度
	if (m_ResizeDirection & RD_RIGHT)
	{
		if ((m_rcResizing.right + nOffx) - m_rcResizing.left <= m_nMincx)
			nOffx = m_nMincx - m_rcResizing.Width();

		m_rcResizing.right += nOffx;
	}

	// 限制最小的高度
	if (m_ResizeDirection & RD_BOTTOM)
	{
		if ((m_rcResizing.bottom + nOffy) - m_rcResizing.top <= m_nMincy)
			nOffy = m_nMincy - m_rcResizing.Height();

		m_rcResizing.bottom += nOffy;
	}

	// 保存上一次鼠标的位置
	if (nOffx != 0)
		m_ptOld.x = pt.x;
	if (nOffy != 0)
		m_ptOld.y = pt.y;

	// 如果调整窗口时自动靠近则通过改变 nOffx, nOffy来调整
	if (m_bResizingAutoCloseto)
	{
		AutoCloseToWindows(nOffx, nOffy);
		m_ptOld.x += nOffx;
		m_ptOld.y += nOffy;
	}

	// remove old lines and draw new lines
	if (m_bInstanceResizing)
	{
		CRect		rc;
		m_pWnd->GetWindowRect(&rc);
		if (!rc.EqualRect(&m_rcResizing))
		{
			// m_ptOld = pt;
			m_pWnd->MoveWindowSafely(m_rcResizing.left, m_rcResizing.top, m_rcResizing.Width(), m_rcResizing.Height(), true);
		}
	}
	else
	{
#ifdef _WIN32
		HDC			hdc;
		HBRUSH		hbrOld;
		HPEN		hpenOld;

		hdc = GetDC(NULL);
		hbrOld = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		hpenOld = (HPEN)SelectObject(hdc, GetStockObject(WHITE_PEN));
		SetROP2(hdc, R2_XORPEN);

		// 删除原来的矩形框
		MyRectangle(hdc, rcOld.left, rcOld.top, rcOld.right, rcOld.bottom);

		// 画出新的矩形框
		MyRectangle(hdc, m_rcResizing.left, m_rcResizing.top, m_rcResizing.right, m_rcResizing.bottom);

		SelectObject(hdc, hbrOld);
		SelectObject(hdc, hpenOld);
		ReleaseDC(NULL, hdc);
#endif // #ifdef _WIN32
	}
}

void CWndResizer::SetResizeArea(int nIDArea, uint32 resizeDirection, RECT &rcResizeArea)
{
	ResizeArea	ra;
	ListResizeArea::iterator	it;

	if (nIDArea != -1)
	{
		for (it = m_listResizeArea.begin(); it != m_listResizeArea.end(); ++it)
		{
			ResizeArea	&t = *it;
			if (t.nID == nIDArea)
			{
				t.resizeDirection = resizeDirection;
				t.rcResizeArea = rcResizeArea;
				return;
			}
		}
	}

	ra.nID = nIDArea;
	ra.resizeDirection = resizeDirection;
	ra.rcResizeArea = rcResizeArea;
	m_listResizeArea.push_back(ra);
}

void CWndResizer::RemoveResizeArea(int nIDArea)
{
	ListResizeArea::iterator	it;

	if (nIDArea != -1)
	{
		for (it = m_listResizeArea.begin(); it != m_listResizeArea.end(); ++it)
		{
			ResizeArea	&t = *it;
			if (t.nID == nIDArea)
			{
				m_listResizeArea.erase(it);
				return;
			}
		}
	}
}

// 结束调整窗口大小
void CWndResizer::EndSizing(UINT fwKeys, POINT &pt)
{
	m_bResizing = false;

	m_ResizeDirection = 0;

	m_pWnd->ReleaseCapture();

	if (!m_bInstanceResizing)
	{
#ifdef _WIN32
		HDC			hdc;
		HBRUSH		hbrOld;
		HPEN		hpenOld;

		hdc = GetDC(NULL);
		hbrOld = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		hpenOld = (HPEN)SelectObject(hdc, GetStockObject(WHITE_PEN));
		SetROP2(hdc, R2_XORPEN);

		// 擦除画的矩形
		MyRectangle(hdc, m_rcResizing.left, m_rcResizing.top, m_rcResizing.right, m_rcResizing.bottom);

		SelectObject(hdc, hbrOld);
		SelectObject(hdc, hpenOld);
		ReleaseDC(NULL, hdc);

#if !defined(_WINCE_) && defined(_WIN32)
		LockWindowUpdate(NULL);
#endif
#endif // #ifdef _WIN32

		CRect		rc;
		m_pWnd->GetWindowRect(&rc);
		if (!rc.EqualRect(&m_rcResizing))
		{
			m_pWnd->MoveWindowSafely(m_rcResizing.left, m_rcResizing.top, m_rcResizing.Width(), m_rcResizing.Height(), true);
		}
	}
}

void CWndResizer::AutoCloseToWindows(int &nOffx, int &nOffy)
{
	nOffx = 0;
	nOffy = 0;
}

uint32 CWndResizer::HitTestResizeArea(POINT &pt, const RECT &rc)
{
	ListResizeArea::iterator	it;

	uint32 resizeDirection = 0;

	for (it = m_listResizeArea.begin(); it != m_listResizeArea.end(); ++it)
	{
		ResizeArea		&t = *it;
		if (t.rcResizeArea.PtInRect(pt))
			resizeDirection |= t.resizeDirection;
	}

	if (pt.x <= rc.left + SIZING_REGION)
		resizeDirection |= RD_LEFT;
	else if (pt.x >= rc.right - SIZING_REGION)
		resizeDirection |= RD_RIGHT;

	if (pt.y <= rc.top + SIZING_REGION)
		resizeDirection |= RD_TOP;
	else if (pt.y >= rc.bottom - SIZING_REGION)
		resizeDirection |= RD_BOTTOM;

	if (pt.x > rc.right - SIZING_REGION * 2 && pt.y > rc.bottom - SIZING_REGION * 2)
		resizeDirection |= RD_RIGHT | RD_BOTTOM;

	// Remove fixed direction
	if (m_bFixedWidth)
		resizeDirection &= ~ (RD_LEFT | RD_RIGHT);
	if (m_bFixedHeight)
		resizeDirection &= ~ (RD_TOP | RD_BOTTOM);

	// Remove exclude area
	if (resizeDirection & RD_LEFT)
		resizeDirection &= ~RD_RIGHT;
	if (resizeDirection & RD_TOP)
		resizeDirection &= ~RD_BOTTOM;

	return resizeDirection;
}

void CWndResizer::SetCursor()
{
	if (IsFlagSet(m_ResizeDirection, RD_LEFT | RD_TOP)
		|| IsFlagSet(m_ResizeDirection, RD_RIGHT | RD_BOTTOM))
		MLWidget::SetCursor(m_cursorNWSE);
	else if (IsFlagSet(m_ResizeDirection, RD_LEFT | RD_BOTTOM)
		|| IsFlagSet(m_ResizeDirection, RD_RIGHT | RD_TOP))
		MLWidget::SetCursor(m_cursorNESW);
	else if (m_ResizeDirection & (RD_LEFT | RD_RIGHT))
		MLWidget::SetCursor(m_cursorWE);
	else if (m_ResizeDirection & (RD_TOP | RD_BOTTOM))
		MLWidget::SetCursor(m_cursorNS);
}
