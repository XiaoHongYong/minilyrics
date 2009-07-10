/********************************************************************
	Created  :	2001年7月21日 20:26:35
	FileName :	WndResizer.cpp
	Author   :	xhy
	
	Purpose  :	调整窗口的大小
*********************************************************************/

#include "StdAfx.h"
#include "desktop.h"
#include "WndResizer.h"

#define SIZING_REGION		5		// 调整大小的范围
/*

void MyRectangle(HDC hdc, int x, int y, int x2, int y2)
{
	MoveToEx(hdc, x, y2, NULL);
	LineTo(hdc, x, y);
	LineTo(hdc, x2, y);
	LineTo(hdc, x2, y2);
	LineTo(hdc, x, y2);
}
*/

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

	m_bAtLeft = false;
	m_bAtTop = false;
	m_bAtRight = false;
	m_bAtBottom = false;
	
    m_bResizingAutoCloseto = true;
	
    m_pWnd = NULL;
}

CWndResizer::~CWndResizer()
{

}

void CWndResizer::OnMouseMessage(UINT fwKeys, POINT pt)
{
	if (m_bFixedWidth && m_bFixedHeight)
		return;

	if (fwKeys & MK_LBUTTON)
	{
		if (m_bResizing)
		{
			OnSizing(fwKeys, pt);
		}
		else
		{
			BeginSize(fwKeys, pt);
		}
	}
	else if (m_bResizing)
	{
		EndSizing(fwKeys, pt);

		MLWidget::SetCursor(m_cursorArrow);
		return;
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
	if (m_pWnd->IsZoomed())
		return;

	// 第一次进入, 调整大小

	// 记住鼠标位置
	MLWidget::GetCursorPos(&m_ptOld);

	// 记住窗口位置
	m_pWnd->GetWindowRect(&m_rcResizing);

	// 鼠标不在窗口中
	if (!m_rcResizing.PtInRect(m_ptOld))
		return;

	if (!m_bFixedWidth)
	{
		if (m_ptOld.x < m_rcResizing.left + SIZING_REGION)
		{
			// 在<左边>大小调整位置区
			m_bAtLeft = true;
			m_bResizing = true;
		}
		else if (m_ptOld.x > m_rcResizing.right - SIZING_REGION)
		{
			// 在<右边>大小调整位置区
			m_bAtRight = true;
			m_bResizing = true;
		}
	}

	if (!m_bFixedHeight)
	{
		if (m_ptOld.y < m_rcResizing.top + SIZING_REGION)
		{
			// 在<上边>大小调整位置区
			m_bAtTop = true;
			m_bResizing = true;
		}
		else if (m_ptOld.y > m_rcResizing.bottom - SIZING_REGION)
		{
			// 在<下边>大小调整位置区
			m_bAtBottom = true;
			m_bResizing = true;
		}
	}

	if (m_bResizing)
	{
		// 开始调整窗口大小了, 画出矩形框
/*
		HDC			hdc;
		HBRUSH		hbrOld;
		HPEN		hpenOld;
		
		hdc = GetDC(NULL);
		hbrOld = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		hpenOld = (HPEN)SelectObject(hdc, GetStockObject(WHITE_PEN));
		SetROP2(hdc, R2_XORPEN);

		// 画出矩形框
		MyRectangle(hdc, m_rcSizing.left, m_rcSizing.top, m_rcSizing.right, m_rcSizing.bottom);
		
		SelectObject(hdc, hbrOld);
		SelectObject(hdc, hpenOld);
		ReleaseDC(NULL, hdc);
*/

		// 捕捉鼠标输入
		m_pWnd->SetCapture();
#if !defined(_WINCE_) && defined(_WIN32)
		LockWindowUpdate(m_pWnd->GetHandle());
#endif
	}

	SetSizeCursor(fwKeys, pt);
}

void CWndResizer::OnSizing(UINT fwKeys, POINT &pt)
{
	// 正在调整大小
	int			nOffx;
	int			nOffy;
	CRect		rcOld = m_rcResizing;

	MLWidget::GetCursorPos(&pt);

	// 只有鼠标移动了才调整大小
	if ((pt.x == m_ptOld.x) && (pt.y == m_ptOld.y))
		return;

	nOffx = pt.x - m_ptOld.x;	// x 偏移
	nOffy = pt.y - m_ptOld.y;	// y 偏移

	// 保存上一次鼠标的位置
	m_ptOld = pt;

	// 限制最小的宽度
	if (m_bAtLeft)
	{
		if (m_rcResizing.right - (m_rcResizing.left + nOffx) <= m_nMincx)
			nOffx = m_rcResizing.Width() - m_nMincx;

		m_rcResizing.left += nOffx;
	}
	
	// 限制最小的高度
	if (m_bAtTop)
	{
		if (m_rcResizing.bottom - (m_rcResizing.top + nOffy) <= m_nMincy)
			nOffy = m_rcResizing.Height() - m_nMincy;

		m_rcResizing.top += nOffy;
	}
	
	// 限制最小的宽度
	if (m_bAtRight)
	{
		if ((m_rcResizing.right + nOffx) - m_rcResizing.left <= m_nMincx)
			nOffx = m_nMincx - m_rcResizing.Width();

		m_rcResizing.right += nOffx;
	}

	// 限制最小的高度
	if (m_bAtBottom)
	{
		if ((m_rcResizing.bottom + nOffy) - m_rcResizing.top <= m_nMincy)
			nOffy = m_nMincy - m_rcResizing.Height();

		m_rcResizing.bottom += nOffy;
	}

	// 如果调整窗口时自动靠近则通过改变 nOffx, nOffy来调整
	if (m_bResizingAutoCloseto)
	{
		AutoCloseToWindows(nOffx, nOffy);
		m_ptOld.x += nOffx;
		m_ptOld.y += nOffy;
	}
/*
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
	MyRectangle(hdc, m_rcSizing.left, m_rcSizing.top, m_rcSizing.right, m_rcSizing.bottom);

	SelectObject(hdc, hbrOld);
	SelectObject(hdc, hpenOld);
	ReleaseDC(NULL, hdc);
*/

	SetSizeCursor(fwKeys, pt);
}

// 结束调整窗口大小
void CWndResizer::EndSizing(UINT fwKeys, POINT &pt)
{
	m_bResizing = false;

	m_bAtLeft = false;
	m_bAtTop = false;
	m_bAtRight = false;
	m_bAtBottom = false;
/*

	HDC			hdc;
	HBRUSH		hbrOld;
	HPEN		hpenOld;
	
	hdc = GetDC(NULL);
	hbrOld = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	hpenOld = (HPEN)SelectObject(hdc, GetStockObject(WHITE_PEN));
	SetROP2(hdc, R2_XORPEN);

	// 擦除画的矩形
	MyRectangle(hdc, m_rcSizing.left, m_rcSizing.top, m_rcSizing.right, m_rcSizing.bottom);
	
	SelectObject(hdc, hbrOld);
	SelectObject(hdc, hpenOld);
	ReleaseDC(NULL, hdc);
*/

	m_pWnd->ReleaseCapture();
#if !defined(_WINCE_) && defined(_WIN32)
	LockWindowUpdate(NULL);
#endif

	CRect		rc;
	m_pWnd->GetWindowRect(&rc);

	if (!rc.EqualRect(&m_rcResizing))
	{
		// 调整窗口大小
		m_pWnd->MoveWindowSafely(m_rcResizing.left, m_rcResizing.top, m_rcResizing.Width(), m_rcResizing.Height(), true);
		// MyMoveWindow(m_rcResizing);
	}
}

// 根据条件设置鼠标关闭的形状
void CWndResizer::SetSizeCursor(UINT fwKeys, POINT pt)
{
	if (m_bFixedWidth && m_bFixedHeight)
		return;

	if (m_pWnd->IsZoomed())
		return;

	// 设置鼠标光标
	if (m_bResizing)
	{
		if (m_bAtLeft && m_bAtTop || m_bAtBottom && m_bAtRight)
			MLWidget::SetCursor(m_cursorNWSE);
		else if (m_bAtLeft && m_bAtBottom || m_bAtTop && m_bAtRight)
			MLWidget::SetCursor(m_cursorNESW);
		else if (m_bAtLeft || m_bAtRight)
			MLWidget::SetCursor(m_cursorWE);
		else if (m_bAtTop || m_bAtBottom)
			MLWidget::SetCursor(m_cursorNS);
	}
	else
	{
		CRect		rc;
		CPoint		point;

		m_pWnd->GetWindowRect(&rc);
		
		MLWidget::GetCursorPos(&point);
		
		if (rc.PtInRect(point))
		{
			bool	bAtLeft = false, bAtTop = false;
			bool	bAtRight = false, bAtBottom = false;

			if (!m_bFixedWidth)
			{
				if (point.x < rc.left + SIZING_REGION)
					bAtLeft = true;
				else if (point.x > rc.right - SIZING_REGION)
					bAtRight = true;
			}

			if (!m_bFixedHeight)
			{
				if (point.y < rc.top + SIZING_REGION)
					bAtTop = true;
				else if (point.y > rc.bottom - SIZING_REGION)
					bAtBottom = true;
			}

			if (bAtLeft && bAtTop || bAtBottom && bAtRight)
				MLWidget::SetCursor(m_cursorNWSE);
			else if (bAtLeft && bAtBottom || bAtTop && bAtRight)
				MLWidget::SetCursor( m_cursorNESW);
			else if (bAtLeft || bAtRight)
				MLWidget::SetCursor(m_cursorWE);
			else if (bAtTop || bAtBottom)
				MLWidget::SetCursor(m_cursorNS);
		}
	}
}

void CWndResizer::AutoCloseToWindows(int &nOffx, int &nOffy)
{
	nOffx = 0;
	nOffy = 0;
}
