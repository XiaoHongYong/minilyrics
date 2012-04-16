// LyricShowSingleRowObj.cpp: implementation of the CLyricShowSingleRowObj class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowSingleRowObj.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// set classname
LPCTSTR CLyricShowSingleRowObj::ms_szClassName = _T("LyricShowSingleRow");

CLyricShowSingleRowObj::CLyricShowSingleRowObj()
{
	m_nDarkenTopArea = float(0.65);
	m_nDarkenBottomArea = float(0.65);

	m_nXMargin = 5;
	m_nYMargin = 0;
}

CLyricShowSingleRowObj::~CLyricShowSingleRowObj()
{

}

#define  FIO_TIME		500

//
// 取得此行歌词的Fade out color
//
void CLyricShowSingleRowObj::GetCurLineFadeOutColor(LyricsLine *pLine, MLColor clrIn, MLColor clrOut, MLColor &color)
{
	int				nTime;

	if (!pLine)
		return;

	nTime = pLine->nEndTime - m_pMLData->GetPlayElapsedTime();
	if (nTime <= FIO_TIME && nTime >= 0)
	{
		int		nPart1;

		nPart1 = FIO_TIME - nTime;
		color.set((clrOut.r() * nPart1 + clrIn.r() * nTime) / FIO_TIME,
			(clrOut.g() * nPart1 + clrIn.g() * nTime) / FIO_TIME,
			(clrOut.b() * nPart1 + clrIn.b() * nTime) / FIO_TIME);
	}
	else
		color = clrOut;
}

//
// 取得此行歌词的Fade In color
//
void CLyricShowSingleRowObj::GetNextLineFadeInColor(LyricsLine *pLine, MLColor clrIn, MLColor clrOut, MLColor &color)
{
	int				nTime;

	if (!pLine)
		return;

	// nTime 变小
	nTime = pLine->nBegTime - m_pMLData->GetPlayElapsedTime();
	if (nTime <= FIO_TIME && nTime >= 0)
	{
		int		nPart1;

		nPart1 = nTime;
		nTime = FIO_TIME - nTime;

		color.set((clrOut.r() * nPart1 + clrIn.r() * nTime) / FIO_TIME,
			(clrOut.g() * nPart1 + clrIn.g() * nTime) / FIO_TIME,
			(clrOut.b() * nPart1 + clrIn.b() * nTime) / FIO_TIME);
	}
	else
		color = clrIn;
}

void AlphaBlendColor(MLColor &clr1, MLColor &clr2, int nAlpha, MLColor &clrOut);

void CLyricShowSingleRowObj::DrawCurLineFadeInNextLine(CRawGraph *pGraphics, LyricsLine *pLineCur, LyricsLine *pLineNext, int x, int y)
{
	int				nTime;
	int				nAlphaCurLine, nAlphaNextLine;

	nTime = pLineCur->nEndTime - m_pMLData->GetPlayElapsedTime();
	if (nTime <= FIO_TIME && nTime >= 0)
		nAlphaCurLine = (FIO_TIME - nTime) * 255 / FIO_TIME;
	else
		nAlphaCurLine = 255;

	if (pLineNext)
	{
		nTime = pLineNext->nBegTime - m_pMLData->GetPlayElapsedTime();
		if (nTime <= FIO_TIME && nTime >= 0)
			nAlphaNextLine = nTime * 255 / FIO_TIME;
		else
			nAlphaNextLine = 255;
	}
	else
		nAlphaNextLine = 255 - nAlphaCurLine;


	if (m_pSkin->GetEnableTranslucencyLayered() || m_img.IsValid())
	{
		// transparent window
		byte		alphaOld = pGraphics->GetOpacityPainting();
		pGraphics->SetOpacityPainting(nAlphaNextLine);
		DrawRow(pGraphics, pLineCur, AUTO_CAL_X, y, GetHighlightColor(), m_tobHilight.clr[TCI_BORDER]);

		if (pLineNext)
		{
			pGraphics->SetOpacityPainting(nAlphaCurLine);

			DrawRow(pGraphics, pLineNext, AUTO_CAL_X, y, GetHighlightColor(), m_tobHilight.clr[TCI_BORDER]);
		}
		pGraphics->SetOpacityPainting(alphaOld);
	}
	else
	{
		MLColor		clrTxt, clrTxtBorder;

		AlphaBlendColor(m_tobHilight.clr[TCI_BORDER], m_clrBg, nAlphaCurLine, clrTxtBorder);
		AlphaBlendColor(GetHighlightColor(), m_clrBg, nAlphaCurLine, clrTxt);

		DrawRow(pGraphics, pLineCur, AUTO_CAL_X, y, clrTxt, clrTxtBorder);

		if (pLineNext)
		{
			AlphaBlendColor(m_tobHilight.clr[TCI_BORDER], m_clrBg, nAlphaNextLine, clrTxtBorder);
			AlphaBlendColor(GetHighlightColor(), m_clrBg, nAlphaNextLine, clrTxt);

			DrawRow(pGraphics, pLineNext, AUTO_CAL_X, y, clrTxt, clrTxtBorder);
		}
	}
}

// OUTPUT:
//		rcUpdate	-	更新的矩形区域
void CLyricShowSingleRowObj::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate/* = NULL*/)
{
	int			y;
	LyricsLine *pLyricRow;
	int			nRowCur;
	int			nPlayPos;

	pGraphics->SetFont(&m_font);

	if (prcUpdate)
		*prcUpdate = m_rcObj;

	// get the current playing row
	nRowCur = m_pMLData->GetCurPlayLine(m_lyrLines);
	if (nRowCur == -1)
	{
		// no lyrics, redraw background
		UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);
		return;
	}

	pLyricRow = m_lyrLines[nRowCur];

	// clear back buffer
	UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);

	nPlayPos = m_pMLData->GetPlayElapsedTime();
	y = GetLineVertAlignPos() - GetFontHeight() / 2;

	// 在当前行结束前0.5秒渐变切换到下一行
	if (nPlayPos + FIO_TIME >= pLyricRow->nEndTime)
	{
		// 计算渐变显示的颜色
		LyricsLine *pLineNext = NULL;

		CRect	rcClip;

		rcClip.SetRect(m_rcObj.left + m_nXMargin, 
							m_rcObj.top + m_nYMargin,
							m_rcObj.right - m_nXMargin,
							m_rcObj.bottom - m_nYMargin);
		// 设置剪裁区域为: 左边边框-->nSeperatePos
		CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraphics);
		pGraphics->SetClipBoundBox(rcClip);

		if (nRowCur + 1 < (int)m_lyrLines.size())
			pLineNext = m_lyrLines[nRowCur + 1];

		DrawCurLineFadeInNextLine(pGraphics, pLyricRow, pLineNext, -1, y);
	}
	else
	{

		CRect	rcClip;

		rcClip.SetRect(m_rcObj.left + m_nXMargin, 
							m_rcObj.top + m_nYMargin,
							m_rcObj.right - m_nXMargin,
							m_rcObj.bottom - m_nYMargin);
		// 设置剪裁区域为: 左边边框-->右边歌词的开始显示部分
		CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraphics);
		pGraphics->SetClipBoundBox(rcClip);

		DrawRow(pGraphics, pLyricRow, AUTO_CAL_X, y, LP_CUR_LINE);
	}
}

LPCTSTR CLyricShowSingleRowObj::GetClassName()
{
	return ms_szClassName;
}

BOOL CLyricShowSingleRowObj::IsKindOf(LPCTSTR szClassName)
{
	if (CLyricShowObj::IsKindOf(szClassName))
		return TRUE;

	return _tcsicmp(szClassName, ms_szClassName) == 0;
}
