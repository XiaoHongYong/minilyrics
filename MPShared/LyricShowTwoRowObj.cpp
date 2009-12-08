/********************************************************************
	Created  :	2001年12月27日 0:01:36
	FileName :	LyricShowTwoRowObj.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowTwoRowObj.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// set classname
LPCTSTR CLyricShowTwoRowObj::ms_szClassName = _T("LyricShowTwoRow");

CLyricShowTwoRowObj::CLyricShowTwoRowObj()
{
	m_nCurRowOld = -1;
	m_nRowTheOtherOld = -1;
	m_nDarkenTopArea = float(1.25);
	m_nDarkenBottomArea = float(1.25);
}

CLyricShowTwoRowObj::~CLyricShowTwoRowObj()
{
}

void CLyricShowTwoRowObj::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate/* = NULL*/)
{
	if (m_pMLData == NULL)
		return;

	int			y;
	PML_ROW		pLyricRow;
	int			nRowHeight;// center row height
	bool		bTheOtherRowHiglight = false;
	int			nRowTheOther = 0;
	int			nRowCur;
	PML_ROW			pRowTheOther = NULL;

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

	nRowHeight = GetLineHeight();

	y = GetLineVertAlignPos() - GetLineHeight() + m_nLineSpacing / 2;
	if (y < m_rcObj.top + m_nYMargin)
		y = m_rcObj.top + m_rcObj.Height() / 2 - GetLineHeight() + m_nLineSpacing / 2;
	
	// 如果当前歌词行的显示时间超过一半，则预显示下一行歌词
	if ( (m_pMLData->GetPlayElapsedTime() - pLyricRow->nBegTime) * 2 >= pLyricRow->nEndTime - m_pMLData->GetPlayElapsedTime())
		nRowTheOther = nRowCur + 1;//bShowNextRow = TRUE;
	else
	{
		nRowTheOther = nRowCur - 1;
		if (nRowTheOther < 0)
			// 当前行为最开始的一行，则显示下一行
			nRowTheOther = nRowCur + 1;
	}

	if (nRowTheOther >= (int)m_lyrLines.size())
	{
		// 当前行为最后一行，则显示前一行
		nRowTheOther = nRowCur - 1;
	}
	if (nRowTheOther < 0)
		nRowTheOther = -1;
	else
		pRowTheOther = m_lyrLines[nRowTheOther];

	int		yCurRow;
	int		yTheOtherRow;
	if (nRowCur % 2 == 0)
	{
		yCurRow = y;
		yTheOtherRow = y + nRowHeight;
	}
	else
	{
		yCurRow = y + nRowHeight;
		yTheOtherRow = y;
	}

	if (prcUpdate && m_nCurRowOld == nRowCur 
		&& m_nRowTheOtherOld == nRowTheOther
		&& m_LyricsDisplayOpt == DO_NORMAL)
	{
		if (!IsKaraoke())
		{
			prcUpdate->SetRect(0, 0, 0, 0);
			// 如果没有使用KOROKE方式则直接返回
			return;
		}

		// 只重绘当前歌词行
		prcUpdate->top = yCurRow;
		prcUpdate->bottom = yCurRow + nRowHeight;
		if (prcUpdate->top < m_rcObj.top + m_nYMargin)
			prcUpdate->top = m_rcObj.top + m_nYMargin;
		if (prcUpdate->bottom > m_rcObj.bottom - m_nYMargin)
			prcUpdate->bottom = m_rcObj.bottom - m_nYMargin;

		// 当前行歌词没有超出歌词显示区域
		prcUpdate->left = m_rcObj.left + m_nXMargin;
		prcUpdate->right = m_rcObj.right - m_nXMargin;

		// clear background
		UpdateLyricDrawBufferBackground(pGraphics, *prcUpdate);

		CRect	rcClip;

		rcClip.SetRect(prcUpdate->left, 
							prcUpdate->top,
							prcUpdate->right,
							prcUpdate->bottom);
		//
		// 保存旧的剪裁区域, 并且设置剪裁区域为该UIObject的大小
		pGraphics->SetClipBoundBox(rcClip);

		// 显示当前歌词行
		DrawCurrentRow(pGraphics, pLyricRow, AUTO_CAL_X, yCurRow);

		// 恢复为原来的剪裁区域大小
		pGraphics->RecoverClipBox();

		return;
	}
	else
	{
		m_nCurRowOld = nRowCur;
		m_nRowTheOtherOld = nRowTheOther;
	}

	// clear back buffer
	if (prcUpdate)
	{
		prcUpdate->SetRect(m_rcObj.left + m_nXMargin, y, m_rcObj.right - m_nXMargin, y + nRowHeight * 2);
		if (prcUpdate->top < m_rcObj.top + m_nYMargin)
			prcUpdate->top = m_rcObj.top + m_nYMargin;
		if (prcUpdate->bottom > m_rcObj.bottom - m_nYMargin)
			prcUpdate->bottom = m_rcObj.bottom - m_nYMargin;
		UpdateLyricDrawBufferBackground(pGraphics, *prcUpdate);
		pGraphics->SetClipBoundBox(*prcUpdate);
	}
	else
	{
		UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);

		CRect	rcClip;

		rcClip.SetRect(m_rcObj.left + m_nXMargin, 
							m_rcObj.top + m_nYMargin,
							m_rcObj.right - m_nXMargin,
							m_rcObj.bottom - m_nYMargin);
		pGraphics->SetClipBoundBox(rcClip);
	}

	// 显示当前歌词行
	DrawCurrentRow(pGraphics, pLyricRow, AUTO_CAL_X, yCurRow);

	if (pRowTheOther)
	{
		// 显示前（后）一歌词行
		DrawRow(pGraphics, pRowTheOther, AUTO_CAL_X, yTheOtherRow, 
			nRowTheOther < nRowCur ? LP_ABOVE_CUR_LINE : LP_BELOW_CUR_LINE);
	}

	// 恢复为原来的剪裁区域大小
	pGraphics->RecoverClipBox();
}

LPCTSTR CLyricShowTwoRowObj::GetClassName()
{
	return ms_szClassName;
}

BOOL CLyricShowTwoRowObj::IsKindOf(LPCTSTR szClassName)
{
	if (CLyricShowObj::IsKindOf(szClassName))
		return TRUE;

	return _tcsicmp(szClassName, ms_szClassName) == 0;
}
