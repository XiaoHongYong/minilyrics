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

	m_nAlphaOld = 0;

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

	int				y;
	PML_ROW			pLyricRow;
	int				nRowHeight;// center row height
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

	pLyricRow = m_lyrLines[nRowCur];

	// 行高
	nRowHeight = GetLineHeight();

	// 此行歌词的时间
	nDeltaTime = pLyricRow->nEndTime - pLyricRow->nBegTime;

	// 计算此行歌词显示的座标 Y
	if (nDeltaTime == 0 && m_nCurRowOld == nRowCur)
		y = m_yDrawingOld;
	else
	{
		y = GetLineVertAlignPos();
		if (m_pMLData->GetPlayElapsedTime() < pLyricRow->nBegTime || nDeltaTime == 0)
			;
		else if (m_pMLData->GetPlayElapsedTime() > pLyricRow->nEndTime)
			y += -nRowHeight;
		else
			y += -nRowHeight + 
				nRowHeight * (pLyricRow->nEndTime - m_pMLData->GetPlayElapsedTime()) / nDeltaTime;
	}

	if (prcUpdate && y == m_yDrawingOld && nRowCur == m_nCurRowOld)
	{
		if (m_LyricsDisplayOpt == DO_FADEOUT_BG)
		{
			if (FastestDrawFadeInOut(pGraphics, prcUpdate, y, pLyricRow))
				return;
		}
		else if (m_LyricsDisplayOpt == DO_FADEOUT_LOWCOLOR)
		{
			if (FastestDrawFadeInOut(pGraphics, prcUpdate, y, pLyricRow))
				return;
		}
		else
		{
			// DO_NORMAL
			if (FastestDrawNormal(pGraphics, prcUpdate, y, pLyricRow))
				return;
		}
	}
	else
	{
		// 保存这次的值
		m_nCurRowOld = nRowCur;
		m_yDrawingOld = y;
	}

	// clear background
	UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);

	CRect	rcClip;

	rcClip.SetRect(m_rcObj.left + m_nXMargin, 
						m_rcObj.top + m_nYMargin,
						m_rcObj.right - m_nXMargin,
						m_rcObj.bottom - m_nYMargin);
	//
	// 设置剪裁区域为该UIObject的大小
	pGraphics->SetClipBoundBox(rcClip);

	// draw center row, the row lyric is current
	// select Center font
	int nRow, nRowEnd;
	int h;

	DrawCurrentRow(pGraphics, pLyricRow, AUTO_CAL_X, y);

	// 显示当前播放行前的歌词
	nRow = nRowCur - 1;
	nRowEnd = 0;
	for (h = y - nRowHeight; h + nRowHeight > m_rcObj.top && nRow >= nRowEnd; h -= nRowHeight)
	{
		pRow = m_lyrLines[nRow];

		if (!DrawRow(pGraphics, pRow, AUTO_CAL_X, h, LP_ABOVE_CUR_LINE))
			break;

		nRow--;
	}

	// 显示此行之后的歌词行
	nRow = nRowCur + 1;
	nRowEnd = m_lyrLines.size();
	for (h = y + nRowHeight; h < m_rcObj.bottom && nRow < nRowEnd; h += nRowHeight)
	{
		pRow = m_lyrLines[nRow];
		if (!DrawRow(pGraphics, pRow, AUTO_CAL_X, h, LP_BELOW_CUR_LINE))
			break;
		nRow++;
	}

	// 恢复为原来的剪裁区域大小
	pGraphics->RecoverClipBox();

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
}

bool CLyricShowMultiRowObj::FastestDrawFadeInOut(CRawGraph *pGraphics, CRect *prcUpdate, int y, PML_ROW pLyricRow)
{
	int			nPlayPos = m_pMLData->GetPlayElapsedTime();
	int			nRowHeight = GetLineHeight() + CRawBmpFont::OUTLINED_BORDER;
	int			nAlpha;

	if (nPlayPos >= pLyricRow->nBegTime - 1000 || nPlayPos <= pLyricRow->nBegTime + 1000)
		nAlpha = GetAlpha(pLyricRow);
	else if (nPlayPos >= pLyricRow->nEndTime - 1000 || nPlayPos <= pLyricRow->nEndTime + 1000)
		nAlpha = GetAlpha(pLyricRow);
	else
		nAlpha = 255;

	if (m_nAlphaOld == nAlpha)
	{
		if (IsKaraoke())
		{
// 		}
// 	}
// 	if (nPlayPos - pLyricRow->nBegTime >= 1000 && pLyricRow->nEndTime - nPlayPos >= 1000)
// 	{
// 		if (m_bKaraoke)
// 		{
			prcUpdate->SetRect(m_rcObj.left + m_nXMargin, 
								m_rcObj.top + m_nYMargin,
								m_rcObj.right - m_nXMargin,
								m_rcObj.bottom - m_nYMargin);

			if (prcUpdate->top < y)
				prcUpdate->top = y;
			if (prcUpdate->bottom > y + nRowHeight)
				prcUpdate->bottom = y + nRowHeight;

			// clear background
			UpdateLyricDrawBufferBackground(pGraphics, *prcUpdate);

			pGraphics->SetClipBoundBox(*prcUpdate);

			DrawCurrentRow(pGraphics, pLyricRow, AUTO_CAL_X, y);

			pGraphics->RecoverClipBox();
		}
		else
			prcUpdate->SetRect(0, 0, 0, 0);
		return true;
	}

	return false;
}

bool CLyricShowMultiRowObj::FastestDrawNormal(CRawGraph *pGraphics, CRect *prcUpdate, int y, PML_ROW pLyricRow)
{
	int			nRowHeight = GetLineHeight() + CRawBmpFont::OUTLINED_BORDER;

	//
	// DO_NORMAL
	//
	if (!IsKaraoke())
	{
		prcUpdate->SetRect(0, 0, 0, 0);
		return true;
	}

	prcUpdate->top = y;
	prcUpdate->bottom = y + nRowHeight;
	if (prcUpdate->top < m_rcObj.top + m_nYMargin)
		prcUpdate->top = m_rcObj.top + m_nYMargin;
	if (prcUpdate->bottom > m_rcObj.bottom - m_nYMargin)
		prcUpdate->bottom = m_rcObj.bottom - m_nYMargin;

	prcUpdate->left = m_rcObj.left + m_nXMargin;
	prcUpdate->right = m_rcObj.right - m_nXMargin;

	// clear background
	UpdateLyricDrawBufferBackground(pGraphics, *prcUpdate);

	//
	// 保存旧的剪裁区域, 并且设置剪裁区域为该UIObject的大小
	pGraphics->SetClipBoundBox(*prcUpdate);
	// pGraphics->SetFont(m_hfntLyric);

	// 显示当前歌词行
	DrawRowKaraoke(pGraphics, pLyricRow, AUTO_CAL_X, y);

	// 恢复为原来的剪裁区域大小
	pGraphics->RecoverClipBox();

	return true;
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
