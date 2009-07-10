// SkinWndDrag.cpp: implementation of the CSkinWndDrag class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SkinWndDrag.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL IsStickedWindows(CBaseWnd *pWnd1, CBaseWnd *pWnd2);
void RectCloseToRect(CRect &rcs, CRect &rcd, int &offx, int &offy, int nCloseExt);
void RectCloseToWindow(CRect &rcs, CBaseWnd *pWnd, int nCloseExt, int &offx, int &offy);

#ifdef _WIN32
void GetTaskBarRect(RECT &rc)
{
	::GetWindowRect(::FindWindow(_T("Shell_TrayWnd"),NULL), &rc);
}
#endif

CSkinWndDrag::CSkinWndDrag()
{

}

CSkinWndDrag::~CSkinWndDrag()
{

}

void CSkinWndDrag::Init(CBaseWnd *pWnd, ISkinWndDragHost *pSkinWndDragHost)
{
	CWndDrag::Init(pWnd);

	m_pSkinWndDragHost = pSkinWndDragHost;
}

typedef vector<CBaseWnd *>		V_WNDS;

static void VWnds_Add(V_WNDS &vWnds, CBaseWnd *pWnd)
{
	for (int i = 0; i < (int)vWnds.size(); i++)
	{
		if (vWnds[i]->IsSameWnd(pWnd))
			return;
	}
	vWnds.push_back(pWnd);
}

static void VWnds_Remove(V_WNDS &vWnds, CBaseWnd *pWnd)
{
	for (int i = 0; i < (int)vWnds.size(); i++)
	{
		if (vWnds[i]->IsSameWnd(pWnd))
		{
			vWnds.erase(vWnds.begin() + i);
			return;
		}
	}
}

void GetStickedWindows(CBaseWnd *pMainWnd, V_WNDS &vAllWnds, V_WNDS &vStickedWnds)
{
	bool			bFoundSticked;

	vStickedWnds.push_back(pMainWnd);

	// find all sticked windows, save in vStickedWnds
	do
	{
		bFoundSticked = false;

		for (int i = 0; i < (int)vAllWnds.size(); i++)
		{
			CBaseWnd	*pWnd = vAllWnds[i];

			for (int n = 0; n < (int)vStickedWnds.size(); n++)
			{
				if (!pWnd->IsSameWnd(vStickedWnds[n]) && IsStickedWindows(pWnd, vStickedWnds[n]))
				{
					VWnds_Add(vStickedWnds, pWnd);
					vAllWnds.erase(vAllWnds.begin() + i);
					i--;
					bFoundSticked = true;
					break;
				}
			}
		}
	}
	while (bFoundSticked);
}

// input:
//		nOffx, nOffy: offset of window by mouse drag
// output:
//		nOffx, nOffy: offset of window by auto close to
BOOL CSkinWndDrag::AutoCloseToWindows(int &nOffx, int &nOffy, BOOL bMoveWindow)
{
	CBaseWnd			*pWnd = NULL;
	V_WNDS				vWnds;
	UINT				i;
	vector<CRect>		vRcSticked, vRcToClosedTo;
    CRect				rc;

	if (m_pSkinWndDragHost)
	{
		m_pSkinWndDragHost->GetWndDragAutoCloseTo(vWnds);
	}

	// enum all windows that should auto close to
	for (i = 0; i < m_vWndCloseTo.size(); i ++)
	{
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
		if (pWnd)
			VWnds_Add(vWnds, pWnd);
	}

	// offset the rect of all sticked windows.
	for (i = 0; i < m_vStickedWnds.size(); i++)
	{
		VWnds_Remove(vWnds, m_vStickedWnds[i]);

		m_vStickedWnds[i]->GetWindowRect(&rc);
		rc.OffsetRect(nOffx, nOffy);
		vRcSticked.push_back(rc);
	}

	// make the sticked windows auto be close to vWnds.
	int		nOffXTemp, nOffYTemp;
	nOffx = 0; nOffy = 0;
	for (i = 0; i < vWnds.size(); i ++)
	{
		pWnd = vWnds[i];
		for (int k = 0; k < (int)m_vStickedWnds.size(); k++)
		{
			CBaseWnd	*pWndToMove = m_vStickedWnds[k];
			if (pWnd && !pWnd->IsSameWnd(pWndToMove))
			{
				RectCloseToWindow(vRcSticked[k], pWnd, 10, nOffXTemp, nOffYTemp);

				if (nOffXTemp != 0)
					nOffx = nOffXTemp;
				if (nOffYTemp != 0)
					nOffy = nOffYTemp;

				if (nOffx != 0 && nOffy != 0)
				{
					i = vWnds.size();
					break;
				}
			}
		}
	}

	if (nOffx == 0 && nOffy == 0)
	{
		// close to display edge
		GetMonitorRestrictRect(vRcSticked[0], rc);
		vRcToClosedTo.push_back(rc);

#ifdef _WIN32
		// close to taskbar
		GetTaskBarRect(rc);
		vRcToClosedTo.push_back(rc);
#endif

		// make the sticked windows auto be close to vRcToClosedTo.
		for (int i =0; i < (int)vRcToClosedTo.size(); i++)
		{
			for (int k = 0; k < (int)m_vStickedWnds.size(); k++)
			{
				RectCloseToRect(vRcSticked[k], vRcToClosedTo[i], nOffXTemp, nOffYTemp, 10);

				if (nOffXTemp != 0)
					nOffx = nOffXTemp;
				if (nOffYTemp != 0)
					nOffy = nOffYTemp;

				if (nOffx != 0 && nOffy != 0)
				{
					i = vRcToClosedTo.size();
					break;
				}
			}
		}
	}

	// vRcSticked[0] is the rect of m_pWnd
	vRcSticked[0].OffsetRect(nOffx, nOffy);

    m_pWnd->GetWindowRect(&rc);
	if (rc.EqualRect(&vRcSticked[0]))
	{
		// no move
	}
	else
	{
		if (bMoveWindow)
		{
			// m_vStickedWnds[0] is m_pWnd, and vRcSticked[0] is the new pos now.
			m_pWnd->SetWindowPosSafely(vRcSticked[0].left, vRcSticked[0].top);

			// move  all sticked window from m_vStickedWnds[1]
			for (int k = 1; k < (int)m_vStickedWnds.size(); k++)
			{
				CBaseWnd	*pWndToMove = m_vStickedWnds[k];

				pWndToMove->SetWindowPosSafely(vRcSticked[k].left + nOffx, vRcSticked[k].top + nOffy);
			}
		}

		return TRUE;
	}

	return FALSE;

}

void CSkinWndDrag::OnDrag(UINT fwKeys, POINT pt)
{
	BOOL	bDragStatOld = m_bDragWnd;

	CWndDrag::OnDrag(fwKeys, pt);

	if (bDragStatOld != m_bDragWnd)
	{
		if (m_bDragWnd)
		{
			// begin drag
			V_WNDS		vTrackMoveWnds;

			if (m_pSkinWndDragHost)
				m_pSkinWndDragHost->GetWndDragTrackMove(vTrackMoveWnds);

			// get all windows sticked with m_pWnd
			GetStickedWindows(m_pWnd, vTrackMoveWnds, m_vStickedWnds);
		}
		else
		{
			// end drag
			// clear drag status
			m_vStickedWnds.clear();
		}
	}
}

void CSkinWndDrag::SetWindowPosSafely(int xOld, int yOld, int nOffsetx, int nOffsety)
{
	CWndDrag::SetWindowPosSafely(xOld, yOld, nOffsetx, nOffsety);
}

//////////////////////////////////////////////////////////////////////////
//

CSkinWndResizer::CSkinWndResizer()
{
	m_pSkinWndDragHost = NULL;
}

CSkinWndResizer::~CSkinWndResizer()
{

}

void CSkinWndResizer::Init(CBaseWnd *pWnd, ISkinWndDragHost *pSkinWndDragHost)
{
	CWndResizer::Init(pWnd);

	m_pSkinWndDragHost = pSkinWndDragHost;
}

void VertLineCloseToRect(int nSrcY1, int nSrcY2, int nSrcX, CRect &rcd, int &offx, int nCloseExt);
void HorzLineCloseToRect(int nSrcX1, int nSrcX2, int nSrcY, CRect &rcd, int &offy, int nCloseExt);


// input:
//		nOffx, nOffy: offset of window by mouse drag
// output:
//		nOffx, nOffy: offset of window by auto close to
void CSkinWndResizer::AutoCloseToWindows(int &nOffx, int &nOffy)
{
	V_WNDS			vWnds;
	UINT			i;
	CBaseWnd		*pWnd;
	CRect			rcWnd, rc;
	int				nOffXTemp, nOffYTemp;
	vector<CRect>	vRcToClosedTo;

	nOffx = 0; nOffy = 0;

	if (!m_pSkinWndDragHost)
		return;

	m_pSkinWndDragHost->GetWndDragAutoCloseTo(vWnds);

	// close to display edge
	m_pWnd->GetWindowRect(&rcWnd);
	GetMonitorRestrictRect(rcWnd, rc);
	vRcToClosedTo.push_back(rc);

#ifdef _WIN32
	// close to taskbar
	GetTaskBarRect(rc);
	vRcToClosedTo.push_back(rc);
#endif

	for (i = 0; i < vWnds.size(); i++)
	{
		pWnd = vWnds[i];

		if (pWnd->IsSameWnd(m_pWnd) || !pWnd->IsVisible() || pWnd->IsIconic())
			continue;

		pWnd->GetWindowRect(&rc);
		vRcToClosedTo.push_back(rc);
	}

	if (m_bAtBottom || m_bAtTop)
	{
		// top or bottom line close to vWnds
		int		y;

		if (m_bAtTop)
			y = m_rcResizing.top;
		else
			y = m_rcResizing.bottom;
		
		for (i = 0; i < vRcToClosedTo.size(); i++)
		{
			HorzLineCloseToRect(m_rcResizing.left, m_rcResizing.right, y, vRcToClosedTo[i], nOffYTemp, 10);
			if (nOffYTemp != 0)
				nOffy = nOffYTemp;
			if (nOffy != 0)
			{
				if (m_bAtTop)
					m_rcResizing.top += nOffy;
				else
					m_rcResizing.bottom += nOffy;
				break;
			}
		}
	}

	if (m_bAtLeft || m_bAtRight)
	{
		// top or bottom line close to vWnds
		int		x;

		if (m_bAtLeft)
			x = m_rcResizing.left;
		else
			x = m_rcResizing.right;
	
		for (i = 0; i < vRcToClosedTo.size(); i++)
		{
			VertLineCloseToRect(m_rcResizing.top, m_rcResizing.bottom, x, vRcToClosedTo[i], nOffXTemp, 10);
			if (nOffXTemp != 0)
				nOffx = nOffXTemp;
			if (nOffx != 0)
			{
				if (m_bAtLeft)
					m_rcResizing.left += nOffx;
				else
					m_rcResizing.right += nOffx;
				break;
			}
		}
	}
}
