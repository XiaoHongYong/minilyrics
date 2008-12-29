// WndHelper.cpp: implementation of the CMLWndHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
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

void CMLWndHelper::Add(int nChildID, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, int nChildIDRelation)
{
	HWND		hChild = GetDlgItem(m_pWndParent->GetHandle(), nChildID);

	if (!hChild)
		return;

	Add(hChild,
		bAlignRight, bAlignBottom, bSizeXFix, bSizeYFix, 
		nChildIDRelation == 0 ? NULL : GetDlgItem(m_pWndParent->GetHandle(), nChildIDRelation));
}

void CMLWndHelper::AddAlignCenter(int nChildID, bool bAlignVert)
{
	HWND		hChild = GetDlgItem(m_pWndParent->GetHandle(), nChildID);

	if (!hChild)
		return;

	AddAlignCenter(hChild, bAlignVert);
}

void CMLWndHelper::AddAlignRight(int nChildID, bool bSizeXFixed)
{
	Add(nChildID, true, false, bSizeXFixed, true, 0);
}

void CMLWndHelper::AddAlignBottom(int nChildID, bool bSizeYFixed)
{
	Add(nChildID, false, true, true, bSizeYFixed, 0);
}

void CMLWndHelper::AddAlignRightBottom(int nChildID, bool bSizeXFixed, bool bSizeYFixed)
{
	Add(nChildID, true, true, bSizeXFixed, bSizeYFixed, 0);
}

void CMLWndHelper::OnResize()
{
	int		i;

	for (i = 0; i < (int)m_vChildWnd.size(); i++)
	{
		ChildWndProp		&childWnd = m_vChildWnd[i];

		CRect	rc, rcRalation;

		::GetWindowRect(childWnd.hWnd, &rc);

		if (childWnd.hWndRelation)
		{
			::GetWindowRect(childWnd.hWndRelation, &rcRalation);
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
		::MoveWindow(childWnd.hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	}

	for (i = 0; i < (int)m_vAlignCenterWnds.size(); i++)
	{
		CRect					rc, rcParent;
		ChildAlignCenter		&align = m_vAlignCenterWnds[i];

		m_pWndParent->GetWindowRect(&rcParent);
		::GetWindowRect(align.hWnd, &rc);
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
		MoveWindow(align.hWnd, rc.left, rc.top, rc.Width(), rc.Height(), true);
	}
}

void CMLWndHelper::Add(HWND hWndChild, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, HWND hChildRelation)
{
	ChildWndProp		childWnd;

	childWnd.hWnd = hWndChild;
	childWnd.bAlignBottom = bAlignBottom;
	childWnd.bAlignRight = bAlignRight;
	childWnd.bSizeXFix = bSizeXFix;
	childWnd.bSizeYFix = bSizeYFix;
	childWnd.hWndRelation = hChildRelation;

	RECT	rc, rcRalation;

	::GetWindowRect(hWndChild, &rc);

	if (hChildRelation)
	{
		::GetWindowRect(hChildRelation, &rcRalation);
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

void CMLWndHelper::AddAlignCenter(HWND hWndChild, bool bAlignVert)
{
	ChildAlignCenter	align;
	CRect				rc, rcChild;

	align.hWnd = hWndChild;
	align.bVert = bAlignVert;

	m_pWndParent->GetWindowRect(&rc);
	::GetWindowRect(hWndChild, &rcChild);
	if (bAlignVert)
		align.nOffsetToCenter = (rc.top + rc.bottom) / 2 - rcChild.top;
	else
		align.nOffsetToCenter = (rc.left + rc.right) / 2 - rcChild.left;

	m_vAlignCenterWnds.push_back(align);
}
