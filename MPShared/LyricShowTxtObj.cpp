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

	PML_ROW	pLyricRow;
	int		nRowHeight;

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

	// 行高
	nRowHeight = GetLineHeight();

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
	CDispOptRecover		dispOptRecover(this);
	m_LyricsDisplayOpt = DO_NORMAL;

	for (int i = m_nRowCur, y = m_rcObj.top + m_nYMargin;
		i < (int)m_lyrLines.size() && y < m_rcObj.bottom - m_nYMargin; i++, y += nRowHeight)
	{
		DrawRow(pGraphics, m_lyrLines[i], AUTO_CAL_X, y, LP_CUR_LINE);
	}
}

void CLyricShowTxtObj::OnLyricsChanged()
{
	CLyricShowObj::OnLyricsChanged();

	m_nRowCur = 0;

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
		if (m_nRowCur < 0 || m_nRowCur >= m_lyrLines.size())
			m_nRowCur = 0;

		int				nPage = (m_rcObj.Height() - m_nYMargin * 2) / GetLineHeight();
		m_pScrollBar->SetScrollInfo(0, m_lyrLines.size(), nPage, m_nRowCur);
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

void CLyricShowTxtObj::OnSize(int cx, int cy)
{
	CLyricShowObj::OnSize(cx, cy);

	SetScrollInfo();
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
