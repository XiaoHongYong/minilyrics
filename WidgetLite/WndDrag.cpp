/********************************************************************
	Created :	2001/12/17    0:49
	FileName:	wnddrag.cpp
	Author  :	xhy
	Purpose :	
*********************************************************************/

#include "StdAfx.h"
#include "WidgetLite.h"
#include "WndDrag.h"

int HorzLineCloseToLine(int lsx1, int lsx2, int lsy, int ldx1, int ldx2, int ldy, int nCloseExt)
{
	int nOffy = 0;
	if (abs(lsy - ldy) < nCloseExt)
	{
		int lx, rx, cx;

		if (lsx1 < ldx1)
			lx = lsx1;
		else
			lx = ldx1;

		if (lsx2 > ldx2)
			rx = lsx2;
		else
			rx = ldx2;
		cx = lsx2 - lsx1 + ldx2 - ldx1;

		if (rx - lx <= cx + nCloseExt)
			nOffy = ldy - lsy;
	}
	return nOffy;
}

void VertLineCloseToRect(int nSrcY1, int nSrcY2, int nSrcX, CRect &rcd, int &offx, int nCloseExt)
{
	offx = HorzLineCloseToLine(nSrcY1, nSrcY2, nSrcX,
		rcd.top, rcd.bottom, rcd.left, nCloseExt);
	if (offx == 0)
	{
		offx = HorzLineCloseToLine(nSrcY1, nSrcY2, nSrcX,
			rcd.top, rcd.bottom, rcd.right, nCloseExt);
	}
}

void HorzLineCloseToRect(int nSrcX1, int nSrcX2, int nSrcY, CRect &rcd, int &offy, int nCloseExt)
{
	offy = HorzLineCloseToLine(nSrcX1, nSrcX2, nSrcY,
		rcd.left, rcd.right, rcd.top, nCloseExt);
	if (offy == 0)
	{
		offy = HorzLineCloseToLine(nSrcX1, nSrcX2, nSrcY,
			rcd.left, rcd.right, rcd.bottom, nCloseExt);
	}
}

void RectCloseToRect(CRect &rcs, CRect &rcd, int &offx, int &offy, int nCloseExt)
{
	offx = 0, offy = 0;
	offy = HorzLineCloseToLine(rcs.left, rcs.right, rcs.top,
		rcd.left, rcd.right, rcd.top, nCloseExt);
	if (offy == 0)
	{
		offy = HorzLineCloseToLine(rcs.left, rcs.right, rcs.top,
			rcd.left, rcd.right, rcd.bottom, nCloseExt);
		if (offy == 0)
		{
			offy = HorzLineCloseToLine(rcs.left, rcs.right, rcs.bottom,
				rcd.left, rcd.right, rcd.bottom, nCloseExt);
			if (offy == 0)
				offy = HorzLineCloseToLine(rcs.left, rcs.right, rcs.bottom,
					rcd.left, rcd.right, rcd.top, nCloseExt);
		}
	}

	offx = HorzLineCloseToLine(rcs.top, rcs.bottom, rcs.left,
		rcd.top, rcd.bottom, rcd.left, nCloseExt);
	if (offx == 0)
	{
		offx = HorzLineCloseToLine(rcs.top, rcs.bottom, rcs.left,
			rcd.top, rcd.bottom, rcd.right, nCloseExt);
		if (offx == 0)
		{
			offx = HorzLineCloseToLine(rcs.top, rcs.bottom, rcs.right,
				rcd.top, rcd.bottom, rcd.right, nCloseExt);
			if (offx == 0)
				offx = HorzLineCloseToLine(rcs.top, rcs.bottom, rcs.right,
					rcd.top, rcd.bottom, rcd.left, nCloseExt);
		}
	}
}

void RectCloseToWindow(CRect &rcs, CBaseWnd *pWnd, int nCloseExt, int &offx, int &offy)
{
	CRect rcd;
	offx = 0;
	offy = 0;
	if (pWnd)
	{
		if (pWnd->IsVisible())
		{
			pWnd->GetWindowRect(&rcd);
			RectCloseToRect(rcs, rcd, offx, offy, nCloseExt);
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWndDrag::CWndDrag()
{
	m_bEnableDrag = TRUE;

    m_bDragWnd = FALSE;

    m_bDragAutoCloseto = TRUE;

    m_pWnd = NULL;

	// m_arrszWndClassCloseTo = NULL;
	// m_pvWndClassCloseTo = NULL;

	m_bSticked = FALSE;
	m_pWndToTrack = NULL;

	m_ptWndTrack.x = 0;
	m_ptWndTrack.y = 0;
}

CWndDrag::~CWndDrag()
{

}

void CWndDrag::OnDrag(UINT fwKeys, POINT pt)
{
	if (!m_bEnableDrag)
		return;

	if (fwKeys & MK_LBUTTON)
	{
		MLWidget::GetCursorPos(&pt);
		// DBGLOG2(_T("Drag, cursor x: %d, y: %d"), pt.x, pt.y);
		if (m_bDragWnd)
		{
			// dragging
			// move window
			if ( (pt.x - m_ptDragOld.x != 0) || (pt.y - m_ptDragOld.y != 0))
			{
				if (m_bDragAutoCloseto)
				{
					int nOffx, nOffy;	

					nOffx = pt.x - m_ptDragOld.x;
					nOffy = pt.y - m_ptDragOld.y;
					AutoCloseToWindows(nOffx, nOffy);
					m_ptDragOld.y = pt.y + nOffy;
					m_ptDragOld.x = pt.x + nOffx;
				}
				else
				{
					RECT		rcWnd;
					m_pWnd->GetWindowRect(&rcWnd);
					SetWindowPosSafely(rcWnd.left, rcWnd.top, pt.x - m_ptDragOld.x, pt.y - m_ptDragOld.y);

					m_ptDragOld.y = pt.y;
					m_ptDragOld.x = pt.x;
				}
			}
		}
		else
		{
			if (m_pWnd->IsZoomed())
				return;

			// begin drag
			m_bDragWnd = TRUE;
			m_ptDragOld.x = pt.x;
			m_ptDragOld.y = pt.y;
			m_pWnd->SetCapture();
   		}
	}
	else if (m_bDragWnd)
	{
		// drag end
		m_bDragWnd = FALSE;
		m_pWnd->ReleaseCapture();
	}
}

void CWndDrag::SetWindowPosSafely(int xOld, int yOld, int nOffsetx, int nOffsety)
{
	m_pWnd->SetWindowPosSafely(xOld + nOffsetx, yOld + nOffsety);
}

BOOL CWndDrag::AutoCloseToWindows(int &nOffx, int &nOffy, BOOL bMoveWindow /* = TRUE */)
{
	CRect		rcw, rcd;
	CBaseWnd	*pWnd;

    m_pWnd->GetWindowRect(&rcw);

	rcw.OffsetRect(nOffx, nOffy);
	nOffx = 0;
	nOffy = 0;

	// Move Window close to winamp window and desktop and tray_window
	for (UINT i = 0; i < m_vWndCloseTo.size(); i ++)
	{
		if (nOffx == 0 || nOffy == 0)
		{
			int nOffxt, nOffyt;
			// Get Window close to rect
			if (m_vWndCloseTo[i].pWnd)
				pWnd = m_vWndCloseTo[i].pWnd;
			else
			{
				const TCHAR	*szClass = m_vWndCloseTo[i].strClass.c_str();
				if (IsEmptyString(szClass))
					szClass = NULL;
				const TCHAR	*szWnd = m_vWndCloseTo[i].strWndName.c_str();
				if (IsEmptyString(szWnd))
					szWnd = NULL;
				pWnd = MLWidget::FindWindow(szClass, szWnd);
			}
			if (pWnd && !pWnd->IsSameWnd(m_pWnd))
			{
				RectCloseToWindow(rcw, pWnd, 10, nOffxt, nOffyt);
				if (nOffx == 0)
					nOffx = nOffxt;
				if (nOffy == 0)
					nOffy = nOffyt;
			}
		}
		else
			break;
	}

	if (nOffx == 0 || nOffy == 0)
	{
		int nOffxt, nOffyt;
		CRect rcd;
		GetMonitorRestrictRect(rcw, rcd);
		// close to monitor
		RectCloseToRect(rcw, rcd, nOffxt, nOffyt, 10);
		if (nOffx == 0)
			nOffx = nOffxt;
		if (nOffy == 0)
			nOffy = nOffyt;
	}
	// ok 
	rcw.OffsetRect(nOffx, nOffy);

    CRect       rc;
    m_pWnd->GetWindowRect(&rc);
	if (rc.EqualRect(&rcw))
	{
		// no move
	}
	else
	{
		// move to new rect
		if (bMoveWindow)
		{
			SetWindowPosSafely(rc.left, rc.top, nOffx, nOffy);
		}

		return TRUE;
	}

	return FALSE;
}

void CWndDrag::Init(CBaseWnd *pWnd)
{
    m_pWnd = pWnd;

	m_pWndToTrack = NULL;
}

void CWndDrag::AddWndCloseto(CBaseWnd *pWnd, LPCTSTR szWndClass, LPCTSTR szWndName)
{
	WndCloseTo	CloseTo;

	if (pWnd)
	{
		CloseTo.pWnd = pWnd;
		CloseTo.strClass = _T("");
		CloseTo.strWndName = _T("");
		for (UINT i = 0; i < m_vWndCloseTo.size(); i++)
		{
			if (m_vWndCloseTo[i].pWnd == pWnd)
			{
				return;
			}
		}
	}
	else
	{
		CloseTo.pWnd = NULL;
		if (szWndClass)
			CloseTo.strClass = szWndClass;
		else
			CloseTo.strClass = _T("");
		if (szWndName)
			CloseTo.strWndName = szWndName;
		else
			CloseTo.strWndName = _T("");
		for (UINT i = 0; i < m_vWndCloseTo.size(); i++)
		{
			if (_tcsicmp(m_vWndCloseTo[i].strWndName.c_str(), CloseTo.strWndName.c_str()) == 0 
				&& _tcsicmp(m_vWndCloseTo[i].strClass.c_str(), CloseTo.strClass.c_str()) == 0)
			{
				return;
			}
		}
	}

	m_vWndCloseTo.push_back(CloseTo);
}

BOOL IsHorzStickLines(int yA, int xA1, int xA2, int yB, int xB1, int xB2)
{
	return (yA == yB) && ((xA1 <= xB1 && xB1 <= xA2) || (xA1 <= xB2 && xB2 <= xA2) || (xB1 <= xA1 && xB2 >= xA2));
}

BOOL IsVertStickLines(int xA, int yA1, int yA2, int xB, int yB1, int yB2)
{
	return (xA == xB) && ((yA1 <= yB1 && yB1 <= yA2) || (yA1 <= yB2 && yB2 <= yA2) || (yB1 <= yA1 && yB2 >= yA2));
}

BOOL IsStickedWindows(CBaseWnd *pWnd1, CBaseWnd *pWnd2)
{
	CRect rcw, rcd;

    pWnd1->GetWindowRect(&rcw);

	pWnd2->GetWindowRect(&rcd);

	return (IsHorzStickLines(rcd.top, rcd.left, rcd.right, rcw.top, rcw.left, rcw.right) ||
		IsHorzStickLines(rcd.top, rcd.left, rcd.right, rcw.bottom, rcw.left, rcw.right) ||
		IsHorzStickLines(rcd.bottom, rcd.left, rcd.right, rcw.top, rcw.left, rcw.right) ||
		IsHorzStickLines(rcd.bottom, rcd.left, rcd.right, rcw.bottom, rcw.left, rcw.right) ||
		IsVertStickLines(rcd.left, rcd.top, rcd.bottom, rcw.left, rcw.top, rcw.bottom) || 
		IsVertStickLines(rcd.left, rcd.top, rcd.bottom, rcw.right, rcw.top, rcw.bottom) ||
		IsVertStickLines(rcd.right, rcd.top, rcd.bottom, rcw.left, rcw.top, rcw.bottom) || 
		IsVertStickLines(rcd.right, rcd.top, rcd.bottom, rcw.right, rcw.top, rcw.bottom));
}

void CWndDrag::TrackMoveWith(CBaseWnd *pWnd, int x, int y)
{
	if (m_bEnableDrag && m_bSticked && m_pWndToTrack && !m_pWndToTrack->IsIconic())
	{
		CRect	rcw;

		m_pWnd->GetWindowRect(&rcw);

		SetWindowPosSafely(rcw.left, rcw.top, x - m_ptWndTrack.x, y - m_ptWndTrack.y);

		rcw.OffsetRect(x - m_ptWndTrack.x, y - m_ptWndTrack.y);

		m_pWndToTrack->GetWindowRect(&rcw);

		m_ptWndTrack.x = rcw.left;
		m_ptWndTrack.y = rcw.top;
	}
}

//
// 此函数在目标窗口移动之前进行调用，可以跟随目标窗口移动
//
// hWndChain 是中间的传递窗口;
// nCount 是其中包含的个数
// hWndToTrack 是将要跟随移动的窗口
//
// 如果检查的结果是本窗口靠近hWndToCheck中的任何一个，则
void CWndDrag::BeforeTrackMoveWith(CBaseWnd *pWndChain[], int nCount, CBaseWnd *pWndToTrack)
{
	m_bSticked = FALSE;
	vector<CBaseWnd *>	vIn, vOut;

	for (int i = 0; i < nCount; i++)
	{
		if (pWndChain[i] && pWndChain[i]->IsVisible())
			vOut.push_back(pWndChain[i]);
	}

	vIn.push_back(pWndToTrack);

	while (1)
	{
		int		i;
		for (i = 0; i < (int)vIn.size(); i++)
		{
			if (IsStickedWindows(m_pWnd, vIn[i]))
			{
				m_bSticked = TRUE;
				goto STICKED_FOUND;
			}
		}
		BOOL	bFound = FALSE;
		for (i = 0; i < (int)vIn.size(); i++)
		{
			for (int k = 0; k < (int)vOut.size(); k++)
			{
				if (IsStickedWindows(vOut[k], vIn[i]))
				{
					vIn.push_back(vOut[k]);
					vOut.erase(vOut.begin() + k);
					bFound = TRUE;
					goto FIND_CHAIN_OUT;
				}
			}
		}
FIND_CHAIN_OUT:
		if (!bFound)
			break;
	}
	return;

STICKED_FOUND:
	CRect		rc;

	m_pWndToTrack = pWndToTrack;

	m_pWndToTrack->GetWindowRect(&rc);

	m_ptWndTrack.x = rc.left;
	m_ptWndTrack.y = rc.top;

	m_bSticked = TRUE;
}
