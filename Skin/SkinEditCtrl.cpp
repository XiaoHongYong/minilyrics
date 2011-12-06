#include "StdAfx.h"
#include "Skin.h"
#include "SkinMenu.h"
#include "SkinEditCtrl.h"
#include "../MPShared/resource.h"

UIOBJECT_CLASS_NAME_IMP(CSkinEditCtrl, _T("EditCtrl"))


#ifdef _WINCE_
#ifndef TPM_RIGHTBUTTON
#define TPM_RIGHTBUTTON		0
#endif
#endif

#define GLYPH_WIDTH_INVALID		0xFF

#define PASSWORD_CHAR			_T("*")

//////////////////////////////////////////////////////////////////////////

class CEditExAction : public CUndoAction
{
public:
	CEditExAction() { m_nCaretRow = m_nCaretCol = 0; m_pEdit = NULL; }
	virtual ~CEditExAction() { }

protected:
	CSkinEditCtrl		*m_pEdit;
	int			m_nCaretRow, m_nCaretCol;
	tstring		m_str;

};

class CEditInsertAction : public CEditExAction
{
public:
	CEditInsertAction(CSkinEditCtrl *pEdit, int nCaretRow, int nCaretCol, LPCTSTR str)
	{
		m_pEdit = pEdit;
		m_nCaretRow = nCaretRow;
		m_nCaretCol = nCaretCol;
		m_str = str;
	}
	CEditInsertAction(CSkinEditCtrl *pEdit, int nCaretRow, int nCaretCol, TCHAR ch)
	{
		m_pEdit = pEdit;
		m_nCaretRow = nCaretRow;
		m_nCaretCol = nCaretCol;
		m_str = ch;
	}
	virtual ~CEditInsertAction() { }

	virtual void Undo()
	{
		if (m_str.size() == 1)
		{
			ASSERT(m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol));
			if (m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol))
			{
				m_pEdit->RemoveChar(m_nCaretRow, m_nCaretCol);
				m_pEdit->m_nCaretRow = m_nCaretRow;
				m_pEdit->m_nCaretCol = m_nCaretCol;
				if (!m_pEdit->IsInBatchAction())
				{
					m_pEdit->UpdateScrollInfo();
					m_pEdit->MakeCaretInSight();
				}
			}
		}
		else
		{
			ASSERT(m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol));
			if (m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol))
			{
				int			n = 0;
				LPCTSTR		szText = m_str.c_str();
				while (*szText)
				{
					if (*szText == '\r')
					{
						szText++;
						if (*szText == '\n')
							szText++;
						n++;
					}
					else if (*szText == '\n')
						n++;
					else
					{
						szText++;
						n++;
					}
				}
				m_pEdit->RemoveStr(m_nCaretRow, m_nCaretCol, n);
				m_pEdit->m_nCaretRow = m_nCaretRow;
				m_pEdit->m_nCaretCol = m_nCaretCol;
				if (!m_pEdit->IsInBatchAction())
				{
					m_pEdit->UpdateScrollInfo();
					m_pEdit->MakeCaretInSight();
				}
			}
		}
	}
	virtual void Redo()
	{
		if (m_str.size() == 1)
		{
			ASSERT(m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol));
			if (m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol))
			{
				m_pEdit->InsertChar(m_nCaretRow, m_nCaretCol, m_str[0]);
				m_pEdit->m_nCaretRow = m_nCaretRow;
				m_pEdit->m_nCaretCol = m_nCaretCol;
				m_pEdit->NextPos(m_pEdit->m_nCaretRow, m_pEdit->m_nCaretCol);
				if (!m_pEdit->IsInBatchAction())
				{
					m_pEdit->UpdateScrollInfo();
					m_pEdit->MakeCaretInSight();
				}
			}
		}
		else
		{
			ASSERT(m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol));
			if (m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol))
			{
				m_pEdit->InsertStr(m_nCaretRow, m_nCaretCol, m_str.c_str(), m_pEdit->m_nCaretRow, m_pEdit->m_nCaretCol);
				if (!m_pEdit->IsInBatchAction())
				{
					m_pEdit->UpdateScrollInfo();
					m_pEdit->MakeCaretInSight();
				}
			}
		}
	}

};


class CEditDelAction : public CEditInsertAction
{
public:
  	CEditDelAction(CSkinEditCtrl *pEdit, int nCaretRow, int nCaretCol, LPCTSTR str)
		: CEditInsertAction(pEdit, nCaretRow, nCaretCol, str)
  	{
  	}
  	CEditDelAction(CSkinEditCtrl *pEdit, int nCaretRow, int nCaretCol, TCHAR ch)
		: CEditInsertAction(pEdit, nCaretRow, nCaretCol, ch)
  	{
  	}
  	virtual ~CEditDelAction() { }

	virtual void Undo()
	{
		CEditInsertAction::Redo();
	}
	virtual void Redo()
	{
		CEditInsertAction::Undo();
	}

};

class CEditRepAction : public CUndoAction
{
public:
	CEditRepAction(CSkinEditCtrl *pEdit, int nCaretRow, int nCaretCol, LPCTSTR szSrc, LPCTSTR szTarg)
		: m_del(pEdit, nCaretRow, nCaretCol, szSrc),
		m_insert(pEdit, nCaretRow, nCaretCol, szTarg)
	{
	}
	CEditRepAction(CSkinEditCtrl *pEdit, int nCaretRow, int nCaretCol, LPCTSTR szSrc, TCHAR chTarg)
		: m_del(pEdit, nCaretRow, nCaretCol, szSrc),
		m_insert(pEdit, nCaretRow, nCaretCol, chTarg)
	{
	}

	virtual void Undo()
	{
		m_insert.Undo();
		m_del.Undo();
	}
	virtual void Redo()
	{
		m_del.Redo();
		m_insert.Redo();
	}

protected:
	CEditDelAction			m_del;
	CEditInsertAction		m_insert;

};

//////////////////////////////////////////////////////////////////////
//
// CSkinEditCtrl
//
//////////////////////////////////////////////////////////////////////

CSkinEditCtrl::CSkinEditCtrl()
{
	m_bEnableBorder = true;

	m_nEditorStyles = 0;

	m_nCaretMaxXLatest = m_nCaretX = 0;
	m_nCaretY = 0;
	m_nLineSpace = 2;
	m_nTopVisibleLine = 0;

	m_vClrTable.resize(CN_CUSTOMIZED_START);
	m_vClrTable[CN_BG].set(RGB(255, 255, 255));
	m_vClrTable[CN_TEXT].set(RGB(0, 0, 0));
	m_vClrTable[CN_SEL_BG].set(RGB(0x7A, 0x96, 0xDF));
	m_vClrTable[CN_SEL_TEXT].set(RGB(255, 255, 255));

	m_nBegSelCol = m_nEndSelCol = m_nBegSelRow = m_nEndSelRow = -1;
	m_nOneCharDx = 5;
	m_bInMouseSel = false;
	m_nScrollPosx = 0;

	m_bMbcsA = false;
	m_bWin9x = GetOperationSystemType() <= OPS_WIN9XMORE;

	m_pEditSyntaxParser = NULL;
	m_pEditNotification = NULL;

	m_undoMgr.SetNotification(this);

	m_bPrevSelectedStatus = false;

	// At least one line is in editor
	m_vLines.push_back(NewOneLine());

	m_nCaretRow = 0;
	m_nCaretCol = 0;

	m_nIDTimerCaret = 0;
	m_cursor.LoadStdCursor(CCursor::C_IBEAM);

	m_nMsgWant = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON | UO_MSG_WANT_RBUTTON | UO_MSG_WANT_KEY | UO_MSG_WANT_MOUSEWHEEL;

	m_xMargin = m_yMargin = 0;
}

CSkinEditCtrl::~CSkinEditCtrl()
{
	for (int i = 0; i < (int)m_vLines.size(); i++)
		delete m_vLines[i];
	m_vLines.clear();
}

bool CSkinEditCtrl::CGlyph::iCmp(const TCHAR *str)
{
	return _tcsnicmp(str, chGlyph, chGlyph.size()) == 0;
}

bool CSkinEditCtrl::CGlyph::ReverseiCmp(const TCHAR *str)
{
	return _tcsnicmp(str - chGlyph.size() + 1, chGlyph, chGlyph.size()) == 0;
}

void CSkinEditCtrl::OnFontChanged()
{
	CRawGraph			*pGraph;
	COneLine			*pLine = NULL;
	SIZE				size;

	pGraph = GetMemGraphics();

	m_nOneCharDx = 0;
	for (TCHAR ch = 'A'; ch <= 'Z'; ch++)
	{
		pGraph->GetTextExtentPoint32(&ch, 1, &size);
		m_nOneCharDx += size.cx;
	}
	m_nOneCharDx /= 'Z' - 'A';

	for (int i = 0; i < (int)m_vLines.size(); i++)
	{
		COneLine		*pLine = m_vLines[i];

		UpdateWidthInfoOfLine(pGraph, pLine);
	}

	UpdateScrollInfo();
}

void CSkinEditCtrl::ReplaceSel(LPCTSTR szText)
{
	if (!IsSelected())
		return;

	CAutoUpdate		updater(this);

	int			nBegSelRow, nBegSelCol;
	tstring		strToDel;

	SelectionToText(strToDel);
	RemoveSelected(&nBegSelRow, &nBegSelCol);
	m_undoMgr.AddAction(new CEditRepAction(this, nBegSelRow, nBegSelCol, strToDel.c_str(), szText));
	InsertStr(nBegSelRow, nBegSelCol, szText, m_nCaretRow, m_nCaretCol);

	if (!IsInBatchAction())
	{
		UpdateScrollInfo();
		MakeCaretInSight();
	}
}

void CSkinEditCtrl::RemoveSel()
{
	if (!IsSelected())
		return;

	CAutoUpdate		updater(this);

	int			nBegSelRow, nBegSelCol;
	tstring		strToDel;

	SelectionToText(strToDel);
	RemoveSelected(&nBegSelRow, &nBegSelCol);
	m_undoMgr.AddAction(new CEditDelAction(this, nBegSelRow, nBegSelCol, strToDel.c_str()));

	if (!IsInBatchAction())
	{
		UpdateScrollInfo();
		MakeCaretInSight();
	}
}

void CSkinEditCtrl::SetSel(int nBeg, int nEnd)
{
	int		n = 0;

	m_nBegSelRow = -1;
	m_nEndSelRow = -1;

	if (nBeg == -1)
	{
		OnNotifySelChanged();
		return;
	}

	for (int i = 0; i < (int)m_vLines.size(); i++)
	{
		COneLine		*pLine = m_vLines[i];

		for (int k = 0; k < (int)pLine->size(); k++)
		{
			if (n == nBeg)
			{
				m_nBegSelRow = i;
				m_nBegSelCol = k;
			}
			else if (n = nEnd)
			{
				m_nEndSelRow = i;
				m_nEndSelCol = k;
			}
			n++;
		}
		if (pLine->newLineType == NLT_RN)
			n += 2;
		else if (pLine->newLineType == NLT_N)
			n++;
		else if (pLine->newLineType == NLT_R)
			n++;
		if (n >= nBeg && m_nBegSelRow == -1)
		{
			m_nBegSelRow = i;
			m_nBegSelCol = pLine->size();
		}
		else if (n >= nEnd && m_nEndSelRow == -1)
		{
			m_nEndSelRow = i;
			m_nEndSelCol = pLine->size();
		}
	}

	OnNotifySelChanged();
}

void CSkinEditCtrl::SetSel(int nBegLine, int nBegCol, int nEndLine, int nEndCol)
{
	if ((nBegLine >= 0 && nBegLine < (int)m_vLines.size()) && 
		(nBegCol >= 0 && nBegCol <= (int)m_vLines[nBegLine]->size()) &&
		(nEndLine >= 0 && nEndLine < (int)m_vLines.size()) && 
		(nEndCol >= 0 && nEndCol <= (int)m_vLines[nEndLine]->size()))
	{
		m_nBegSelRow = nBegLine;
		m_nBegSelCol = nBegCol;
		m_nEndSelRow = nEndLine;
		m_nEndSelCol = nEndCol;

		OnNotifySelChanged();
	}
}

void CSkinEditCtrl::SetSelOfLine(int nLine, int nCol, int nNumOfSel)
{
	if (nLine >= 0 && nLine < (int)m_vLines.size())
	{
		if (nCol >= 0 && nCol <= (int)m_vLines[nLine]->size())
		{
			m_nBegSelRow = nLine;
			m_nBegSelCol = nCol;

			m_nEndSelRow = nLine;
			ASSERT(nNumOfSel + nCol <= (int)m_vLines[nLine]->size());
			if (nNumOfSel + nCol <= (int)m_vLines[nLine]->size())
				m_nEndSelCol = nCol + nNumOfSel;
			else
				m_nEndSelCol = m_vLines[nLine]->size();

			OnNotifySelChanged();
			return;
		}
	}
}

void CSkinEditCtrl::SetCaret(int nRow, int nCol)
{
	if (nRow == -1 || nRow >= (int)m_vLines.size())
	{
		m_nCaretRow = m_vLines.size() - 1;
		m_nCaretCol = GetEndColPosOfLine(m_nCaretRow);
		return;
	}

	m_nCaretRow = nRow;
	if (nCol > GetEndColPosOfLine(m_nCaretRow))
		m_nCaretCol = GetEndColPosOfLine(m_nCaretRow);
	else
		m_nCaretCol = nCol;

	MakeCaretInSight();
	Invalidate();
}

void CSkinEditCtrl::SetText(LPCTSTR szText)
{
	m_undoMgr.Clear();
	DoSetText(szText);
	UpdateScrollInfo();
}

int CSkinEditCtrl::GetText(tstring &str)
{
	for (int i = 0; i < (int)m_vLines.size(); i++)
	{
		COneLine		*pLine = m_vLines[i];
		for (int k = 0; k < (int)pLine->size(); k++)
		{
			CGlyph	&glyph = (*pLine)[k];
			str += glyph.chGlyph;
		}
		if (pLine->newLineType == NLT_RN)
			str += _T("\r\n");
		else if (pLine->newLineType == NLT_N)
			str += _T("\n");
		else if (pLine->newLineType == NLT_R)
			str += _T("\r");
	}

	return str.size();
}

void CSkinEditCtrl::SelectionToText(tstring &str)
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
		return;
	}

	// to text
	for (int i = nBegSelRow; i <= nEndSelRow; i++)
	{
		COneLine		*pLine = m_vLines[i];

		for (int k = 0; k < (int)pLine->size(); k++)
		{
			CGlyph	&glyph = (*pLine)[k];

			bool		bSel = true;
			if (i == nBegSelRow)
			{
				if (k < nBegSelCol)
				{
					// don't fill
					bSel = false;
				}
			}
			if (i == nEndSelRow)
			{
				if (k >= nEndSelCol)
				{
					// don't fill
					bSel = false;
				}
			}
			if (bSel)
			{
				str += glyph.chGlyph;
			}
		}
		if (i < nEndSelRow)
			str += pLine->GetReturnStr(pLine->newLineType);
	}
}

bool CSkinEditCtrl::GetTextOfLine(int nLine, tstring &strLine)
{
	strLine = _T("");
	if (nLine < 0 || nLine >= (int)m_vLines.size())
		return false;

	COneLine		*pLine = m_vLines[nLine];

	for (int k = 0; k < (int)pLine->size(); k++)
	{
		CGlyph	&glyph = (*pLine)[k];

		strLine += glyph.chGlyph;
	}

	return true;
}

bool CSkinEditCtrl::SetTextOfLine(int nLine, LPCTSTR szLine)
{
	if (nLine >= 0 && nLine < (int)m_vLines.size())
	{
		m_nBegSelRow = nLine;
		m_nBegSelCol = 0;
		m_nEndSelRow = nLine;
		m_nEndSelCol = m_vLines[nLine]->size();

		ASSERT(_tcschr(szLine, '\n') == NULL);
		ReplaceSel(szLine);

		return true;
	}
	else
		return false;
}

int CSkinEditCtrl::GetLength()
{
	int		n = 0;

	for (int i = 0; i < (int)m_vLines.size(); i++)
	{
		COneLine		*pLine = m_vLines[i];

		for (int k = 0; k < (int)pLine->size(); k++)
			n++;
		if (pLine->newLineType == NLT_RN)
			n += 2;
		else if (pLine->newLineType == NLT_N)
			n++;
		else if (pLine->newLineType == NLT_R)
			n++;
	}
	return n;
}

bool CSkinEditCtrl::CanUndo()
{
	return m_undoMgr.CanUndo();
}

bool CSkinEditCtrl::CanRedo()
{
	return m_undoMgr.CanRedo();
}

void CSkinEditCtrl::OnCmdCopy()
{
	if (IsSelected())
	{
		tstring		str;
		SelectionToText(str);
		if (str.size())
			CopyTextToClipboard(str.c_str());
	}
}

// cut selected
void CSkinEditCtrl::OnCmdCut()
{
	if (IsReadOnly())
		return;

	if (!IsSelected())
		return;

	CAutoUpdate		updater(this);
	tstring			strToDel;
	int				nBegSelRow, nBegSelCol;

	SelectionToText(strToDel);

	CopyTextToClipboard(strToDel.c_str());

	RemoveSelected(&nBegSelRow, &nBegSelCol);
	m_undoMgr.AddAction(new CEditDelAction(this, nBegSelRow, nBegSelCol, strToDel.c_str()));

	MakeCaretInSight();
}

void CSkinEditCtrl::OnCmdPaste()
{
	if (IsReadOnly())
		return;

	CAutoUpdate		updater(this);
	tstringex		str;

	GetClipBoardText(str);
	if (str.size())
	{
		if (IsSelected())
		{
			tstring			strToDel;
			int				nBegSelRow, nBegSelCol;

			SelectionToText(strToDel);
			RemoveSelected(&nBegSelRow, &nBegSelCol);
			m_undoMgr.AddAction(new CEditRepAction(this, nBegSelRow, nBegSelCol, strToDel.c_str(), str.c_str()));
			InsertStr(nBegSelRow, nBegSelCol, str.c_str(), m_nCaretRow, m_nCaretCol);
		}
		else
		{
			m_undoMgr.AddAction(new CEditInsertAction(this, m_nCaretRow, m_nCaretCol, str.c_str()));
			InsertStr(m_nCaretRow, m_nCaretCol, str.c_str(), m_nCaretRow, m_nCaretCol);
		}

		updater.SetUpdateFlag(true);
		UpdateScrollInfo();
	}

	MakeCaretInSight();
}

void CSkinEditCtrl::OnCmdUndo()
{
	if (IsReadOnly())
		return;

	CAutoUpdate		updater(this);

	if (m_undoMgr.CanUndo())
	{
		m_undoMgr.Undo();
		updater.SetUpdateFlag(true);
	}

	MakeCaretInSight();
}

void CSkinEditCtrl::OnCmdRedo()
{
	if (IsReadOnly())
		return;

	CAutoUpdate		updater(this);

	if (m_undoMgr.CanRedo())
	{
		m_undoMgr.Redo();
		updater.SetUpdateFlag(true);
	}

	MakeCaretInSight();
}

void CSkinEditCtrl::OnCmdDelete()
{
	if (IsReadOnly())
		return;

	CAutoUpdate		updater(this);

	if (IsSelected())
	{
		int				nBegSelRow, nBegSelCol;
		tstring			strToDel;

		SelectionToText(strToDel);
		RemoveSelected(&nBegSelRow, &nBegSelCol);
		m_undoMgr.AddAction(new CEditDelAction(this, nBegSelRow, nBegSelCol, strToDel.c_str()));
	}
	else
	{
		tstring		strToDel;
		GetStrAtPos(m_nCaretRow, m_nCaretCol, strToDel);
		m_undoMgr.AddAction(new CEditDelAction(this, m_nCaretRow, m_nCaretCol, strToDel.c_str()));
		RemoveChar(m_nCaretRow, m_nCaretCol);
	}

	updater.SetUpdateFlag(true);
	UpdateScrollInfo();

	MakeCaretInSight();
}

void CSkinEditCtrl::GetStrAtPos(int nRow, int nCol, tstring &str)
{
	str.resize(0);

	ASSERT(IsPosValid(nRow, nCol));
	if (!IsPosValid(nRow, nCol))
		return;

	COneLine		*pLine = m_vLines[nRow];

	if (nCol == pLine->size())
	{
		// remove the "\n".
		if (nRow == m_vLines.size() - 1)
			return;

		str = pLine->GetReturnStr(pLine->newLineType);
	}
	else
	{
		str = pLine->at(nCol).chGlyph;
	}
}

bool CSkinEditCtrl::IsPosValid(int nRow, int nCol)
{
	if (nRow >= 0 && nRow < (int)m_vLines.size())
	{
		COneLine		*pLine = m_vLines[nRow];

		return (nCol >= 0 && nCol <= (int)pLine->size());
	}

	return false;
}

void CSkinEditCtrl::SortSelectPos(int &nBegSelRow, int &nBegSelCol, int &nEndSelRow, int &nEndSelCol, bool &bSelectedEmpty)
{
	ASSERT(m_nBegSelRow != -1);
	bSelectedEmpty = false;
	if (m_nBegSelRow <= m_nEndSelRow)
	{
		nBegSelRow = m_nBegSelRow;
		nEndSelRow = m_nEndSelRow;
		if (m_nBegSelRow == m_nEndSelRow && m_nEndSelCol == m_nBegSelCol)
			bSelectedEmpty = true;
		if (m_nBegSelRow == m_nEndSelRow && m_nEndSelCol < m_nBegSelCol)
		{
			nBegSelCol = m_nEndSelCol;
			nEndSelCol = m_nBegSelCol;
		}
		else
		{
			nBegSelCol = m_nBegSelCol;
			nEndSelCol = m_nEndSelCol;
		}
	}
	else
	{
		nBegSelRow = m_nEndSelRow;
		nEndSelRow = m_nBegSelRow;
		nBegSelCol = m_nEndSelCol;
		nEndSelCol = m_nBegSelCol;
	}
}

void CSkinEditCtrl::OnCreate()
{
	if (IsFlagSet(m_nEditorStyles, S_MULTILINE))
		m_bHorzScrollBar = m_bVertScrollBar = true;

	CSkinScrollFrameCtrlBase::OnCreate();

	m_nCaretX = m_xMargin;
	m_nCaretY = m_yMargin;

	m_font.OnCreate(m_pSkin);
}

void CSkinEditCtrl::FillSelectedLineBg(CRawGraph *pGraph, int x, int y, COneLine *pLine, int nBegSelCol, int nEndSelCol)
{
	CRect	rc;

	ASSERT(nBegSelCol <= (int)pLine->size() && nEndSelCol <= (int)pLine->size());

	for (int k = 0; k < nBegSelCol; k++)
	{
		CGlyph	&glyph = (*pLine)[k];
		x += glyph.width;
	}
	rc.left = x;
	rc.top = y;
	rc.right = rc.left;
	rc.bottom = rc.top + GetLineDy();

	for (int k = nBegSelCol; k < nEndSelCol; k++)
	{
		CGlyph	&glyph = (*pLine)[k];
		rc.right += glyph.width;
	}

	FillGraph(pGraph, &rc, CN_SEL_BG);
}

void CSkinEditCtrl::Draw(CRawGraph *pGraph)
{
	CSkinScrollFrameCtrlBase::Draw(pGraph);

	pGraph->SetFont(m_font.GetFont());

	FillGraph(pGraph, &m_rcContent, CN_BG);

	CRect	rc = m_rcContent;
	rc.DeflateRect(m_xMargin, m_yMargin);

	int		nBegSelRow, nEndSelRow;
	int		nBegSelCol, nEndSelCol;
	bool	bSelectedEmpty = true;
	CRect	rcClip;

	if (m_nBegSelRow != -1)
	{
		bSelectedEmpty = false;
		SortSelectPos(nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol, bSelectedEmpty);
	}

	if (m_vLines.empty())
	{
		m_caret.Draw(pGraph);
		return;
	}

	rcClip = rc;

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraph);
	pGraph->SetClipBoundBox(rcClip);

	int		x, y, xStart, xMax, yMax;
	int		i;

	xStart = rcClip.left;
	y = rcClip.top;
	xMax = rcClip.right;
	yMax = rcClip.bottom;
	ASSERT(m_nTopVisibleLine >= 0 && m_nTopVisibleLine < (int)m_vLines.size());

	if (!IsFlagSet(m_nEditorStyles, S_MULTILINE))
		y = rc.top + (rc.Height() - GetFontHeight()) / 2;

	if (bSelectedEmpty)
	{
		// If no lines are selected, draw same
		for (int i = m_nTopVisibleLine; i < (int)m_vLines.size(); i++)
		{
			if (y + GetLineDy() > yMax && i > m_nTopVisibleLine)
				break;

			DrawLine(pGraph, m_vLines[i], xStart, xMax, y);

			y += GetLineDy();
		}
		m_caret.Draw(pGraph);
		return;
	}

	// draw lines before selection
	for (i = m_nTopVisibleLine; i < nBegSelRow; i++)
	{
		if (y + GetLineDy() > yMax && i > m_nTopVisibleLine)
			break;

		DrawLine(pGraph, m_vLines[i], xStart, xMax, y);

		y += GetLineDy();
	}

	// draw lines at selection
	for (i = nBegSelRow; i <= nEndSelRow; i++)
	{
		if (i < m_nTopVisibleLine)
			continue;
		if (y + GetLineDy() > yMax && i > m_nTopVisibleLine)
			break;

		COneLine	*pLine = m_vLines[i];
		int			nBegSelColCurLine, nEndSelColCurLine;

		if (i == nBegSelRow)
			nBegSelColCurLine = nBegSelCol;
		else
			nBegSelColCurLine = 0;

		if (i == nEndSelRow)
			nEndSelColCurLine = nEndSelCol;
		else
			nEndSelColCurLine = pLine->size();

		x = xStart - m_nScrollPosx;

		FillSelectedLineBg(pGraph, x, y, pLine, nBegSelColCurLine, nEndSelColCurLine);

		for (int k = 0; k < (int)pLine->size(); k++)
		{
			CGlyph	&glyph = (*pLine)[k];
			ASSERT(glyph.width != GLYPH_WIDTH_INVALID);
			if (x + glyph.width < m_xMargin)
			{
				x += glyph.width;
				continue;
			}
			if (x + glyph.width > xMax)
				break;

			if (k >= nBegSelColCurLine && k < nEndSelColCurLine)
			{
				CRect		rcLine;
				rcLine.SetRect(x, y, x + glyph.width, y + GetLineDy());
				pGraph->SetTextColor(GetColor(CN_SEL_TEXT));
			}
			else
			{
				if (glyph.clrIndex >= m_vClrTable.size())
					pGraph->SetTextColor(GetColor(CN_TEXT));
				else
					pGraph->SetTextColor(m_vClrTable[glyph.clrIndex]);
			}
			if (glyph.chGlyph != '\t')
			{
				if (IsPassword())
					pGraph->TextOut(x, y, PASSWORD_CHAR, 1);
				else
					pGraph->TextOut(x, y, glyph.chGlyph, glyph.chGlyph.size());
			}
			x += glyph.width;
		}

		y += GetLineDy();
	}

//	pGraph->SetTextColor(m_clrText);
	for (i = nEndSelRow + 1; i < (int)m_vLines.size(); i++)
	{
		if (i < m_nTopVisibleLine)
			continue;
		if (y + GetLineDy() > yMax && i > m_nTopVisibleLine)
			break;

		DrawLine(pGraph, m_vLines[i], xStart, xMax, y);

		y += GetLineDy();
	}

	m_caret.Draw(pGraph);
}

MLColor & CSkinEditCtrl::GetColor(int nColorName)
{
	ASSERT(m_vClrTable.size() >= CN_CUSTOMIZED_START);
	/*if (nColorName == CN_BG && m_bUseParentBg)
		return m_pSkin->GetBgColor();
	else */
	if (nColorName < 0 || nColorName >= (int)m_vClrTable.size())
		return m_vClrTable[CN_TEXT];
	else
		return m_vClrTable[nColorName];
}

void CSkinEditCtrl::SetColor(int nColorName, const MLColor & clr)
{
	ASSERT(m_vClrTable.size() >= CN_CUSTOMIZED_START);
	if (nColorName >= (int)m_vClrTable.size())
	{
		if (nColorName > CN_MAX)
			return;
		m_vClrTable.resize(nColorName + 1);
	}

	m_vClrTable[nColorName] = clr;
}

byte CSkinEditCtrl::GetBgAlpha()
{
#ifdef _WIN32_DESKTOP
	if (m_bTranslucencyWithSkin)
		return int(m_pSkin->m_nCurTranslucencyAlpha);
	else
#endif
		return 255;
}

void CSkinEditCtrl::UpdateRectToScreen(const CRect *rc)
{
	if (!IsVisible() || !IsParentVisible())
		return;

	m_pContainer->UpdateMemGraphicsToScreen(rc, this);
}

void CSkinEditCtrl::FillGraph(CRawGraph *pGraph, const CRect *rc, int nColorName)
{
	int			alpha = GetBgAlpha();
	MLColor		clr = GetColor(nColorName);
	clr.setAlpha(alpha);

	pGraph->FillRect(rc, clr);
}

void CSkinEditCtrl::DrawLine(CRawGraph *pGraph, COneLine *pLine, int x, int xMax, int y)
{
	BYTE	byClrLast = 0;

	x -= m_nScrollPosx;

	pGraph->SetTextColor(GetColor(CN_TEXT));
	for (int k = 0; k < (int)pLine->size(); k++)
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
			break;

		if (glyph.chGlyph != '\t')
		{
			if (IsPassword())
				pGraph->TextOut(x, y, PASSWORD_CHAR, 1);
			else
				pGraph->TextOut(x, y, glyph.chGlyph, glyph.chGlyph.size());
		}
		x += glyph.width;
	}
}

void CSkinEditCtrl::UpdateWidthInfoOfLine(CRawGraph *pGraph, COneLine *pLine)
{
	int		x = 0;
	SIZE	size;
	for (int k = 0; k < (int)pLine->size(); k++)
	{
		CGlyph	&glyph = (*pLine)[k];
		if (glyph.chGlyph == '\t')
		{
			// tab
			glyph.width = m_nOneCharDx * 4 - x % (m_nOneCharDx * 4);
		}
		else
		{
			if (IsPassword())
				pGraph->GetTextExtentPoint32(PASSWORD_CHAR, 1, &size);
			else
				pGraph->GetTextExtentPoint32(glyph.chGlyph, glyph.chGlyph.size(), &size);
			glyph.width = (BYTE)size.cx;
		}
		x += glyph.width;
	}
}

bool CSkinEditCtrl::DoSetText(LPCTSTR szText)
{
	COneLine			*pLine = NULL;

	// delete old text
	Clear();

	SelectNone();

	m_nCaretRow = 0;
	m_nCaretCol = 0;

	pLine = m_vLines[0];
	while (*szText)
	{
		CGlyph		glyph;

		glyph.chGlyph = szText;
		glyph.clrIndex = CN_TEXT;
		if (*szText == '\r')
		{
// 			glyph.width = 0;
// 			pLine->push_back(glyph);
			szText++;
			if (*szText == '\n')
			{
				pLine->newLineType = NLT_RN;
				OnNotifyParseLine(m_vLines.size() - 1);
				pLine = NewOneLine();
				m_vLines.push_back(pLine);
			}
			else
			{
				pLine->newLineType = NLT_R;
				OnNotifyParseLine(m_vLines.size() - 1);
				pLine = NewOneLine();
				m_vLines.push_back(pLine);
			}
		}
		else if (*szText == '\n')
		{
			pLine->newLineType = NLT_N;
			OnNotifyParseLine(m_vLines.size() - 1);
			pLine = NewOneLine();
			m_vLines.push_back(pLine);
		}
		else
		{
			glyph.width = GLYPH_WIDTH_INVALID;
			pLine->push_back(glyph);
		}

		ASSERT(glyph.chGlyph.size() > 0);
		szText += glyph.chGlyph.size();
	}

	OnNotifyParseLine(m_vLines.size() - 1);


	//
	// Update line width
	//
	CRawGraph			*pGraph = NULL;

	pGraph = GetMemGraphics();

	for (int i = 0; i < (int)m_vLines.size(); i++)
	{
		UpdateWidthInfoOfLine(pGraph, m_vLines[i]);
	}

	return true;
}

void CSkinEditCtrl::Clear()
{
	m_nCaretMaxXLatest = m_nCaretX = m_xMargin;
	m_nCaretY = m_yMargin;
	m_nTopVisibleLine = 0;
	m_nBegSelCol = m_nEndSelCol = m_nBegSelRow = m_nEndSelRow = -1;
	m_nScrollPosx = 0;

	m_nCaretRow = m_nCaretCol = 0;

	for (int i = 0; i < (int)m_vLines.size(); i++)
	{
		delete m_vLines[i];
	}
	m_vLines.clear();

	m_vLines.push_back(NewOneLine());
}

void CSkinEditCtrl::ClearWidth()
{
	for (int i = 0; i < (int)m_vLines.size(); i++)
	{
		COneLine		*pLine = m_vLines[i];

		for (int k = 0; k < (int)pLine->size(); k++)
		{
			CGlyph	&glyph = (*pLine)[k];
			if (glyph.width != 0)
			{
				glyph.width = GLYPH_WIDTH_INVALID;
			}
		}
	}
}

void CSkinEditCtrl::ShowCaret(int x, int y, int nWidth, int nHeight)
{
	CRawGraph		*pGraph;

	pGraph = m_pContainer->GetMemGraph();
	ASSERT(pGraph);

	if (!m_caret.IsOn())
		m_nIDTimerCaret = m_pSkin->RegisterTimerObject(this, TIMER_DURATION_CARET);

	if (m_caret.IsOn() && m_caret.IsDrawed())
	{
		m_caret.DrawFlash(pGraph);
		UpdateRectToScreen(&m_caret.GetUpdateRect());
	}

	if (m_pSkin->GetFocusUIObj() == (CUIObject *)this)
		m_caret.ShowCaret(x, y, nWidth, nHeight);
}

void CSkinEditCtrl::HideCaret()
{
	if (m_nIDTimerCaret)
	{
		m_pSkin->UnregisterTimerObject(this, m_nIDTimerCaret);
		m_nIDTimerCaret = 0;
	}
	m_caret.HideCaret();
}

void CSkinEditCtrl::ShowCaret()
{
	if (IsFlagSet(m_nEditorStyles, S_MULTILINE))
		m_nCaretY = m_yMargin + (m_nCaretRow - m_nTopVisibleLine) * GetLineDy();
	else
	{
		m_nCaretY = (m_rcContent.Height() - GetFontHeight()) / 2;
	}
	ShowCaret(m_nCaretX, m_nCaretY, 2, GetFontHeight());
}

void CSkinEditCtrl::MakeCaretInSight()
{
	CRect		rc = m_rcContent;

	if (m_nCaretRow < 0 || m_nCaretRow >= (int)m_vLines.size())
	{
		m_nCaretRow = 0;
		m_nCaretCol = 0;
	}

	// rc.DeflateRect(m_xMargin, m_yMargin);
// 	if (m_nCaretRow < m_nTopVisibleLine)
// 		m_nTopVisibleLine = m_nCaretRow;
// 	else
// 	{
// 		m_nCaretY = m_yMargin + (m_nCaretRow - m_nTopVisibleLine) * GetLineDy();
// 		if (m_nCaretY >= rc.bottom - m_yMargin)
// 		{
// 			m_nTopVisibleLine = m_nCaretRow;
// 		}
// 	}

	if (m_pVertScrollBar && m_pVertScrollBar->IsEnabled())
	{
		if (m_nCaretRow - m_nTopVisibleLine == m_pVertScrollBar->GetPage())
			m_nTopVisibleLine++;
		else if (m_nTopVisibleLine - m_nCaretRow == 1)
			m_nTopVisibleLine--;
		else if (m_nCaretRow - m_nTopVisibleLine > m_pVertScrollBar->GetPage())
		{
			m_nTopVisibleLine += (m_nCaretRow - m_nTopVisibleLine) - (m_nCaretRow - m_nTopVisibleLine) % m_pVertScrollBar->GetPage();
		}
		else if (m_nCaretRow < m_nTopVisibleLine)
		{
			m_nTopVisibleLine -= (m_nTopVisibleLine - m_nCaretRow) - (m_nTopVisibleLine - m_nCaretRow) % m_pVertScrollBar->GetPage();
			if (m_nCaretRow < m_nTopVisibleLine)
				m_nTopVisibleLine -= m_pVertScrollBar->GetPage();
		}

		if (m_nTopVisibleLine > m_pVertScrollBar->GetMax())
			m_nTopVisibleLine = m_pVertScrollBar->GetMax();
	}
	else
		m_nTopVisibleLine = 0;

	if (m_nTopVisibleLine < 0)
		m_nTopVisibleLine = 0;

	m_nCaretY = m_yMargin + (m_nCaretRow - m_nTopVisibleLine) * GetLineDy();

	if (m_vLines.empty())
		m_nCaretX = m_xMargin;
	else
	{
		COneLine		*pLine;
		int				nNextCaretCharX;

		pLine = m_vLines[m_nCaretRow];
		nNextCaretCharX = m_nCaretX = m_xMargin;
		ASSERT(m_nCaretCol <= (int)pLine->size());
		for (int i = 0; i < (int)pLine->size() && i <= m_nCaretCol; i++)
		{
			CGlyph	&glyph = (*pLine)[i];

			ASSERT(glyph.width != GLYPH_WIDTH_INVALID);
			m_nCaretX = nNextCaretCharX;
			nNextCaretCharX += glyph.width;
		}
		if (m_nCaretCol == pLine->size())
			m_nCaretX = nNextCaretCharX;

 		if (nNextCaretCharX - m_nScrollPosx >= rc.right - m_xMargin)
 		{
 			// horz scroll +
 			m_nScrollPosx = nNextCaretCharX - (rc.right - m_xMargin - 16 * m_nOneCharDx);
			if (nNextCaretCharX - m_nScrollPosx <= 0)
				m_nScrollPosx = nNextCaretCharX - rc.Width() / 2 - m_xMargin;
			if (m_pHorzScrollBar && m_nScrollPosx > m_pHorzScrollBar->GetMax())
				m_nScrollPosx = m_pHorzScrollBar->GetMax();
 		}
 		if (nNextCaretCharX < m_nScrollPosx + m_xMargin)
 		{
 			// horz scroll -
 			m_nScrollPosx = nNextCaretCharX - m_xMargin - 16 * m_nOneCharDx;
 			if (m_nScrollPosx < 0)
 				m_nScrollPosx = 0;
 		}
	}

 	m_nCaretX -= m_nScrollPosx;
	m_nCaretMaxXLatest = m_nCaretX;

	ShowCaret();

	if (m_pVertScrollBar)
		m_pVertScrollBar->SetScrollPos(m_nTopVisibleLine);
	if (m_pHorzScrollBar)
		m_pHorzScrollBar->SetScrollPos(m_nScrollPosx);
}

void CSkinEditCtrl::SetCaretToPos(int x, int y)
{
	COneLine		*pLine;
	int				i;

	x -= m_rcContent.left;
	y -= m_rcContent.top;

	m_nCaretRow = m_nTopVisibleLine + (y - m_yMargin)/ GetLineDy();
	ASSERT(!m_vLines.empty());
	if (m_nCaretRow >= (int)m_vLines.size())
		m_nCaretRow = m_vLines.size() - 1;
	else if (m_nCaretRow < 0)
		m_nCaretRow = 0;

	if (m_pVertScrollBar && m_pVertScrollBar->IsEnabled() && m_pVertScrollBar->GetPage() <= m_pVertScrollBar->GetMax())
	{
		if (m_nCaretRow - m_nTopVisibleLine == m_pVertScrollBar->GetPage())
			m_nTopVisibleLine++;
		else if (m_nTopVisibleLine - m_nCaretRow == 1)
			m_nTopVisibleLine--;
		else if (m_nCaretRow - m_nTopVisibleLine > m_pVertScrollBar->GetPage())
		{
			m_nTopVisibleLine += (m_nCaretRow - m_nTopVisibleLine) - (m_nCaretRow - m_nTopVisibleLine) % m_pVertScrollBar->GetPage();
		}
		else if (m_nCaretRow < m_nTopVisibleLine)
		{
			m_nTopVisibleLine += (m_nCaretRow - m_nTopVisibleLine) - (m_nCaretRow - m_nTopVisibleLine) % m_pVertScrollBar->GetPage();
		}
	}

	if (m_nTopVisibleLine < 0)
		m_nTopVisibleLine = 0;

	m_nCaretY = m_yMargin + (m_nCaretRow - m_nTopVisibleLine) * GetLineDy();

	x += m_nScrollPosx;

	pLine = m_vLines[m_nCaretRow];
	m_nCaretX = m_xMargin;
	for (i = 0; i < (int)pLine->size(); i++)
	{
		CGlyph	&glyph = (*pLine)[i];

		ASSERT(glyph.width != GLYPH_WIDTH_INVALID);
		if (x <= m_nCaretX + glyph.width / 2)
			break;
		m_nCaretX += glyph.width;
	}
	m_nCaretMaxXLatest = m_nCaretX;
	m_nCaretCol = i;
}

int CSkinEditCtrl::GetEndColPosOfLine(int nLine)
{
	ASSERT(nLine >=0 && nLine < (int)m_vLines.size());

	COneLine		*pLine;

	pLine = m_vLines[nLine];
	return pLine->size();
}

void CSkinEditCtrl::NextPos(int &Row, int &nCol)
{
	if (nCol >= GetEndColPosOfLine(Row))
	{
		if (Row < (int)m_vLines.size() - 1)
		{
			Row++;
			nCol = 0;
		}
		else
		{
			nCol = GetEndColPosOfLine(Row);
		}
	}
	else
	{
		nCol++;
	}
}

void CSkinEditCtrl::PrevPos(int &Row, int &nCol)
{
	if (nCol > 0)
		nCol--;
	else
	{
		if (Row > 0)
		{
			Row--;
			nCol = GetEndColPosOfLine(Row);
		}
	}
}

void CSkinEditCtrl::NextWord(int &Row, int &nCol)
{
	bool			bAlpha, bBegAlpha;
	COneLine		*pLine;
	bool			bMoved = false;

	pLine = m_vLines[Row];

	if (nCol == pLine->size())
		bBegAlpha = false;
	else
		bBegAlpha = Tobool(_istalnum((*pLine)[nCol].chGlyph[0])) || (*pLine)[nCol].chGlyph == '_';

	while (1)
	{
		pLine = m_vLines[Row];

		if (nCol == pLine->size())
			bAlpha = false;
		else
			bAlpha = Tobool(_istalnum((*pLine)[nCol].chGlyph[0])) || (*pLine)[nCol].chGlyph == '_';

		if (bAlpha != bBegAlpha)
		{
			if (bBegAlpha)
			{
				// the space followed by text, should belong to that word.
				while (nCol < (int)pLine->size() && (*pLine)[nCol].chGlyph == ' ')
					nCol++;
			}
			break;
		}

		if (nCol >= (int)pLine->size())
		{
			if (bMoved)
				break; // End at the end of line.

			if (Row < (int)m_vLines.size() - 1)
			{
				// End at next line
				Row++;
				nCol = 0;
				return;
			}
			else
			{
				// end of text.
				return;
			}
		}
		else
		{
			nCol++;
			bMoved = true;
		}
	}
}

void CSkinEditCtrl::PrevWord(int &Row, int &nCol)
{
	bool			bAlpha, bBegAlpha;
	COneLine		*pLine;
	bool			bBegSpace;

	PrevPos(Row, nCol);

	pLine = m_vLines[Row];

	if (nCol == pLine->size())
	{
		bBegAlpha = false;
		bBegSpace = true;
	}
	else
	{
		bBegAlpha = Tobool(_istalnum((*pLine)[nCol].chGlyph[0])) || (*pLine)[nCol].chGlyph == '_';
		bBegSpace = (*pLine)[nCol].chGlyph == ' ';
	}

	// word ends with space is a whole word.
	while (bBegSpace && nCol > 0)
	{
		nCol--;
		if ((*pLine)[nCol].chGlyph[0] == ' ')
			continue;
		else if (Tobool(_istalnum((*pLine)[nCol].chGlyph[0])) || (*pLine)[nCol].chGlyph == '_')
		{
			bBegAlpha = true;
			break;
		}
		else
			break;
	}

	while (1)
	{
		pLine = m_vLines[Row];

		if (nCol == pLine->size())
			bAlpha = false;
		else
			bAlpha = Tobool(_istalnum((*pLine)[nCol].chGlyph[0])) || (*pLine)[nCol].chGlyph == '_';

		if (bAlpha != bBegAlpha)
		{
			NextWord(Row, nCol);
			break;
		}

		if (nCol > 0)
			nCol--;
		else
		{
			break;
// 			if (Row > 0)
// 			{
// 				Row--;
// 				nCol = m_vLines[Row]->size();
// 				break;
// 			}
// 			else
// 			{
// 				// begin of text
// 				return;
// 			}
		}
	}
}

bool CSkinEditCtrl::IsSelected()
{
	return m_nBegSelRow != -1;
}

void CSkinEditCtrl::SelectNone()
{
	m_nBegSelRow = -1;
	OnNotifySelChanged();
}

void CSkinEditCtrl::RemoveSelected(int *pnBegSelRow, int *pnBegSelCol)
{
	int		nBegSelRow, nEndSelRow;
	int		nBegSelCol, nEndSelCol;
	bool	bSelectedEmpty = true;

	if (m_nBegSelRow != -1)
	{
		bSelectedEmpty = false;
		SortSelectPos(nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol, bSelectedEmpty);
		if (pnBegSelRow)
			*pnBegSelRow = nBegSelRow;
		if (pnBegSelCol)
			*pnBegSelCol = nBegSelCol;
	}

	if (bSelectedEmpty)
	{
		SelectNone();
		return;
	}

	CRawGraph			*pGraph;
	// SIZE				size;

	pGraph = GetMemGraphics();


	// remove selection
	for (int i = nEndSelRow; i >= nBegSelRow; i--)
	{
		COneLine		*pLine = m_vLines[i];

		if (i == nBegSelRow)
		{
			if (i == nEndSelRow)
				pLine->erase(pLine->begin() + nBegSelCol, pLine->begin() + nEndSelCol);
			else
			{
				pLine->erase(pLine->begin() + nBegSelCol, pLine->end());

				COneLine	*pNextLine = m_vLines[i + 1];
				m_vLines.erase(m_vLines.begin() + i + 1);
				pLine->insert(pLine->end(), pNextLine->begin(), pNextLine->end());
				delete pNextLine;
			}
			UpdateWidthInfoOfLine(pGraph, pLine);
		}
		else if (i == nEndSelRow)
		{
			pLine->erase(pLine->begin(), pLine->begin() + nEndSelCol);
			UpdateWidthInfoOfLine(pGraph, pLine);
		}
		else
		{
			m_vLines.erase(m_vLines.begin() + i);
			delete pLine;
		}
	}

	OnNotifyParseLine(nBegSelRow);

	m_nCaretRow = nBegSelRow;
	m_nCaretCol = nBegSelCol;

	SelectNone();
}

void CSkinEditCtrl::RemoveStr(int nRow, int nCol, int nSize)
{
	ASSERT(nRow >= 0 && nRow < (int)m_vLines.size());

	COneLine			*pLine = NULL;
	CRawGraph			*pGraph;
	// SIZE				size;

	pGraph = GetMemGraphics();

	while (nRow < (int)m_vLines.size() && nSize > 0)
	{
		pLine = m_vLines[nRow];
		ASSERT(nCol >= 0 && nCol <= (int)pLine->size());

		if (nCol < (int)pLine->size())
		{
			if (nCol + nSize >= (int)pLine->size())
			{
				nSize -= pLine->size() - nCol;
				pLine->erase(pLine->begin() + nCol, pLine->end());
			}
			else
			{
				pLine->erase(pLine->begin() + nCol, pLine->begin() + (nCol + nSize));
				nSize = 0;
			}
		}
		if (nSize > 0 && nCol == pLine->size())
		{
			// remove the "\n".
			if (nRow == m_vLines.size() - 1)
				return;

			COneLine	*pNextLine = m_vLines[nRow + 1];

			// nSize -= pLine->GetReturnSize();
			nSize--;
			ASSERT(nSize >= 0);

			pLine->insert(pLine->end(), pNextLine->begin(), pNextLine->end());

			m_vLines.erase(m_vLines.begin() + nRow + 1);
			delete pNextLine;
		}
	}

	OnNotifyParseLine(nRow);

	if (pLine)
		UpdateWidthInfoOfLine(pGraph, pLine);

}

void CSkinEditCtrl::RemoveChar(int nRow, int nCol)
{
	ASSERT(nRow >= 0 && nRow < (int)m_vLines.size());

	COneLine		*pLine = m_vLines[nRow];
	ASSERT(nCol >= 0 && nCol <= (int)pLine->size());

	if (nCol == pLine->size())
	{
		// remove the "\n".
		if (nRow == m_vLines.size() - 1)
			return;

		COneLine	*pNextLine = m_vLines[nRow + 1];
		m_vLines.erase(m_vLines.begin() + nRow + 1);
		pLine->insert(pLine->end(), pNextLine->begin(), pNextLine->end());
		delete pNextLine;
	}
	else
	{
		pLine->erase(pLine->begin() + nCol);
	}

	OnNotifyParseLine(nRow);

	CRawGraph			*pGraph;
	// SIZE				size;

	pGraph = GetMemGraphics();

	UpdateWidthInfoOfLine(pGraph, pLine);
}

void CSkinEditCtrl::InsertChar(int nRow, int nCol, WCHAR chInsert)
{
	ASSERT(nRow >= 0 && nRow < (int)m_vLines.size());

	COneLine		*pLine = m_vLines[nRow];
	ASSERT(nCol >= 0 && nCol <= (int)pLine->size());

	CGlyph		glyph;

	CRawGraph			*pGraph;
	// SIZE				size;

	pGraph = GetMemGraphics();

	glyph.chGlyph = chInsert;
	glyph.clrIndex = CN_TEXT;

	if (nCol == pLine->size())
	{
		pLine->insert(pLine->end(), glyph);
	}
	else
		pLine->insert(pLine->begin() + nCol, glyph);

	OnNotifyParseLine(nRow);

	UpdateWidthInfoOfLine(pGraph, pLine);
}

void CSkinEditCtrl::InsertStr(int nRow, int nCol, LPCTSTR szText, int &nRowNext, int &nColNext)
{
	CRawGraph			*pGraph;
	tstring				strSingleLine;

	// Insert only first line, if it's single line
	if (IsSingleLine())
	{
		LPCTSTR		p = szText;
		while (*p && *p != '\r' && *p != '\n')
			p++;
		strSingleLine.append(szText, p);
		szText = strSingleLine.c_str();
	}

	pGraph = GetMemGraphics();

	ASSERT(nRow >= 0 && nRow < (int)m_vLines.size());
	COneLine		leftGlyphOfLine;
	COneLine		*pLine = m_vLines[nRow];
	ASSERT(nCol >= 0 && nCol <= (int)pLine->size());

	leftGlyphOfLine.insert(leftGlyphOfLine.begin(), pLine->begin() + nCol, pLine->end());
	pLine->erase(pLine->begin() + nCol, pLine->end());

	NewLineType		nltInsertLine = pLine->newLineType;

	nRowNext = nRow;
	while (*szText)
	{
		CGlyph		glyph;

		glyph.chGlyph = szText;
		glyph.clrIndex = CN_TEXT;
		if (*szText == '\r')
		{
// 			glyph.width = 0;
// 			pLine->push_back(glyph);
			szText++;
			UpdateWidthInfoOfLine(pGraph, pLine);
			if (*szText == '\n')
			{
				pLine->newLineType = NLT_RN;
			}
			else
			{
				pLine->newLineType = NLT_R;
			}
			OnNotifyParseLine(nRowNext);
			pLine = NewOneLine();
			nRowNext++;
			m_vLines.insert(m_vLines.begin() + nRowNext, pLine);
		}
		else if (*szText == '\n')
		{
			UpdateWidthInfoOfLine(pGraph, pLine);
			pLine->newLineType = NLT_N;
			OnNotifyParseLine(nRowNext);
			pLine = NewOneLine();
			nRowNext++;
			m_vLines.insert(m_vLines.begin() + nRowNext, pLine);
		}
		else
		{
			glyph.width = GLYPH_WIDTH_INVALID;
			pLine->push_back(glyph);
		}

		ASSERT(glyph.chGlyph.size() > 0);
		szText += glyph.chGlyph.size();
	}

	pLine->newLineType = nltInsertLine;

	nColNext = pLine->size();

	pLine->insert(pLine->end(), leftGlyphOfLine.begin(), leftGlyphOfLine.end());

	OnNotifyParseLine(nRowNext);

	UpdateWidthInfoOfLine(pGraph, pLine);
}

void CSkinEditCtrl::UpdateScrollInfo(bool bHorz, bool bVert)
{
	CRect			rc = m_rcContent;

	rc.DeflateRect(m_xMargin, m_yMargin);

	if (bHorz)
	{
		int			wMax = 0, width;
		for (int i = 0; i < (int)m_vLines.size(); i++)
		{
			COneLine		*pLine = m_vLines[i];
			width = 0;

			for (int k = 0; k < (int)pLine->size(); k++)
			{
				CGlyph		&glyph = (*pLine)[k];
				width += glyph.width;
			}
			if (width > wMax)
				wMax = width;
		}

		int		nPage;

		nPage = rc.Width() - m_xMargin * 2;
		if (nPage <= 0)
			nPage = 1;

		if (m_pHorzScrollBar)
		{
			if (wMax > nPage)
				m_pHorzScrollBar->SetScrollInfo(0, wMax, nPage, m_nScrollPosx, m_nOneCharDx);
			else
			{
				m_nScrollPosx = 0;
				m_pHorzScrollBar->DisableScrollBar();
			}
		}
	}

	if (bVert)
	{
		int		nPage;

		nPage = rc.Height() / GetLineDy();
		if (nPage <= 0)
			nPage = 2;

		if (m_pVertScrollBar)
		{
			if ((int)m_vLines.size() > nPage)
				m_pVertScrollBar->SetScrollInfo(0, m_vLines.size(), nPage, m_nTopVisibleLine);
			else
				m_pVertScrollBar->DisableScrollBar();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
// CUIObject messages to draw or handle user input, etc.
//
//////////////////////////////////////////////////////////////////////////

void CSkinEditCtrl::OnSize()
{
	CSkinScrollFrameCtrlBase::OnSize();

	m_caret.SetOffset(m_rcContent.left, m_rcContent.top);

	UpdateScrollInfo();

	CRect		rc = m_rcContent;

	rc.DeflateRect(m_xMargin, m_yMargin);

	if (!rc.PtInRect(CPoint(m_nCaretX + rc.left, m_nCaretY + rc.top)))
		HideCaret();
}

void CSkinEditCtrl::OnSetFocus()
{
	CRect		rc = m_rcContent;

	rc.DeflateRect(m_xMargin, m_yMargin);

	if (rc.PtInRect(CPoint(m_nCaretX + rc.left, m_nCaretY + rc.top)))
		ShowCaret();

	Invalidate();
}

void CSkinEditCtrl::OnKillFocus()
{
	m_bInMouseSel = false;

	HideCaret();

	Invalidate();
}

bool CSkinEditCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (CSkinScrollFrameCtrlBase::OnLButtonDown(nFlags, point))
		return true;

	MLWidget::SetCursor(m_cursor);

	bool control = IsKeyPressed(VK_CONTROL);
	CAutoUpdate		updater(this);

	m_bInMouseSel = false;

	SetFocus();

	if (control)
	{
		// Select the word of cursor
		SetCaretToPos(point.x, point.y);

		m_nBegSelRow = m_nCaretRow;
		m_nBegSelCol = m_nCaretCol;
		PrevWord(m_nBegSelRow, m_nBegSelCol);

		m_nEndSelRow = m_nBegSelRow;
		m_nEndSelCol = m_nBegSelCol;
		NextWord(m_nEndSelRow, m_nEndSelCol);

		m_nCaretRow = m_nEndSelRow;
		m_nCaretCol = m_nEndSelCol;

		MakeCaretInSight();
	}
	else
	{
		bool shift = IsKeyPressed(VK_SHIFT);

		if (shift && !IsSelected())
		{
			// Select text from the current caret.
			m_nBegSelRow = m_nCaretRow;
			m_nBegSelCol = m_nCaretCol;
		}

		m_pSkin->SetCaptureMouse(this);
		SetCaretToPos(point.x, point.y);
		MakeCaretInSight();

		if (shift)
		{
			// Select text to the current caret now.
			m_nEndSelRow = m_nCaretRow;
			m_nEndSelCol = m_nCaretCol;
		}

		if (!shift && IsSelected())
		{
			SelectNone();
		}
	}

	return true;
}

bool CSkinEditCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (CSkinScrollFrameCtrlBase::OnLButtonDblClk(nFlags, point))
		return true;

	MLWidget::SetCursor(m_cursor);

	CAutoUpdate		updater(this);

	m_bInMouseSel = true;

	SetFocus();

	// Select the word of cursor
	SetCaretToPos(point.x, point.y);

	m_nEndSelRow = m_nCaretRow;
	m_nEndSelCol = m_nCaretCol;
	NextWord(m_nEndSelRow, m_nEndSelCol);

	m_nBegSelRow = m_nEndSelRow;
	m_nBegSelCol = m_nEndSelCol;
	PrevWord(m_nBegSelRow, m_nBegSelCol);

	m_nCaretRow = m_nEndSelRow;
	m_nCaretCol = m_nEndSelCol;

	MakeCaretInSight();

	return true;
}

bool CSkinEditCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (CSkinScrollFrameCtrlBase::OnLButtonUp(nFlags, point))
		return true;

	MLWidget::SetCursor(m_cursor);

#ifdef _WIN32
	ReleaseCapture();
#endif
	m_bInMouseSel = false;
	if (m_nBegSelRow == m_nEndSelRow && m_nBegSelCol == m_nEndSelCol)
		m_nBegSelRow = -1;

	OnNotifySelChanged();

	return true;
}

bool CSkinEditCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (CSkinScrollFrameCtrlBase::OnMouseMove(nFlags, point))
		return true;

	MLWidget::SetCursor(m_cursor);

	if (nFlags & MK_LBUTTON )
	{
		bool shift = IsKeyPressed(VK_SHIFT);
		CAutoUpdate		updater(this);

		if (shift)
		{
			if (!IsSelected())
			{
				// Select text from the current caret.
				m_nBegSelRow = m_nCaretRow;
				m_nBegSelCol = m_nCaretCol;
			}

			SetCaretToPos(point.x, point.y);

			MakeCaretInSight();

			ShowCaret();

			// Select text to the current caret now.
			m_nEndSelRow = m_nCaretRow;
			m_nEndSelCol = m_nCaretCol;
			return true;
		}

		// mouse select
		if (!m_bInMouseSel)
		{
			m_bInMouseSel = true;

			m_nBegSelRow = m_nCaretRow;
			m_nBegSelCol = m_nCaretCol;

			SetCaretToPos(point.x, point.y);

			m_nEndSelRow = m_nCaretRow;
			m_nEndSelCol = m_nCaretCol;
		}
		else
		{
			if (!IsSelected())
			{
				// Select text from the current caret.
				m_nBegSelRow = m_nCaretRow;
				m_nBegSelCol = m_nCaretCol;
			}

			SetCaretToPos(point.x, point.y);

			m_nEndSelRow = m_nCaretRow;
			m_nEndSelCol = m_nCaretCol;
		}

		MakeCaretInSight();
		ShowCaret();
	}

	return true;
}

bool CSkinEditCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (CSkinScrollFrameCtrlBase::OnRButtonUp(nFlags, point))
		return true;

	MLWidget::SetCursor(m_cursor);

	m_pSkin->ClientToScreen(point);
	OnContexMenu(point.x, point.y);

	return true;
}

void CSkinEditCtrl::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	int			nOffset;

	if (!m_pVertScrollBar || !m_pHorzScrollBar)
		return;

	nOffset = nWheelDistance / 120;
	if (IsFlagSet(nMkeys, MK_SHIFT))
	{
		// horz scroll bar
		nOffset *= m_nOneCharDx;
		if (m_pHorzScrollBar->GetScrollPos() - nOffset < 0)
			return;
		if (m_pHorzScrollBar->GetScrollPos() - nOffset > m_pHorzScrollBar->GetMax())
			return;
		m_pHorzScrollBar->SetScrollPos(m_pHorzScrollBar->GetScrollPos() - nOffset, true);

		if (m_nScrollPosx != m_pHorzScrollBar->GetScrollPos())
		{
			m_nCaretX += m_nScrollPosx - m_pHorzScrollBar->GetScrollPos();
			m_nScrollPosx = m_pHorzScrollBar->GetScrollPos();

			if (m_nCaretX - m_nScrollPosx - m_xMargin >= m_pHorzScrollBar->GetPage()
				|| m_nCaretX < m_xMargin)
				HideCaret();
			else
				ShowCaret();

			Invalidate();
		}
	}
	else
	{
		if (m_pVertScrollBar->GetScrollPos() - nOffset < 0)
			return;
		if (m_pVertScrollBar->GetScrollPos() - nOffset > m_pVertScrollBar->GetMax())
			return;
		m_pVertScrollBar->SetScrollPos(m_pVertScrollBar->GetScrollPos() - nOffset, true);

		if (m_nTopVisibleLine != m_pVertScrollBar->GetScrollPos())
		{
			m_nTopVisibleLine = m_pVertScrollBar->GetScrollPos();
			if (m_nTopVisibleLine >= (int)m_vLines.size())
				m_nTopVisibleLine = m_vLines.size() - 1;
			if (m_nTopVisibleLine < 0)
				m_nTopVisibleLine = 0;

			if (m_nCaretRow - m_nTopVisibleLine >= m_pVertScrollBar->GetPage()
				|| m_nTopVisibleLine > m_nCaretRow)
				HideCaret();
			else
				ShowCaret();

			Invalidate();
		}
	}
}

void CSkinEditCtrl::OnKeyDown(UINT nChar, UINT nFlags)
{
	bool ctrl = IsKeyPressed(VK_CONTROL);
	bool shift = IsKeyPressed(VK_SHIFT);

	bool			bSelKey = false;
	int				nBegSelRow, nBegSelCol;
	CAutoUpdate		updater(this);
	int				nCaretMaxXLatestOrg = m_nCaretMaxXLatest;
	bool			bRecoverCaretMaxXLatestOrg = false;


	if (shift && !IsSelected())
	{
		// Select text from the current caret.
		nBegSelRow = m_nCaretRow;
		nBegSelCol = m_nCaretCol;
	}

	switch (nChar)
	{
	case 'A':
		{
			// select all
			if (ctrl)
			{
				m_nBegSelRow = 0;
				m_nBegSelCol = 0;
				m_nEndSelRow = m_vLines.size() - 1;
				ASSERT(m_vLines.size());
				m_nEndSelCol = m_vLines[m_vLines.size() - 1]->size();
			}
			break;
		}
		break;
	case 'Z':
		{
			// undo
			if (ctrl)
			{
				if (shift)
				{
					if (m_undoMgr.CanRedo())
					{
						m_undoMgr.Redo();
						updater.SetUpdateFlag(true);
					}
				}
				else
				{
					if (m_undoMgr.CanUndo())
					{
						m_undoMgr.Undo();
						updater.SetUpdateFlag(true);
					}
				}
			}
			break;
		}
	case 'Y':
		if (ctrl)
		{
			if (m_undoMgr.CanRedo())
			{
				m_undoMgr.Redo();
				updater.SetUpdateFlag(true);
			}
		}
		break;
	case 'X':
		{
			// cut
			if (ctrl)
				OnCmdCut();
			// just return, the OnCmdXXX will update the view.
			return;
		}
	case VK_INSERT:
		{
			// paste
			if (shift)
				OnCmdPaste();
			// just return, the OnCmdXXX will update the view.
			return;
		}
	case 'V':
		{
			// paste
			if (ctrl)
				OnCmdPaste();
			// just return, the OnCmdXXX will update the view.
			return;
		}
	case 'C':
		{
			// copy
			if (ctrl)
				OnCmdCopy();
			// just return, the OnCmdXXX will update the view.
			return;
		}
	case VK_END:
		{
			bSelKey = true;
			if (ctrl)
			{
				// end of text
				ASSERT(m_vLines.size());
				m_nCaretRow = m_vLines.size() - 1;
				m_nCaretCol = GetEndColPosOfLine(m_nCaretRow);
			}
			else
			{
				// end of line
				m_nCaretCol = GetEndColPosOfLine(m_nCaretRow);
			}
			break;
		}
	case VK_HOME:
		{
			bSelKey = true;
			if (ctrl)
			{
				// begin of text
				m_nCaretRow = 0;
				m_nCaretCol = 0;
			}
			else
			{
				// begin of line
				m_nCaretCol = GetCaretLineHomePos();
			}
			break;
		}
	case VK_LEFT: // left arrow
		{
			bSelKey = true;
			if (ctrl)
			{
				// to prev word
				PrevWord(m_nCaretRow, m_nCaretCol);
			}
			else
			{
				PrevPos(m_nCaretRow, m_nCaretCol);
			}
			break;
		}
	case VK_RIGHT: // right arrow
		{
			bSelKey = true;
			if (ctrl)
			{
				// to next word
				NextWord(m_nCaretRow, m_nCaretCol);
			}
			else
			{
				NextPos(m_nCaretRow, m_nCaretCol);
			}
			break;
		}
	case VK_UP: // up arrow
		{
			bSelKey = true;
			if (m_nCaretRow > 0)
			{
				bRecoverCaretMaxXLatestOrg = true;
				SetCaretToPos(m_rcContent.left + m_nCaretMaxXLatest, m_rcContent.top + m_nCaretY - GetLineDy());
			}
			break;
		}
	case VK_DOWN: // down arrow
		{
			bSelKey = true;
			if (m_nCaretRow < (int)m_vLines.size() - 1)
			{
				bRecoverCaretMaxXLatestOrg = true;
				SetCaretToPos(m_rcContent.left + m_nCaretMaxXLatest, m_rcContent.top + m_nCaretY + GetLineDy());
			}
			break;
		}
	case VK_PRIOR: // page up
		{
			bSelKey = true;
			if (m_nCaretRow > 0 && m_pVertScrollBar&& m_pVertScrollBar->IsEnabled())
			{
				bRecoverCaretMaxXLatestOrg = true;
				SetCaretToPos(m_rcContent.left + m_nCaretMaxXLatest, m_rcContent.top + m_nCaretY - GetLineDy() * (m_pVertScrollBar->GetPage() - 1));
				m_nTopVisibleLine -= m_pVertScrollBar->GetPage() - 1;
				if (m_nTopVisibleLine < 0)
					m_nTopVisibleLine = 0;
			}
			break;
		}
	case VK_NEXT: // page down
		{
			bSelKey = true;
			if (m_nCaretRow < (int)m_vLines.size() && m_pVertScrollBar && m_pVertScrollBar->IsEnabled())
			{
				bRecoverCaretMaxXLatestOrg = true;
				SetCaretToPos(m_rcContent.left + m_nCaretMaxXLatest, m_rcContent.top + m_nCaretY + GetLineDy() * (m_pVertScrollBar->GetPage() - 1));
				m_nTopVisibleLine += m_pVertScrollBar->GetPage() - 1;
				if (m_nTopVisibleLine > m_pVertScrollBar->GetMax())
				{
					m_nTopVisibleLine = m_pVertScrollBar->GetMax();
				}
			}
			break;
		}
	case VK_BACK:
		{
			if (!IsReadOnly())
			{
				if (IsSelected())
				{
					int				nBegSelRow, nBegSelCol;
					tstring			strToDel;

					SelectionToText(strToDel);
					RemoveSelected(&nBegSelRow, &nBegSelCol);
					m_undoMgr.AddAction(new CEditDelAction(this, nBegSelRow, nBegSelCol, strToDel.c_str()));
				}
				else
				{
					tstring			strToDel;

					PrevPos(m_nCaretRow, m_nCaretCol);
					GetStrAtPos(m_nCaretRow, m_nCaretCol, strToDel);
					m_undoMgr.AddAction(new CEditDelAction(this, m_nCaretRow, m_nCaretCol, strToDel.c_str()));
					RemoveChar(m_nCaretRow, m_nCaretCol);
				}
				updater.SetUpdateFlag(true);
				UpdateScrollInfo();
			}
			break;
		}
	case VK_DELETE:
		{
			if (!IsReadOnly())
			{
				if (shift)
				{
					// shift + VK_DELETE = cut
					OnCmdCut();
					// just return, the OnCmdXXX will update the view.
					return;
				}
				else
				{
					OnCmdDelete();
				}
			}
			break;
		}
	case VK_RETURN:
		{
			if (IsSingleLine())
			{
				if (m_pEditNotification)
				{
					if (ctrl)
						m_pEditNotification->OnSpecialKey(IEditNotification::SK_CTRL_ENTER);
					else if (shift)
						m_pEditNotification->OnSpecialKey(IEditNotification::SK_SHIFT_ENTER);
					else
						m_pEditNotification->OnSpecialKey(IEditNotification::SK_ENTER);
				}
				break;
			}

			if (!IsReadOnly())
			{
				if (IsSelected())
				{
					int				nBegSelRow, nBegSelCol;
					tstring			strToDel;

					SelectionToText(strToDel);
					RemoveSelected(&nBegSelRow, &nBegSelCol);
					m_undoMgr.AddAction(new CEditRepAction(this, nBegSelRow, nBegSelCol, strToDel.c_str(), COneLine::GetReturnStr(NLT_RN)));
					InsertStr(nBegSelRow, nBegSelCol, COneLine::GetReturnStr(NLT_RN), m_nCaretRow, m_nCaretCol);
				}
				else
				{
					m_undoMgr.AddAction(new CEditInsertAction(this, m_nCaretRow, m_nCaretCol, COneLine::GetReturnStr(NLT_RN)));
					InsertStr(m_nCaretRow, m_nCaretCol, COneLine::GetReturnStr(NLT_RN), m_nCaretRow, m_nCaretCol);
				}

				updater.SetUpdateFlag(true);
				UpdateScrollInfo();
			}

			break;
		}
	default:
		return;
	}

	if (bSelKey)
	{
		if (shift)
		{
			if (!IsSelected())
			{
				m_nBegSelRow = nBegSelRow;
				m_nBegSelCol = nBegSelCol;
			}

			// Select text to the current caret now.
			m_nEndSelRow = m_nCaretRow;
			m_nEndSelCol = m_nCaretCol;

			OnNotifySelChanged();
		}
		else
			SelectNone();
	}

	// Where is Caret? Make it in sight
	MakeCaretInSight();

	if (bRecoverCaretMaxXLatestOrg)
		m_nCaretMaxXLatest = nCaretMaxXLatestOrg;
}

void CSkinEditCtrl::OnChar(UINT nChar)
{
	// CStrPrintf		strp(_T("%x -"), wUniChar);
	TCHAR			wUniChar;

#ifdef _WIN32
	if (m_bWin9x)
	{
		if (m_bMbcsA)
		{
			m_bMbcsA = false;
			m_szMbcsA[1] = nChar;
			WCHAR		szOut[2];
			ConvertStr2(m_szMbcsA, 2, szOut, CountOf(szOut));
			wUniChar = szOut[0];
		}
		else
		{
			if (nChar <= 127 && nChar != '\t' && !isprint(nChar))
 				return;

			if (IsDBCSLeadByte(nChar))
			{
				m_bMbcsA = true;
				m_szMbcsA[0] = nChar;
				return;
			}
			else
				wUniChar = nChar;
		}
	}
	else
#endif
	{
		if (nChar <= 127 && nChar != '\t' && !isprint(nChar))
 			return;

		wUniChar = nChar;
	}

	CAutoUpdate		updater(this);

	if (IsSelected())
	{
		int				nBegSelRow, nBegSelCol;
		tstring			strToDel;

		SelectionToText(strToDel);
		RemoveSelected(&nBegSelRow, &nBegSelCol);
		m_undoMgr.AddAction(new CEditRepAction(this, nBegSelRow, nBegSelCol, strToDel.c_str(), (WCHAR)wUniChar));
		InsertChar(nBegSelRow, nBegSelCol, (WCHAR)wUniChar);
	}
	else
	{
		m_undoMgr.AddAction(new CEditInsertAction(this, m_nCaretRow, m_nCaretCol, (WCHAR)wUniChar));
		InsertChar(m_nCaretRow, m_nCaretCol, (WCHAR)wUniChar);
	}

	updater.SetUpdateFlag(true);
	NextPos(m_nCaretRow, m_nCaretCol);

	// Where is Caret? Make it in sight
	UpdateScrollInfo(true, false);
	MakeCaretInSight();
}

void CSkinEditCtrl::OnContexMenu(int xPos, int yPos)
{
	CSkinMenu	menu(m_pSkin->GetSkinFactory());
	bool		bEnable;

	menu.LoadPopupMenu(IDM_MENU, 2);

	bEnable = IsSelected();
	menu.EnableItem(IDC_EDIT_CUT, bEnable);
	menu.EnableItem(IDC_EDIT_COPY, bEnable);

	bEnable = m_undoMgr.CanUndo();
	menu.EnableItem(IDC_EDIT_UNDO, bEnable);

	bEnable = m_undoMgr.CanRedo();
	menu.EnableItem(IDC_EDIT_REDO, bEnable);

	m_pSkin->RegisterHandleContextMenuCmd(this);

	menu.TrackPopupMenu(xPos, yPos, m_pSkin);
}

void CSkinEditCtrl::OnTimer(int nId)
{
	if (nId == m_nIDTimerCaret)
	{
		if (!IsVisible() || !IsParentVisible())
		{
			m_pSkin->UnregisterTimerObject(this, m_nIDTimerCaret);
			m_caret.HideCaret();
			return;
		}

		// Redraw flashing caret
		CRawGraph		*pGraph;

		pGraph = m_pContainer->GetMemGraph();
		ASSERT(pGraph);
		m_caret.DrawFlash(pGraph);

		UpdateRectToScreen(&m_caret.GetUpdateRect());
	}
	else
		CSkinScrollFrameCtrlBase::OnTimer(nId);
}

bool CSkinEditCtrl::OnCommand(int nId)
{
	switch (nId)
	{
	case IDC_EDIT_UNDO:
		OnCmdUndo();
		break;
	case IDC_EDIT_REDO:
		OnCmdRedo();
		break;
	case IDC_EDIT_CUT:
		OnCmdCut();
		break;
	case IDC_EDIT_COPY:
		OnCmdCopy();
		break;
	case IDC_EDIT_PASTE:
		OnCmdPaste();
		break;
	case IDC_EDIT_DELETE:
		OnCmdDelete();
		break;
	default:
		return false;
	}

	return true;
}

void CSkinEditCtrl::OnAdjustHue(float hue, float saturation, float luminance)
{
	CSkinScrollFrameCtrlBase::OnAdjustHue(hue, saturation, luminance);

	m_font.OnAdjustHue(m_pSkin, hue, saturation, luminance);
}

void CSkinEditCtrl::OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	if (!m_pVertScrollBar)
		return;

	CAutoUpdate		updater(this);

	// changes...
	m_nTopVisibleLine = m_pVertScrollBar->GetScrollPos();
	if (m_nTopVisibleLine < 0)
		m_nTopVisibleLine = 0;

	if (m_nCaretRow - m_nTopVisibleLine >= m_pVertScrollBar->GetPage()
		|| m_nTopVisibleLine > m_nCaretRow)
		HideCaret();
	else
		ShowCaret();

	updater.SetUpdateFlag(true);
}

void CSkinEditCtrl::OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	if (!m_pHorzScrollBar)
		return;

	CAutoUpdate		updater(this);

	// changes...
	m_nCaretX += m_nScrollPosx - m_pHorzScrollBar->GetScrollPos();
	m_nScrollPosx = m_pHorzScrollBar->GetScrollPos();

	if (m_nCaretX - m_nScrollPosx - m_xMargin >= m_pHorzScrollBar->GetPage()
		|| m_nCaretX < m_xMargin)
		HideCaret();
	else
		ShowCaret();

	updater.SetUpdateFlag(true);
}


//
// IUndoMgrNotify interface
//
void CSkinEditCtrl::OnStatusChanged(Status status, bool bVal)
{
	if (!m_pEditNotification)
		return;

	IEditNotification::Status	s;

	switch (status)
	{
	case S_CAN_UNDO:
		s = IEditNotification::S_CAN_UNDO;
		break;
	case S_CAN_REDO:
		s = IEditNotification::S_CAN_REDO;
		break;
	default:
		return;
	}

	m_pEditNotification->OnStatusChanged(s, bVal);
}


void CSkinEditCtrl::OnAction(Action action)
{
	if (!m_pEditNotification)
		return;

	m_pEditNotification->OnTextChanged();
}

ID_TO_STRING __EditStyleText[] = 
{
	{ CSkinEditCtrl::S_MULTILINE, _T("MULTILINE") },
	{ CSkinEditCtrl::S_PASSWORD, _T("PASSWORD") },
	{ CSkinEditCtrl::S_READ_ONLY, _T("READ_ONLY") },
	{ 0, NULL },
};

BOOL CSkinEditCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinScrollFrameCtrlBase::SetProperty(szProperty, szValue))
		return TRUE;

	if (m_font.SetProperty(szProperty, szValue))
		return TRUE;
	else if (IsPropertyName(szProperty, _T("XMargin")))
		m_xMargin = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("YMargin")))
		m_yMargin = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("Style")))
		m_nEditorStyles = GetCombinationValue(__EditStyleText, szValue);
	else
		return FALSE;

	return true;
}

tstring &CSkinEditCtrl::GetText()
{
	m_strText.resize(0);
	CSkinEditCtrl::GetText(m_strText);

	return m_strText;
}

#ifdef _SKIN_EDITOR_
void CSkinEditCtrl::EnumProperties(CUIObjProperties &listProperties)
{
	CSkinScrollFrameCtrlBase::EnumProperties(listProperties);

	m_font.EnumProperties(listProperties);
}
#endif // _SKIN_EDITOR_
