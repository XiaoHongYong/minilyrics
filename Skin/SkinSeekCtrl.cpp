// SkinSeekCtrl.cpp: implementation of the CSkinSeekCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinSeekCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// set classname
UIOBJECT_CLASS_NAME_IMP(CSkinSeekCtrl, _T("SeekCtrl"));

CSkinSeekCtrl::CSkinSeekCtrl()
{
	// 基本设置
	m_nWidthEnd = 17;	// 上、下按钮的高度
	m_nWidthThumb = 41;		// 滚动条的高度
	m_nPosThumb = 0;		// 滚动条的位置(上面按钮的下方 －> thumb的上方的距离)

	// 虚拟滚动位置
	m_nVirtualMin = 0;		// 最小的值
	m_nVirtualMax = 10;		// 最大的值
	m_nVirtualLine = 1;
	m_nVirtualPage = 10;
	m_nVirtualCurPos = m_nVirtualMin;

	m_nMsgWant = UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_MOUSEWHEEL;

	m_nXCursorToThumbBeg = 0;
	m_PushDownPos = PUSH_DOWN_NONE;
	m_CursorPosLatest = PUSH_DOWN_NONE;

	m_pScrollNotify = NULL;
}

CSkinSeekCtrl::~CSkinSeekCtrl()
{
}

void CSkinSeekCtrl::OnKeyDown(UINT nChar, UINT nFlags)
{
	if (nChar == VK_UP)
		SetScrollPos(GetScrollPos() - m_nVirtualLine);
	else if (nChar == VK_DOWN)
		SetScrollPos(GetScrollPos() + m_nVirtualLine);
	else if (nChar == VK_LEFT || nChar == VK_PRIOR)
		SetScrollPos(GetScrollPos() - m_nVirtualPage);
	else if (nChar == VK_RIGHT || nChar == VK_NEXT)
		SetScrollPos(GetScrollPos() + m_nVirtualPage);

	OnVScroll(SB_THUMBPOSITION);
}

bool CSkinSeekCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	assert(m_pSkin);

	m_PushDownPos = GetPushDownPos(point);
	m_CursorPosLatest = m_PushDownPos;

	switch (m_PushDownPos)
	{
	case PUSH_DOWN_THUMB:
		// 滚动条的拖动box(thumb)
		ThumbOnLButtonDown(nFlags, point);
		break;
	case PUSH_DOWN_TOPTRACK:
	case PUSH_DOWN_BOTTOMTRACK:
		// 滚动条的空白位置，进行seek操作
		TrackOnLButtonDown(nFlags, point);
		break;
	}

	return true;
}

void CSkinSeekCtrl::ThumbOnLButtonDown(UINT nFlags, CPoint point)
{
	// 捕捉鼠标输入
	m_pSkin->SetCaptureMouse(this);

	m_nXCursorToThumbBeg = point.x - (m_rcObj.left + m_nPosThumb);

	Invalidate();

	NotifyEvent(CSkinSeekCtrlEventNotify::C_BEG_DRAG);
}

void CSkinSeekCtrl::TrackOnLButtonDown(UINT nFlags, CPoint point)
{
	// 捕捉鼠标输入
	m_pSkin->SetCaptureMouse(this);

	int		nVirtualPosOld;
	int		nPosThumbOld;

	nVirtualPosOld = m_nVirtualCurPos;
	nPosThumbOld = m_nPosThumb;

	m_nPosThumb = point.x - m_rcObj.left;
	if (m_PushDownPos == PUSH_DOWN_BOTTOMTRACK)
		m_nPosThumb -= m_nWidthThumb / 2;
	else
		m_nPosThumb -= m_nWidthThumb / 2;

	if (m_nPosThumb < 0)
		m_nPosThumb = 0;
	else if (m_nPosThumb > GetTrackRealWidth())
		m_nPosThumb = GetTrackRealWidth();

	m_nVirtualCurPos = ObjectPosToVirtualPos(m_nPosThumb);

	m_CursorPosLatest = m_PushDownPos = PUSH_DOWN_THUMB;

	m_nXCursorToThumbBeg = point.x - (m_rcObj.left + m_nPosThumb);

	Invalidate();

	NotifyEvent(CSkinSeekCtrlEventNotify::C_BEG_DRAG);

	OnVScroll(SB_THUMBPOSITION);
}

bool CSkinSeekCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	// DBGLOG0(_T("OnLButtonUp "));
	switch (m_PushDownPos)
	{
	case PUSH_DOWN_THUMB:
		{
			// 滚动条的拖动box(thumb)
			m_pSkin->ReleaseCaptureMouse(this);

			OnVScroll(SB_THUMBPOSITION);
			m_pSkin->OnCustomCommand(m_nId);

			NotifyEvent(CSkinSeekCtrlEventNotify::C_END_DRAG);
		}
		break;
	}

	m_PushDownPos = PUSH_DOWN_NONE;

	PUSH_DOWN_POS		CursorPosNew;

	CursorPosNew = GetPushDownPos(point);
// 	if (CursorPosNew != PUSH_DOWN_NONE)
// 		m_pSkin->SetCaptureMouse(this);

	Invalidate();

	return true;
}

void CSkinSeekCtrl::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	if (nMkeys == 0)
	{
		int		nVirtualPosOld;

		nVirtualPosOld = m_nVirtualCurPos;

		if (nWheelDistance > 0)
		{
			if (m_nVirtualCurPos > m_nVirtualMin)
				m_nVirtualCurPos -= m_nVirtualLine;
			if (m_nVirtualCurPos < m_nVirtualMin)
				m_nVirtualCurPos = m_nVirtualMin;

		}
		else
		{
			if (m_nVirtualCurPos < m_nVirtualMax)
				m_nVirtualCurPos += m_nVirtualLine;
			if (m_nVirtualCurPos > m_nVirtualMax)
				m_nVirtualCurPos = m_nVirtualMax;
		}

		if (m_nVirtualCurPos == nVirtualPosOld)
			return;

		m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);

		OnVScroll(nWheelDistance > 0 ? SB_LINEDOWN : SB_LINEUP);
		Invalidate();

		m_pSkin->OnCustomCommand(m_nId);
	}
}

bool CSkinSeekCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	assert(m_pSkin);

	switch (m_PushDownPos)
	{
	case PUSH_DOWN_THUMB:
		// 滚动条的拖动box(thumb)
		ThumbOnMouseMove(nFlags, point);
		break;
	case PUSH_DOWN_NONE:
		{
			PUSH_DOWN_POS		CursorPosNew;

			CursorPosNew = GetPushDownPos(point);

			// DBGLOG2(_T("CursorPosNew: %d, m_CursorPosLatest: %d"), CursorPosNew, m_CursorPosLatest);
			if (m_pSkin->GetCaptureMouse() != this)
			{
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
				// DBGLOG0(_T("Invalidate!"));
			}
		}
		break;
	default:
		break;
	}

	return true;
}

void CSkinSeekCtrl::ThumbOnMouseMove(UINT nFlags, CPoint point)
{
	int		nVirtualPosOld;
	int		nPosThumbOld;

	nVirtualPosOld = m_nVirtualCurPos;
	nPosThumbOld = m_nPosThumb;

	m_nPosThumb = point.x - m_rcObj.left - m_nXCursorToThumbBeg;

	if (m_nPosThumb < 0)
		m_nPosThumb = 0;
	else if (m_nPosThumb > GetTrackRealWidth())
		m_nPosThumb = GetTrackRealWidth();

	// // DBGLOG1(_T("m_nPosThumb: %d"), m_nPosThumb);

	m_nVirtualCurPos = ObjectPosToVirtualPos(m_nPosThumb);

	if (m_nVirtualCurPos == nVirtualPosOld)
	{
		if (m_nPosThumb != nPosThumbOld)
			Invalidate();
		return;
	}

	OnVScroll(SB_THUMBPOSITION);

	Invalidate();
}

void CSkinSeekCtrl::Draw(CRawGraph *pGraph)
{
	if (m_imgSimple.IsValid())
	{
		int		nHeight = m_imgSimple.height() / 2;

		m_imgSimple.Blt(pGraph, m_rcObj.left, m_rcObj.top, m_nPosThumb, nHeight, 0, 0);

		m_imgSimple.Blt(pGraph, m_rcObj.left + m_nPosThumb, m_rcObj.top, m_rcObj.Width() - m_nPosThumb, nHeight, m_nPosThumb, nHeight);

		return;
	}

	int		nTrackHeight = m_imgTrackNormal.height();
	int		nThumbHeight = m_imgThumb.height() / 3;

	int			x, y;
	x = m_rcObj.left;
	y = (m_rcObj.top + m_rcObj.bottom - nTrackHeight) / 2;

	if (m_imgTrackPlayed.IsValid())
	{
		// Draw left
		CRawGraph::CClipBoxAutoRecovery	clipAR(pGraph);

		pGraph->SetClipBoundBox(CRect(x, y, x + m_nPosThumb, y + nTrackHeight));
		m_imgTrackPlayed.XScaleBlt(pGraph, x, y, 
			m_rcObj.Width(), nTrackHeight, 
			m_nWidthEnd, m_imgTrackPlayed.width() - m_nWidthEnd,
			true, BPM_BLEND);
	}

	if (m_imgTrackNormal.IsValid())
	{
		// Draw right
		CRawGraph::CClipBoxAutoRecovery	clipAR(pGraph);

		pGraph->SetClipBoundBox(CRect(x + m_nPosThumb, y, m_rcObj.right, y + nTrackHeight));
		m_imgTrackNormal.XScaleBlt(pGraph, x, y, 
			m_rcObj.Width(), nTrackHeight, 
			m_nWidthEnd, m_imgTrackNormal.width() - m_nWidthEnd,
			true, BPM_BLEND);
	}

	// Draw thumb
	if (m_imgThumb.IsValid())
	{
		int		yThumbSrc = 0;
		if (m_PushDownPos != PUSH_DOWN_THUMB)
		{
			if (m_CursorPosLatest == PUSH_DOWN_THUMB)
				yThumbSrc = nThumbHeight; // Hover on thumb.
		}
		else
		{
			if (m_CursorPosLatest == PUSH_DOWN_THUMB)
				yThumbSrc = nThumbHeight * 2; // Push down on thumb.
		}

		m_imgThumb.Blt(pGraph, m_rcObj.left + m_nPosThumb, (m_rcObj.top + m_rcObj.bottom - nThumbHeight) / 2,
			m_imgThumb.m_cx, nThumbHeight, 
			0, yThumbSrc, BPM_BLEND);
	}
}

BOOL CSkinSeekCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("EndWidth")) == 0)
		m_nWidthEnd = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("ImageSimple")) == 0)
		m_imgSimple.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	else if (IsPropertyName(szProperty, _T("ImageTrack")))
	{
		m_strImageTrack = szValue;

		m_imgTrackNormal.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imgTrackPlayed.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);

		m_imgTrackNormal.m_cy = m_imgTrackPlayed.m_cy = m_imgTrackNormal.m_cy / 2;
		m_imgTrackPlayed.m_y = m_imgTrackNormal.m_cy;
	}
	else if (IsPropertyName(szProperty, _T("ImageThumb")))
	{
		m_strImageThumb = szValue;

		m_imgThumb.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_nWidthThumb = m_imgThumb.width();
	}
	else
		return false;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinSeekCtrl::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropInt(_T("EndWidth"), m_nWidthEnd);
	listProperties.AddPropImageFile(SZ_PN_IMAGE, m_strBmpFile.c_str());
}
#endif // _SKIN_EDITOR_

// COMMENT:
//		设置新的滚动位置
// RETURN:
//		返回设置以前的滚动位置的值
int CSkinSeekCtrl::SetScrollPos(int nPos, bool bRedraw)
{
	int		nPosOld, nPosThumbOld;

	if (m_PushDownPos == PUSH_DOWN_THUMB)
		return m_nVirtualCurPos;

	nPosThumbOld = m_nPosThumb;
	nPosOld = m_nVirtualCurPos;
	m_nVirtualCurPos = nPos;

	if (m_nVirtualCurPos > m_nVirtualMax)
		m_nVirtualCurPos = m_nVirtualMax;
	else if (m_nVirtualCurPos < m_nVirtualMin)
		m_nVirtualCurPos = m_nVirtualMin;

	m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);

	if (bRedraw && m_nPosThumb != nPosThumbOld)
		Invalidate();

	return nPosOld;
}

void CSkinSeekCtrl::SetScrollInfo(int nMin, int nMax, int nPage, int nPos, int nLine, bool bRedraw)
{
	m_nVirtualMin = nMin;
	m_nVirtualMax = nMax;
	m_nVirtualLine = nLine;
	m_nVirtualPage = nPage;

	m_nVirtualCurPos = nPos;

	if (m_nVirtualCurPos > m_nVirtualMax)
		m_nVirtualCurPos = m_nVirtualMax;
	else if (m_nVirtualCurPos < m_nVirtualMin)
		m_nVirtualCurPos = m_nVirtualMin;

	m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);

	if (bRedraw)
		Invalidate();
}

int CSkinSeekCtrl::GetMax() const
{
	return m_nVirtualMax;
}

void CSkinSeekCtrl::SetScrollNotify(IScrollNotify *pNofity)
{
	m_pScrollNotify = pNofity;
}

CSkinSeekCtrl::PUSH_DOWN_POS CSkinSeekCtrl::GetPushDownPos(CPoint pt)
{
	//assert(IsPtIn(pt));
	if (!IsPtIn(pt))
		return PUSH_DOWN_NONE;

	int		x;

	x = m_rcObj.left + m_nPosThumb;
	if (pt.x < x)
		return PUSH_DOWN_TOPTRACK;

	x += m_nWidthThumb;
	if (pt.x < x)
		return PUSH_DOWN_THUMB;

	return PUSH_DOWN_BOTTOMTRACK;
}

int CSkinSeekCtrl::VirtualPosToObjectPos(int nVirtualPos)
{
	if (m_nVirtualMax - m_nVirtualMin == 0)
		return 0;

	return int((double)nVirtualPos * (GetTrackRealWidth())) / (m_nVirtualMax - m_nVirtualMin);
}

int CSkinSeekCtrl::ObjectPosToVirtualPos(int nThumbPos)
{
	assert(GetTrackRealWidth() != 0);

	if (GetTrackRealWidth() == 0)
		return 0;

	return int((double)nThumbPos * (m_nVirtualMax - m_nVirtualMin) / (GetTrackRealWidth()));
}

// COMMENTS:
//		滚动后的大小，其大小通过 GetScrollPos() 取得
// INPUT:
//		nSBCode:
//		SB_BOTTOM   Scroll to bottom.
//		SB_ENDSCROLL   End scroll.
//		SB_LINEDOWN   Scroll one line down.
//		SB_LINEUP   Scroll one line up.
//		SB_PAGEDOWN   Scroll one page down.
//		SB_PAGEUP   Scroll one page up.
//		SB_THUMBPOSITION   Scroll to the absolute position. The current position is provided in nPos.
//		SB_THUMBTRACK   Drag scroll box to specified position. The current position is provided in nPos.
//		SB_TOP   Scroll to top. 
void CSkinSeekCtrl::OnVScroll(UINT nSBCode)
{
	if (m_pScrollNotify)
		m_pScrollNotify->OnVScroll(nSBCode, GetScrollPos(), this);
	else
		m_pSkin->OnVScroll(nSBCode, GetScrollPos(), this);
}

void CSkinSeekCtrl::OnSize()
{
	CUIObject::OnSize();

	m_nPosThumb = VirtualPosToObjectPos(m_nVirtualCurPos);
}

void CSkinSeekCtrl::NotifyEvent(CSkinSeekCtrlEventNotify::Command cmd)
{
	CSkinSeekCtrlEventNotify	event(this);

	event.cmd = cmd;
	m_pSkin->DispatchUIObjNotify(&event);
}
