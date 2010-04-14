/********************************************************************
	Created  :	2001年12月27日 0:02:48
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

	m_bPaused = false;
	m_bResume = false;
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

void CLyricShowMultiRowObj::Pause()
{
	m_bPaused = true;
	m_bResume = false;
}

void CLyricShowMultiRowObj::Resume()
{
	m_bPaused = false;
	m_bResume = true;
	m_nTimeBegSync = GetTickCount();
}

void CLyricShowMultiRowObj::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate/* = NULL*/)
{
	assert(m_pMLData);
	if (m_pMLData == NULL)
		return;

	if (!m_pMLData->HasLyricsOpened())
	{
		FastDrawMediaInfo(pGraphics, prcUpdate);
		return;
	}

	pGraphics->SetFont(&m_font);

	CDispOptRecover		dispOptRecover(this);
	if (m_img.IsValid())
		m_LyricsDisplayOpt = DO_FADEOUT_BG;

	if (prcUpdate)
		*prcUpdate = m_rcObj;

	int				y, yCurRow;
	PML_ROW			pRowCur;
	int				nRowHeight;
	int				nDeltaTime;
	int				nRowCur;
	PML_ROW			pRow;


	// get the current playing line
	nRowCur = m_pMLData->GetCurPlayLine(m_lyrLines);
	if (nRowCur == -1)
	{
		// no lyrics, redraw the whole background
		UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);
		return;
	}

	pRowCur = m_lyrLines[nRowCur];

	// 行高
	nRowHeight = GetLineHeight();

	// 此行歌词的时间
	nDeltaTime = pRowCur->nEndTime - pRowCur->nBegTime;

	// 计算此行歌词显示的座标 Y
	if (nDeltaTime == 0 && m_nCurRowOld == nRowCur)
		yCurRow = m_yDrawingOld;
	else
	{
		yCurRow = GetLineVertAlignPos();
		if (m_pMLData->GetPlayElapsedTime() < pRowCur->nBegTime || nDeltaTime == 0)
			;
		else if (m_pMLData->GetPlayElapsedTime() > pRowCur->nEndTime)
			yCurRow += -nRowHeight;
		else
			yCurRow += -nRowHeight + 
				nRowHeight * (pRowCur->nEndTime - m_pMLData->GetPlayElapsedTime()) / nDeltaTime;

		if (m_bPaused && m_yDrawingOld != -1)
			yCurRow = m_yDrawingOld;
	}

	if (m_bResume)
	{
		// 
		uint32 nDeltaTime = GetTickCount() - m_nTimeBegSync;
		if (nDeltaTime < 2000)
		{
			if (nRowCur == m_nCurRowOld && yCurRow > m_yDrawingOld)
				yCurRow = m_yDrawingOld;
		}
		else
			m_bResume = false;
	}

	CRect	rcClip;

	rcClip.SetRect(m_rcObj.left + m_nXMargin, 
		m_rcObj.top + m_nYMargin,
		m_rcObj.right - m_nXMargin,
		m_rcObj.bottom - m_nYMargin);

	if (prcUpdate && yCurRow == m_yDrawingOld && nRowCur == m_nCurRowOld)
	{
		*prcUpdate = rcClip;

		if (m_LyricsDisplayOpt == DO_FADEOUT_BG || m_LyricsDisplayOpt == DO_FADEOUT_LOWCOLOR)
			FastestDraw_GetUpdateRectOfFadeInOut(pGraphics, prcUpdate, yCurRow, nRowCur);
		else
			FastestDraw_GetUpdateRectOfNormal(pGraphics, prcUpdate, yCurRow);

		rcClip = *prcUpdate;

		if (rcClip.IsRectEmpty())
			return;

		// clear background
		UpdateLyricDrawBufferBackground(pGraphics, rcClip);
	}
	else
	{
		// 保存这次的值
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
	DrawCurrentRow(pGraphics, pRowCur, AUTO_CAL_X, yCurRow);

	// Draw lyrics lines above current line
	nRow = nRowCur - 1;
	nRowEnd = 0;
	for (y = yCurRow - nRowHeight; y + nRowHeight > rcClip.top && nRow >= nRowEnd; y -= nRowHeight)
	{
		pRow = m_lyrLines[nRow];

		if (!DrawRow(pGraphics, pRow, AUTO_CAL_X, y, LP_ABOVE_CUR_LINE))
			break;
		nRow--;
	}
	yLyrStart = y + nRowHeight;

	// Draw lyrics lines below current line
	nRow = nRowCur + 1;
	nRowEnd = m_lyrLines.size();
	for (y = yCurRow + nRowHeight; y < rcClip.bottom && nRow < nRowEnd; y += nRowHeight)
	{
		pRow = m_lyrLines[nRow];
		if (!DrawRow(pGraphics, pRow, AUTO_CAL_X, y, LP_BELOW_CUR_LINE))
			break;
		nRow++;
	}
	yLyrEnd = y;

#ifdef _WIN32_DESKTOP
	if ((!m_pSkin->IsBgImageUsed() && !m_img.IsValid()) || m_pSkin->m_nTranslucencyAlphaDynamic <= 0)
	{
		CRect	rc;

		rc.SetRect(m_rcObj.left, m_rcObj.top, m_rcObj.right, m_rcObj.top + m_nFontHeight);
		if (rc.bottom > yLyrStart)
		{
			if (m_pSkin->GetEnableTranslucencyLayered() && m_pSkin->m_nTranslucencyAlphaDynamic <= 0)
				pGraphics->VertAlphaFadeOut(&rc, true);
			else
				pGraphics->VertFadeOut(&rc, m_pSkin->GetBgColor(), true);
		}

		rc.bottom = m_rcObj.bottom;
		rc.top = m_rcObj.bottom - m_nFontHeight;
		if (rc.top < yLyrEnd)
		{
			if (m_pSkin->GetEnableTranslucencyLayered() && m_pSkin->m_nTranslucencyAlphaDynamic <= 0)
				pGraphics->VertAlphaFadeOut(&rc, false);
			else
				pGraphics->VertFadeOut(&rc, m_pSkin->GetBgColor(), false);
		}
	}
#endif

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

void CLyricShowMultiRowObj::FastDrawMediaInfo(CRawGraph *pGraph, CRect *prcUpdate)
{
	if (prcUpdate)
	{
		prcUpdate->left = prcUpdate->top = prcUpdate->right = prcUpdate->bottom = 0;
		return;
	}

	UpdateLyricDrawBufferBackground(pGraph, m_rcObj);

	pGraph->SetFont(&m_font);

	int		nLineHeight = GetLineHeight();
	int		y = (m_rcObj.top + m_rcObj.bottom) / 2 - m_lyrLines.size() * nLineHeight / 2;
	int		x;

	if (m_lyrLines.size() > 1)
		x = m_rcObj.left + m_nXMargin;
	else
		x = AUTO_CAL_X;

	if (y < m_rcObj.top + m_nYMargin)
		y = m_rcObj.top + m_nYMargin;

	CRect	rcClip;

	rcClip.SetRect(m_rcObj.left + m_nXMargin, 
		m_rcObj.top + m_nYMargin,
		m_rcObj.right - m_nXMargin,
		m_rcObj.bottom - m_nYMargin);

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraph);
	pGraph->SetClipBoundBox(rcClip);

	for (size_t i = 0; i < m_lyrLines.size(); i++)
	{
		DrawRow(pGraph, m_lyrLines[i], x, y, LP_CUR_LINE);
		y += nLineHeight;
	}
}

void CLyricShowMultiRowObj::FastestDraw_GetUpdateRectOfFadeInOut(CRawGraph *pGraphics, CRect *prcUpdate, int y, int nCurLine)
{
	int			nRowHeight = GetLineHeight();
	int			nAlpha;

	ASSERT(nCurLine >= 0 && nCurLine < m_lyrLines.size());

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
		PML_ROW		pPrevLine;
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
		PML_ROW		pNextLine;
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
