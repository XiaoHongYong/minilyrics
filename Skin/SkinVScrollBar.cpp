// SkinVScrollBar.cpp: implementation of the CSkinVScrollBar class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinVScrollBar.h"

//////////////////////////////////////////////////////////////////////////

CSkinSrollBarBase::CSkinSrollBarBase()
{
	// 虚拟滚动位置
	m_nVirtualMin = 0;		// 最小的值
	m_nVirtualMax = 10;		// 最大的值
	m_nVirtualPage = 1;		// 滚动一页的值
	m_nVirtualLine = 1;		// 滚动一行的值，缺省为1
	m_nVirtualCurPos = m_nVirtualMin;

	m_nPosThumb = 0;		// 滚动条的位置(上面按钮的下方 －> thumb的上方的距离)

	m_bStretchedThumb = false;
	m_nSizeThumbStretched = 41;

	m_nMsgWant = UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_MOUSEWHEEL;

	m_nClickDelayTimerId = 0;
	m_nRepeatTimerId = 0;

	m_pScrollNotify = NULL;

	m_nCursorToThumbBeg = 0;
	m_PushDownPos = PUSH_DOWN_NONE;
	m_CursorPosLatest = PUSH_DOWN_NONE;

	m_nLinesPerWheel = 3;
}

CSkinSrollBarBase::~CSkinSrollBarBase()
{
	UnregisterTimer();
}

void CSkinSrollBarBase::SetScrollInfo(int nMin, int nMax, int nPage, int nPos, int nLine, bool bRedraw)
{
	m_nVirtualMin = nMin;
	m_nVirtualMax = nMax - nPage;
	m_nVirtualPage = nPage;
	m_nVirtualLine = nLine;

	m_nVirtualCurPos = nPos;

	if (m_nVirtualMax < 0)
		m_nVirtualMax = 0;

	if (m_nVirtualCurPos > m_nVirtualMax)
		m_nVirtualCurPos = m_nVirtualMax;
	else if (m_nVirtualCurPos < m_nVirtualMin)
		m_nVirtualCurPos = m_nVirtualMin;

	AdjustThumbSize();

	m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);

	if (bRedraw)
		Invalidate();
}

// COMMENT:
//		设置新的滚动位置
// RETURN:
//		返回设置以前的滚动位置的值
int CSkinSrollBarBase::SetScrollPos(int nPos, bool bRedraw)
{
	int		nPosOld;

	nPosOld = m_nVirtualCurPos;
	m_nVirtualCurPos = nPos;

	if (m_nVirtualCurPos > m_nVirtualMax)
		m_nVirtualCurPos = m_nVirtualMax;
	else if (m_nVirtualCurPos < m_nVirtualMin)
		m_nVirtualCurPos = m_nVirtualMin;

	m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);

	if (bRedraw)
		Invalidate();

	return nPosOld;
}

int CSkinSrollBarBase::GetMax() const
{
	return m_nVirtualMax;
}

void CSkinSrollBarBase::SetScrollNotify(IScrollNotify *pNofity)
{
	m_pScrollNotify = pNofity;
}

bool CSkinSrollBarBase::IsEnabled() const
{
	return m_nVirtualMax != 0;
}

void CSkinSrollBarBase::DisableScrollBar()
{
	m_nVirtualMin = 0;
	m_nVirtualMax = 0;
	m_nVirtualPage = 1;
	m_nVirtualLine = 1;

	m_nPosThumb = 0;

	if (m_nVirtualCurPos != 0)
	{
		m_nVirtualCurPos = 0;
		OnScroll(SB_THUMBPOSITION);
	}
}

void CSkinSrollBarBase::OnSize()
{
	CUIObject::OnSize();

	AdjustThumbSize();

	m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);
}

bool CSkinSrollBarBase::OnLButtonDown(UINT nFlags, CPoint point)
{
	assert(m_pSkin);

	m_PushDownPos = GetPushDownPos(point);
	m_CursorPosLatest = m_PushDownPos;

	switch (m_PushDownPos)
	{
	case PUSH_DOWN_TOPBT:
		// 滚动条上面的按钮
		TopBtOnLButtonDown(nFlags, point);
		break;
	case PUSH_DOWN_BOTTOMBT:
		// 滚动条下面的按钮
		BottomBtOnLButtonDown(nFlags, point);
		break;
	case PUSH_DOWN_THUMB:
		// 滚动条的拖动box(thumb)
		ThumbOnLButtonDown(nFlags, point);
		break;
	case PUSH_DOWN_TOPTRACK:
	case PUSH_DOWN_BOTTOMTRACK:
		// 滚动条的空白位置，进行翻页操作
		TrackOnLButtonDown(nFlags, point);
		break;
	}

	return true;
}

void CSkinSrollBarBase::TopBtOnLButtonDown(UINT nFlags, CPoint point)
{
	// 捕捉鼠标输入
	m_pSkin->SetCaptureMouse(this);

	int		nVirtualPosOld;

	nVirtualPosOld = m_nVirtualCurPos;

	if (m_nVirtualCurPos > m_nVirtualMin)
		m_nVirtualCurPos -= m_nVirtualLine;
	if (m_nVirtualCurPos < m_nVirtualMin)
		m_nVirtualCurPos = m_nVirtualMin;

	if (m_nVirtualCurPos == nVirtualPosOld)
	{
		Invalidate();
		return;
	}

	m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);

	OnScroll(SB_LINEUP);

	RegisterTimer();
	// DBGLOG0(_T("Top button down"));

	Invalidate();
}

void CSkinSrollBarBase::BottomBtOnLButtonDown(UINT nFlags, CPoint point)
{
	// 捕捉鼠标输入
	m_pSkin->SetCaptureMouse(this);

	int		nVirtualPosOld;

	nVirtualPosOld = m_nVirtualCurPos;

	if (m_nVirtualCurPos < m_nVirtualMax)
		m_nVirtualCurPos += m_nVirtualLine;
	if (m_nVirtualCurPos > m_nVirtualMax)
		m_nVirtualCurPos = m_nVirtualMax;

	if (m_nVirtualCurPos == nVirtualPosOld)
	{
		Invalidate();
		return;
	}

	m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);
	OnScroll(SB_LINEDOWN);

	RegisterTimer();
	// DBGLOG0(_T("Bottom button down"));

	Invalidate();
}

void CSkinSrollBarBase::TrackOnLButtonDown(UINT nFlags, CPoint point)
{
	// 捕捉鼠标输入
	m_pSkin->SetCaptureMouse(this);

	RegisterTimer();
	// DBGLOG0(_T("Track button down"));

	if (m_PushDownPos == PUSH_DOWN_TOPTRACK)
		OnScroll(SB_PAGEUP);
	else if (m_PushDownPos == PUSH_DOWN_BOTTOMTRACK)
		OnScroll(SB_PAGEDOWN);

	Invalidate();
}

bool CSkinSrollBarBase::OnLButtonUp(UINT nFlags, CPoint point)
{
	// DBGLOG0(_T("OnLButtonUp "));
	switch (m_PushDownPos)
	{
	case PUSH_DOWN_TOPBT:
		// 滚动条上面的按钮
		TopBtOnLButtonUp(nFlags, point);
		break;
	case PUSH_DOWN_BOTTOMBT:
		// 滚动条下面的按钮
		BottomBtOnLButtonUp(nFlags, point);
		break;
	case PUSH_DOWN_THUMB:
		// 滚动条的拖动box(thumb)
		ThumbOnLButtonUp(nFlags, point);
		break;
	case PUSH_DOWN_TOPTRACK:
	case PUSH_DOWN_BOTTOMTRACK:
		// 滚动条的空白位置，进行翻页操作
		TrackOnLButtonUp(nFlags, point);
		break;
	}

	m_PushDownPos = PUSH_DOWN_NONE;

	PUSH_DOWN_POS		CursorPosNew;

	CursorPosNew = GetPushDownPos(point);

	Invalidate();

	return true;
}

void CSkinSrollBarBase::TopBtOnLButtonUp(UINT nFlags, CPoint point)
{
	UnregisterTimer();

	// 释放鼠标输入
	m_pSkin->ReleaseCaptureMouse(this);

	// DBGLOG0(_T("TopBt button up"));
}

void CSkinSrollBarBase::BottomBtOnLButtonUp(UINT nFlags, CPoint point)
{
	UnregisterTimer();

	// 释放鼠标输入
	m_pSkin->ReleaseCaptureMouse(this);

	// DBGLOG0(_T("BottomBt button up"));
}

void CSkinSrollBarBase::ThumbOnLButtonUp(UINT nFlags, CPoint point)
{
	// 释放鼠标输入
	m_pSkin->ReleaseCaptureMouse(this);

	// DBGLOG0(_T("Thumb button up"));
}

void CSkinSrollBarBase::TrackOnLButtonUp(UINT nFlags, CPoint point)
{
	OnTimer(m_nRepeatTimerId);

	UnregisterTimer();

	// 释放鼠标输入
	m_pSkin->ReleaseCaptureMouse(this);

	// DBGLOG0(_T("Track button up"));
}

void CSkinSrollBarBase::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	if (nMkeys == 0)
	{
		int		nVirtualPosOld;

		nVirtualPosOld = m_nVirtualCurPos;

		m_nVirtualCurPos -= m_nVirtualLine * (nWheelDistance / WHEEL_DELTA) * m_nLinesPerWheel;
		if (m_nVirtualCurPos < m_nVirtualMin)
			m_nVirtualCurPos = m_nVirtualMin;
		else if (m_nVirtualCurPos > m_nVirtualMax)
			m_nVirtualCurPos = m_nVirtualMax;

		if (m_nVirtualCurPos == nVirtualPosOld)
		{
			Invalidate();
			return;
		}

		m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);

		OnScroll(SB_THUMBPOSITION);

		Invalidate();
	}
}

bool CSkinSrollBarBase::OnMouseMove(UINT nFlags, CPoint point)
{
	assert(m_pSkin);

	switch (m_PushDownPos)
	{
	case PUSH_DOWN_TOPBT:
		// 滚动条上面的按钮
		TopBtOnMouseMove(nFlags, point);
		break;
	case PUSH_DOWN_BOTTOMBT:
		// 滚动条下面的按钮
		BottomBtOnMouseMove(nFlags, point);
		break;
	case PUSH_DOWN_THUMB:
		// 滚动条的拖动box(thumb)
		ThumbOnMouseMove(nFlags, point);
		break;
	case PUSH_DOWN_TOPTRACK:
	case PUSH_DOWN_BOTTOMTRACK:
		// 滚动条的空白位置，进行翻页操作
		TrackOnMouseMove(nFlags, point);
		break;
	case PUSH_DOWN_NONE:
		{
			PUSH_DOWN_POS		CursorPosNew;

			CursorPosNew = GetPushDownPos(point);

			// DBGLOG2(_T("CursorPosNew: %d, m_CursorPosLatest: %d"), CursorPosNew, m_CursorPosLatest);
			// if (m_pSkin->GetCaptureMouse() != this)
			// if (CursorPosNew != PUSH_DOWN_NONE && m_CursorPosLatest == PUSH_DOWN_NONE)
			if (m_pSkin->GetCaptureMouse() != this)
			{
				// 捕捉鼠标输入
				m_pSkin->SetCaptureMouse(this);
			}
			else if (CursorPosNew == PUSH_DOWN_NONE)
			{
				if (m_pSkin->GetCaptureMouse() == this)
				{
					m_pSkin->ReleaseCaptureMouse(this);
				}
			}

			if (CursorPosNew != m_CursorPosLatest)
			{
				m_CursorPosLatest = CursorPosNew;
				Invalidate();
			}
		}
		break;
	default:
		break;
	}

	return true;
}

void CSkinSrollBarBase::TopBtOnMouseMove(UINT nFlags, CPoint point)
{
	PUSH_DOWN_POS		CursorPosNew;

	CursorPosNew = GetPushDownPos(point);

	if (CursorPosNew != m_CursorPosLatest)
	{
		m_CursorPosLatest = CursorPosNew;
		Invalidate();
	}
}

void CSkinSrollBarBase::BottomBtOnMouseMove(UINT nFlags, CPoint point)
{
	PUSH_DOWN_POS		CursorPosNew;

	CursorPosNew = GetPushDownPos(point);

	if (CursorPosNew != m_CursorPosLatest)
	{
		m_CursorPosLatest = CursorPosNew;
		Invalidate();
	}
}

void CSkinSrollBarBase::TrackOnMouseMove(UINT nFlags, CPoint point)
{
	PUSH_DOWN_POS		CursorPosNew;

	CursorPosNew = GetPushDownPos(point);

	if (CursorPosNew != m_CursorPosLatest)
	{
		m_CursorPosLatest = CursorPosNew;
		Invalidate();
	}
}

void CSkinSrollBarBase::OnTimer(int nId)
{
	if (nId == 	m_nClickDelayTimerId)
	{
		m_pSkin->UnregisterTimerObject(this, m_nClickDelayTimerId);
		m_nClickDelayTimerId = 0;

		m_nRepeatTimerId = m_pSkin->RegisterTimerObject(this, 100);
	}
	else if (nId == m_nRepeatTimerId)
	{
		switch (m_PushDownPos)
		{
		case PUSH_DOWN_TOPBT:
			// 滚动条上面的按钮
			{
				if (m_CursorPosLatest == PUSH_DOWN_TOPBT)
				{
					// DBGLOG0(_T("Top button down: Timer..."));
					int		nVirtualPosOld;

					nVirtualPosOld = m_nVirtualCurPos;

					if (m_nVirtualCurPos > m_nVirtualMin)
						m_nVirtualCurPos -= m_nVirtualLine;
					if (m_nVirtualCurPos < m_nVirtualMin)
						m_nVirtualCurPos = m_nVirtualMin;

					if (m_nVirtualCurPos == nVirtualPosOld)
					{
						UnregisterTimer();
						return;
					}

					m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);

					OnScroll(SB_LINEUP);

					Invalidate();
				}
			}
			break;
		case PUSH_DOWN_BOTTOMBT:
			// 滚动条下面的按钮
			{
				if (m_CursorPosLatest == PUSH_DOWN_BOTTOMBT)
				{
					// DBGLOG0(_T("Bottom button down: Timer..."));

					int		nVirtualPosOld;

					nVirtualPosOld = m_nVirtualCurPos;

					if (m_nVirtualCurPos < m_nVirtualMax)
						m_nVirtualCurPos += m_nVirtualLine;
					if (m_nVirtualCurPos > m_nVirtualMax)
						m_nVirtualCurPos = m_nVirtualMax;

					if (m_nVirtualCurPos == nVirtualPosOld)
					{
						UnregisterTimer();
						return;
					}

					m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);

					OnScroll(SB_LINEDOWN);

					Invalidate();
				}
			}
			break;
		// case PUSH_DOWN_THUMB:
			// 滚动条的拖动box(thumb)
			// break;
		case PUSH_DOWN_TOPTRACK:
			// 滚动条的空白位置，进行翻页操作
			{
				if (m_CursorPosLatest == PUSH_DOWN_TOPTRACK)
				{
					// DBGLOG0(_T("Track up : Timer..."));

					int		nVirtualPosOld;

					nVirtualPosOld = m_nVirtualCurPos;

					if (m_nVirtualCurPos > m_nVirtualMin)
						m_nVirtualCurPos -= m_nVirtualPage;
					if (m_nVirtualCurPos < m_nVirtualMin)
						m_nVirtualCurPos = m_nVirtualMin;

					if (m_nVirtualCurPos == nVirtualPosOld)
					{
						UnregisterTimer();
						return;
					}

					m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);

					//
					// 取得新的位置
					CPoint	pt;

					m_pSkin->GetCursorClientPos(pt);
					m_CursorPosLatest = GetPushDownPos(pt);

					OnScroll(SB_PAGEUP);

					Invalidate();
				}
				break;
			}
		case PUSH_DOWN_BOTTOMTRACK:
			// 滚动条的空白位置，进行翻页操作
			{
				if (m_CursorPosLatest == PUSH_DOWN_BOTTOMTRACK)
				{
					// DBGLOG0(_T("Track down: Timer..."));

					int		nVirtualPosOld;

					nVirtualPosOld = m_nVirtualCurPos;

					if (m_nVirtualCurPos < m_nVirtualMax)
						m_nVirtualCurPos += m_nVirtualPage;
					if (m_nVirtualCurPos > m_nVirtualMax)
						m_nVirtualCurPos = m_nVirtualMax;

					// 按下后的坐标
					m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);

					if (m_nVirtualCurPos == nVirtualPosOld)
					{
						UnregisterTimer();
						return;
					}

					//
					// 取得新的位置
					CPoint	pt;

					m_pSkin->GetCursorClientPos(pt);
					m_CursorPosLatest = GetPushDownPos(pt);

					OnScroll(SB_LINEDOWN);

					Invalidate();
				}
			}
			break;
		}
	}
}

void CSkinSrollBarBase::RegisterTimer()
{
	m_nClickDelayTimerId = m_pSkin->RegisterTimerObject(this, 300);
	m_nRepeatTimerId = 0;
}

void CSkinSrollBarBase::UnregisterTimer()
{
	if (m_nClickDelayTimerId)
	{
		m_pSkin->UnregisterTimerObject(this, m_nClickDelayTimerId);
		m_nClickDelayTimerId = 0;
	}
	if (m_nRepeatTimerId)
	{
		m_pSkin->UnregisterTimerObject(this, m_nRepeatTimerId);
		m_nRepeatTimerId = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinVScrollBar, _T("VScrollBar"))

CSkinVScrollBar::CSkinVScrollBar()
{
	// 基本设置
	// m_nSBWidth = 15;			// 宽度
	// m_nHeight = 17 * 2 + 31;			// 高度
	m_nHeightPushBt = 17;	// 上、下按钮的高度
	m_nHeightThumb = 41;		// 滚动条的高度
	m_nHeightTrack = 56;
	m_nSBWidth = 15;
}

CSkinVScrollBar::~CSkinVScrollBar()
{
}

void CSkinVScrollBar::Draw(CRawGraph *pGraph)
{
	int			y;
	CSFImage	*pImg;

	if (m_strBmpFile.size() <= 0)
		return;

	// 1、绘窗口上面的按钮
	if (m_PushDownPos != PUSH_DOWN_TOPBT)
	{
		if (m_CursorPosLatest == PUSH_DOWN_TOPBT)
			pImg = &m_imgTopBtFocus;	// 光标 hover 在上面的按钮上
		else
			pImg = &m_imgTopBtNormal;	// 普通
	}
	else
	{
		if (m_CursorPosLatest == PUSH_DOWN_TOPBT)
			pImg = &m_imgTopBtPushDown;	// 按下状态
		else
			pImg = &m_imgTopBtNormal;	// 普通
	}

	y = m_rcObj.top;
	pImg->Blt(pGraph, m_rcObj.left, y);
	y += m_nHeightPushBt;


	// 2、绘窗口中的空白区域
	if (m_nPosThumb > 0)
	{
		m_imgTrack.YTileBlt(pGraph, m_rcObj.left, y, 
			m_rcObj.Width(), m_nPosThumb);
		y += m_nPosThumb;
	}


	// 3、绘窗口中间的 Thumb
	if (m_PushDownPos != PUSH_DOWN_THUMB)
	{
		if (m_CursorPosLatest == PUSH_DOWN_THUMB)
			pImg = &m_imgThumbFocus;	// 光标 hover 在上面的按钮上
		else
			pImg = &m_imgThumbNormal;	// 普通
	}
	else
	{
		if (m_CursorPosLatest == PUSH_DOWN_THUMB)
			pImg = &m_imgThumbPushDown;	// 按下状态
		else
			pImg = &m_imgThumbNormal;	// 普通
	}

	if (m_bStretchedThumb)
	{
		pImg->Blt(pGraph, m_rcObj.left, y, pImg->m_cx, MARGIN_THUMB, pImg->m_x, pImg->m_y);
		pImg->StretchBlt(pGraph, m_rcObj.left, y + MARGIN_THUMB, pImg->m_cx, m_nSizeThumbStretched - MARGIN_THUMB * 2, pImg->m_x, pImg->m_y + MARGIN_THUMB, pImg->m_cx, pImg->m_cy - MARGIN_THUMB * 2);
		pImg->Blt(pGraph, m_rcObj.left, y + m_nSizeThumbStretched - MARGIN_THUMB, pImg->m_cx, MARGIN_THUMB, pImg->m_x, pImg->m_y + pImg->m_cy - MARGIN_THUMB);
	}
	else
		pImg->Blt(pGraph, m_rcObj.left, y);
	y += m_nSizeThumbStretched;


	// 4、绘窗口中的空白区域
	if (m_rcObj.bottom - y - m_nHeightPushBt > 0)
	{
		m_imgTrack.YTileBlt(pGraph, m_rcObj.left, y, 
			m_rcObj.Width(), m_rcObj.bottom - y - m_nHeightPushBt);
		y = m_rcObj.top + m_rcObj.Height() - m_nHeightPushBt;
	}

	// 4、绘窗口下面的按钮
	if (m_PushDownPos != PUSH_DOWN_BOTTOMBT)
	{
		if (m_CursorPosLatest == PUSH_DOWN_BOTTOMBT)
			pImg = &m_imgBottomBtFocus;	// 光标 hover 在上面的按钮上
		else
			pImg = &m_imgBottomBtNormal;	// 普通
	}
	else
	{
		if (m_CursorPosLatest == PUSH_DOWN_BOTTOMBT)
			pImg = &m_imgBottomBtPushDown;	// 按下状态
		else
			pImg = &m_imgBottomBtNormal;	// 普通
	}

	pImg->Blt(pGraph, m_rcObj.left, y);
}

BOOL CSkinVScrollBar::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("PushBtHeight")) == 0)
		m_nHeightPushBt = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("ThumbHeight")) == 0)
		m_nSizeThumbStretched = m_nHeightThumb = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("ScalableThumb")) == 0)
		m_bStretchedThumb = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("TrackHeight")) == 0)
		m_nHeightTrack = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("ScrollBarWidth")) == 0)
		m_nSBWidth = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("LinesPerWheel")) == 0)
		m_nLinesPerWheel = _ttoi(szValue);
	else if (_tcsicmp(szProperty, SZ_PN_IMAGE) == 0)
	{
		// 图片
		m_strBmpFile = szValue;

		m_imgTopBtNormal.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgTopBtFocus.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgTopBtPushDown.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);

		m_imgBottomBtNormal.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgBottomBtFocus.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgBottomBtPushDown.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);

		m_imgThumbNormal.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgThumbFocus.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgThumbPushDown.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);

		m_imgTrack.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);

		// 根据前面设置的
		// 上面的按钮
		m_imgTopBtNormal.m_x = 0;
		m_imgTopBtPushDown.m_x = m_nSBWidth;
		m_imgTopBtFocus.m_x = m_nSBWidth * 2;
		m_imgTopBtPushDown.m_y = m_imgTopBtFocus.m_y = m_imgTopBtNormal.m_y = 0;
		m_imgTopBtPushDown.m_cx = m_imgTopBtFocus.m_cx = m_imgTopBtNormal.m_cx = m_nSBWidth;
		m_imgTopBtPushDown.m_cy = m_imgTopBtFocus.m_cy = m_imgTopBtNormal.m_cy = m_nHeightPushBt;

		// 下面的按钮
		m_imgBottomBtNormal.m_x = 0;
		m_imgBottomBtPushDown.m_x = m_nSBWidth;
		m_imgBottomBtFocus.m_x = m_nSBWidth * 2;
		m_imgBottomBtPushDown.m_y = m_imgBottomBtFocus.m_y = m_imgBottomBtNormal.m_y = m_nHeightPushBt + m_nHeightThumb + m_nHeightTrack;
		m_imgBottomBtPushDown.m_cx = m_imgBottomBtFocus.m_cx = m_imgBottomBtNormal.m_cx = m_nSBWidth;
		m_imgBottomBtPushDown.m_cy = m_imgBottomBtFocus.m_cy = m_imgBottomBtNormal.m_cy = m_nHeightPushBt;

		// 拖动的thumb
		m_imgThumbNormal.m_x = 0;
		m_imgThumbPushDown.m_x = m_nSBWidth;
		m_imgThumbFocus.m_x = m_nSBWidth * 2;
		m_imgThumbPushDown.m_y = m_imgThumbFocus.m_y = m_imgThumbNormal.m_y = m_nHeightPushBt;
		m_imgThumbPushDown.m_cx = m_imgThumbFocus.m_cx = m_imgThumbNormal.m_cx = m_nSBWidth;
		m_imgThumbPushDown.m_cy = m_imgThumbFocus.m_cy = m_imgThumbNormal.m_cy = m_nHeightThumb;

		// 空白的轨道Track
		m_imgTrack.m_x = 0;
		m_imgTrack.m_y = m_nHeightPushBt + m_nHeightThumb;
		m_imgTrack.m_cx = m_nSBWidth;
		m_imgTrack.m_cy = m_nHeightTrack;
	}

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinVScrollBar::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropInt(_T("PushBtHeight"), m_nHeightPushBt);
	listProperties.AddPropInt(_T("ThumbHeight"), m_nHeightThumb);
	listProperties.AddPropBoolStr(_T("ScalableThumb"), m_bStretchedThumb, m_bStretchedThumb);
	listProperties.AddPropInt(_T("TrackHeight"), m_nHeightTrack);
	listProperties.AddPropInt(_T("ScrollBarWidth"), m_nSBWidth);

	listProperties.AddPropImageFile(SZ_PN_IMAGE, m_strBmpFile.c_str());
}
#endif // _SKIN_EDITOR_

void CSkinVScrollBar::OnScroll(UINT nSBCode)
{
	if (m_pScrollNotify)
		m_pScrollNotify->OnVScroll(nSBCode, GetScrollPos(), this);
	else
		m_pSkin->OnVScroll(nSBCode, GetScrollPos(), this);
}

void CSkinVScrollBar::AdjustThumbSize()
{
	if (m_bStretchedThumb)
	{
		// Thumb is stretched?
		if (m_nVirtualMax - m_nVirtualMin + m_nVirtualPage == 0 || m_nVirtualMax - m_nVirtualMin <= 0)
			m_nSizeThumbStretched = m_rcObj.Height() - m_nHeightPushBt * 2;
		else
			m_nSizeThumbStretched = (m_rcObj.Height() - m_nHeightPushBt * 2) * m_nVirtualPage / (m_nVirtualMax - m_nVirtualMin + m_nVirtualPage);
		if (m_nSizeThumbStretched < MARGIN_THUMB * 2)
			m_nSizeThumbStretched = MARGIN_THUMB * 2;
	}
	else
		m_nSizeThumbStretched = m_nHeightThumb;
}

CSkinVScrollBar::PUSH_DOWN_POS CSkinVScrollBar::GetPushDownPos(CPoint pt)
{
	//assert(IsPtIn(pt));
	if (!IsPtIn(pt))
		return PUSH_DOWN_NONE;

	int		y;

	y = m_rcObj.top + m_nHeightPushBt;
	if (pt.y < y)
		return PUSH_DOWN_TOPBT;

	y += m_nPosThumb;
	if (pt.y < y)
		return PUSH_DOWN_TOPTRACK;

	y += m_nSizeThumbStretched;
	if (pt.y < y)
		return PUSH_DOWN_THUMB;

	y = m_rcObj.bottom - m_nHeightPushBt;
	if (pt.y < y)
		return PUSH_DOWN_BOTTOMTRACK;

	return PUSH_DOWN_BOTTOMBT;
}

void CSkinVScrollBar::ThumbOnLButtonDown(UINT nFlags, CPoint point)
{
	// 捕捉鼠标输入
	m_pSkin->SetCaptureMouse(this);

	// DBGLOG0(_T("Thumb button down"));

	m_nCursorToThumbBeg = point.y - (m_rcObj.top + m_nHeightPushBt + m_nPosThumb);

	Invalidate();
}

void CSkinVScrollBar::ThumbOnMouseMove(UINT nFlags, CPoint point)
{
	int		nVirtualPosOld;
	int		nPosThumbOld;

	nVirtualPosOld = m_nVirtualCurPos;
	nPosThumbOld = m_nPosThumb;

	m_nPosThumb = point.y - (m_rcObj.top + m_nHeightPushBt) - m_nCursorToThumbBeg;

	if (m_nPosThumb < 0)
		m_nPosThumb = 0;
	else if (m_nPosThumb > m_rcObj.Height() - m_nHeightPushBt * 2 - m_nSizeThumbStretched)
		m_nPosThumb = m_rcObj.Height() - m_nHeightPushBt * 2 - m_nSizeThumbStretched;

	// // DBGLOG1(_T("m_nPosThumb: %d"), m_nPosThumb);

	m_nVirtualCurPos = ObjectPosToVirtualPos(m_nPosThumb);

	if (m_nVirtualCurPos == nVirtualPosOld)
	{
		if (m_nPosThumb != nPosThumbOld)
			Invalidate();
		return;
	}

	OnScroll(SB_THUMBPOSITION);

	Invalidate();
}

int CSkinVScrollBar::VirtualPosToObjectPos(int nVirtualPos)
{
	// assert(m_nVirtualMax - m_nVirtualMin != 0);

	if (m_nVirtualMax - m_nVirtualMin == 0)
		return 0;

	return int((double)nVirtualPos * (m_rcObj.Height() - m_nHeightPushBt * 2 - m_nSizeThumbStretched)) / (m_nVirtualMax - m_nVirtualMin);
}

int CSkinVScrollBar::ObjectPosToVirtualPos(int nThumbPos)
{
	if (m_rcObj.Height() - m_nHeightPushBt * 2 - m_nSizeThumbStretched == 0)
		return 0;

	return int((double)nThumbPos * (m_nVirtualMax - m_nVirtualMin) / (m_rcObj.Height() - m_nHeightPushBt * 2 - m_nSizeThumbStretched));
}

//////////////////////////////////////////////////////////////////////
// CSkinHScrollBar
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinHScrollBar, _T("HScrollBar"))

CSkinHScrollBar::CSkinHScrollBar()
{
	m_nWidthPushBt = 17;	// 上、下按钮的高度
	m_nWidthThumb = 41;		// 滚动条的高度
	m_nWidthTrack = 56;
	m_nSBHeight = 15;
}

CSkinHScrollBar::~CSkinHScrollBar()
{
}

void CSkinHScrollBar::Draw(CRawGraph *pGraph)
{
	int			x;
	CSFImage	*pImg;

	if (m_strBmpFile.size() <= 0)
		return;

	// 1、绘窗口上面的按钮
	if (m_PushDownPos != PUSH_DOWN_TOPBT)
	{
		if (m_CursorPosLatest == PUSH_DOWN_TOPBT)
			pImg = &m_imgTopBtFocus;	// 光标 hover 在上面的按钮上
		else
			pImg = &m_imgTopBtNormal;	// 普通
	}
	else
	{
		if (m_CursorPosLatest == PUSH_DOWN_TOPBT)
			pImg = &m_imgTopBtPushDown;	// 按下状态
		else
			pImg = &m_imgTopBtNormal;	// 普通
	}

	x = m_rcObj.left;
	pImg->Blt(pGraph, x, m_rcObj.top);
	x += m_nWidthPushBt;


	// 2、绘窗口中的空白区域
	if (m_nPosThumb > 0)
	{
		m_imgTrack.XTileBlt(pGraph, x, m_rcObj.top,
			m_nPosThumb, m_rcObj.Height());
		x += m_nPosThumb;
	}


	// 3、绘窗口中间的 Thumb
	if (m_PushDownPos != PUSH_DOWN_THUMB)
	{
		if (m_CursorPosLatest == PUSH_DOWN_THUMB)
			pImg = &m_imgThumbFocus;	// 光标 hover 在上面的按钮上
		else
			pImg = &m_imgThumbNormal;	// 普通
	}
	else
	{
		if (m_CursorPosLatest == PUSH_DOWN_THUMB)
			pImg = &m_imgThumbPushDown;	// 按下状态
		else
			pImg = &m_imgThumbNormal;	// 普通
	}

	if (m_bStretchedThumb)
	{
		pImg->Blt(pGraph, x, m_rcObj.top, MARGIN_THUMB, pImg->m_cy, pImg->m_x, pImg->m_y);
		pImg->StretchBlt(pGraph, x + MARGIN_THUMB, m_rcObj.top, m_nSizeThumbStretched - MARGIN_THUMB * 2, pImg->m_cy, pImg->m_x + MARGIN_THUMB, pImg->m_y, pImg->m_cx - MARGIN_THUMB * 2, pImg->m_cy);
		pImg->Blt(pGraph, x + m_nSizeThumbStretched - MARGIN_THUMB, m_rcObj.top, MARGIN_THUMB, pImg->m_cy, pImg->m_x + pImg->m_cx - MARGIN_THUMB, pImg->m_y);
	}
	else
		pImg->Blt(pGraph, x, m_rcObj.top);
	x += m_nSizeThumbStretched;


	// 4、绘窗口中的空白区域
	if (m_rcObj.right - x - m_nWidthPushBt > 0)
	{
		m_imgTrack.XTileBlt(pGraph, x, m_rcObj.top,
			m_rcObj.right - x - m_nWidthPushBt, m_rcObj.Height());
		x = m_rcObj.right - m_nWidthPushBt;
	}

	// 4、绘窗口下面的按钮
	if (m_PushDownPos != PUSH_DOWN_BOTTOMBT)
	{
		if (m_CursorPosLatest == PUSH_DOWN_BOTTOMBT)
			pImg = &m_imgBottomBtFocus;	// 光标 hover 在上面的按钮上
		else
			pImg = &m_imgBottomBtNormal;	// 普通
	}
	else
	{
		if (m_CursorPosLatest == PUSH_DOWN_BOTTOMBT)
			pImg = &m_imgBottomBtPushDown;	// 按下状态
		else
			pImg = &m_imgBottomBtNormal;	// 普通
	}

	pImg->Blt(pGraph, x, m_rcObj.top);
}

BOOL CSkinHScrollBar::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("PushBtWidth")) == 0)
		m_nWidthPushBt = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("ThumbWidth")) == 0)
		m_nSizeThumbStretched = m_nWidthThumb = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("ScalableThumb")) == 0)
		m_bStretchedThumb = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("TrackWidth")) == 0)
		m_nWidthTrack = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("ScrollBarHeight")) == 0)
		m_nSBHeight = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("LinesPerWheel")) == 0)
		m_nLinesPerWheel = _ttoi(szValue);
	else if (_tcsicmp(szProperty, SZ_PN_IMAGE) == 0)
	{
		// 图片
		m_strBmpFile = szValue;

		m_imgTopBtNormal.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgTopBtFocus.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgTopBtPushDown.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);

		m_imgBottomBtNormal.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgBottomBtFocus.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgBottomBtPushDown.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);

		m_imgThumbNormal.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgThumbFocus.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgThumbPushDown.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);

		m_imgTrack.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);

		// 根据前面设置的
		// 上面的按钮
		m_imgTopBtNormal.m_y = 0;
		m_imgTopBtPushDown.m_y = m_nSBHeight;
		m_imgTopBtFocus.m_y = m_nSBHeight * 2;
		m_imgTopBtPushDown.m_x = m_imgTopBtFocus.m_x = m_imgTopBtNormal.m_x = 0;
		m_imgTopBtPushDown.m_cy = m_imgTopBtFocus.m_cy = m_imgTopBtNormal.m_cy = m_nSBHeight;
		m_imgTopBtPushDown.m_cx = m_imgTopBtFocus.m_cx = m_imgTopBtNormal.m_cx = m_nWidthPushBt;

		// 下面的按钮
		m_imgBottomBtNormal.m_y = 0;
		m_imgBottomBtPushDown.m_y = m_nSBHeight;
		m_imgBottomBtFocus.m_y = m_nSBHeight * 2;
		m_imgBottomBtPushDown.m_x = m_imgBottomBtFocus.m_x = m_imgBottomBtNormal.m_x = m_nWidthPushBt + m_nWidthThumb + m_nWidthTrack;
		m_imgBottomBtPushDown.m_cy = m_imgBottomBtFocus.m_cy = m_imgBottomBtNormal.m_cy = m_nSBHeight;
		m_imgBottomBtPushDown.m_cx = m_imgBottomBtFocus.m_cx = m_imgBottomBtNormal.m_cx = m_nWidthPushBt;

		// 拖动的thumb
		m_imgThumbNormal.m_y = 0;
		m_imgThumbPushDown.m_y = m_nSBHeight;
		m_imgThumbFocus.m_y = m_nSBHeight * 2;
		m_imgThumbPushDown.m_x = m_imgThumbFocus.m_x = m_imgThumbNormal.m_x = m_nWidthPushBt;
		m_imgThumbPushDown.m_cy = m_imgThumbFocus.m_cy = m_imgThumbNormal.m_cy = m_nSBHeight;
		m_imgThumbPushDown.m_cx = m_imgThumbFocus.m_cx = m_imgThumbNormal.m_cx = m_nWidthThumb;

		// 空白的轨道Track
		m_imgTrack.m_y = 0;
		m_imgTrack.m_x = m_nWidthPushBt + m_nWidthThumb;
		m_imgTrack.m_cy = m_nSBHeight;
		m_imgTrack.m_cx = m_nWidthTrack;
	}

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinHScrollBar::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropInt(_T("PushBtWidth"), m_nWidthPushBt);
	listProperties.AddPropInt(_T("ThumbWidth"), m_nWidthThumb);
	listProperties.AddPropBoolStr(_T("ScalableThumb"), m_bStretchedThumb, m_bStretchedThumb);
	listProperties.AddPropInt(_T("TrackWidth"), m_nWidthTrack);
	listProperties.AddPropInt(_T("ScrollBarHeight"), m_nSBHeight);

	listProperties.AddPropImageFile(SZ_PN_IMAGE, m_strBmpFile.c_str());
}
#endif // _SKIN_EDITOR_

void CSkinHScrollBar::OnScroll(UINT nSBCode)
{
	if (m_pScrollNotify)
		m_pScrollNotify->OnHScroll(nSBCode, GetScrollPos(), this);
	else
		m_pSkin->OnHScroll(nSBCode, GetScrollPos(), this);
}

void CSkinHScrollBar::AdjustThumbSize()
{
	if (m_bStretchedThumb)
	{
		// Thumb is stretched?
		if (m_nVirtualMax - m_nVirtualMin + m_nVirtualPage == 0 || m_nVirtualMax - m_nVirtualMin <= 0)
			m_nSizeThumbStretched = m_rcObj.Width() - m_nWidthPushBt * 2;
		else
			m_nSizeThumbStretched = (m_rcObj.Width() - m_nWidthPushBt * 2) * m_nVirtualPage / (m_nVirtualMax - m_nVirtualMin + m_nVirtualPage);
		if (m_nSizeThumbStretched < MARGIN_THUMB * 2)
			m_nSizeThumbStretched = MARGIN_THUMB * 2;
	}
	else
		m_nSizeThumbStretched = m_nWidthThumb;
}

CSkinHScrollBar::PUSH_DOWN_POS CSkinHScrollBar::GetPushDownPos(CPoint pt)
{
	//assert(IsPtIn(pt));
	if (!IsPtIn(pt))
		return PUSH_DOWN_NONE;

	int		x;

	x = m_rcObj.left + m_nWidthPushBt;
	if (pt.x < x)
		return PUSH_DOWN_TOPBT;

	x += m_nPosThumb;
	if (pt.x < x)
		return PUSH_DOWN_TOPTRACK;

	x += m_nSizeThumbStretched;
	if (pt.x < x)
		return PUSH_DOWN_THUMB;

	x = m_rcObj.right - m_nWidthPushBt;
	if (pt.x < x)
		return PUSH_DOWN_BOTTOMTRACK;

	return PUSH_DOWN_BOTTOMBT;
}

void CSkinHScrollBar::ThumbOnLButtonDown(UINT nFlags, CPoint point)
{
	// 捕捉鼠标输入
	m_pSkin->SetCaptureMouse(this);

	// DBGLOG0(_T("Thumb button down"));

	m_nCursorToThumbBeg = point.x - (m_rcObj.left + m_nWidthPushBt + m_nPosThumb);

	Invalidate();
}

void CSkinHScrollBar::ThumbOnMouseMove(UINT nFlags, CPoint point)
{
	int		nVirtualPosOld;
	int		nPosThumbOld;
	// DBGLOG2(_T("Drag Thumb OK! x: %d, y: %d"), point.x, point.y);

	nVirtualPosOld = m_nVirtualCurPos;
	nPosThumbOld = m_nPosThumb;

	m_nPosThumb = point.x - (m_rcObj.left + m_nWidthPushBt) - m_nCursorToThumbBeg;

	if (m_nPosThumb < 0)
		m_nPosThumb = 0;
	else if (m_nPosThumb > m_rcObj.Width() - m_nWidthPushBt * 2 - m_nSizeThumbStretched)
		m_nPosThumb = m_rcObj.Width() - m_nWidthPushBt * 2 - m_nSizeThumbStretched;

	m_nVirtualCurPos = ObjectPosToVirtualPos(m_nPosThumb);

	if (m_nVirtualCurPos == nVirtualPosOld)
	{
		if (m_nPosThumb != nPosThumbOld)
			Invalidate();
		return;
	}

	OnScroll(SB_THUMBPOSITION);

	Invalidate();
}

int CSkinHScrollBar::VirtualPosToObjectPos(int nVirtualPos)
{
	// assert(m_nVirtualMax - m_nVirtualMin != 0);

	if (m_nVirtualMax - m_nVirtualMin == 0)
		return 0;

	return int((double)nVirtualPos * (m_rcObj.Width() - m_nWidthPushBt * 2 - m_nSizeThumbStretched)) / (m_nVirtualMax - m_nVirtualMin);
}

int CSkinHScrollBar::ObjectPosToVirtualPos(int nThumbPos)
{
	if (m_rcObj.Width() - m_nWidthPushBt * 2 - m_nSizeThumbStretched == 0)
		return 0;

	return int((double)nThumbPos * (m_nVirtualMax - m_nVirtualMin) / (m_rcObj.Width() - m_nWidthPushBt * 2 - m_nSizeThumbStretched));
}
