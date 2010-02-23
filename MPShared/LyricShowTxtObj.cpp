/********************************************************************
	Created  :	2002年12月2日 10:48:18
	FileName :	LyricShowTxtObj.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowTxtObj.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// set classname
LPCTSTR CLyricShowTxtObj::ms_szClassName = _T("LyricShowTxt");

CLyricShowTxtObj::CLyricShowTxtObj()
{
	m_nRowCur = 0;

	m_nMsgWant |= UO_MSG_WANT_MOUSEWHEEL;

	m_brCurRow.CreateSolidBrush(m_clrFocusLineBg);

	m_nCurRowOld = -1;

	m_nDarkenTopArea = float(0.5);
	m_nDarkenBottomArea = float(0.5);

	m_pScrollBar = NULL;
	m_pObjScrollBar = NULL;
}

CLyricShowTxtObj::~CLyricShowTxtObj()
{
	if (m_pObjScrollBar)
		m_pSkin->RemoveUIObject(m_pObjScrollBar, true);
}

void CLyricShowTxtObj::Invalidate()
{
	// m_nRowCur = 0;
	m_nCurRowOld = -1;
	CLyricShowObj::Invalidate();
}

void CLyricShowTxtObj::OnKeyDown(UINT nChar, UINT nFlags)
{
	BOOL	bProcessed = FALSE;
	int		nRowOld;

	nRowOld = m_nRowCur;

	switch (nChar)
	{
	case VK_UP:
		// 跳到前一行歌词
		m_nRowCur--;
		if (m_nRowCur < 0)
			m_nRowCur = 0;
		bProcessed = TRUE;
		break;
	case VK_DOWN:
		m_nRowCur++;
		if (m_nRowCur >= (int)m_lyrLines.size())
			m_nRowCur = m_lyrLines.size() - 1;
		bProcessed = TRUE;
		break;
	case VK_HOME:
		m_nRowCur = 0;
		bProcessed = TRUE;
		break;
	case VK_END:
		m_nRowCur = m_lyrLines.size() - 1;
		bProcessed = TRUE;
		break;
	case VK_PRIOR:
		if (m_pScrollBar)
		{
			m_nRowCur -= m_pScrollBar->GetPage() - 1;
			if (m_nRowCur < 0)
				m_nRowCur = 0;
		}
		bProcessed = TRUE;
		break;
	case VK_NEXT:
		if (m_pScrollBar)
		{
			m_nRowCur += m_pScrollBar->GetPage() - 1;
			if (m_nRowCur >= (int)m_lyrLines.size())
				m_nRowCur = m_lyrLines.size() - 1;
		}
		bProcessed = TRUE;
		break;
	}

	if (!bProcessed)
	{
		CLyricShowObj::OnKeyDown(nChar, nFlags);
		bProcessed = TRUE;
	}

	if (nRowOld != m_nRowCur)
	{
		if (m_pScrollBar)
			m_pScrollBar->SetScrollPos(m_nRowCur);
		Invalidate();
	}
}

LPCTSTR CLyricShowTxtObj::GetClassName()
{
	return ms_szClassName;
}

void CLyricShowTxtObj::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate/* = NULL*/)
{
	assert(m_pMLData);

	int		y;
	PML_ROW	pLyricRow;
	int		nRowHeight;
	int		nCurRowReal;

	pGraphics->SetFont(&m_font);

	if (prcUpdate)
	{
		// *prcUpdate = m_rcObj;
		prcUpdate->SetRect(0, 0, 0, 0);
		return;
	}

	if (m_lyrLines.size() == 0)
	{
		UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);
		return;
	}

	if (m_nRowCur >= (int)m_lyrLines.size() || m_nRowCur < 0)
		m_nRowCur = 0;

	pLyricRow = m_lyrLines[m_nRowCur];

	nCurRowReal = m_pMLData->GetCurPlayLine(m_lyrLines);

	// 行高
	nRowHeight = GetLineHeight();

	y = GetLineVertAlignPos() - GetLineHeight() / 2;
	if (m_nRowCur == m_nCurRowOld)
		return;

	// clear background
	UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);

	CRect	rcClip;

	rcClip.SetRect(m_rcObj.left + m_nXMargin, 
						m_rcObj.top + m_nYMargin,
						m_rcObj.right - m_nXMargin,
						m_rcObj.bottom - m_nYMargin);

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraphics);
	pGraphics->SetClipBoundBox(rcClip);

	// draw center row, the row lyric is current
	// select Center font
	int nRow;
	int h;

	CDispOptRecover		dispOptRecover(this);
	m_LyricsDisplayOpt = DO_NORMAL;

	// 显示当前歌词行
	// DrawRow(pGraphics, pLyricRow, AUTO_CAL_X, y, GetHighlightColor(), m_tobHilight.clrBorder);
	DrawRow(pGraphics, pLyricRow, AUTO_CAL_X, y, LP_CUR_LINE);

	// 显示此行之前的歌词行
	nRow = m_nRowCur - 1;
	for (h = y - nRowHeight; h + nRowHeight > m_rcObj.top && nRow >= 0; h -= nRowHeight)
	{
		// DrawRow(pGraphics, m_lyrLines[nRow], AUTO_CAL_X, h, GetLowlightColor(), m_tobLowlight.clrBorder);
		DrawRow(pGraphics, m_lyrLines[nRow], AUTO_CAL_X, h, LP_ABOVE_CUR_LINE);
		nRow--;
	}
	
	// 显示此行之后的歌词行
	nRow = m_nRowCur + 1;
	for (h = y + nRowHeight; h < m_rcObj.bottom && nRow < (int)m_lyrLines.size(); h += nRowHeight)
	{
		// DrawRow(pGraphics, m_lyrLines[nRow], AUTO_CAL_X, h, GetLowlightColor(), m_tobLowlight.clrBorder);
		DrawRow(pGraphics, m_lyrLines[nRow], AUTO_CAL_X, h, LP_BELOW_CUR_LINE);
		nRow++;
	}
}

void CLyricShowTxtObj::OnLyricsChanged()
{
	CLyricShowObj::OnLyricsChanged();

	SetScrollInfo();
}

void CLyricShowTxtObj::OnLyrDrawContextChanged()
{
	CLyricShowObj::OnLyrDrawContextChanged();

	SetScrollInfo();
}

void CLyricShowTxtObj::SetScrollInfo()
{
	if (m_pScrollBar)
	{
		int				nPage = m_rcObj.Height() / GetLineHeight();

		m_nRowCur = (GetLineVertAlignPos() - m_rcObj.top) / GetLineHeight() - 1;
		if (m_nRowCur < 0)
			m_nRowCur = 0;

		m_pScrollBar->SetScrollInfo(0, m_lyrLines.size() - 1 + nPage, nPage, m_nRowCur);
	}
}

BOOL CLyricShowTxtObj::IsKindOf(LPCTSTR szClassName)
{
	if (CLyricShowObj::IsKindOf(szClassName))
		return TRUE;

	return _tcsicmp(szClassName, ms_szClassName) == 0;
}

void CLyricShowTxtObj::OnPlayTimeChangedUpdate()
{
	// No need to update lyrics with time
}

void CLyricShowTxtObj::OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	m_nRowCur = pScrollBar->GetScrollPos();

	if (m_nRowCur >= (int)m_lyrLines.size())
		m_nRowCur = m_lyrLines.size() - 1;

	Invalidate();
}

void CLyricShowTxtObj::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	if (m_pObjScrollBar)
		m_pObjScrollBar->OnMouseWheel(nWheelDistance, nMkeys, pt);
	else
		CLyricShowObj::OnMouseWheel(nWheelDistance, nMkeys, pt);
}

void CLyricShowTxtObj::OnCreate()
{
	CLyricShowObj::OnCreate();

	//
	// Create Scrollbar
	//
	m_pObjScrollBar = m_pSkin->GetSkinFactory()->CreateDynamicCtrl(m_pContainer, 
		CSkinVScrollBar::ClassName(),
		SKIN_UIOBJ_ID_UNDEFINE, NULL, NULL, NULL, NULL);
	ASSERT(m_pObjScrollBar);
	if (!m_pObjScrollBar)
		return;

	TCHAR		szTemp[MAX_PATH];

	// set left
	_stprintf_s(szTemp, CountOf(szTemp), _T("%s+%s-%s"), m_formLeft.GetFormula(), m_formWidth.GetFormula(),
		m_pObjScrollBar->m_formWidth.GetFormula());
	m_pObjScrollBar->SetProperty(SZ_PN_LEFT, szTemp);

	// top
	m_pObjScrollBar->SetProperty(SZ_PN_TOP, m_formTop.GetFormula());

	// width of this
	_stprintf_s(szTemp, CountOf(szTemp), _T("%s-%s"), m_formWidth.GetFormula(), m_pObjScrollBar->m_formWidth.GetFormula());
	m_formWidth.SetFormula(szTemp);

	// height.
	m_pObjScrollBar->SetProperty(SZ_PN_HEIGHT, m_formHeight.GetFormula());

	if (m_pContainer)
		m_pContainer->RecalculateUIObjSizePos(m_pObjScrollBar);

	if (m_pObjScrollBar->IsKindOf(CSkinVScrollBar::ClassName()))
		m_pScrollBar = (CSkinVScrollBar*)m_pObjScrollBar;
	else if (m_pObjScrollBar->IsKindOf(CSkinVScrollBarOSStyle::ClassName()))
		m_pScrollBar = (CSkinVScrollBarOSStyle*)m_pObjScrollBar;

	if (m_pScrollBar)
		m_pScrollBar->SetScrollNotify(this);

	SetScrollInfo();
}

void CLyricShowTxtObj::OnSize(int cx, int cy)
{
	CLyricShowObj::OnSize(cx, cy);

	SetScrollInfo();
}

BOOL CLyricShowTxtObj::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CLyricShowObj::SetProperty(szProperty, szValue))
	{
		if (_tcsicmp(szProperty, _T("FocusLineBgColor")) == 0)
		{
			m_brCurRow.Destroy();
			m_brCurRow.CreateSolidBrush(m_clrFocusLineBg);
		}
	}
	else
		return FALSE;

	return TRUE;
}
