// ScrollBarHandler.cpp: implementation of the CScrollBarHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WidgetLite.h"
#include "ScrollBarHandler.h"

CScrollBarHandler::CScrollBarHandler()
{
	m_bDisabled = false;
	m_nOneline  = 1;
}

CScrollBarHandler::~CScrollBarHandler()
{

}

void CScrollBarHandler::Init(CBaseWnd *pWnd, int nBar)
{
	ASSERT(pWnd);

	m_nScrollBar = nBar;
}

void CScrollBarHandler::SetScrollInfo(int nMin, int nMax, int nPage, int nPos, int nLine, bool bRedraw)
{

}

int CScrollBarHandler::SetScrollPos(int nPos, bool bRedraw)
{
	return nPos;
}

int CScrollBarHandler::GetID() const
{
	return 0;
}

int CScrollBarHandler::GetScrollPos() const
{
	return 0;
}

int CScrollBarHandler::GetMax() const
{
	return 0;
}

void CScrollBarHandler::DisableScrollBar()
{
}

bool CScrollBarHandler::HandleScrollCode(UINT nSBCode, int nPos)
{
	return false;
}
