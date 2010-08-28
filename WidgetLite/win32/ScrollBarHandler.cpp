// ScrollBarHandler.cpp: implementation of the CScrollBarHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseWnd.h"
#include "ScrollBarHandler.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScrollBarHandler::CScrollBarHandler()
{
	m_hWnd = NULL;
	memset(&m_si, 0, sizeof(m_si));
	m_nScrollBar = SB_VERT;
	m_bDisabled = false;
	m_nOneline  = 1;
}

CScrollBarHandler::~CScrollBarHandler()
{

}

void CScrollBarHandler::Init(CBaseWnd *pWnd, int nBar)
{
	ASSERT(pWnd);
	m_hWnd = pWnd->GetHandle();

	m_si.cbSize = sizeof(m_si);
	m_si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;

	m_nScrollBar = nBar;
}

void CScrollBarHandler::SetScrollInfo(int nMin, int nMax, int nPage, int nPos, int nLine, bool bRedraw)
{
	m_si.cbSize = sizeof(m_si);
	m_si.nMax = nMax;
	m_si.nMin = nMin;
	m_si.nPage = nPage;
	m_nOneline = nLine;
	if (nPos <= GetMax())
		m_si.nPos = nPos;
	else
		m_si.nPos = GetMax();

	if (m_bDisabled)
	{
#ifndef _WINCE_
		EnableScrollBar(m_hWnd, m_nScrollBar, ESB_ENABLE_BOTH);
#endif
		m_bDisabled = false;
	}

	::SetScrollInfo(m_hWnd, m_nScrollBar, &m_si, bRedraw);
}

int CScrollBarHandler::SetScrollPos(int nPos, bool bRedraw)
{
	m_si.nPos = nPos;
	return ::SetScrollPos(m_hWnd, m_nScrollBar, nPos, bRedraw);
}

int CScrollBarHandler::GetID() const
{
	return 0;
}

int CScrollBarHandler::GetScrollPos() const
{
	if (m_bDisabled)
		return 0;
	else
		return m_si.nPos;
}

int CScrollBarHandler::GetMax() const
{
	return m_si.nMax -  m_si.nPage + m_nOneline;
}

void CScrollBarHandler::DisableScrollBar()
{
	m_bDisabled = true;
	m_si.nPos = 0;
#ifndef _WINCE_
	EnableScrollBar(m_hWnd, m_nScrollBar, ESB_DISABLE_BOTH);
#endif
}

bool CScrollBarHandler::HandleScrollCode(UINT nSBCode, int nPos)
{
	int		nPosOld = m_si.nPos;

	switch (nSBCode)
	{
	case SB_LINEDOWN:
		m_si.nPos += m_nOneline;
		break;
	case SB_LINEUP:
		if (m_si.nPos > m_si.nMin)
			m_si.nPos -= m_nOneline;
		break;
	case SB_PAGEDOWN:
		m_si.nPos += m_si.nPage - m_nOneline;
		break;
	case SB_PAGEUP:
		m_si.nPos -= m_si.nPage - m_nOneline;
		if (m_si.nPos < m_si.nMin)
			m_si.nPos = m_si.nMin;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			m_si.nPos = nPos;
			if (m_si.nPos < m_si.nMin)
				m_si.nPos = m_si.nMin;
		}
		break;
	case SB_TOP:
		m_si.nPos = m_si.nMin;
		break;
	case SB_BOTTOM:
		m_si.nPos = m_si.nMax;
	default:
		break;
	}

	if (m_si.nPos > GetMax())
		m_si.nPos = GetMax();

	if (m_si.nPos != nPosOld)
		SetScrollPos(m_si.nPos);

	return m_si.nPos != nPosOld;
}
