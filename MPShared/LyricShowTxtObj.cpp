/********************************************************************
	Created  :	2002年12月2日 10:48:18
	FileName :	LyricShowTxtObj.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowTxtObj.h"
#include "AutoScrollLyrics.h"

#define SCROLL_ANIMATION_DURATION	500
#define SCROLL_ANIMATION_TIMEOUT	50

UIOBJECT_CLASS_NAME_IMP(CLyricShowTxtObj, _T("LyricShowTxt"))

CLyricShowTxtObj::CLyricShowTxtObj()
{
	m_nCurLine = 0;

	m_nMsgWant |= UO_MSG_WANT_MOUSEWHEEL;

	m_nDarkenTopArea = float(0.5);
	m_nDarkenBottomArea = float(0.5);

	m_pScrollBar = NULL;
	m_pObjScrollBar = NULL;

	m_bInScrollingToNewLine = false;
	m_nCurLineNew = 0;
	m_yCurLineLatest = 0, m_nCurLineLatest = 0;
	m_nBeginScrollTime = 0;
	m_bInVerticalScrollingMode = false;
}

CLyricShowTxtObj::~CLyricShowTxtObj()
{
	g_eventDispather.UnregisterHanlder(ET_LYRICS_ON_SAVE_EDIT, this);

	if (m_pObjScrollBar)
		m_pSkin->RemoveUIObject(m_pObjScrollBar, true);
}

void CLyricShowTxtObj::OnKeyDown(UINT nChar, UINT nFlags)
{
	BOOL	bProcessed = FALSE;
	int		nCurLineNew = m_nCurLineNew;

	int		nMax = m_lyrLines.size() - 1;
	if (m_pScrollBar)
		nMax = m_pScrollBar->GetMax();

	switch (nChar)
	{
	case VK_UP:
		// 跳到前一行歌词
		nCurLineNew--;
		if (nCurLineNew < 0)
			nCurLineNew = 0;
		bProcessed = TRUE;
		break;
	case VK_DOWN:
		nCurLineNew++;
		if (nCurLineNew > nMax)
			nCurLineNew--;
		bProcessed = TRUE;
		break;
	case VK_HOME:
		nCurLineNew = 0;
		bProcessed = TRUE;
		break;
	case VK_END:
		nCurLineNew = nMax;
		bProcessed = TRUE;
		break;
	case VK_PRIOR:
		nCurLineNew -= GetLinesPerPage() - 1;
		bProcessed = TRUE;
		break;
	case VK_NEXT:
		nCurLineNew += GetLinesPerPage() - 1;
		if (nCurLineNew > nMax)
			nCurLineNew = nMax;
		bProcessed = TRUE;
		break;
	}

	if (bProcessed)
	{
		if (nCurLineNew < 0)
			nCurLineNew = 0;

		if (nCurLineNew != m_nCurLine)
		{
			if (m_pScrollBar)
				m_pScrollBar->SetScrollPos(nCurLineNew);
			StartScrollAnimation(nCurLineNew);
		}
	}
	else
	{
		CLyricShowMultiRowObj::OnKeyDown(nChar, nFlags);
		bProcessed = TRUE;
	}
}

void CLyricShowTxtObj::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate/* = NULL*/)
{
	if (!m_pMLData->HasLyricsOpened() || m_lyrLines.size() == 0)
	{
		FastDrawMediaInfo(pGraphics, prcUpdate);
		return;
	}

	int nCurLine = m_pMLData->GetCurPlayLine(m_lyrLines);
	if (nCurLine < 0 || nCurLine >= m_lyrLines.size())
		return;

	if (m_lyrLines[nCurLine]->nEndTime != CAutoScrollLyrics::MAX_TIME)
	{
		// in Scrolling mode
		m_bInVerticalScrollingMode = true;
		if (m_nCurLine != nCurLine)
		{
			m_nCurLineNew = m_nCurLine = nCurLine;
			if (m_pScrollBar)
				m_pScrollBar->SetScrollPos(m_nCurLine);
		}

		CDispOptRecover		dispOptRecover(this);
		m_LyricsDisplayOpt = DO_FADEOUT_LOWCOLOR;

		CLyricShowMultiRowObj::FastDraw(pGraphics, prcUpdate);
		return;
	}

	if (m_bInVerticalScrollingMode)
	{
		// Switched from vertical scrolling mode.
		m_nCurLine = m_nCurLineNew = nCurLine;
		m_bInVerticalScrollingMode = false;
	}
	else
	{
		if (prcUpdate && !m_bInScrollingToNewLine)
		{
			// Fast draw, don't update anything
			prcUpdate->SetRect(0, 0, 0, 0);
			return;
		}
	}

	if (prcUpdate)
		*prcUpdate = m_rcObj;

	if (m_lyrLines.size() == 0)
	{
		UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);
		return;
	}

	int		nLineHeight = GetLineHeight();
	int		yCurLine = GetLineVertAlignPos();

	if (m_bInScrollingToNewLine)
	{
		uint32 nTimeCur = GetTickCount();
		if (nTimeCur < m_nBeginScrollTime
			|| (nTimeCur - m_nBeginScrollTime) >= SCROLL_ANIMATION_DURATION)
		{
			m_bInScrollingToNewLine = false;
			m_nCurLine = m_nCurLineNew;
			m_pSkin->UnregisterTimerObject(this);
		}
		else
		{
			int	distance = (m_nCurLine - m_nCurLineNew) * nLineHeight
				- m_yOffsetScroll;
			yCurLine = yCurLine + m_yOffsetScroll + (distance * (int)(nTimeCur - m_nBeginScrollTime) / SCROLL_ANIMATION_DURATION);
		}

		if (prcUpdate
			&& yCurLine == m_yCurLineLatest
			&& m_nCurLine == m_nCurLineLatest)
		{
			prcUpdate->SetRectEmpty();
			return;
		}
		m_nCurLineLatest = m_nCurLine;
		m_yCurLineLatest = yCurLine;
	}

	if (m_nCurLine >= (int)m_lyrLines.size() || m_nCurLine < 0)
		m_nCurLine = 0;

	// clear background
	UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);

	CRect	rcClip;

	rcClip.SetRect(m_rcObj.left + m_nXMargin, 
						m_rcObj.top + m_nYMargin,
						m_rcObj.right - m_nXMargin,
						m_rcObj.bottom - m_nYMargin);

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraphics);
	pGraphics->SetClipBoundBox(rcClip);

	CDispOptRecover		dispOptRecover(this);
	m_LyricsDisplayOpt = DO_FADEOUT_LOWCOLOR;

	pGraphics->SetFont(&m_font);

	// Draw lines before m_nCurLine
	for (int i = m_nCurLine - 1, y = yCurLine;
		i >= 0 && y > rcClip.top; i--)
	{
		y -= nLineHeight;
		DrawRow(pGraphics, m_lyrLines[i], AUTO_CAL_X, y, LP_ABOVE_CUR_LINE);
	}

	// Draw current line.
	DrawRow(pGraphics, m_lyrLines[m_nCurLine], AUTO_CAL_X, yCurLine, LP_CUR_LINE);

	// Draw lines from m_nCurLine + 1
	for (int i = m_nCurLine + 1, y = yCurLine + nLineHeight;
		i < (int)m_lyrLines.size() && y < rcClip.bottom; i++, y += nLineHeight)
	{
		if (y + nLineHeight > rcClip.top)
			DrawRow(pGraphics, m_lyrLines[i], AUTO_CAL_X, y, LP_BELOW_CUR_LINE);
	}

	FadeOutVertBorder(pGraphics, rcClip.top, rcClip.bottom);
}

void CLyricShowTxtObj::OnLyricsChanged()
{
	CLyricShowMultiRowObj::OnLyricsChanged();

	m_nCurLine = 0;
	m_nCurLineNew = 0;

	SetScrollInfo();
}

void CLyricShowTxtObj::OnLyrDrawContextChanged()
{
	CLyricShowMultiRowObj::OnLyrDrawContextChanged();

	SetScrollInfo();

	if (g_LyricData.GetLyrContentType() == LCT_TXT)
		g_autoScrollLyrics.UpdateTimeTagByEvents(m_lyrLines);
}

void CLyricShowTxtObj::SetScrollInfo()
{
	if (m_pScrollBar)
	{
		if (m_nCurLine < 0 || m_nCurLine >= (int)m_lyrLines.size())
			m_nCurLine = 0;

		m_pScrollBar->SetScrollInfo(0, m_lyrLines.size() + GetLinesPerPage() - 1, GetLinesPerPage(), m_nCurLine);
	}
}

void CLyricShowTxtObj::OnPlayTimeChangedUpdate()
{
	// No need to update lyrics with time
	if (!m_bInScrollingToNewLine && !m_bInVerticalScrollingMode)
		return;

	CLyricShowMultiRowObj::OnPlayTimeChangedUpdate();
}

void CLyricShowTxtObj::OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	int nCurLineNew = pScrollBar->GetScrollPos();
	if (nCurLineNew != m_nCurLine)
		StartScrollAnimation(nCurLineNew);
}

void CLyricShowTxtObj::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	if (nMkeys == MK_CONTROL)
	{
		CLyricShowMultiRowObj::OnMouseWheel(nWheelDistance, nMkeys, pt);
		return;
	}

	if (m_pObjScrollBar)
		m_pObjScrollBar->OnMouseWheel(nWheelDistance, nMkeys, pt);
	else
		CLyricShowMultiRowObj::OnMouseWheel(nWheelDistance, nMkeys, pt);
}

void CLyricShowTxtObj::OnCreate()
{
	CLyricShowMultiRowObj::OnCreate();

	g_eventDispather.RegisterHanlder(ET_LYRICS_ON_SAVE_EDIT, this);

	//
	// Create Scrollbar
	//
	m_pObjScrollBar = m_pSkin->GetSkinFactory()->CreateDynamicCtrl(m_pContainer, 
		CSkinVScrollBar::ClassName(),
		SKIN_UIOBJ_ID_UNDEFINE, NULL, NULL, NULL, NULL);
	ASSERT(m_pObjScrollBar);
	if (!m_pObjScrollBar)
		return;

	m_pObjScrollBar->SetProperty(_T("LinesPerWheel"), _T("1"));

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

void CLyricShowTxtObj::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_LYRICS_ON_SAVE_EDIT)
	{
		if (g_LyricData.GetLyrContentType() == LCT_TXT)
			g_LyricData.SetTxtLyrScrollEvents(g_autoScrollLyrics.EventsDataToString().c_str());
	}
	else
		CLyricShowMultiRowObj::OnEvent(pEvent);
}

void CLyricShowTxtObj::OnSize(int cx, int cy)
{
	CLyricShowMultiRowObj::OnSize(cx, cy);

	SetScrollInfo();

	if (g_LyricData.GetLyrContentType() == LCT_TXT)
		g_autoScrollLyrics.UpdateTimeTagByEvents(m_lyrLines);
}

void CLyricShowTxtObj::OnTimer(int nId)
{
	OnPlayTimeChangedUpdate();
}

BOOL CLyricShowTxtObj::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (IsPropertyName(szProperty, _T("MarginY")))
	{
		m_nYMargin = _ttoi(szValue);
	}
	else
		return CLyricShowMultiRowObj::SetProperty(szProperty, szValue);

	return TRUE;
}

int CLyricShowTxtObj::GetLinesPerPage()
{
	return (m_rcObj.Height() - m_nYMargin * 2) / GetLineHeight();
}

void CLyricShowTxtObj::StartScrollAnimation(int nCurLineNew)
{
	if (g_LyricData.GetLyrContentType() == LCT_TXT)
		g_autoScrollLyrics.ScrollToLine(m_lyrLines, nCurLineNew, g_LyricData.GetPlayElapsedTime(), true);

	if (m_bInScrollingToNewLine)
	{
		m_yOffsetScroll = m_yCurLineLatest - GetLineVertAlignPos();
	}
	else
		m_yOffsetScroll = 0;

	m_nCurLineNew = nCurLineNew;
	m_bInScrollingToNewLine = true;
	m_nBeginScrollTime = GetTickCount();

	m_pSkin->RegisterTimerObject(this, SCROLL_ANIMATION_TIMEOUT);
}
