// LyricShowHorzScrollObj.cpp: implementation of the CLyricShowHorzScrollObj class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowHorzScrollObj.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// set classname
LPCTSTR CLyricShowHorzScrollObj::ms_szClassName = _T("LyricShowHorzScroll");

CLyricShowHorzScrollObj::CLyricShowHorzScrollObj()
{
	m_nDarkenTopArea = float(0.65);
	m_nDarkenBottomArea = float(0.65);

	g_Player.SetTimeUpdateFreq(2);
}

CLyricShowHorzScrollObj::~CLyricShowHorzScrollObj()
{
	g_Player.SetTimeUpdateFreq(1);
}

// OUTPUT:
//		rcUpdate	-	更新的矩形区域
void CLyricShowHorzScrollObj::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate/* = NULL*/)
{
	int			x, y;
	PML_ROW		pLyricRow;
	int			nRowWidth;
	int			nRowHeight;// center row height
	int			nLeft, nRight, nCX, nCY;
	int			nRowCur;
	PML_ROW			pRow;

	int				_ROW_INTERVAL;


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

	pGraphics->SetFont(&m_font);

	pLyricRow = m_lyrLines[nRowCur];

	if (m_lyrLines.size() != m_vA.size())
		CalculateAccelerate();

	nRowHeight = GetLineHeight();

	_ROW_INTERVAL = m_nFontHeight / 2;

	y = GetLineVertAlignPos() - 
		GetLineHeight() / 2;

	nLeft = m_rcObj.left + m_nXMargin;
	nRight = m_rcObj.right - m_nXMargin;
	nCX = nRight - nLeft;
	nCY = m_rcObj.Height() - m_nYMargin * 2;

	int		nRow;
	int		nCurRowPosx;
	int		nStartX;

	nRowWidth = GetLyricRowTextWidth(pGraphics, pLyricRow);

	int		nDeltaT = m_pMLData->GetPlayElapsedTime() - pLyricRow->nBegTime;
	if (nDeltaT <= 0)
		nCurRowPosx = 0;
	else
		nCurRowPosx = int((m_vA[nRowCur] * nDeltaT * nDeltaT) / 2 + m_vV[nRowCur] * nDeltaT);

	if (nCurRowPosx > nRowWidth + _ROW_INTERVAL)
		nCurRowPosx = nRowWidth + _ROW_INTERVAL;
	else if (nCurRowPosx < 0)
		nCurRowPosx = nRowWidth + _ROW_INTERVAL;
	else if (nDeltaT > 0)
	{
		double		v = m_vV[nRowCur] + m_vA[nRowCur] * nDeltaT;
		if (v < 0.0f)
			nCurRowPosx = nRowWidth + _ROW_INTERVAL;
	}

	nStartX = nLeft + nCX / 2 - nCurRowPosx;
	x = nStartX;

	// clear back buffer
	UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);

	CRect	rcClip;
	
	rcClip.SetRect(m_rcObj.left + m_nXMargin, 
						m_rcObj.top + m_nYMargin,
						m_rcObj.right - m_nXMargin,
						m_rcObj.bottom - m_nYMargin);

	pGraphics->SetClipBoundBox(rcClip);

	nRow = nRowCur - 1;

	// 显示当前行之前的歌词行
	while (x > nLeft && nRow >= 0)
	{
		pRow = m_lyrLines[nRow];
		nRowWidth = GetLyricRowTextWidth(pGraphics, pRow);

		x -= nRowWidth + _ROW_INTERVAL;

		DrawRow(pGraphics, pRow, x, y, LP_ABOVE_CUR_LINE);

		nRow--;
	}

	// 显示当前歌词行
	x = nStartX;

	DrawCurrentRow(pGraphics, pLyricRow, x, y);

	x += GetLyricRowTextWidth(pGraphics, pLyricRow) + _ROW_INTERVAL;
	nRow = nRowCur + 1;

	// 显示以后歌词行
	while (x < nRight && nRow < (int)m_lyrLines.size())
	{
		pRow = m_lyrLines[nRow];

		DrawRow(pGraphics, pRow, x, y, LP_BELOW_CUR_LINE);

		nRowWidth = GetLyricRowTextWidth(pGraphics, pRow);

		x += nRowWidth + _ROW_INTERVAL;
		nRow++;
	}

	pGraphics->RecoverClipBox();

#ifdef _WIN32_DESKTOP
	// if (m_pSkin->GetEnableTranslucencyLayered())
	{
		CRect	rc;

		rc.SetRect(m_rcObj.left, m_rcObj.top, m_rcObj.left + 30, m_rcObj.bottom);
		if (rc.right > m_rcObj.right)
			rc.right = m_rcObj.right;
		if (m_bFadeOutBorder)
			pGraphics->HorzAlphaFadeOut(&rc, true);
		else
			pGraphics->HorzFadeOut(&rc, m_pSkin->GetBgColor(), true);

		rc.right = m_rcObj.right;
		rc.left = m_rcObj.right - 30;
		if (rc.left < m_rcObj.left)
			rc.left = m_rcObj.left;
		if (m_bFadeOutBorder)
			pGraphics->HorzAlphaFadeOut(&rc, false);
		else
			pGraphics->HorzFadeOut(&rc, m_pSkin->GetBgColor(), false);
	}
#endif
}

LPCTSTR CLyricShowHorzScrollObj::GetClassName()
{
	return ms_szClassName;
}

BOOL CLyricShowHorzScrollObj::IsKindOf(LPCTSTR szClassName)
{
	if (CLyricShowObj::IsKindOf(szClassName))
		return TRUE;

	return _tcsicmp(szClassName, ms_szClassName) == 0;
}

void CLyricShowHorzScrollObj::OnLyrDrawContextChanged()
{
	CLyricShowObj::OnLyrDrawContextChanged();

	CalculateAccelerate();

	AutoHeightSkinAccordLyrics();
}

void CLyricShowHorzScrollObj::CalculateAccelerate()
{
	if (!m_pSkin)
		return;

	CRawGraph	*pGraphics;
	PML_ROW			pRow;
	int				nRowWidth;
	double			V0 = 0, v = 0, a, deltat;

	int				_ROW_INTERVAL;

	pGraphics = m_pSkin->GetMemGraphics();
	ASSERT(pGraphics);

	pGraphics->SetFont(&m_font);

	_ROW_INTERVAL = m_nFontHeight / 2;

	m_vA.clear();
	m_vV.clear();
	for (int i = 0; i < (int)m_lyrLines.size(); i++)
	{
		V0 = v;
		pRow = m_lyrLines[i];
		nRowWidth = GetLyricRowTextWidth(pGraphics, pRow) + _ROW_INTERVAL;
		deltat = pRow->nEndTime - pRow->nBegTime;
//			static int nLastEndTime = 0;
//			if (nLastEndTime != pRow->nBegTime)
//			{
//				DBGLOG3(_T("Row: %d, E: %d, B: %d"), i, nLastEndTime, pRow->nBegTime);
//			}
//			nLastEndTime = pRow->nEndTime;
		if (deltat == 0)
		{
			v = V0;
			a = 0;
		}
		else
		{
			// a = (nRowWidth * 2 - V0 * deltat) / (deltat * deltat);
			// v = V0 + a * deltat;
			a = (nRowWidth - V0 * deltat) * 2 / (deltat * deltat);
			v = V0 + a * deltat;
			if (v < 0)
			{
				a = - (V0 * V0) / (nRowWidth * 2);
				v = 0;
			}
		}
		// DBGLOG3(_T("Row: %d, V: %d, A: %d"), i, (int)(V0 * 1000), (int)(a * 1000 * 1000));
		m_vV.push_back(V0);
		m_vA.push_back(a);
	}
}
