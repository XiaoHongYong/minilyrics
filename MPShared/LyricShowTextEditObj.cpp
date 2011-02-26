// LyricShowTextEditObj.cpp: implementation of the CLyricShowTextEditObj class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MLCmd.h"
#include "LyricShowTextEditObj.h"
#include "LyricShowObj.h"
#include "SkinBarFrame.h"

#define TIMER_SPAN_LYR_PREVIEW_UPDATE	500



#define GLYPH_WIDTH_INVALID		0xFF

bool GetTimeTagOfLine(LPCTSTR szLine, tstring &strTimeTag);

int GetTimeTagValue(const TCHAR *szTag, int nLen);

CLyrEditSyntaxParser::CLyrEditSyntaxParser()
{
	m_pLyrEdit = NULL;
}

CLyrEditSyntaxParser::~CLyrEditSyntaxParser()
{
}

void CLyrEditSyntaxParser::Init(CSkinEditCtrl *pEdit)
{
	m_pLyrEdit = (CLyricShowTextEditObj*)pEdit;

	MLColor		clrHilight(RGB(255, 255, 255)), clrLowlight(RGB(192, 192, 192));
	MLColor		clrTag(RGB(255, 0, 0)), clrEditLineBg(RGB(0, 0, 0));
	m_pLyrEdit->SetColorTheme(clrHilight, clrLowlight, clrTag, clrEditLineBg);
}

void CLyrEditSyntaxParser::OnNotifyParseLine(int nLine)
{
	tstring					strLine;
	CLyricShowTextEditObj::CLyrOneLine	*pLine2;
	CLyricShowTextEditObj::CLyrOneLine	*pLine = m_pLyrEdit->GetLine(nLine);
	if (pLine && m_pLyrEdit->GetTextOfLine(nLine, strLine))
	{
		tstring		strTime;
		int			i;
		if (GetTimeTagOfLine(strLine.c_str(), strTime))
		{
			pLine->nBegTime = GetTimeTagValue(strTime.c_str(), strTime.size());
			pLine->bTimeLine = true;
			// DBGLOG2(_T("Line: %d Changed: %s"), nLine, strLine.c_str());
			for (i = 0; i < (int)strTime.size(); i++)
			{
				(*pLine)[i].clrIndex = C_TAG;
			}
			for (i = (int)strTime.size(); i < (int)pLine->size(); i++)
			{
				(*pLine)[i].clrIndex = C_LOWLIGHT;
			}

			if (nLine == m_pLyrEdit->GetLineCount() - 1 || pLine->nEndTime < pLine->nBegTime)
				pLine->nEndTime = pLine->nBegTime + 1000 * 5;

			do
			{
				nLine--;
				pLine2 = m_pLyrEdit->GetLine(nLine);
			}
			while (pLine2 && !pLine2->bTimeLine);

			if (pLine2 && pLine2->bTimeLine)
				pLine2->nEndTime = pLine->nBegTime;
		}
		else
		{
			// other text ...
			for (int i = 0; i < (int)pLine->size(); i++)
			{
				(*pLine)[i].clrIndex = C_LOWLIGHT;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
//
// CLyricShowTextEditObj
//
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CLyricShowTextEditObj, _T("LyricShowTextEdit"));

CLyricShowTextEditObj::CLyricShowTextEditObj()
{
	m_bEnableBorder = false;

	m_findReplaceStatus = FRS_NONE;

	m_lyrEditSyntaxParser.Init(this);
	SetEditSyntaxParser(&m_lyrEditSyntaxParser);

	m_nMsgWant |= UO_MSG_WANT_MENU_CMD | UO_MSG_WANT_CUSTOM_CMD;
	m_nEditorStyles |= S_MULTILINE;

	m_bDiscardLyrChangeEvent = false;
	m_nTimerIDUpdateLyrics = 0;
}

CLyricShowTextEditObj::~CLyricShowTextEditObj()
{
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_LYRICS_ON_SAVE_EDIT, this);
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_LYRICS_EDITOR_RELOAD_TAG, this);
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_LYRICS_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_LYRICS_DRAW_UPDATE, this);
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_LYRICS_DISPLAY_SETTINGS, this);
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
}

void CLyricShowTextEditObj::OnCreate()
{
	SetEditNotification(this);

	CSkinEditCtrl::OnCreate();

	GET_ID_BY_NAME(CID_MATCHCASE);
	GET_ID_BY_NAME(CID_FIND_PREV);
	GET_ID_BY_NAME(CID_FIND_NEXT);
	GET_ID_BY_NAME(CID_E_FIND);
	GET_ID_BY_NAME(CID_REPLACE);
	GET_ID_BY_NAME(CID_REPLACE_ALL);
	GET_ID_BY_NAME(CID_E_REPLACE);
	GET_ID_BY_NAME(CID_E_ARTIST);
	GET_ID_BY_NAME(CID_E_TITLE);
	GET_ID_BY_NAME(CID_E_ALBUM);
	GET_ID_BY_NAME(CID_E_BY);
	GET_ID_BY_NAME(CID_E_OFFSET);
	GET_ID_BY_NAME(CID_E_MEDIA_LENGTH);

	OnLyricsChanged();

	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_LYRICS_ON_SAVE_EDIT, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_LYRICS_EDITOR_RELOAD_TAG, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_LYRICS_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_LYRICS_DRAW_UPDATE, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_LYRICS_DISPLAY_SETTINGS, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);

	UpdateTextFontColor();

	m_pSkin->SetUIObjectText(CID_E_FIND, g_Profile.GetProfileString(_T("FindWhat"), _T("")));
	m_pSkin->SetUIObjectText(CID_E_REPLACE, g_Profile.GetProfileString(_T("ReplaceWith"), _T("")));

	// If there's no lyrics opened, fill media info.
	if (!g_LyricData.HasLyricsOpened())
		OnCustomCommand(CMD_AUTO_FILL_LYR_INFO);
}

void CLyricShowTextEditObj::OnDestroy()
{
	OnSaveLyrics();

	tstring	strFindWhat = m_pSkin->GetUIObjectText(CID_E_FIND);
	tstring	strReplaceWith = m_pSkin->GetUIObjectText(CID_E_REPLACE);
	g_Profile.WriteProfileString(_T("FindWhat"), strFindWhat.c_str());
	g_Profile.WriteProfileString(_T("ReplaceWith"), strReplaceWith.c_str());

	CSkinEditCtrl::OnDestroy();
}

void CLyricShowTextEditObj::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_LYRICS_ON_SAVE_EDIT)
		OnSaveLyrics();
	else if (pEvent->eventType == ET_LYRICS_EDITOR_RELOAD_TAG)
		UpdateLyricsProperties(true);
	else if (pEvent->eventType == ET_LYRICS_CHANGED)
		OnLyricsChanged();
	else if (pEvent->eventType == ET_LYRICS_DRAW_UPDATE)
		OnPlayTimeChangedUpdate();
	else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED)
	{
		if (!g_LyricData.HasLyricsOpened())
			OnCustomCommand(CMD_AUTO_FILL_LYR_INFO);
	}
	else if (pEvent->eventType == ET_LYRICS_DISPLAY_SETTINGS)
	{
		LPCTSTR		szProperty = pEvent->strName.c_str();

		if (IsPropertyName(szProperty, _T("Ed_HighColor"))
			|| IsPropertyName(szProperty, _T("Ed_LowColor"))
			|| IsPropertyName(szProperty, _T("Ed_BgColor"))
			|| IsPropertyName(szProperty, _T("Ed_TagColor"))
			|| IsPropertyName(szProperty, _T("Ed_FocusLineBgColor"))
			|| IsPropertyName(szProperty, _T("Font")))
		{
			UpdateTextFontColor();
			Invalidate();
		}
	}
}

void CLyricShowTextEditObj::AutoVScrollDown(int nDownLine)
{
	if (m_nCaretRow + nDownLine < m_pVertScrollBar->GetPage() / 2)
		return;

	if (!m_pVertScrollBar->IsEnabled())
		return;

	if (m_pVertScrollBar->GetScrollPos() >= m_pVertScrollBar->GetMax())
		return;

	m_nTopVisibleLine += nDownLine;

	if (m_nTopVisibleLine >= (int)m_vLines.size())
		m_nTopVisibleLine = (int)m_vLines.size() - 1;
	if (m_nTopVisibleLine < 0)
		m_nTopVisibleLine = 0;

	m_pVertScrollBar->SetScrollPos(m_nTopVisibleLine);
	ShowCaret();

	Invalidate();
}

void CLyricShowTextEditObj::OnKeyDown(UINT nChar, UINT nFlags)
{
	bool ctrl = IsKeyPressed(VK_CONTROL);
	bool shift = IsKeyPressed(VK_SHIFT);

	if (!ctrl && !shift)
	{
		if (nChar == VK_F3)
		{
			FindNext();
			return;
		}
		else if ((nChar == VK_UP || nChar == VK_DOWN) && !IsSelected())
		{
			//
			// Adjust the time tag value, with up/down key.
			//
			tstring		strLine, strTimeTag;

			if (GetTextOfLine(m_nCaretRow, strLine)
				&& GetTimeTagOfLine(strLine.c_str(), strTimeTag))
			{
				// 				int		nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol;
				// 				bool	bSelectedEmpty;
				// 				SortSelectPos(nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol, bSelectedEmpty);
				// 				if (!bSelectedEmpty && nBegSelRow == nEndSelRow
				// 					&& nBegSelRow == m_nCaretRow)
				// [00:12.00]
				//   ^  ^  ^
				const int	nTimeTagStdLen = 10;
				const int	nMinutePos = 2;
				const int	nSecondPos = 5;
				const int	nMSPos = 8;
				if (strTimeTag.size() == nTimeTagStdLen
					&& (m_nCaretCol == nMinutePos || m_nCaretCol == nSecondPos || m_nCaretCol == nMSPos))
				{
					// Get value
					int		n = _ttoi(strTimeTag.c_str() + m_nCaretCol - 1);
					int		nUnit = 1, nMax = 60;

					if (m_nCaretCol == nMSPos)
					{
						nUnit = 10;
						nMax = 100;
					}
					else if (m_nCaretCol == nMinutePos)
						nMax = 99;

					// Set new value
					if (nChar == VK_DOWN)
					{
						n += nUnit;
						if (n >= nMax)
							n = n % nUnit;
					}
					else
					{
						n -= nUnit;
						if (n < 0)
						{
							if (m_nCaretCol == nMinutePos)
								n = 0;
							else
								n += nMax;
						}
					}

					// Replace selected tag value.
					int		nCaretColOld = m_nCaretCol;
					SetSel(m_nCaretRow, m_nCaretCol - 1, m_nCaretRow, m_nCaretCol + 1);
					ReplaceSel(CStrPrintf(_T("%02d"), n).c_str());
					SetCaret(m_nCaretRow, nCaretColOld);

					return;
				}
			}
		}
	}

	if (ctrl)
	{
		switch (nChar)
		{
		case 'F':
			ShowFindDialog();
			break;
		case 'H':
			ShowFindDialog(false);
			break;
		}
	}

	CSkinEditCtrl::OnKeyDown(nChar, nFlags);
}

bool CLyricShowTextEditObj::FindText(LPCTSTR szText, DWORD dwFlags)
{
	bool		bDown = IsFlagSet(dwFlags, FTF_DOWN);
	bool		bCase = IsFlagSet(dwFlags, FTF_MATCH_CASE);
	// bool		bWholeWord = IsFlagSet(dwFlags, FTF_WHOLEWORD);

	int			nLine, nCol;
	int			nBegSelRow, nEndSelRow;
	int			nBegSelCol, nEndSelCol;
	bool		bSelectedEmpty = true;

	GetCaret(nLine, nCol);

	if (m_nBegSelRow != -1)
	{
		bSelectedEmpty = false;
		SortSelectPos(nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol, bSelectedEmpty);
		if (!bSelectedEmpty)
		{
			if (bDown)
			{
				nLine = nEndSelRow;
				nCol = nEndSelCol;
			}
			else
			{
				nLine = nBegSelRow;
				nCol = nBegSelCol;
			}
		}
	}

	if (bDown)
	{
		// search down
		int		nLineCount = m_vLines.size();
		int		k;

		for (int i = nLine; i < nLineCount; i++)
		{
			COneLine		*pLine = m_vLines[i];

			for (; nCol < (int)pLine->size(); nCol++)
			{
				LPCTSTR	szTextCmp = szText;
				for (k = nCol; k < (int)pLine->size(); k++)
				{
					CGlyph	&glyph = (*pLine)[k];
					if (bCase)
					{
						if (glyph.Cmp(szTextCmp))
							szTextCmp += glyph.Length();
						else
							break;
					}
					else
					{
						if (glyph.iCmp(szTextCmp))
							szTextCmp += glyph.Length();
						else
							break;
					}
				}
				if (*szTextCmp == '\0')
				{
					// Found it.
					m_nBegSelRow = i;
					m_nBegSelCol = nCol;
					m_nEndSelRow = i;
					m_nEndSelCol = k;

					OnNotifySelChanged();

					SetCaret(i, k);
					return true;
				}
			}
			nCol = 0;
		}
	}
	else
	{
		// search up
		int		k;
		int		nLenCmp = _tcslen(szText);

		// Previous character...
		PrevPos(nLine, nCol);

		if (nCol >= (int)m_vLines[nLine]->size())
			nCol = (int)m_vLines[nLine]->size() - 1;
		for (int i = nLine; i >= 0; i--)
		{
			COneLine		*pLine = m_vLines[i];

			for (; nCol >= 0; nCol--)
			{
				LPCTSTR	szTextCmp = szText + nLenCmp - 1;
				for (k = nCol; k >= 0 && szTextCmp >= szText; k--)
				{
					CGlyph	&glyph = (*pLine)[k];
					if (bCase)
					{
						if (glyph.ReverseCmp(szTextCmp))
							szTextCmp -= glyph.Length();
						else
							break;
					}
					else
					{
						if (glyph.ReverseiCmp(szTextCmp))
							szTextCmp -= glyph.Length();
						else
							break;
					}
				}
				if (szTextCmp == szText - 1)
				{
					// Found it.
					m_nBegSelRow = i;
					m_nBegSelCol = nCol + 1;
					m_nEndSelRow = i;
					m_nEndSelCol = k + 1;

					OnNotifySelChanged();

					SetCaret(i, nCol + 1);
					return true;
				}
			}
			if (i > 0)
				nCol = m_vLines[i - 1]->size() - 1;
		}
	}

	return false;
}

CLyricShowTextEditObj::CLyrOneLine *CLyricShowTextEditObj::GetLine(int nLine)
{
	if (nLine >= 0 && nLine < (int)m_vLines.size())
		return (CLyrOneLine*)m_vLines[nLine];
	else
		return NULL;
}

void CLyricShowTextEditObj::Draw(CRawGraph *pGraph)
{
	int		nBegSelRow, nEndSelRow;
	int		nBegSelCol, nEndSelCol;
	bool	bSelectedEmpty = true;

	if (m_nBegSelRow != -1)
	{
		bSelectedEmpty = false;
		SortSelectPos(nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol, bSelectedEmpty);
	}

	if (bSelectedEmpty)
	{
		m_nTopVisibleLineOld = -1;

		ReDraw(pGraph);
		return;
	}

	CSkinEditCtrl::Draw(pGraph);
}

void CLyricShowTextEditObj::FastDraw(CRawGraph *pGraph, CRect *prcUpdate/* = NULL*/)
{
	CRect	rc = m_rcCtrl;
	pGraph->SetFont(m_font.GetFont());

	if (prcUpdate)
		*prcUpdate = rc;

	int			xStart = rc.left + m_xMargin;
	int			nTimePos = g_LyricData.GetPlayElapsedTime();
	CLyrOneLine *pLyrLine;

	if (m_nTopVisibleLine == m_nTopVisibleLineOld)
	{
		// Fast draw ...?
		if (m_nCurPosLineOld >= 0 && m_nCurPosLineOld < (int)m_vLines.size())
		{
			pLyrLine = (CLyrOneLine*)m_vLines[m_nCurPosLineOld];
			if (pLyrLine->bTimeLine)
			{
				if (pLyrLine->nBegTime <= nTimePos && nTimePos <= pLyrLine->nEndTime)
				{
					int		nTimeTagLen;
					// current line of cur pos doesn't change, only update this line
					for (nTimeTagLen = 0; nTimeTagLen < (int)pLyrLine->size(); nTimeTagLen++)
					{
						if ((*pLyrLine)[nTimeTagLen].IsEqual(']'))
						{
							nTimeTagLen++;
							break;
						}
					}

					int		nXOfCurPosLine = 0;
					int		dt = (pLyrLine->nEndTime - pLyrLine->nBegTime);
					ASSERT(dt > 0);
					if (dt > 0)
						nXOfCurPosLine = (pLyrLine->size() - nTimeTagLen) * (nTimePos - pLyrLine->nBegTime) / dt + nTimeTagLen;
					if (nXOfCurPosLine > (int)pLyrLine->size())
						nXOfCurPosLine = pLyrLine->size();

					if (m_nXOfCurPosLineOld == nXOfCurPosLine)
					{
						prcUpdate->right = prcUpdate->left;
						prcUpdate->bottom = prcUpdate->top;
						// no need to update, just return
						// DBGLOG0(_T("No update, fastest."));
						return;
					}
					else
					{
						// only update the current line.
						// DBGLOG0(_T("Update only one line, faster."));
						prcUpdate->top = rc.top + m_yMargin + GetLineDy() * (m_nCurPosLineOld - m_nTopVisibleLine);
						prcUpdate->bottom = prcUpdate->top + GetLineDy();
						FillGraph(pGraph, prcUpdate, 
							m_nCaretRow == m_nCurPosLineOld ? CN_EDIT_LINE_BG : CN_BG);
						DrawLineGradual(pGraph, pLyrLine, xStart, rc.right - m_xMargin, prcUpdate->top);
						return;
					}
				}
			}
		}

		if (m_nCurPosLineOld == -1)
		{
			// current line isn't being displayed.
			int		y, yMax;
			int		i;
			y = m_yMargin;
			yMax = rc.bottom - m_yMargin;

			for (i = 0; i < (int)m_vLines.size(); i++)
			{
				if (y + GetLineDy() > yMax && i > m_nTopVisibleLine)
					break;
				pLyrLine = (CLyrOneLine *)m_vLines[i];
				if (pLyrLine->bTimeLine &&
					pLyrLine->nBegTime <= nTimePos && nTimePos <= pLyrLine->nEndTime)
				{
					m_nCurPosLineOld = i;
					break;
				}
				y += GetLineDy();
			}
			if (m_nCurPosLineOld == -1)
			{
				// DBGLOG0(_T("No update, no current line, fastest."));
				prcUpdate->right = prcUpdate->left;
				prcUpdate->bottom = prcUpdate->top;
				return;
			}
		}
	}

	ReDraw(pGraph);

	CRect	rcCaret;

	rcCaret.IntersectRect(&m_caret.GetUpdateRect(), prcUpdate);
	if (!rcCaret.IsRectEmpty())
	{
		m_caret.Draw(pGraph);
	}
}

void CLyricShowTextEditObj::ReDraw(CRawGraph *pGraph)
{
	CSkinScrollFrameCtrlBase::Draw(pGraph);

	CRect &rc = m_rcCtrl;

	int		y, xStart, xMax, yMax;
	int		i;
	CLyrOneLine *pLyrLine;
	CRect	rcClip;

	pGraph->SetFont(m_font.GetFont());

	FillGraph(pGraph, &rc, CN_BG);

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraph);
	rcClip.SetRect(rc.left + m_xMargin, rc.top + m_yMargin, rc.right - m_xMargin, rc.bottom - m_yMargin);
	pGraph->SetClipBoundBox(rcClip);

	xStart = rcClip.left;
	y = rcClip.top;
	xMax = rcClip.right;
	yMax = rcClip.bottom;
	ASSERT(m_nTopVisibleLine >= 0 && m_nTopVisibleLine < (int)m_vLines.size());

	int			nTimePos = g_LyricData.GetPlayElapsedTime();

	m_nCurPosLineOld = -1;
	m_nXOfCurPosLineOld = -1;
	m_nTopVisibleLineOld = m_nTopVisibleLine;

	for (i = m_nTopVisibleLine; i < (int)m_vLines.size(); i++)
	{
		if (y + GetLineDy() > yMax && i > m_nTopVisibleLine)
			break;

		if (m_nCaretRow == i)
			FillGraph(pGraph, m_xMargin, y, xMax - m_xMargin * 2, GetLineDy(), CN_EDIT_LINE_BG);

		pLyrLine = (CLyrOneLine *)m_vLines[i];
		if (!pLyrLine->bTimeLine)
			DrawLine(pGraph, pLyrLine, xStart, xMax, y);
		else if (pLyrLine->nEndTime < nTimePos)
		{
			// draw in highlight
			MLColor		clrOld;
			clrOld = GetColor(CLyrEditSyntaxParser::C_LOWLIGHT);
			SetColor(CLyrEditSyntaxParser::C_LOWLIGHT, GetColor(CLyrEditSyntaxParser::C_HILIGHT));
			CSkinEditCtrl::DrawLine(pGraph, pLyrLine, xStart, xMax, y);
			SetColor(CLyrEditSyntaxParser::C_LOWLIGHT, clrOld);
		}
		else if (pLyrLine->nBegTime >= nTimePos)
		{
			// draw in lowlight
			CSkinEditCtrl::DrawLine(pGraph, pLyrLine, xStart, xMax, y);
		}
		else if (pLyrLine->nBegTime >= pLyrLine->nEndTime)
		{
			// draw in highlight
			MLColor		clrOld;
			clrOld = GetColor(CLyrEditSyntaxParser::C_LOWLIGHT);
			SetColor(CLyrEditSyntaxParser::C_LOWLIGHT, GetColor(CLyrEditSyntaxParser::C_HILIGHT));
			CSkinEditCtrl::DrawLine(pGraph, pLyrLine, xStart, xMax, y);
			SetColor(CLyrEditSyntaxParser::C_LOWLIGHT, clrOld);
		}
		else if (m_nCurPosLineOld == -1)
		{
			m_nCurPosLineOld = i;
			DrawLineGradual(pGraph, pLyrLine, xStart, xMax, y);
		}
		else
		{
			MLColor		clrOld;
			clrOld = GetColor(CLyrEditSyntaxParser::C_LOWLIGHT);
			SetColor(CLyrEditSyntaxParser::C_LOWLIGHT, GetColor(CLyrEditSyntaxParser::C_HILIGHT));
			DrawLine(pGraph, pLyrLine, xStart, xMax, y);
			SetColor(CLyrEditSyntaxParser::C_LOWLIGHT, clrOld);
		}

		y += GetLineDy();
	}
}

void CLyricShowTextEditObj::DrawLineGradual(CRawGraph *pGraph, COneLine *pLine, int x, int xMax, int y)
{
	CLyrOneLine *pLyrLine = (CLyrOneLine *)pLine;

	if (!pLyrLine->bTimeLine)
	{
		CSkinEditCtrl::DrawLine(pGraph, pLine, x, xMax, y);
		return;
	}

	int			nTimePos = g_LyricData.GetPlayElapsedTime();

	BYTE		byClrLast = 0;
	int			k;
	int			nTimeTagLen;

	x -= m_nScrollPosx;

	pGraph->SetTextColor(m_vClrTable[byClrLast]);

	ASSERT(pLyrLine->nBegTime <= nTimePos && nTimePos <= pLyrLine->nEndTime);

	// draw time tag
	for (k = 0; k < (int)pLine->size(); k++)
	{
		CGlyph	&glyph = (*pLine)[k];
		ASSERT(glyph.width != GLYPH_WIDTH_INVALID);

		if (glyph.clrIndex != byClrLast)
		{
			if (glyph.clrIndex >= m_vClrTable.size())
				byClrLast = 0;
			else
				byClrLast = glyph.clrIndex;
			pGraph->SetTextColor(m_vClrTable[byClrLast]);
		}
		if (x + glyph.width < m_xMargin)
		{
			x += glyph.width;
			continue;
		}
		if (x + glyph.width > xMax)
			return;
		if (!glyph.IsEqual('\t'))
		{
			pGraph->TextOut(x, y, glyph.chGlyph, glyph.chGlyph.size());
		}
		x += glyph.width;
		if (glyph.IsEqual(']'))
			break;
	}
	k++;
	nTimeTagLen = k;

	int			nXOfCurPosLine = 0;
	int dt = (pLyrLine->nEndTime - pLyrLine->nBegTime);
	ASSERT(dt > 0);
	if (dt > 0)
		nXOfCurPosLine = (pLyrLine->size() - nTimeTagLen) * (nTimePos - pLyrLine->nBegTime) / dt + nTimeTagLen;

	if (nXOfCurPosLine > (int)pLyrLine->size())
		nXOfCurPosLine = pLyrLine->size();

	m_nXOfCurPosLineOld = nXOfCurPosLine;

	pGraph->SetTextColor(GetColor(CLyrEditSyntaxParser::C_HILIGHT));
	for (; k < nXOfCurPosLine; k++)
	{
		CGlyph	&glyph = (*pLine)[k];
		ASSERT(glyph.width != GLYPH_WIDTH_INVALID);

		if (x + glyph.width < m_xMargin)
		{
			x += glyph.width;
			continue;
		}
		if (x + glyph.width > xMax)
			return;
		if (!glyph.IsEqual('\t'))
		{
			pGraph->TextOut(x, y, glyph.chGlyph, glyph.chGlyph.size());
		}
		x += glyph.width;
	}

	pGraph->SetTextColor(GetColor(CLyrEditSyntaxParser::C_LOWLIGHT));
	for (; k < (int)pLine->size(); k++)
	{
		CGlyph	&glyph = (*pLine)[k];
		ASSERT(glyph.width != GLYPH_WIDTH_INVALID);

		if (x + glyph.width < m_xMargin)
		{
			x += glyph.width;
			continue;
		}
		if (x + glyph.width > xMax)
			return;
		if (!glyph.IsEqual('\t'))
		{
			pGraph->TextOut(x, y, glyph.chGlyph, glyph.chGlyph.size());
		}
		x += glyph.width;
	}
}

void CLyricShowTextEditObj::SetColorTheme(MLColor &clrHilight, MLColor &clrLowlight, MLColor &clrTag, MLColor &clrEditLineBg)
{
	SetColor(CLyrEditSyntaxParser::C_HILIGHT, clrHilight);
	SetColor(CLyrEditSyntaxParser::C_LOWLIGHT, clrLowlight);
	SetColor(CLyrEditSyntaxParser::C_TAG, clrTag);

	SetSelColor(clrHilight, clrEditLineBg);
	SetColor(CN_EDIT_LINE_BG, clrEditLineBg);
}

bool CLyricShowTextEditObj::IsTextChanged()
{
	return m_undoMgr.CanUndo();
}

void CLyricShowTextEditObj::OnCmdDelete()
{
	if (IsSelected() || m_nCaretCol != 0)
	{
		CSkinEditCtrl::OnCmdDelete();
		return;
	}

	//
	// Delete the whole tag, if the first char of tag is deleted.
	//

	tstring		strToDel;
	int			nContentBegPos;
	tstring		strLine, strContent;

	if (!GetTextOfLine(m_nCaretRow, strLine))
		return;

	if (GetTimeTagOfLine(strLine.c_str(), strToDel))
		nContentBegPos = strToDel.size();
	else
	{
		CSkinEditCtrl::OnCmdDelete();
		return;
	}

	SetSel(m_nCaretRow, m_nCaretCol, m_nCaretRow, nContentBegPos);

	CSkinEditCtrl::OnCmdDelete();
}

int CLyricShowTextEditObj::GetCaretLineHomePos()
{
	tstring	strLine, strTimeTag;
	int		nColHome = 0;

	if (GetTextOfLine(m_nCaretRow, strLine))
	{

		if (GetTimeTagOfLine(strLine.c_str(), strTimeTag))
		{
			nColHome = strTimeTag.size();
		}

		if (nColHome == m_nCaretCol)
			nColHome = 0;
	}

	return nColHome;
}

bool GetTimeTagOfLine(LPCTSTR szLine, tstring &strTimeTag)
{
	LPCTSTR		szPtr = szLine;
	int			i;

	strTimeTag = _T("");

	// ignore blank space
	while (*szPtr == ' ' || *szPtr == '\t')
		szPtr++;
	if (*szPtr != '[')
		return false;
	szPtr++;

	// ignore minute
	if (*szPtr == '-')
		szPtr++;
	for (i = 0; i < 3 && IsDigit(*szPtr); i++)
		szPtr++;

	// :
	if (*szPtr != ':')
		return false;
	szPtr++;

	// ignore sec
	if (*szPtr == '-')
		szPtr++;
	for (i = 0; i < 3 && IsDigit(*szPtr); i++)
		szPtr++;

	if (*szPtr == ']')
	{
		szPtr++;
		strTimeTag.append(szLine, (int)(szPtr - szLine));
		return true;
	}
	if (*szPtr == '.' || *szPtr == ':')
	{
		szPtr++;

		// ignore ms
		if (*szPtr == '-')
			szPtr++;
		for (i = 0; i < 3 && IsDigit(*szPtr); i++)
			szPtr++;

		if (*szPtr == ']')
		{
			szPtr++;
			strTimeTag.append(szLine, (int)(szPtr - szLine));
			return true;
		}
	}

	return false;
}

#define _IsCharBlank(ch) (ch == ' ' || ch == '	')

int GetTimeTagValue(const TCHAR *szTag, int nLen)
{
	int		nTime;

	nTime = 0;

	if (*szTag != '[')
		goto S_ERROR;

	szTag++;
	// time tag ?
	// seek to ':'
	while (_IsCharBlank(*szTag))
		szTag ++;

	nTime += _xtoi(szTag) * 60 * 1000;	// nTime += atoi(szTag) * 60s * 1000ms
	if (*szTag == '-')
		szTag++;
	while (IsDigit(*szTag))
		szTag ++;
	while (_IsCharBlank(*szTag))
		szTag ++;

	// ':'
	if (*szTag != ':' && *szTag != '.')
		goto S_ERROR;
	szTag ++;

	// number
	while (_IsCharBlank(*szTag))
		szTag ++;

	nTime += _xtoi(szTag) * 1000;	// nTime += atoi(szTag) * 1000ms
	if (*szTag == '-')
		szTag++;
	while (IsDigit(*szTag))
		szTag ++;
	while (_IsCharBlank(*szTag))
		szTag ++;

	if (*szTag == '.' || *szTag == ':')
	{
		szTag ++;
		// new version of lrc file
		while (_IsCharBlank(*szTag))
			szTag ++;

		if (IsDigit(szTag[1]))
		{
			// .000 or // .00
			if (IsDigit(szTag[2]))
				nTime += _xtoi(szTag);
			else
				nTime += _xtoi(szTag) * 10;
		}
		else
			nTime += _xtoi(szTag) * 100;	// .0
		if (*szTag == '-')
			szTag++;

		while (IsDigit(*szTag))
			szTag ++;
		while (_IsCharBlank(*szTag))
			szTag ++;
	}
	if (*szTag != ']')
		goto S_ERROR;

S_ERROR:
	return nTime;
}

//////////////////////////////////////////////////////////////////////

bool CLyricShowTextEditObj::OnCustomCommand(int nId)
{
	CSkinWnd::CAutoRedrawLock		arl(m_pSkin);

	if (nId == CID_FIND_PREV)
	{
		FindNext(false);
	}
	else if (nId == CID_FIND_NEXT)
	{
		FindNext();
	}
	else if (nId == CID_REPLACE)
	{
		// replace
		tstring	strFindWhat = m_pSkin->GetUIObjectText(CID_E_FIND);
		tstring	strReplaceWith = m_pSkin->GetUIObjectText(CID_E_REPLACE);
		tstring strSel;

		if (strFindWhat.empty())
			return true;

		SelectionToText(strSel);

		if (strSel.size() && _tcscmp(strSel.c_str(), strFindWhat.c_str()) == 0)
			ReplaceSel(strReplaceWith.c_str());
		FindNext();
	}
	else if (nId == CID_REPLACE_ALL)
	{
		// replace all
		tstring	strFindWhat = m_pSkin->GetUIObjectText(CID_E_FIND);
		tstring	strReplaceWith = m_pSkin->GetUIObjectText(CID_E_REPLACE);

		if (strFindWhat.empty())
			return true;

		SetCaret(0, 0);
		m_nBegSelCol = -1;

		CAutoBatchUndo		autoBatchUndo(this);
		while (FindNext())
			ReplaceSel(strReplaceWith.c_str());
	}

	switch (nId)
	{
	case CMD_JUMP:
		{
			int		nLine, nCol, nTime;
			tstring	strLine, strTimeTag;

			GetCaret(nLine, nCol);
			GetTextOfLine(nLine, strLine);
			if (GetTimeTagOfLine(strLine.c_str(), strTimeTag))
			{
				nTime = GetTimeTagValue(strTimeTag.c_str(), strTimeTag.size())
					- g_LyricData.GetOffsetTime();
				g_Player.SeekTo(nTime);
			}
			return true;
		}
		break;
	case CMD_DEL_TAG:
		{
			tstring	strLine, strTimeTag;

			int		nBegSelRow, nEndSelRow;

			if (m_nBegSelRow != -1)
			{
				if (m_nBegSelRow <= m_nEndSelRow)
				{
					nBegSelRow = m_nBegSelRow;
					nEndSelRow = m_nEndSelRow;
				}
				else
				{
					nBegSelRow = m_nEndSelRow;
					nEndSelRow = m_nBegSelRow;
				}
			}
			else
			{
				int		nLine, nCol;

				GetCaret(nLine, nCol);
				nBegSelRow = nEndSelRow = nLine;
			}

			CAutoBatchUndo		autoBatchUndo(this);

			for (int i = nBegSelRow; i <= nEndSelRow; i++)
			{
				GetTextOfLine(i, strLine);

				if (!GetTimeTagOfLine(strLine.c_str(), strTimeTag))
					continue;

				SetSelOfLine(i, 0, strTimeTag.size());
				RemoveSel();
			}

			return TRUE;
		}
		break;
	case CMD_INSERTTAG:
		SyncTimeTag(false);
		break;
	case CMD_INSERTTAG_DOWN:
		SyncTimeTag(true);
		break;
	case CMD_FORWARD_CUR_LINE:
		AdjustSyncTimeOfSelected(true);
		break;
	case CMD_BACKWARD_CUR_LINE:
		AdjustSyncTimeOfSelected(false);
		break;
	case CMD_FORWARD_REMAIN_LINES:
	case CMD_BACKWARD_REMAIN_LINES:
		{
			int		nLine, nCol;
			tstring	strLine, strTimeTag;
			TCHAR	szTimeTagNew[256];
			int		nTime;

			GetCaret(nLine, nCol);

			CAutoBatchUndo		autoBatchUndo(this);

			for (int i = nLine; GetTextOfLine(i, strLine); i++)
			{
				if (GetTimeTagOfLine(strLine.c_str(), strTimeTag))
				{
					nTime = GetTimeTagValue(strTimeTag.c_str(), strTimeTag.size());
					if (nId == CMD_FORWARD_REMAIN_LINES)
						nTime += 200;
					else
						nTime -= 200;

					FormtLrcTimeTag(nTime, true, szTimeTagNew);

					SetSelOfLine(i, 0, strTimeTag.size());
					ReplaceSel(szTimeTagNew);
				}
			}

			SetCaret(nLine, nCol);
			return TRUE;
		}
	case CMD_EDIT_UNDO:
		OnCmdUndo();
		break;
	case CMD_EDIT_REDO:
		OnCmdRedo();
		break;
	case CMD_EDIT_CUT:
		OnCmdCut();
		break;
	case CMD_EDIT_COPY:
		OnCmdCopy();
		break;
	case CMD_EDIT_PASTE:
		OnCmdPaste();
		break;
	case CMD_EDIT_DELETE:
		OnCmdDelete();
		break;
	case CMD_EDIT_FIND:
		ShowFindDialog();
		break;
	case CMD_EDIT_FINDNEXT:
		FindNext();
		break;
	case CMD_EDIT_REPLACE:
		ShowFindDialog(false);
		break;
	case CMD_AUTO_FILL_LYR_INFO:
		{
			LyricsProperties &prop = g_LyricData.Properties();

			// Auto fill artist, album and title info
			prop.m_strTitle = g_Player.GetTitle();
			prop.m_strArtist = g_Player.GetArtist();
			prop.m_strAlbum = g_Player.GetAlbum();
			prop.SetMediaLength(g_Player.GetMediaLength() / 1000);
			tstring		strLogName = g_Profile.GetProfileString(_T("LoginName"), _T(""));
			if (strLogName.size())
				prop.m_strBy = strLogName;

			UpdateLyricsProperties(true);
		}
		break;
	case CMD_REMOVE_ALL_TAG:
		{
			int		nLineCount;
			tstring	strLine, strTimeTag;

			nLineCount = GetLineCount();

			CAutoBatchUndo		autoBatchUndo(this);
			for (int i = 0; i < nLineCount; i++)
			{
				GetTextOfLine(i, strLine);

				if (!GetTimeTagOfLine(strLine.c_str(), strTimeTag))
					continue;

				SetSelOfLine(i, 0, strTimeTag.size());
				RemoveSel();
			}
		}
		break;
	case CMD_REMOVE_BLANK_LINE:
		{
			tstring	strLine;

			CAutoBatchUndo		autoBatchUndo(this);
			for (int i = GetLineCount() - 1; i >= 0; i--)
			{
				GetTextOfLine(i, strLine);

				LPCTSTR		szPtr = strLine.c_str();
				while (*szPtr == ' ' || *szPtr == '\t')
					szPtr++;
				if (*szPtr != '\0')
					continue;

				if (i == GetLineCount() - 1)
					SetSel(i, 0, i, strLine.size());
				else
					SetSel(i, 0, i + 1, 0);
				RemoveSel();
			}
		}
		break;
	case CMD_TRIM_WHITESPACE:
		{
			int		nLineCount;
			tstring	strLine, strTimeTag;
			LPCTSTR	szPtr;

			nLineCount = GetLineCount();

			CAutoBatchUndo		autoBatchUndo(this);
			for (int i = 0; i < nLineCount; i++)
			{
				GetTextOfLine(i, strLine);

				szPtr = strLine.c_str();
				// trim leading.
				while (*szPtr == ' ' || *szPtr == '\t')
					szPtr++;
				if (szPtr != strLine.c_str())
				{
					SetSelOfLine(i, 0, (int)(szPtr - strLine.c_str()));
					RemoveSel();
				}

				GetTextOfLine(i, strLine);
				// trim leading after time-tag
				if (GetTimeTagOfLine(strLine.c_str(), strTimeTag))
				{
					szPtr = strLine.c_str() + strTimeTag.size();

					while (*szPtr == ' ' || *szPtr == '\t')
						szPtr++;
					if (szPtr != strLine.c_str() + strTimeTag.size())
					{
						SetSelOfLine(i, strTimeTag.size(), (int)(szPtr - strLine.c_str() - strTimeTag.size()));
						RemoveSel();
					}
				}

				GetTextOfLine(i, strLine);
				szPtr = strLine.c_str() + strLine.size() - 1;
				// trim trailing
				while (szPtr != strLine.c_str() && (*szPtr == ' ' || *szPtr == '\t'))
					szPtr--;
				if (*szPtr != ' ' && *szPtr != '\t')
					szPtr++;
				if (szPtr != strLine.c_str() + strLine.size())
				{
					SetSelOfLine(i, (int)(szPtr - strLine.c_str()), (int)(strLine.c_str() + strLine.size() - szPtr));
					RemoveSel();
				}
			}
		}
		break;
	case CMD_REMOVE_UNSYNC_LINES:
		{
			tstring	strLine, strTimeTag;
			CLyrOneLine	*pLine;

			CAutoBatchUndo		autoBatchUndo(this);
			for (int i = GetLineCount() - 1; i >= 0; i--)
			{
				GetTextOfLine(i, strLine);
				pLine = GetLine(i);

				if (pLine && !pLine->bTimeLine && !GetTimeTagOfLine(strLine.c_str(), strTimeTag))
				{
					if (i == GetLineCount() - 1)
						SetSel(i, 0, i, strLine.size());
					else
						SetSel(i, 0, i + 1, 0);
					RemoveSel();
				}
			}
		}
		break;
	case CMD_CAPITALIZE_LEADING_LETTER:
		{
			tstring	strLine, strTimeTag;
			CLyrOneLine	*pLine;
			TCHAR		szReplaceWith[32];

			memset(szReplaceWith, 0, sizeof(szReplaceWith));

			CAutoBatchUndo		autoBatchUndo(this);
			for (int i = GetLineCount() - 1; i >= 0; i--)
			{
				GetTextOfLine(i, strLine);
				pLine = GetLine(i);
				if (!pLine)
					continue;

				if (pLine->bTimeLine && GetTimeTagOfLine(strLine.c_str(), strTimeTag))
				{
					LPCTSTR	szPos = strLine.c_str() + strTimeTag.size();
					while (*szPos == ' ' || *szPos == '\t' || *szPos == '\'' || *szPos == '\"')
						szPos++;
					if (*szPos == '(' || *szPos == '[')
						szPos++;
					if ((unsigned)(*szPos) <= 255 && islower(*szPos))
					{
						szReplaceWith[0] = toupper(*szPos);
						int		nPos = int(szPos - strLine.c_str());
						SetSel(i, nPos, i, nPos + 1);
						ReplaceSel(szReplaceWith);
					}
				}
				else
				{
					LPCTSTR	szPos = strLine.c_str();
					while (*szPos == ' ' || *szPos == '\t' || *szPos == '\'' || *szPos == '\"')
						szPos++;
					if ((unsigned)(*szPos) <= 255 && islower(*szPos))
					{
						szReplaceWith[0] = toupper(*szPos);
						int		nPos = int(szPos - strLine.c_str());
						SetSel(i, nPos, i, nPos + 1);
						ReplaceSel(szReplaceWith);
					}
				}
			}
		}
		break;
	case CMD_LYRICS_TO_LOWERCASE:
		{
			tstring	strLine, strTimeTag;
			CLyrOneLine	*pLine;

			CAutoBatchUndo		autoBatchUndo(this);
			for (int i = GetLineCount() - 1; i >= 0; i--)
			{
				GetTextOfLine(i, strLine);
				pLine = GetLine(i);

				strTimeTag.resize(0);
				GetTimeTagOfLine(strLine.c_str(), strTimeTag);

				LPTSTR	szPos = (LPTSTR)strLine.c_str() + strTimeTag.size();
				bool	bConverted = false;
				while (*szPos)
				{
					if (isupper(*szPos))
					{
						bConverted = true;
						*szPos = tolower(*szPos);
					}
					szPos++;
				}
				if (bConverted)
				{
					SetSel(i, 0, i, strLine.size());
					ReplaceSel(strLine.c_str());
				}
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

void CLyricShowTextEditObj::OnSaveLyrics()
{
	tstring			strEdit;

	GetText(strEdit);

	CHAR_ENCODING	nEncodingID = g_LyricData.Properties().GetEncodingIndex();
	g_LyricData.Properties().clear();
	g_LyricData.Properties().SetEncoding(nEncodingID);
	g_LyricData.FromString(strEdit.c_str());

#define SET_PROP(strProp, propCtrlId)				\
	str = m_pSkin->GetUIObjectText(propCtrlId);		\
	if (str.size())									\
		strProp = str;								\
	else if (_tcscmp(strProp.c_str(), str.c_str()) != 0) \
		m_pSkin->SetUIObjectText(propCtrlId, strProp.c_str());

	LyricsProperties &prop = g_LyricData.Properties();
	tstring			str, strOffset = prop.GetOffsetTimeStr();
	SET_PROP(prop.m_strArtist, CID_E_ARTIST);
	SET_PROP(prop.m_strTitle, CID_E_TITLE);
	SET_PROP(prop.m_strAlbum, CID_E_ALBUM);
	SET_PROP(prop.m_strBy, CID_E_BY);
	SET_PROP(strOffset, CID_E_OFFSET);
	SET_PROP(prop.m_strMediaLength, CID_E_MEDIA_LENGTH);
	if (strOffset.size())
		prop.SetOffsetTime(strOffset.c_str());
};

void CLyricShowTextEditObj::OnLyricsChanged()
{
	if (m_bDiscardLyrChangeEvent)
		return;

	tstring		str, strTags, strValue;
	g_LyricData.ToString(str, FT_LYRICS_LRC, false);

	UpdateLyricsProperties();

	SetText(str.c_str());

	int		nCurLine;
	nCurLine = g_LyricData.GetCurPlayLine(g_LyricData.GetRawLyrics());
	SetCaret(nCurLine, 0);
	// ::SendMessage(GetHandle(), EM_LINESCROLL, 0, nCurLine);

	m_pSkin->InvalidateRect();

	CSkinToolbar		*pToolBar;

	pToolBar = (CSkinToolbar *)m_pSkin->GetUIObjectById(ID_TOOLBAR, CSkinToolbar::ClassName());
	if (pToolBar)
	{
		pToolBar->EnableBt(CMD_EDIT_REDO, CanRedo());
		pToolBar->EnableBt(CMD_EDIT_UNDO, CanUndo());
		pToolBar->EnableBt(CMD_EDIT_CUT, IsSelected());
		pToolBar->EnableBt(CMD_EDIT_COPY, IsSelected());
		pToolBar->Invalidate();
	}
}

void CLyricShowTextEditObj::OnPlayTimeChangedUpdate()
{
	if (m_pSkin->IsIconic() || !IsVisible() || !IsParentVisible())
		return;

	{
		// Prevent player forward to next track, if lyrics editor is focus, and lyrics changed.
		if ((int)g_Player.GetMediaLength() > 30 * 1000
			&& CanUndo() && g_LyricData.GetPlayElapsedTime() >= (int)g_Player.GetMediaLength() - 1000
			&& _tcscmp(g_Player.GetSrcMedia(), g_LyricData.GetSongFileName()) == 0)
			g_Player.SeekTo(0);
	}

	CRect	rc;
	int		nBegSelRow, nEndSelRow;
	int		nBegSelCol, nEndSelCol;
	bool	bSelectedEmpty = true;

	if (m_nBegSelRow != -1)
	{
		bSelectedEmpty = false;
		SortSelectPos(nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol, bSelectedEmpty);
	}

	if (bSelectedEmpty)
	{
		CRawGraph		*pGraph;
		CRect			rcUpdate;

		pGraph = GetMemGraphics();

		CRect	rcClip;

		rc = m_rcCtrl;
		rcClip.SetRect(rc.left + m_xMargin, rc.top + m_yMargin, rc.right - m_xMargin, rc.bottom - m_yMargin);

		CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraph);
		pGraph->SetClipBoundBox(rcClip);

		FastDraw(pGraph, &rcUpdate);

		autoCBR.Recover();

		if (rcUpdate.top != rcUpdate.bottom)
		{
			UpdateRectToScreen(&rcUpdate);
		}
	}
}

void CLyricShowTextEditObj::UpdateLyricsProperties(bool bRedraw)
{
	LyricsProperties &prop = g_LyricData.Properties();

	m_pSkin->SetUIObjectText(CID_E_ARTIST, prop.m_strArtist.c_str(), false);
	m_pSkin->SetUIObjectText(CID_E_TITLE, prop.m_strTitle.c_str(), false);
	m_pSkin->SetUIObjectText(CID_E_ALBUM, prop.m_strAlbum.c_str(), false);
	m_pSkin->SetUIObjectText(CID_E_BY, prop.m_strBy.c_str(), false);
	m_pSkin->SetUIObjectText(CID_E_OFFSET, prop.GetOffsetTimeStr(), false);
	m_pSkin->SetUIObjectText(CID_E_MEDIA_LENGTH, prop.m_strMediaLength.c_str(), false);

	if (bRedraw)
		m_pSkin->InvalidateRect();
}

void CLyricShowTextEditObj::OnStatusChanged(IEditNotification::Status status, bool bVal)
{
	CSkinToolbar		*pToolBar;

	pToolBar = (CSkinToolbar *)m_pSkin->GetUIObjectById(ID_TOOLBAR, CSkinToolbar::ClassName());
	if (pToolBar)
	{
		if (status == IEditNotification::S_CAN_REDO)
			pToolBar->EnableBt(CMD_EDIT_REDO, bVal);
		if (status == IEditNotification::S_CAN_UNDO)
			pToolBar->EnableBt(CMD_EDIT_UNDO, bVal);
		if (status == IEditNotification::S_SEL)
		{
			pToolBar->EnableBt(CMD_EDIT_CUT, bVal);
			pToolBar->EnableBt(CMD_EDIT_COPY, bVal);
		}
		pToolBar->Invalidate();
	}
}

void CLyricShowTextEditObj::OnTextChanged()
{
	if (m_nTimerIDUpdateLyrics)
		m_pSkin->UnregisterTimerObject(this, m_nTimerIDUpdateLyrics);
	m_nTimerIDUpdateLyrics = m_pSkin->RegisterTimerObject(this, TIMER_SPAN_LYR_PREVIEW_UPDATE);
// 	m_pSkin->KillTimer(TIMER_LYR_PREVIEW_UPDATE);
// 	m_pSkin->SetTimer(TIMER_LYR_PREVIEW_UPDATE, TIMER_SPAN_LYR_PREVIEW_UPDATE);
}

void CLyricShowTextEditObj::OnTimer(int nId)
{
	if (nId == m_nTimerIDUpdateLyrics)
	{
		m_pSkin->UnregisterTimerObject(this, m_nTimerIDUpdateLyrics);
		
		OnSaveLyrics();

		m_bDiscardLyrChangeEvent = true;
		CMPlayerAppBase::GetInstance()->DispatchLyricsChangedSyncEvent();
		m_bDiscardLyrChangeEvent = false;
	}
	else
		CSkinEditCtrl::OnTimer(nId);
}

void CLyricShowTextEditObj::UpdateTextFontColor()
{
	MLColor				m_clrTag, m_clrFocusLineBg;
	MLColor				m_clrLowlight, m_clrHighlight, m_clrBg;

	ProfileGetColorValue(m_clrHighlight, SZ_SECT_LYR_DISPLAY, _T("Ed_HighColor"));
	ProfileGetColorValue(m_clrLowlight, SZ_SECT_LYR_DISPLAY, _T("Ed_LowColor"));
	ProfileGetColorValue(m_clrBg, SZ_SECT_LYR_DISPLAY, _T("Ed_BgColor"));
	ProfileGetColorValue(m_clrTag, SZ_SECT_LYR_DISPLAY, _T("Ed_TagColor"));
	ProfileGetColorValue(m_clrFocusLineBg, SZ_SECT_LYR_DISPLAY, _T("Ed_FocusLineBgColor"));

	SetColor(CN_BG, m_clrBg);
	SetSelColor(m_clrHighlight, m_clrFocusLineBg);
	SetColorTheme(m_clrHighlight, m_clrLowlight, m_clrTag, m_clrFocusLineBg);

	const int nEditorFontMaxSize = 22;
	int			nHeight, nWeight;
	BYTE		byItalic;
	tstring		strFaceName, strFaceNameOthers;

	ProfileGetLyricsFont(SZ_SECT_LYR_DISPLAY, nHeight, nWeight, byItalic, strFaceName, strFaceNameOthers);
	if (nHeight > nEditorFontMaxSize)
		nHeight = nEditorFontMaxSize;

	m_font.Create(strFaceName.c_str(), strFaceNameOthers.c_str(), nHeight, nWeight, byItalic, false);

	OnFontChanged();
}

void CLyricShowTextEditObj::SyncTimeTag(bool bMoveToNextLine)
{
	int		nBegLine, nEndLine, nCol = 0;
	int		nBegCol, nEndCol;
	bool	bSelectedEmpty = true;
	tstring	strLine, strTimeTag;
	TCHAR	szTimeTagNew[256];
	int		nTime;

	if (m_nBegSelRow != -1)
	{
		SortSelectPos(nBegLine, nBegCol, nEndLine, nEndCol, bSelectedEmpty);
	}
	else
	{
		GetCaret(nBegLine, nCol);
		nEndLine = nBegLine;
	}

	int		nTimeDuarationOffset = 0;
	int		nLine;
	CAutoBatchUndo	autoBatchUndo(this);
	int		nPlayingPos = g_LyricData.GetPlayElapsedTime();
	if (nPlayingPos < 0)
		nPlayingPos = 0;

	for (nLine = nBegLine; nLine <= nEndLine; nLine++)
	{
		if (!GetTextOfLine(nLine, strLine))
			continue;

		if (!GetTimeTagOfLine(strLine.c_str(), strTimeTag))
		{
			// This line wasn't synchronized yet,
			if (nLine == nBegLine)
			{
				FormtLrcTimeTag(nPlayingPos, true, szTimeTagNew);

				SetSelOfLine(nLine, 0, 0);
				ReplaceSel(szTimeTagNew);
				break;
			}

			continue;
		}

		// Adjust the time of this line.
		nTime = GetTimeTagValue(strTimeTag.c_str(), strTimeTag.size());
		if (nLine == nBegLine)
		{
			nTimeDuarationOffset = nTime - nPlayingPos;
			nTime = g_LyricData.GetPlayElapsedTime();
		}
		else
			nTime -= nTimeDuarationOffset;

		// Set new time
		FormtLrcTimeTag(nTime, true, szTimeTagNew);

		SetSelOfLine(nLine, 0, strTimeTag.size());
		ReplaceSel(szTimeTagNew);
	}

	if (!bSelectedEmpty)
	{
		SetSel(nBegLine, nBegCol, nEndLine, nEndCol);
		return;
	}

	if (bMoveToNextLine && nBegLine == nEndLine)
	{
		SetCaret(nEndLine + 1, 0);
		AutoVScrollDown(1);
	}
	else
		SetCaret(nEndLine, nCol);
}

void CLyricShowTextEditObj::AdjustSyncTimeOfSelected(bool bIncreaseTime)
{
	int		nBegLine, nEndLine, nCol;
	tstring	strLine, strTimeTag;
	TCHAR	szTimeTagNew[256];
	int		nTime;
	int		nBegCol, nEndCol;
	bool	bSelectedEmpty = true;

	if (m_nBegSelRow != -1)
	{
		SortSelectPos(nBegLine, nBegCol, nEndLine, nEndCol, bSelectedEmpty);
	}
	else
	{
		GetCaret(nBegLine, nCol);
		nEndLine = nBegLine;
	}

	int		nTimeDuarationOffset = 0;
	int		nLine;
	CAutoBatchUndo	autoBatchUndo(this);
	int		nPlayingPos = g_LyricData.GetPlayElapsedTime();
	if (nPlayingPos < 0)
		nPlayingPos = 0;

	for (nLine = nBegLine; nLine <= nEndLine; nLine++)
	{
		if (!GetTextOfLine(nLine, strLine))
			continue;

		if (!GetTimeTagOfLine(strLine.c_str(), strTimeTag))
		{
			// This line wasn't synchronized yet,
			continue;
		}

		// Adjust the time of this line.
		nTime = GetTimeTagValue(strTimeTag.c_str(), strTimeTag.size());
		if (bIncreaseTime)
			nTime += 200;
		else
			nTime -= 200;

		// Set new time
		FormtLrcTimeTag(nTime, true, szTimeTagNew);

		SetSelOfLine(nLine, 0, strTimeTag.size());
		ReplaceSel(szTimeTagNew);
	}

	if (!bSelectedEmpty)
		SetSel(nBegLine, nBegCol, nEndLine, nEndCol);
}

void CLyricShowTextEditObj::ShowFindDialog(bool bFind)
{
	CSkinBarFrame		*barFrameFindReplace;

	barFrameFindReplace = (CSkinBarFrame *)m_pSkin->GetUIObjectById(
		GetIDByName(_T("CID_EDITOR_FRAME")),
		CSkinBarFrame::ClassName());
	ASSERT(barFrameFindReplace);
	if (barFrameFindReplace)
	{
		int		CID_C_FIND_REPLACE = GetIDByName(_T("CID_C_FIND_REPLACE"));

		if (bFind && m_findReplaceStatus == FRS_FIND
			|| !bFind && m_findReplaceStatus == FRS_REPLACE)
		{
			// Hide
			m_findReplaceStatus = FRS_NONE;
			barFrameFindReplace->ShowBar(CID_C_FIND_REPLACE, false);
			return;
		}

		if (m_findReplaceStatus == FRS_NONE)
		{
			tstring		strFindWhat;
			SelectionToText(strFindWhat);
			if (strFindWhat.size())
				m_pSkin->SetUIObjectText(CID_E_FIND, strFindWhat.c_str(), false);
		}

		// Show
		if (bFind)
		{
			m_findReplaceStatus = FRS_FIND;
			barFrameFindReplace->SetTopBarHeight(35, false);
		}
		else
		{
			m_findReplaceStatus = FRS_REPLACE;
			barFrameFindReplace->SetTopBarHeight(70, false);
		}
		m_pSkin->SetUIObjectVisible(_T("CID_C_REPLACE"), !bFind, false);

		barFrameFindReplace->ShowBar(CID_C_FIND_REPLACE, true);
	}
}

bool CLyricShowTextEditObj::FindNext(bool bSearchDown)
{
	DWORD			dwFlags = 0;

	if (m_pSkin->IsButtonChecked(CID_MATCHCASE))
		dwFlags |= FTF_MATCH_CASE;
	if (bSearchDown)
		dwFlags |= FTF_DOWN;

	tstring	strFindWhat = m_pSkin->GetUIObjectText(CID_E_FIND);

	if (!FindText(strFindWhat.c_str(), dwFlags))
	{
		m_pSkin->MessageOut(CStrPrintf(TOLOCAL(_T("Can't find \"%s\"")), strFindWhat.c_str()).c_str());
		return false;
	}

	return true;
}
