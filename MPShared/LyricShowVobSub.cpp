// LyricShowVobSub.cpp: implementation of the CLyricShowVobSub class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowVobSub.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// set classname
LPCTSTR CLyricShowVobSub::ms_szClassName = _T("LyricShowVobSub");

CLyricShowVobSub::CLyricShowVobSub()
{
	m_nDarkenTopArea = float(0.65);
	m_nDarkenBottomArea = float(0.65);

	m_nCurRowOld = -1;
	m_bClearedOld = FALSE;
}

CLyricShowVobSub::~CLyricShowVobSub()
{

}

void CLyricShowVobSub::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate/* = NULL*/)
{
	int			y;
	PML_ROW		pLyricRow = NULL, pLyricRowNext = NULL;

	pGraphics->SetFont(&m_font);

	if (prcUpdate)
		*prcUpdate = m_rcObj;

	if (m_pMLData == NULL)
		return;

	int		nRowCur, nRowNext;
	int		nPlayPos;

	// get the current playing row
	nRowCur = m_pMLData->GetCurPlayLine(m_lyrLines);
	if (nRowCur == -1)
	{
		// no lyrics, redraw background
		UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);
		return;
	}

	nPlayPos = m_pMLData->GetPlayElapsedTime();

	pLyricRow = m_lyrLines[nRowCur];
	nRowNext = nRowCur +1;

	pLyricRowNext = NULL;
	if (nRowNext < (int)m_lyrLines.size())
	{
		pLyricRowNext = m_lyrLines[nRowNext];
		if (nPlayPos < pLyricRowNext->nBegTime)
			pLyricRowNext = NULL;
	}

	if (pLyricRowNext)
		y = GetLineVertAlignPos() - GetLineHeight() - m_nLineSpacing / 2;
	else
		y = GetLineVertAlignPos() - GetLineHeight() / 2;

	//
	// 增加此判断提高效率
	//
	if (nPlayPos >= pLyricRow->nBegTime && 
		nPlayPos <= pLyricRow->nEndTime &&
		nRowCur == m_nCurRowOld && prcUpdate != NULL &&
		(m_LyricsDisplayOpt == DO_NORMAL && !IsKaraoke()))
		return;

	// clear back buffer
	if (!m_bClearedOld || prcUpdate == NULL)
	{
		UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);
		m_bClearedOld = TRUE;
	}

	CRect	rcClip;

	rcClip.SetRect(m_rcObj.left + m_nXMargin, 
						m_rcObj.top + m_nYMargin,
						m_rcObj.right - m_nXMargin,
						m_rcObj.bottom - m_nYMargin);
	//
	// 保存旧的剪裁区域, 并且设置剪裁区域为该UIObject的大小
	pGraphics->SetClipBoundBox(rcClip);

	if (nPlayPos >= pLyricRow->nBegTime && 
		nPlayPos <= pLyricRow->nEndTime)
	{
		// 显示此行
		DrawCurrentRow(pGraphics, pLyricRow, AUTO_CAL_X, y);
		y += GetLineHeight();
		m_bClearedOld = FALSE;
	}

	if (pLyricRowNext && nPlayPos >= pLyricRowNext->nBegTime && 
		nPlayPos <= pLyricRowNext->nEndTime)
	{
		// 显示此行
		DrawCurrentRow(pGraphics, pLyricRowNext, AUTO_CAL_X, y);
		m_bClearedOld = FALSE;
	}

	// 恢复为原来的剪裁区域大小
	pGraphics->RecoverClipBox();

	if (nPlayPos >= pLyricRow->nBegTime && 
		nPlayPos <= pLyricRow->nEndTime)
		m_nCurRowOld = nRowCur;
}

LPCTSTR CLyricShowVobSub::GetClassName()
{
	return ms_szClassName;
}

BOOL CLyricShowVobSub::IsKindOf(LPCTSTR szClassName)
{
	if (CLyricShowObj::IsKindOf(szClassName))
		return TRUE;

	return _tcsicmp(szClassName, ms_szClassName) == 0;
}

void CLyricShowVobSub::OnLyrDrawContextChanged()
{
	m_bCanWrapLines = true;

	CLyricShowObj::OnLyrDrawContextChanged();

	AutoWidthSkinAccordLyrics();

	AutoHeightSkinAccordLyrics();
}
