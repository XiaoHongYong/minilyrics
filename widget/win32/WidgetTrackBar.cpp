// HyperLinkCtrl.cpp: implementation of the CWidgetColorButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WidgetTrackBar.h"

void CWidgetTrackBar::SetRange(int nMin, int nMax)
{
	SendMessage(m_hWnd, TBM_SETRANGE, TRUE, MAKELONG(nMin, nMax));
}

void CWidgetTrackBar::SetLineSize(int nLine)
{
	SendMessage(m_hWnd, TBM_SETLINESIZE, 0, nLine);
}

void CWidgetTrackBar::SetPageSize(int nPage)
{
	SendMessage(m_hWnd, TBM_SETPAGESIZE, 0, nPage);
}

void CWidgetTrackBar::SetTickFreq(int nFreq)
{
	SendMessage(m_hWnd, TBM_SETTICFREQ, nFreq, 0);
}

void CWidgetTrackBar::SetPos(int nPos)
{
	SendMessage(m_hWnd, TBM_SETPOS, TRUE, nPos);
}

int CWidgetTrackBar::GetPos()
{
	return SendMessage(m_hWnd, TBM_GETPOS, 0, 0);
}
