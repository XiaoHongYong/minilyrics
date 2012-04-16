/********************************************************************
	Created  :	2001-12-27 0:02:48
	FileName :	LyricShowMultiRowObj.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowMultiRowObj.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// set classname
UIOBJECT_CLASS_NAME_IMP(CLyricShowMultiRowObj, _T("LyricShowMultiRow"))

CLyricShowMultiRowObj::CLyricShowMultiRowObj()
{
	m_yDrawingOld = -1;

	m_nCurRowOld = -1;

	m_nPrevLineAlphaOld = m_nNextLineAlphaOld = 0;

	m_nDarkenTopArea = float(1.6);
	m_nDarkenBottomArea = float(1.6);
}

CLyricShowMultiRowObj::~CLyricShowMultiRowObj()
{
}

void CLyricShowMultiRowObj::Invalidate()
{
	m_yDrawingOld = -1;
	m_nCurRowOld = -1;
	CLyricShowObj::Invalidate();
}

void CLyricShowMultiRowObj::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate/* = NULL*/)
{
	assert(m_pMLData);
	if (m_pMLData == NULL)
		return;

	if (!m_pMLData->HasLyricsOpened() || m_lyrLines.size() == 0)
	{
		FastDrawMediaInfo(pGraphics, prcUpdate);
		return;
	}

	pGraphics->SetFont(&m_font);

	if (prcUpdate)
		*prcUpdate = m_rcObj;

	int				y, yCurRow;
	LyricsLine *pLineCur;
	int				nRowHeight;
	int				nDeltaTime;
	int				nRowCur;
	LyricsLine *pLine;


	// get the current playing line
	nRowCur = m_pMLData->GetCurPlayLine(m_lyrLines);
	if (nRowCur == -1)
	{
		// no lyrics, redraw the whole background
		UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);
		return;
	}

	pLineCur = m_lyrLines[nRowCur];
	nRowHeight = GetLineHeight();
	nDeltaTime = pLineCur->nEndTime - pLineCur->nBegTime;

	CDispOptRecover		dispOptRecover(this);
	if (m_img.IsValid())
		m_LyricsDisplayOpt = DO_FADEOUT_BG;
	else if (m_LyricsDisplayOpt == DO_AUTO)
	{
		if (m_rcObj.Height() / nRowHeight >= 5)
			m_LyricsDisplayOpt = DO_FADEOUT_LOWCOLOR;
	}

	if (nDeltaTime == 0 && m_nCurRowOld == nRowCur)
		yCurRow = m_yDrawingOld;
	else
	{
		yCurRow = GetLineVertAlignPos();
		if (m_pMLData->GetPlayElapsedTime() < pLineCur->nBegTime || nDeltaTime == 0)
			;
		else if (m_pMLData->GetPlayElapsedTime() > pLineCur->nEndTime)
			yCurRow += -nRowHeight;
		else
			yCurRow += -nRowHeight + 
				nRowHeight * (pLineCur->nEndTime - m_pMLData->GetPlayElapsedTime()) / nDeltaTime;
	}

	CRect	rcClip;

	rcClip.SetRect(m_rcObj.left + m_nXMargin, 
		m_rcObj.top + m_nYMargin,
		m_rcObj.right - m_nXMargin,
		m_rcObj.bottom - m_nYMargin);

	if (prcUpdate && yCurRow == m_yDrawingOld && nRowCur == m_nCurRowOld)
	{
		*prcUpdate = rcClip;

		if (m_LyricsDisplayOpt == DO_NORMAL)
			FastestDraw_GetUpdateRectOfNormal(pGraphics, prcUpdate, yCurRow);
		else
			FastestDraw_GetUpdateRectOfFadeInOut(pGraphics, prcUpdate, yCurRow, nRowCur);

		rcClip = *prcUpdate;

		if (rcClip.IsRectEmpty())
			return;

		// clear background
		UpdateLyricDrawBufferBackground(pGraphics, rcClip);
	}
	else
	{
		m_nCurRowOld = nRowCur;
		m_yDrawingOld = yCurRow;

		// clear background
		UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);
	}

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraphics);
	pGraphics->SetClipBoundBox(rcClip);

	int		nRow, nRowEnd;
	int		yLyrStart, yLyrEnd;

	// draw center row, the row lyric is current
	DrawCurrentRow(pGraphics, pLineCur, AUTO_CAL_X, yCurRow);

	// Draw lyrics lines above current line
	nRow = nRowCur - 1;
	nRowEnd = 0;
	for (y = yCurRow - nRowHeight; y + nRowHeight > rcClip.top && nRow >= nRowEnd; y -= nRowHeight)
	{
		pLine = m_lyrLines[nRow];

		if (!DrawRow(pGraphics, pLine, AUTO_CAL_X, y, LP_ABOVE_CUR_LINE))
			break;
		nRow--;
	}
	yLyrStart = y + nRowHeight;

	// Draw lyrics lines below current line
	nRow = nRowCur + 1;
	nRowEnd = m_lyrLines.size();
	for (y = yCurRow + nRowHeight; y < rcClip.bottom && nRow < nRowEnd; y += nRowHeight)
	{
		pLine = m_lyrLines[nRow];
		if (!DrawRow(pGraphics, pLine, AUTO_CAL_X, y, LP_BELOW_CUR_LINE))
			break;
		nRow++;
	}
	yLyrEnd = y;

	FadeOutVertBorder(pGraphics, yLyrStart, yLyrEnd);

	// if (g_bSave)
// 	{
// 		static int g_n = 0;
// 		g_n++;
// 		CStrPrintf	strFile(_T("C:\\a_%03d%d.bmp"), g_n, prcUpdate);
// 
// 		SaveBmpFileFromRawImageData(pGraphics->GetRawBuff(), strFile.c_str());
// 		g_bSave = false;
// 	}
}

void CLyricShowMultiRowObj::FastestDraw_GetUpdateRectOfFadeInOut(CRawGraph *pGraphics, CRect *prcUpdate, int y, int nCurLine)
{
	int			nRowHeight = GetLineHeight();
	int			nAlpha;

	ASSERT(nCurLine >= 0 && nCurLine < (int)m_lyrLines.size());

	prcUpdate->SetRect(m_rcObj.left + m_nXMargin, 0, m_rcObj.right - m_nXMargin, 0);
	prcUpdate->top = prcUpdate->bottom = y;

	// Get current line update rect
	if (IsKaraoke())
	{
		prcUpdate->top = y;
		prcUpdate->bottom = y + nRowHeight;
	}

	// Get previous line update rect
	if (nCurLine > 0)
	{
		LyricsLine *pPrevLine;
		pPrevLine = m_lyrLines[nCurLine - 1];

		nAlpha = GetAlpha(pPrevLine);
		if (m_nPrevLineAlphaOld != nAlpha)
		{
			m_nPrevLineAlphaOld = nAlpha;

			// Redraw the whole line.
			prcUpdate->top = y - nRowHeight;
		}
	}

	// Get next line update rect
	if (nCurLine < (int)m_lyrLines.size() - 1)
	{
		LyricsLine *pNextLine;
		pNextLine = m_lyrLines[nCurLine + 1];

		nAlpha = GetAlpha(pNextLine);
		if (m_nNextLineAlphaOld != nAlpha)
		{
			m_nNextLineAlphaOld = nAlpha;

			// Redraw the whole line.
			if (prcUpdate->top == prcUpdate->bottom)
				prcUpdate->top = y + nRowHeight;
			prcUpdate->bottom = y + nRowHeight * 2;
		}
	}

	if (prcUpdate->top < m_rcObj.top + m_nYMargin)
		prcUpdate->top = m_rcObj.top + m_nYMargin;
	if (prcUpdate->bottom > m_rcObj.bottom - m_nYMargin)
		prcUpdate->bottom = m_rcObj.bottom - m_nYMargin;
}

void CLyricShowMultiRowObj::FastestDraw_GetUpdateRectOfNormal(CRawGraph *pGraphics, CRect *prcUpdate, int y)
{
	if (!IsKaraoke())
	{
		prcUpdate->SetRect(0, 0, 0, 0);
		return;
	}

	prcUpdate->top = y;
	prcUpdate->bottom = y + GetLineHeight();
	if (prcUpdate->top < m_rcObj.top + m_nYMargin)
		prcUpdate->top = m_rcObj.top + m_nYMargin;
	if (prcUpdate->bottom > m_rcObj.bottom - m_nYMargin)
		prcUpdate->bottom = m_rcObj.bottom - m_nYMargin;
}
