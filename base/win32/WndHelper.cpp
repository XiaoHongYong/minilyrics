// WndHelper.cpp: implementation of the CWndHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WndHelper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWndHelper::CWndHelper()
{

}

CWndHelper::~CWndHelper()
{

}

void CWndHelper::Add(HWND hWnd, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, HWND hWndRelation)
{
	ChildWndProp		childWnd;

	childWnd.hWnd = hWnd;
	childWnd.bAlignBottom = bAlignBottom;
	childWnd.bAlignRight = bAlignRight;
	childWnd.bSizeXFix = bSizeXFix;
	childWnd.bSizeYFix = bSizeYFix;
	childWnd.hWndRelation = hWndRelation;

	RECT	rc, rcRalation;

	GetWindowRect(hWnd, &rc);

	if (hWndRelation)
	{
		GetWindowRect(hWndRelation, &rcRalation);
	}
	else
	{
		GetWindowRect(m_hWndParent, &rcRalation);
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

void CWndHelper::AddAlignCenter(HWND hWnd, bool bAlignVert)
{
	ChildAlignCenter	align;
	RECT				rc, rcChild;

	align.hWnd = hWnd;
	align.bVert = bAlignVert;

	GetWindowRect(m_hWndParent, &rc);
	GetWindowRect(hWnd, &rcChild);
	if (bAlignVert)
		align.nOffsetToCenter = (rc.top + rc.bottom) / 2 - rcChild.top;
	else
		align.nOffsetToCenter = (rc.left + rc.right) / 2 - rcChild.left;

	m_vAlignCenterWnds.push_back(align);
}

void CWndHelper::Add(int nChildID, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, HWND hWndRelation)
{
	Add(::GetDlgItem(m_hWndParent, nChildID), bAlignRight, bAlignBottom, bSizeXFix, bSizeYFix, hWndRelation);
}

void CWndHelper::AddAlignCenter(int nChildID, bool bAlignVert)
{
	AddAlignCenter(::GetDlgItem(m_hWndParent, nChildID), bAlignVert);
}

void CWndHelper::AddAlignRight(int nChildID, bool bSizeXFixed)
{
	Add(::GetDlgItem(m_hWndParent, nChildID), true, false, bSizeXFixed, true, NULL);
}

void CWndHelper::AddAlignBottom(int nChildID, bool bSizeYFixed)
{
	Add(::GetDlgItem(m_hWndParent, nChildID), false, true, true, bSizeYFixed, NULL);
}

void CWndHelper::AddAlignRightBottom(int nChildID, bool bSizeXFixed, bool bSizeYFixed)
{
	Add(::GetDlgItem(m_hWndParent, nChildID), true, true, bSizeXFixed, bSizeYFixed, NULL);
}

void CWndHelper::OnResize()
{
	UINT		i;

	for (i = 0; i < m_vChildWnd.size(); i++)
	{
		ChildWndProp		&childWnd = m_vChildWnd[i];

		RECT	rc, rcRalation;

		GetWindowRect(childWnd.hWnd, &rc);

		if (childWnd.hWndRelation)
		{
			GetWindowRect(childWnd.hWndRelation, &rcRalation);
		}
		else
		{
			GetWindowRect(m_hWndParent, &rcRalation);
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
		
		ScreenToClient(m_hWndParent, (LPPOINT)&(rc.left));
		ScreenToClient(m_hWndParent, (LPPOINT)&(rc.right));
		MoveWindow(childWnd.hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	}

	for (i = 0; i < (int)m_vAlignCenterWnds.size(); i++)
	{
		RECT					rc, rcParent;
		ChildAlignCenter		&align = m_vAlignCenterWnds[i];
		GetWindowRect(m_hWndParent, &rcParent);
		GetWindowRect(align.hWnd, &rc);
		ScreenToClient(m_hWndParent, (LPPOINT)&rc.left);
		ScreenToClient(m_hWndParent, (LPPOINT)&rc.right);
		if (align.bVert)
		{
			int			h = rc.bottom - rc.top;
			rc.top = (rcParent.right - rcParent.left) / 2 - align.nOffsetToCenter;
			rc.bottom = rc.top + h;
		}
		else
		{
			int			w = rc.right - rc.left;
			rc.left = (rcParent.right - rcParent.left) / 2 - align.nOffsetToCenter;
			rc.right = rc.left + w;
		}
		MoveWindow(align.hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);
	}
}
