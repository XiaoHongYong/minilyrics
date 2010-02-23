// LyricShowTextEditObj.cpp: implementation of the CLyricShowTextEditObj class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MLCmd.h"
#include "LyricShowTextEditObj.h"
#include "SkinToolBar.h"

#ifdef _WIN32
#include "win32/MPSkinMenu.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/MPSkinMenu.h"
#endif

#define TIMER_SPAN_LYR_PREVIEW_UPDATE	500


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

void FormtLrcTimeTag(int nTime, TCHAR szBuff[])
{
	int		nMin, nSec, nMs;

	nMs = (nTime / 10) % 100;
	nSec = (nTime / 1000) % 60;
	nMin = nTime / (1000 * 60);

	_stprintf_s(szBuff, 32, _T("[%02d:%02d.%02d]"), nMin, nSec, nMs);
}

//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CLyricShowTextEditObj, _T("LyricShowTextEdit"));

CLyricShowTextEditObj::CLyricShowTextEditObj()
{
	m_nMsgWant |= UO_MSG_WANT_RBUTTON;

	m_bDiscardLyrChangeEvent = false;
	m_nHeightSbCorner = 0;
	m_nWidthSbConer = 0;
	m_nTimerIDUpdateLyrics = 0;
}

CLyricShowTextEditObj::~CLyricShowTextEditObj()
{
	OnSaveLyrics();

	g_eventDispather.UnregisterHanlder(ET_LYRICS_ON_SAVE_EDIT, this);
	g_eventDispather.UnregisterHanlder(ET_LYRICS_EDITOR_RELOAD_TAG, this);
	g_eventDispather.UnregisterHanlder(ET_LYRICS_EDITOR_CMD, this);
}


MLColor & CLyricShowTextEditObj::GetColor(int nColorName)
{
	if (nColorName == CN_BG)
		return m_pSkin->GetBgColor();
	else
		return CEditHandler::GetColor(nColorName);
}

void CLyricShowTextEditObj::OnCreate()
{
	SetEditNotification(this);

	_BaseClass::OnCreate();

	OnLyricsChanged();

	g_eventDispather.RegisterHanlder(ET_LYRICS_ON_SAVE_EDIT, this);
	g_eventDispather.RegisterHanlder(ET_LYRICS_EDITOR_RELOAD_TAG, this);
	g_eventDispather.RegisterHanlder(ET_LYRICS_EDITOR_CMD, this);

	UpdateTextFontColor();

	CLyrEditHandler::EH_OnCreate();
}

void CLyricShowTextEditObj::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_LYRICS_ON_SAVE_EDIT)
		OnSaveLyrics();
	else if (pEvent->eventType == ET_LYRICS_EDITOR_RELOAD_TAG)
		UpdateLyricsProperties();
	else if (pEvent->eventType == ET_LYRICS_EDITOR_CMD)
	{
		CEventLyricsEditorCmd	*pCmdEvent = (CEventLyricsEditorCmd *)pEvent;
		OnCommand(pCmdEvent->cmd, 0);
	}
	else if (pEvent->eventType == m_etDispSettings)
	{
		if (OnLyrDisplaySettings(pEvent->strName.c_str(), pEvent->strValue.c_str()))
		{
			LPCTSTR		szProperty = pEvent->strName.c_str();
			if (_tcsicmp(szProperty, _T("BgColor")) == 0)
			{
				Invalidate();
			}
			else if (_tcsicmp(szProperty, _T("FgColor")) == 0 || 
				_tcsicmp(szProperty, _T("FgLowColor")) == 0 || 
				_tcsicmp(szProperty, _T("TagColor")) == 0 || 
				_tcsicmp(szProperty, _T("FocusLineBgColor")) == 0 || 
				_tcsicmp(szProperty, _T("Font")) == 0)
			{
				UpdateTextFontColor();
				Invalidate();
			}
		}
	}
	else
		CLyricShowObj::OnEvent(pEvent);
}

BOOL CLyricShowTextEditObj::OnCommand(DWORD dwCmdID, LPARAM lParam)
{
	switch (dwCmdID)
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
					if (dwCmdID == CMD_FORWARD_REMAIN_LINES)
						nTime += 200;
					else
						nTime -= 200;

					FormtLrcTimeTag(nTime, szTimeTagNew);

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
			// Auto fill artist, album and title info
			g_LyricData.Properties().m_strTitle = g_Player.GetTitle();
			g_LyricData.Properties().m_strArtist = g_Player.GetArtist();
			g_LyricData.Properties().m_strAlbum = g_Player.GetAlbum();

			UpdateLyricsProperties();
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

				if (pLine && pLine->lineType == CMLData::LP_NONE && !GetTimeTagOfLine(strLine.c_str(), strTimeTag))
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

				if (pLine->lineType == CMLData::LP_TIME_TAG && GetTimeTagOfLine(strLine.c_str(), strTimeTag))
				{
					LPCTSTR	szPos = strLine.c_str() + strTimeTag.size();
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
				else if (pLine->lineType == CMLData::LP_NONE)
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

				if (pLine && (pLine->lineType == CMLData::LP_NONE || pLine->lineType == CMLData::LP_TIME_TAG))
				{
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

	CLyrEditHandler::GetText(strEdit);

	FILE_ENCODING	fileEncoding;
#ifdef _UNICODE
	BOOL			bUnicode = TRUE;
#else
	BOOL			bUnicode = FALSE;
#endif
	CHAR_ENCODING	nEncodingID = g_LyricData.GetEncodingIndex();
	g_LyricData.Properties().clear();
	g_LyricData.SetEncoding(nEncodingID);
	g_LyricData.FromString(strEdit.c_str(), strEdit.size() * sizeof(TCHAR),
		&fileEncoding, FT_LYRICS_LRC, TRUE, bUnicode, TRUE);
};

void CLyricShowTextEditObj::OnLyricsChanged()
{
	if (m_bDiscardLyrChangeEvent)
		return;

	tstring		str, strTags, strValue;
	g_LyricData.ToString(str, FT_LYRICS_LRC, false);

	CMLData::LYRICS_PROP_NAME	*lyricsPropNames = CMLData::GetSupportedLyricsPropName();

	for (int i = 0; lyricsPropNames[i].szName != NULL; i++)
	{
		if (lyricsPropNames[i].prop == CMLData::LP_ENCODING)
			continue;
		strTags += lyricsPropNames[i].szName;
		strTags += _T(" ");
		g_LyricData.GetProperty(lyricsPropNames[i].prop, strValue);
		strTags += strValue;
		strTags += _T("\r\n");
	}
	str.insert(str.begin(), strTags.begin(), strTags.end());

	SetText(str.c_str());

	int		nCurLine;
	nCurLine = g_LyricData.GetCurPlayLine(m_lyrLines);
	SetCaret(nCurLine, 0);
	// ::SendMessage(GetHandle(), EM_LINESCROLL, 0, nCurLine);

	Invalidate();

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

bool CLyricShowTextEditObj::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_pSkin->ClientToScreen(point);
	OnContexMenu(point.x, point.y);

	return true;
}

void CLyricShowTextEditObj::OnContexMenu(int xPos, int yPos)
{
	CMPSkinMenu	menu;
	bool		bEnable;

	menu.LoadPopupMenu(IDM_MENU, 2);

	bEnable = IsSelected();
	menu.EnableItem(IDC_EDIT_CUT, bEnable);
	menu.EnableItem(IDC_EDIT_COPY, bEnable);

	bEnable = m_undoMgr.CanUndo();
	menu.EnableItem(IDC_EDIT_UNDO, bEnable);

	bEnable = m_undoMgr.CanRedo();
	menu.EnableItem(IDC_EDIT_REDO, bEnable);

	menu.TrackPopupMenu(xPos, yPos, m_pSkin);
}

void CLyricShowTextEditObj::OnPlayTimeChangedUpdate()
{
	if (m_pSkin->IsIconic() || !IsVisible() || !IsParentVisible())
		return;

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

		pGraph = EB_GetGraphics();

		CRect	rcClip;

		EB_GetBoardRect(rc);
		rcClip.SetRect(rc.left + m_nMarginX, rc.top + m_nMarginY, rc.right - m_nMarginX, rc.bottom - m_nMarginY);

		CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraph);
		pGraph->SetClipBoundBox(rcClip);

		FastDraw(pGraph, &rcUpdate);

		autoCBR.Recover();

		if (rcUpdate.top != rcUpdate.bottom)
		{
			EB_UpdateBoardRect(pGraph, &rcUpdate);
		}

		EB_ReleaseGraphics(pGraph);
	}
}

void CLyricShowTextEditObj::Draw(CRawGraph *pGraph)
{
	pGraph->SetFont(&m_font);
	EH_OnPaint(pGraph, m_rcObj);

	m_caret.Draw(pGraph);
}

void CLyricShowTextEditObj::FastDraw(CRawGraph *pGraphics, CRect *prcUpdate)
{
	pGraphics->SetFont(&m_font);

	CLyrEditHandler::FastDraw(pGraphics, prcUpdate);

	CRect	rcCaret;

	rcCaret.IntersectRect(&m_caret.GetUpdateRect(), prcUpdate);
	if (!rcCaret.IsRectEmpty())
	{
		m_caret.Draw(pGraphics);
	}
}

void CLyricShowTextEditObj::UpdateLyricsProperties()
{
	CAutoBatchUndo		autoBatchUndo(this);
	SetLyricsTagValue(CMLData::LP_TITLE, g_LyricData.Properties().m_strTitle.c_str());
	SetLyricsTagValue(CMLData::LP_ARTIST, g_LyricData.Properties().m_strArtist.c_str());
	SetLyricsTagValue(CMLData::LP_ALBUM, g_LyricData.Properties().m_strAlbum.c_str());
	SetLyricsTagValue(CMLData::LP_BY, g_LyricData.Properties().m_strBy.c_str());
	SetLyricsTagValue(CMLData::LP_OFFSET, CStrPrintf(_T("%d"), g_LyricData.Properties().m_nTimeOffset).c_str());
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
		g_mpApp.DispatchLyricsChangedSyncEvent();
		m_bDiscardLyrChangeEvent = false;
	}
	else
		_BaseClass::OnTimer(nId);
}

void CLyricShowTextEditObj::UpdateTextFontColor()
{
	SetSelColor(GetHighlightColor(), m_clrFocusLineBg);
	SetColorTheme(GetHighlightColor(), GetLowlightColor(), m_clrTag, m_clrFocusLineBg);

	const int nEditorFontMaxSize = 18;
	if (m_font.GetHeight() > nEditorFontMaxSize)
	{
		int			nHeight, nWeight;
		BYTE		byItalic;
		tstring		strFaceName, strFaceNameOthers;

		ProfileGetLyricsFont(m_strSectName.c_str(), nHeight, nWeight, byItalic, strFaceName, strFaceNameOthers);

		m_font.Create(strFaceName.c_str(), strFaceNameOthers.c_str(), nEditorFontMaxSize, nWeight, byItalic);
		m_nFontHeight = m_font.GetHeight();
	}

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
	int		nPlayingPos = m_pMLData->GetPlayElapsedTime();
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
				FormtLrcTimeTag(nPlayingPos, szTimeTagNew);

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
			nTime = m_pMLData->GetPlayElapsedTime();
		}
		else
			nTime -= nTimeDuarationOffset;

		// Set new time
		FormtLrcTimeTag(nTime, szTimeTagNew);

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
	int		nPlayingPos = m_pMLData->GetPlayElapsedTime();
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
		FormtLrcTimeTag(nTime, szTimeTagNew);

		SetSelOfLine(nLine, 0, strTimeTag.size());
		ReplaceSel(szTimeTagNew);
	}

	if (!bSelectedEmpty)
		SetSel(nBegLine, nBegCol, nEndLine, nEndCol);
}

void CLyricShowTextEditObj::ReloadThemeOfSkin(bool bSavetoProfile)
{
	CLyricShowObj::ReloadThemeOfSkin(bSavetoProfile);

	UpdateTextFontColor();
}

void CLyricShowTextEditObj::OnFRCommand(IFindReplaceNotify::Command cmd)
{
	DWORD			dwFlags = 0;

	if (!m_dlgFindReplace.m_bSearchUp)
		dwFlags |= FTF_DOWN;
	if (m_dlgFindReplace.m_bMatchCase)
		dwFlags |= FTF_MATCH_CASE;

	switch (cmd)
	{
	case IFindReplaceNotify::C_FIND_NEXT:
		FindNext();
		break;
	case IFindReplaceNotify::C_REPLACE:
		{
			// replace
			if (IsSelected())
			{
				ReplaceSel(m_dlgFindReplace.m_strReplaceWith.c_str());
				FindText(m_dlgFindReplace.m_strFindWhat.c_str(), dwFlags);
			}
			else
			{
				if (!FindText(m_dlgFindReplace.m_strFindWhat.c_str(), dwFlags))
					m_pSkin->MessageOut(CStrPrintf(_T("%s \"%s\""), TOLOCAL(_T("Can't find")), m_dlgFindReplace.m_strFindWhat.c_str()).c_str());
			}
		}
		break;
	case IFindReplaceNotify::C_REPLACE_ALL:
		{
			// replace all
			SetCaret(0, 0);
			m_nBegSelCol = -1;

			CAutoBatchUndo		autoBatchUndo(this);

			while (FindText(m_dlgFindReplace.m_strFindWhat.c_str(), dwFlags))
				ReplaceSel(m_dlgFindReplace.m_strReplaceWith.c_str());
		}
		break;
	case IFindReplaceNotify::C_CLOSE_DLG:
		{
			g_Profile.WriteProfileString(_T("FindWhat"), m_dlgFindReplace.m_strFindWhat.c_str());
			g_Profile.WriteProfileString(_T("ReplaceWith"), m_dlgFindReplace.m_strReplaceWith.c_str());
		}
		break;
	}
}

void CLyricShowTextEditObj::ShowFindDialog(bool bFind)
{
	if (m_dlgFindReplace.IsValid())
	{
		m_dlgFindReplace.ShowWindow(SW_SHOW);
		m_dlgFindReplace.SetForeground();
		return;
	}

	m_dlgFindReplace.m_strFindWhat.resize(0);
	SelectionToText(m_dlgFindReplace.m_strFindWhat);

	m_dlgFindReplace.m_strFindWhat = g_Profile.GetProfileString(_T("FindWhat"), _T(""));
	if (!bFind)
		m_dlgFindReplace.m_strReplaceWith = g_Profile.GetProfileString(_T("ReplaceWith"), _T(""));

	m_dlgFindReplace.ShowDlg(m_pSkin, this, !bFind);
}

void CLyricShowTextEditObj::FindNext()
{
	DWORD			dwFlags = 0;

	if (!m_dlgFindReplace.m_bSearchUp)
		dwFlags |= FTF_DOWN;
	if (m_dlgFindReplace.m_bMatchCase)
		dwFlags |= FTF_MATCH_CASE;

	if (!FindText(m_dlgFindReplace.m_strFindWhat.c_str(), dwFlags))
		m_pSkin->MessageOut(CStrPrintf(_T("%s \"%s\""), TOLOCAL(_T("Can't find")), m_dlgFindReplace.m_strFindWhat.c_str()).c_str());
}
