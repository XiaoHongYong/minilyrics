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

	m_nDarkenTopArea = float(1.5);
	m_nDarkenBottomArea = float(1.5);

	m_bOnlyDisplayCurLineLyr = true;
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


void CLyricShowMultiRowObj::OnCreate()
{
	CLyricShowObj::OnCreate();

	if (m_etDispSettings != ET_LYRICS_FLOATING_SETTINGS)
		m_bOnlyDisplayCurLineLyr = g_Profile.GetProfileBool(SZ_SECT_LYR_DISPLAY, _T("DispCurLineLyr"), true);
}

void CLyricShowMultiRowObj::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate/* = NULL*/)
{
	assert(m_pMLData);
	if (m_pMLData == NULL)
		return;

	pGraphics->SetFont(&m_font);

	CDispOptRecover		dispOptRecover(this);
	if (m_img.IsValid() && m_bOnlyDisplayCurLineLyr)
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

		pGraphics->SetClipBoundBox(rcClip);
	}
	else
	{
		// 保存这次的值
		m_nCurRowOld = nRowCur;
		m_yDrawingOld = yCurRow;

		// clear background
		UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);

		pGraphics->SetClipBoundBox(rcClip);
	}

	int nRow, nRowEnd;

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

#ifdef _WIN32_DESKTOP
	// if (/*m_pSkin->GetEnableTranslucencyLayered() && */m_LyricsDisplayOpt != DO_FADEOUT_BG)
	{
		CRect	rc;

		rc.SetRect(m_rcObj.left, m_rcObj.top, m_rcObj.right, m_rcObj.top + m_nFontHeight);
		if (rc.bottom > m_rcObj.bottom)
			rc.bottom = m_rcObj.bottom;
		if (m_pSkin->GetEnableTranslucencyLayered() && m_pSkin->m_nTranslucencyAlphaDynamic <= 0)
			pGraphics->VertAlphaFadeOut(&rc, true);
		else
			pGraphics->VertFadeOut(&rc, m_pSkin->GetBgColor(), true);

		rc.bottom = m_rcObj.bottom;
		rc.top = m_rcObj.bottom - m_nFontHeight;
		if (rc.top < m_rcObj.top)
			rc.top = m_rcObj.top;
		if (m_pSkin->GetEnableTranslucencyLayered() && m_pSkin->m_nTranslucencyAlphaDynamic <= 0)
			pGraphics->VertAlphaFadeOut(&rc, false);
		else
			pGraphics->VertFadeOut(&rc, m_pSkin->GetBgColor(), false);
	}
#endif

	// 恢复为原来的剪裁区域大小
	pGraphics->RecoverClipBox();
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
	if (prcUpdate->bottom < m_rcObj.bottom - m_nYMargin)
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

bool CLyricShowMultiRowObj::OnLyrDisplaySettings(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (IsPropertyName(szProperty, _T("DispCurLineLyr")))
	{
		m_bOnlyDisplayCurLineLyr = IsTRUE(szValue);
	}
	else
		return CLyricShowObj::OnLyrDisplaySettings(szProperty, szValue);

	return true;
}

void CLyricShowMultiRowObj::OnLyrDrawContextChanged()
{
	m_bCanWrapLines = true;

	CLyricShowObj::OnLyrDrawContextChanged();

	AutoWidthSkinAccordLyrics();

	if (m_etDispSettings == ET_LYRICS_FLOATING_SETTINGS
		&& m_LyricsDisplayOpt == DO_FADEOUT_BG)
		AutoHeightSkinAccordLyrics(3);
}
