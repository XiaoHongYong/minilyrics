/********************************************************************
	Created  :	2002年12月2日 10:48:18
	FileName :	LyricShowTxtObj.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricShowTxtObj.h"

#define SCROLL_ANIMATION_DURATION	500
#define SCROLL_ANIMATION_TIMEOUT	50

UIOBJECT_CLASS_NAME_IMP(CLyricShowTxtObj, _T("LyricShowTxt"))

CLyricShowTxtObj::CLyricShowTxtObj()
{
	m_nFirstVisibleLine = 0;

	m_nMsgWant |= UO_MSG_WANT_MOUSEWHEEL;

	m_nDarkenTopArea = float(0.5);
	m_nDarkenBottomArea = float(0.5);

	m_pScrollBar = NULL;
	m_pObjScrollBar = NULL;

	m_bInScrolling = false;
	m_nFirstVisibleLineNew = 0;
	m_yFirstVisibleLineLatest = 0, m_nFirstVisibleLineLatest = 0;
	m_nBeginScrollTime = 0;
}

CLyricShowTxtObj::~CLyricShowTxtObj()
{
	if (m_pObjScrollBar)
		m_pSkin->RemoveUIObject(m_pObjScrollBar, true);
}

void CLyricShowTxtObj::OnKeyDown(UINT nChar, UINT nFlags)
{
	BOOL	bProcessed = FALSE;
	int		nFirstVisibleLineNew = m_nFirstVisibleLineNew;

	switch (nChar)
	{
	case VK_UP:
		// 跳到前一行歌词
		nFirstVisibleLineNew--;
		if (nFirstVisibleLineNew < 0)
			nFirstVisibleLineNew = 0;
		bProcessed = TRUE;
		break;
	case VK_DOWN:
		nFirstVisibleLineNew++;
		if (nFirstVisibleLineNew + GetLinesPerPage() > (int)m_lyrLines.size())
			nFirstVisibleLineNew--;
		bProcessed = TRUE;
		break;
	case VK_HOME:
		nFirstVisibleLineNew = 0;
		bProcessed = TRUE;
		break;
	case VK_END:
		nFirstVisibleLineNew = m_lyrLines.size() - GetLinesPerPage();
		bProcessed = TRUE;
		break;
	case VK_PRIOR:
		nFirstVisibleLineNew -= GetLinesPerPage() - 1;
		bProcessed = TRUE;
		break;
	case VK_NEXT:
		nFirstVisibleLineNew += GetLinesPerPage() - 1;
		if (nFirstVisibleLineNew + GetLinesPerPage() >= (int)m_lyrLines.size())
		{
			nFirstVisibleLineNew = m_lyrLines.size() - GetLinesPerPage();
		}
		bProcessed = TRUE;
		break;
	}

	if (bProcessed)
	{
		if (nFirstVisibleLineNew < 0)
			nFirstVisibleLineNew = 0;

		if (nFirstVisibleLineNew != m_nFirstVisibleLine)
		{
			if (m_pScrollBar)
				m_pScrollBar->SetScrollPos(nFirstVisibleLineNew);
			StartScrollAnimation(nFirstVisibleLineNew);
		}
	}
	else
	{
		CLyricShowObj::OnKeyDown(nChar, nFlags);
		bProcessed = TRUE;
	}
}

void CLyricShowTxtObj::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate/* = NULL*/)
{
	if (prcUpdate && !m_bInScrolling)
	{
		// Fast draw, don't update anything
		prcUpdate->SetRect(0, 0, 0, 0);
		return;
	}

	if (prcUpdate)
		*prcUpdate = m_rcObj;

	if (m_lyrLines.size() == 0)
	{
		UpdateLyricDrawBufferBackground(pGraphics, m_rcObj);
		return;
	}

	int		nLineHeight = GetLineHeight();
	int		yFirstVisibleLine = m_rcObj.top + m_nYMargin;

	if (m_bInScrolling)
	{
		uint32 nTimeCur = GetTickCount();
		if (nTimeCur < m_nBeginScrollTime
			|| (nTimeCur - m_nBeginScrollTime) >= SCROLL_ANIMATION_DURATION)
		{
			m_bInScrolling = false;
			m_nFirstVisibleLine = m_nFirstVisibleLineNew;
			m_pSkin->UnregisterTimerObject(this);
		}
		else
		{
			int	distance = (m_nFirstVisibleLine - m_nFirstVisibleLineNew) * nLineHeight
				- m_yOffsetScroll;
			yFirstVisibleLine = yFirstVisibleLine + m_yOffsetScroll + distance * (int)(nTimeCur - m_nBeginScrollTime) / SCROLL_ANIMATION_DURATION;
			DBGLOG2(_T("yFirstVisibleLine: %d, yOffset: %d"), yFirstVisibleLine, m_yOffsetScroll);
		}

		if (prcUpdate
			&& yFirstVisibleLine == m_yFirstVisibleLineLatest
			&& m_nFirstVisibleLine == m_nFirstVisibleLineLatest)
		{
			prcUpdate->SetRectEmpty();
			return;
		}
		m_nFirstVisibleLineLatest = m_nFirstVisibleLine;
		m_yFirstVisibleLineLatest = yFirstVisibleLine;
	}

	if (m_nFirstVisibleLine >= (int)m_lyrLines.size() || m_nFirstVisibleLine < 0)
		m_nFirstVisibleLine = 0;

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
	m_LyricsDisplayOpt = DO_NORMAL;

	pGraphics->SetFont(&m_font);

	// Draw lines before m_nFirstVisibleLine
	for (int i = m_nFirstVisibleLine - 1, y = yFirstVisibleLine;
		i >= 0 && y > rcClip.top; i--)
	{
		y -= nLineHeight;
		DrawRow(pGraphics, m_lyrLines[i], AUTO_CAL_X, y, LP_CUR_LINE);
	}

	// Draw lines from m_nFirstVisibleLine
	for (int i = m_nFirstVisibleLine, y = yFirstVisibleLine;
		i < (int)m_lyrLines.size() && y < rcClip.bottom; i++, y += nLineHeight)
	{
		if (y + nLineHeight > rcClip.top)
			DrawRow(pGraphics, m_lyrLines[i], AUTO_CAL_X, y, LP_CUR_LINE);
	}

	FadeOutVertBorder(pGraphics, rcClip.top, rcClip.bottom);
}

void CLyricShowTxtObj::OnLyricsChanged()
{
	CLyricShowObj::OnLyricsChanged();

	m_nFirstVisibleLine = 0;
	m_nFirstVisibleLineNew = 0;

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
		if (m_nFirstVisibleLine < 0 || m_nFirstVisibleLine >= m_lyrLines.size())
			m_nFirstVisibleLine = 0;

		m_pScrollBar->SetScrollInfo(0, m_lyrLines.size(), GetLinesPerPage(), m_nFirstVisibleLine);
	}
}

void CLyricShowTxtObj::OnPlayTimeChangedUpdate()
{
	// No need to update lyrics with time
	if (!m_bInScrolling)
		return;

	CLyricShowObj::OnPlayTimeChangedUpdate();
}

void CLyricShowTxtObj::OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	int nFirstVisibleLineNew = pScrollBar->GetScrollPos();
	if (nFirstVisibleLineNew != m_nFirstVisibleLine)
		StartScrollAnimation(nFirstVisibleLineNew);
}

void CLyricShowTxtObj::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	if (nMkeys == MK_CONTROL)
	{
		CLyricShowObj::OnMouseWheel(nWheelDistance, nMkeys, pt);
		return;
	}

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

	m_pObjScrollBar->SetProperty(_T("LinesPerWheel"), _T("2"));

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
		return CLyricShowObj::SetProperty(szProperty, szValue);

	return TRUE;
}

int CLyricShowTxtObj::GetLinesPerPage()
{
	return (m_rcObj.Height() - m_nYMargin * 2) / GetLineHeight();
}

void CLyricShowTxtObj::StartScrollAnimation(int nFirstVisibleLineNew)
{
	if (m_bInScrolling)
	{
		m_yOffsetScroll = m_yFirstVisibleLineLatest - (m_rcObj.top + m_nYMargin);
	}
	else
		m_yOffsetScroll = 0;

	m_nFirstVisibleLineNew = nFirstVisibleLineNew;
	m_bInScrolling = true;
	m_nBeginScrollTime = GetTickCount();

	m_pSkin->RegisterTimerObject(this, SCROLL_ANIMATION_TIMEOUT);
}
