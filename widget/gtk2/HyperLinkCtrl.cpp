// HyperLinkCtrl.cpp: implementation of the CHyperLinkCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "desktop.h"
#include "BaseWnd.h"
#include "WidgetCtrlBase.h"
#include "HyperLinkCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHyperLinkCtrl::CHyperLinkCtrl()
{
	m_bVisited = FALSE;
}

CHyperLinkCtrl::~CHyperLinkCtrl()
{
}

BOOL CHyperLinkCtrl::Attach(CBaseWnd *pParentWnd, UINT uChildId, LPCTSTR szUrl)
{
	CWidgetCtrlBase::Attach(pParentWnd, uChildId);

	m_strUrl = szUrl;

	return TRUE;
}

void CHyperLinkCtrl::OnPaint(IMLGraphicsBase *pGraph)
{
}

void CHyperLinkCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
}

void CHyperLinkCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
}

void CHyperLinkCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
}
