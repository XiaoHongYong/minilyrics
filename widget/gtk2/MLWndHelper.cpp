// WndHelper.cpp: implementation of the CMLWndHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "BaseWnd.h"
#include "MLWndHelper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLWndHelper::CMLWndHelper()
{

}

CMLWndHelper::~CMLWndHelper()
{

}

void CMLWndHelper::Add(CBaseWnd *pWnd, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, CBaseWnd *pWndRelation)
{
	return;
	ASSERT(pWnd);
	ChildWndProp		childWnd;

	childWnd.pWnd = pWnd;
	childWnd.bAlignBottom = bAlignBottom;
	childWnd.bAlignRight = bAlignRight;
	childWnd.bSizeXFix = bSizeXFix;
	childWnd.bSizeYFix = bSizeYFix;
	childWnd.pWndRelation = pWndRelation;

	RECT	rc, rcRalation;

	pWnd->GetWindowRect(&rc);

	if (pWndRelation)
	{
		pWndRelation->GetWindowRect(&rcRalation);
	}
	else
	{
		m_pWndParent->GetWindowRect(&rcRalation);
		rcRalation.left = rcRalation.right;
		rcRalation.top = rcRalation.bottom;
	}

	if (bAlignRight)
	{
		childWnd.nMarginX = rcRalation.right - rc.right;
	}

	if (bAlignBottom)
		childWnd.nMarginY = rcRalation.bottom - rc.bottom;

	m_vChildWnd.push_back(childWnd);
}

void CMLWndHelper::AddAlignCenter(CBaseWnd *pWnd, bool bAlignVert)
{
	return;
	ASSERT(pWnd);
	ChildAlignCenter	align;
	CRect				rc, rcChild;

	align.pWnd = pWnd;
	align.bVert = bAlignVert;

	m_pWndParent->GetWindowRect(&rc);
	pWnd->GetWindowRect(&rcChild);
	if (bAlignVert)
		align.nOffsetToCenter = (rc.top + rc.bottom) / 2 - rcChild.top;
	else
		align.nOffsetToCenter = (rc.left + rc.right) / 2 - rcChild.left;

	m_vAlignCenterWnds.push_back(align);
}

void CMLWndHelper::Add(int nChildID, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, CBaseWnd *pWndRelation)
{
	Add(m_pWndParent->GetDlgItem(nChildID), bAlignRight, bAlignBottom, bSizeXFix, bSizeYFix, pWndRelation);
}

void CMLWndHelper::AddAlignCenter(int nChildID, bool bAlignVert)
{
	AddAlignCenter(m_pWndParent->GetDlgItem(nChildID), bAlignVert);
}

void CMLWndHelper::AddAlignRight(int nChildID, bool bSizeXFixed)
{
	Add(m_pWndParent->GetDlgItem(nChildID), true, false, bSizeXFixed, true, NULL);
}

void CMLWndHelper::AddAlignBottom(int nChildID, bool bSizeYFixed)
{
	Add(m_pWndParent->GetDlgItem(nChildID), false, true, true, bSizeYFixed, NULL);
}

void CMLWndHelper::AddAlignRightBottom(int nChildID, bool bSizeXFixed, bool bSizeYFixed)
{
	Add(m_pWndParent->GetDlgItem(nChildID), true, true, bSizeXFixed, bSizeYFixed, NULL);
}

void CMLWndHelper::OnResize()
{
	return;

	int		i;

	for (i = 0; i < (int)m_vChildWnd.size(); i++)
	{
		ChildWndProp		&childWnd = m_vChildWnd[i];

		CRect	rc, rcRalation;

		childWnd.pWnd->GetWindowRect(&rc);

		if (childWnd.pWndRelation)
		{
			childWnd.pWndRelation->GetWindowRect(&rcRalation);
		}
		else
		{
			m_pWndParent->GetWindowRect(&rcRalation);
			rcRalation.left = rcRalation.right;
			rcRalation.top = rcRalation.bottom;
		}

		if (childWnd.bAlignRight)
		{
			if (!childWnd.bSizeXFix)
				rc.right = rcRalation.right - childWnd.nMarginX;
			else
			{
				int		nCx;
				nCx = rc.right - rc.left;
				rc.right = rcRalation.right - childWnd.nMarginX;
				rc.left = rc.right - nCx;
			}
		}

		if (childWnd.bAlignBottom)
		{
			if (!childWnd.bSizeYFix)
				rc.bottom = rcRalation.bottom - childWnd.nMarginY;
			else
			{
				int		nCy;
				nCy = rc.bottom - rc.top;
				rc.bottom = rcRalation.bottom - childWnd.nMarginY;
				rc.top = rc.bottom - nCy;
			}
		}
		
		m_pWndParent->ScreenToClient(rc);
		childWnd.pWnd->MoveWindow(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	}

	for (i = 0; i < (int)m_vAlignCenterWnds.size(); i++)
	{
		CRect					rc, rcParent;
		ChildAlignCenter		&align = m_vAlignCenterWnds[i];
		m_pWndParent->GetWindowRect(&rcParent);
		align.pWnd->GetWindowRect(&rc);
		m_pWndParent->ScreenToClient(rc);
		if (align.bVert)
		{
			int			h = rc.Height();
			rc.top = rcParent.Height() / 2 - align.nOffsetToCenter;
			rc.bottom = rc.top + h;
		}
		else
		{
			int			w = rc.Width();
			rc.left = rcParent.Width() / 2 - align.nOffsetToCenter;
			rc.right = rc.left + w;
		}
		align.pWnd->MoveWindow(rc.left, rc.top, rc.Width(), rc.Height(), true);
	}
}
