// EditEx.cpp: implementation of the CEditEx class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BaseWnd.h"
#include "EditEx.h"
#include "desktop.h"

#ifdef _WINCE_
#ifndef TPM_RIGHTBUTTON
#define TPM_RIGHTBUTTON		0
#endif
#endif

#define GLYPH_WIDTH_INVALID		0xFF


#define		IDC_EX_UNDO		100
#define		IDC_EX_REDO		101
#define		IDC_EX_CUT		102
#define		IDC_EX_COPY		103
#define		IDC_EX_PASTE	104
#define		IDC_EX_DELETE	105

LPCSTR ReadUtf8Char(CEditEx::CGlyph &glyph, LPCSTR szText)
{
	unsigned char *p = (unsigned char *)szText;
	if (*p < 0x80)
	{
		glyph.szGlyph[0] = *szText;
		glyph.nLen = 1;
		szText++;
	}
	else if (*p < 0xE0)
	{
		ASSERT(*p >= 0xC0 && "Invalid UTF8 First Byte");
		glyph.szGlyph[0] = szText[0];
		glyph.szGlyph[1] = szText[1];
		szText += 2;
		glyph.nLen = 2;
	}
	else
	{
		ASSERT(*p >= 0xC0 && "Invalid UTF8 First Byte");
		glyph.szGlyph[0] = szText[0];
		glyph.szGlyph[1] = szText[1];
		glyph.szGlyph[2] = szText[2];
		szText += 3;
		glyph.nLen = 3;
	}

	return szText;
}

bool IsOneUtf8Char(const char *szChar)
{
	unsigned char *p = (unsigned char *)szChar;
	if (*p < 0x80)
	{
		szChar++;
		return *szChar == '\0';
	}
	else if (*p < 0xE0)
	{
		ASSERT(*p >= 0xC0 && "Invalid UTF8 First Byte");
		szChar += 2;
		return *szChar == '\0';
	}
	else
	{
		ASSERT(*p >= 0xC0 && "Invalid UTF8 First Byte");
		szChar += 3;
		return *szChar == '\0';
	}
}

//////////////////////////////////////////////////////////////////////////

class CEditExAction : public CUndoAction
{
public:
	CEditExAction() { }
	virtual ~CEditExAction() { }

protected:
	CEditEx		*m_pEdit;
	int			m_nCaretRow, m_nCaretCol;
	tstring		m_str;

};

class CEditInsertAction : public CEditExAction
{
public:
	CEditInsertAction(CEditEx *pEdit, int nCaretRow, int nCaretCol, LPCTSTR str)
	{
		m_pEdit = pEdit;
		m_nCaretRow = nCaretRow;
		m_nCaretCol = nCaretCol;
		m_str = str;
	}
	CEditInsertAction(CEditEx *pEdit, int nCaretRow, int nCaretCol, TCHAR ch)
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
				m_pEdit->UpdateScrollInfo(true);
				m_pEdit->UpdateScrollInfo(false);
				m_pEdit->MakeCaretInSight();
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
				m_pEdit->UpdateScrollInfo(true);
				m_pEdit->UpdateScrollInfo(false);
				m_pEdit->MakeCaretInSight();
			}
		}
	}
	virtual void Redo()
	{
		if (IsOneUtf8Char(m_str.c_str()))
		{
			ASSERT(m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol));
			if (m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol))
			{
				m_pEdit->InsertChar(m_nCaretRow, m_nCaretCol, m_str.c_str());
				m_pEdit->m_nCaretRow = m_nCaretRow;
				m_pEdit->m_nCaretCol = m_nCaretCol;
				m_pEdit->NextPos(m_pEdit->m_nCaretRow, m_pEdit->m_nCaretCol);
				m_pEdit->UpdateScrollInfo(true);
				m_pEdit->UpdateScrollInfo(false);
				m_pEdit->MakeCaretInSight();
			}
		}
		else
		{
			ASSERT(m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol));
			if (m_pEdit->IsPosValid(m_nCaretRow, m_nCaretCol))
			{
				m_pEdit->InsertStr(m_nCaretRow, m_nCaretCol, m_str.c_str(), m_pEdit->m_nCaretRow, m_pEdit->m_nCaretCol);
				m_pEdit->UpdateScrollInfo(true);
				m_pEdit->UpdateScrollInfo(false);
				m_pEdit->MakeCaretInSight();
			}
		}
	}

};


class CEditDelAction : public CEditInsertAction
{
public:
  	CEditDelAction(CEditEx *pEdit, int nCaretRow, int nCaretCol, LPCTSTR str)
		: CEditInsertAction(pEdit, nCaretRow, nCaretCol, str)
  	{
  	}
  	CEditDelAction(CEditEx *pEdit, int nCaretRow, int nCaretCol, TCHAR ch)
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
	CEditRepAction(CEditEx *pEdit, int nCaretRow, int nCaretCol, LPCTSTR szSrc, LPCTSTR szTarg)
		: m_del(pEdit, nCaretRow, nCaretCol, szSrc),
		m_insert(pEdit, nCaretRow, nCaretCol, szTarg)
	{
	}
	CEditRepAction(CEditEx *pEdit, int nCaretRow, int nCaretCol, LPCTSTR szSrc, TCHAR chTarg)
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

//////////////////////////////////////////////////////////////////////////
// CBatchUndoAction
CBatchUndoAction::CBatchUndoAction()
{

}

CBatchUndoAction::~CBatchUndoAction()
{
	LIST_ACTIONS::iterator it, itEnd;
	itEnd = m_listActions.end();
	for (it = m_listActions.begin(); it != itEnd; ++it)
	{
		delete *it;
	}
	m_listActions.clear();
}

void CBatchUndoAction::Undo()
{
	LIST_ACTIONS::reverse_iterator it, itEnd;
	itEnd = m_listActions.rend();
	for (it = m_listActions.rbegin(); it != itEnd; ++it)
	{
		(*it)->Undo();
	}
}

void CBatchUndoAction::Redo()
{
	LIST_ACTIONS::iterator it, itEnd;
	itEnd = m_listActions.end();
	for (it = m_listActions.begin(); it != itEnd; ++it)
	{
		(*it)->Redo();
	}
}

void CBatchUndoAction::AddAction(CUndoAction *pAction)
{
	m_listActions.push_back(pAction);
}

bool CBatchUndoAction::IsEmpty()
{
	return m_listActions.empty();
}

//////////////////////////////////////////////////////////////////////////
// CUndoMgr
CUndoMgr::CUndoMgr()
{
	m_nPosUndo = -1;
	m_pBatchUndoAction = NULL;
	m_pEdit = NULL;

	m_bCanUndoPrev = false;
	m_bCanRedoPrev = false;
}

CUndoMgr::~CUndoMgr()
{
	if (m_pBatchUndoAction)
		delete m_pBatchUndoAction;
	Clear();
}

void CUndoMgr::Init(CEditEx *pEdit)
{
	m_pEdit = pEdit;
}

void CUndoMgr::BeginBatchAction()
{
	ASSERT(!m_pBatchUndoAction);
	if (m_pBatchUndoAction)
	{
		EndBatchAction();
	}
	m_pBatchUndoAction = new CBatchUndoAction;
}

void CUndoMgr::EndBatchAction()
{
	ASSERT(m_pBatchUndoAction);
	if (m_pBatchUndoAction)
	{
		if (m_pBatchUndoAction->IsEmpty())
			delete m_pBatchUndoAction;
		else
			DoAddAction(m_pBatchUndoAction);
		m_pBatchUndoAction = NULL;
	}
}

void CUndoMgr::AddAction(CUndoAction *pAction)
{
	if (m_pBatchUndoAction)
		m_pBatchUndoAction->AddAction(pAction);
	else
		DoAddAction(pAction);
}

bool CUndoMgr::Undo()
{
	ASSERT(!m_pBatchUndoAction);
	if (m_pBatchUndoAction)
		EndBatchAction();

	ASSERT(m_nPosUndo >= -1 && m_nPosUndo < (int)m_listActions.size());
	if (CanUndo())
	{
		m_listActions[m_nPosUndo]->Undo();
		m_nPosUndo--;
		SendChangeNotifyMsg();
		OnDetectRedoStatus();
		OnDetectUndoStatus();
		return true;
	}
	return false;
}

bool CUndoMgr::Redo()
{
	ASSERT(!m_pBatchUndoAction);
	if (m_pBatchUndoAction)
		EndBatchAction();

	// ASSERT(CanRedo);
	if (CanRedo())
	{
		m_listActions[m_nPosUndo + 1]->Redo();
		m_nPosUndo++;
		SendChangeNotifyMsg();
		OnDetectRedoStatus();
		OnDetectUndoStatus();
		return true;
	}
	return false;
}

bool CUndoMgr::CanUndo()
{
	return m_nPosUndo >= 0 && m_listActions.size();
}

bool CUndoMgr::CanRedo()
{
	return m_listActions.size() && m_nPosUndo < int(m_listActions.size() - 1);
}

void CUndoMgr::Clear()
{
	ASSERT(!m_pBatchUndoAction);
	if (m_pBatchUndoAction)
		EndBatchAction();

	if (m_listActions.size())
	{
		OnDetectRedoStatus();
		OnDetectUndoStatus();
	}

	for (LIST_ACTIONS::iterator it = m_listActions.begin(); it != m_listActions.end(); it++)
	{
		delete *it;
	}
	m_listActions.clear();

	m_nPosUndo = -1;
}

void CUndoMgr::SendChangeNotifyMsg()
{
	if (m_pEdit->m_pEditNotification)
	{
		m_pEdit->m_pEditNotification->OnTextChanged();
	}
}

void CUndoMgr::OnDetectUndoStatus()
{
	bool		bNew = CanUndo();
	if (m_bCanUndoPrev != bNew)
	{
		m_bCanUndoPrev = bNew;
		if (m_pEdit->m_pEditNotification)
			m_pEdit->m_pEditNotification->OnStatusChanged(IEditNotification::S_CAN_UNDO, bNew);
	}
}

void CUndoMgr::OnDetectRedoStatus()
{
	bool		bNew = CanRedo();
	if (m_bCanRedoPrev != bNew)
	{
		m_bCanRedoPrev = bNew;
		if (m_pEdit->m_pEditNotification)
			m_pEdit->m_pEditNotification->OnStatusChanged(IEditNotification::S_CAN_REDO, bNew);
	}
}

void CUndoMgr::DoAddAction(CUndoAction *pAction)
{
	ASSERT(m_nPosUndo >= -1);
	if (m_nPosUndo < (int)(m_listActions.size() - 1))
	{
		for (LIST_ACTIONS::iterator it = m_listActions.begin() + (m_nPosUndo + 1);
			it != m_listActions.end(); it++)
		{
			delete *it;
		}
		m_listActions.erase(m_listActions.begin() + (m_nPosUndo + 1), m_listActions.end());
	}

	m_listActions.push_back(pAction);
	m_nPosUndo = m_listActions.size() - 1;

	SendChangeNotifyMsg();

	OnDetectRedoStatus();
	OnDetectUndoStatus();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEditEx::CEditEx()
{
	m_nCaretX = 0;
	m_nCaretY = 0;
	m_nMarginX = 2;
	m_nMarginY = 2;
	m_nLineSpace = 2;
	m_nTopVisibleLine = 0;

	m_clrBk.set(RGB(255, 255, 255));
	m_clrSelText.set(RGB(255, 255, 255));
	m_clrSelBg.set(RGB(0x0A, 0x24, 0x6A));

	m_vClrTable.push_back(MLColor(RGB(0, 0, 0)));

	m_brSelBg.CreateSolidBrush(m_clrSelBg);
	m_brBk.CreateSolidBrush(m_clrBk);
	m_bCaretOn = false;
	m_nBegSelCol = m_nEndSelCol = m_nBegSelRow = m_nEndSelRow = -1;
	m_nOneCharDx = 5;
	m_bInMouseSel = false;
	m_bReadOnly = false;
	m_nScrollPosx = 0;

	m_bMbcsA = false;
	m_bWin9x = GetOperationSystemType() <= OPS_WIN9XMORE;

	m_pEditSyntaxParser = NULL;
	m_pEditNotification = NULL;

	m_undoMgr.Init(this);

	m_bPrevSelectedStatus = false;

	m_pSbHorz = NULL;
	m_pSbVert = NULL;
}

CEditEx::~CEditEx()
{
	Free();
}

bool CEditEx::CGlyph::Cmp(const char *str)
{
	for (int i = 0; i < nLen; i++)
	{
		if (szGlyph[i] != str[i])
			return false;
	}
	return true;
}

bool CEditEx::CGlyph::iCmp(const char *str)
{
	if (nLen == 1)
	{
		int		f, l;
		f = szGlyph[0];
		l = *str;
		if (f >= 'A' && f <= 'Z')
			f -= ('A' - 'a');
		if (l >= 'A' && l <= 'Z')
			l -= ('A' - 'a');
		return f == l;
	}

	for (int i = 0; i < nLen; i++)
	{
		if (szGlyph[i] != str[i])
			return false;
	}
	return true;
	
}

bool CEditEx::CGlyph::ReverseCmp(const char *str)
{
	for (int i = nLen - 1; i >= 0; i--)
	{
		if (szGlyph[i] != *str)
			return false;
		str--;
	}
	return true;
}

bool CEditEx::CGlyph::ReverseiCmp(const char *str)
{
	if (nLen == 1)
	{
		int		f, l;
		f = szGlyph[0];
		l = *str;
		if (f >= 'A' && f <= 'Z')
			f -= ('A' - 'a');
		if (l >= 'A' && l <= 'Z')
			l -= ('A' - 'a');
		return f == l;
	}

	for (int i = nLen - 1; i >= 0; i--)
	{
		if (szGlyph[i] != *str)
			return false;
		str--;
	}
	return true;
}

BOOL CEditEx::Create(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, int nID)
{
	m_nID = nID;

	m_window = gtk_drawing_area_new();
	assert(m_window);

	gtk_widget_set_size_request(m_window, nWidth, nHeight);

	OnCreate();

	DoSetText(szCaption);

	UpdateScrollInfo(true);
	UpdateScrollInfo(false);

	return true;
}

bool CEditEx::OnCreate()
{
	CBaseWnd::OnCreate();

	NewScrollBar();

	m_pSbVert->SetScrollNotify(this);
	m_pSbHorz->SetScrollNotify(this);

	return true;
}

void CEditEx::OnDestroy()
{
	m_undoMgr.Clear();

	CBaseWnd::OnDestroy();
}

void CEditEx::SetFont(CMLFont &font, const MLColor &clrText)
{
	m_vClrTable[0] = clrText;
	m_font = font;

	IMLGraphicsBase		*pGraph;
	COneLine			*pLine = NULL;
	SIZE				size;

	pGraph = GetGraphics();

	pGraph->SetFont(&font);

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

	ReleaseGraphics(pGraph);
}

void CEditEx::SetBgColor(const MLColor &clr)
{
	m_clrBk = clr;
	m_brBk.Destroy();
	m_brBk.CreateSolidBrush(m_clrBk);
}

void CEditEx::SetSelColor(const MLColor &clrSelText, const MLColor &clrSelBg)
{
	m_clrSelText = clrSelText;
	m_clrSelBg = clrSelBg;

	m_brSelBg.Destroy();
	m_brSelBg.CreateSolidBrush(m_clrSelBg);
}

void CEditEx::SetSel(int nBeg, int nEnd)
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

void CEditEx::ReplaceSel(LPCTSTR szText)
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

	MakeCaretInSight();

	UpdateScrollInfo(true);
	UpdateScrollInfo(false);
}

void CEditEx::RemoveSel()
{
	if (!IsSelected())
		return;

	CAutoUpdate		updater(this);

	int			nBegSelRow, nBegSelCol;
	tstring		strToDel;

	SelectionToText(strToDel);
	RemoveSelected(&nBegSelRow, &nBegSelCol);
	m_undoMgr.AddAction(new CEditDelAction(this, nBegSelRow, nBegSelCol, strToDel.c_str()));

	MakeCaretInSight();

	UpdateScrollInfo(true);
	UpdateScrollInfo(false);
}

void CEditEx::SetCaret(int nRow, int nCol)
{
	if (nRow == -1 || nRow >= (int)m_vLines.size())
	{
		m_nCaretRow = m_vLines.size() - 1;
		m_nCaretCol = GetEndColOfLine(m_nCaretRow);
		return;
	}

	m_nCaretRow = nRow;
	if (nCol > GetEndColOfLine(m_nCaretRow))
		m_nCaretCol = GetEndColOfLine(m_nCaretRow);
	else
		m_nCaretCol = nCol;

	MakeCaretInSight();
	InvalidateRect(NULL);
}

int CEditEx::GetLength()
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

bool CEditEx::SetWindowText(LPCTSTR szText)
{
	m_undoMgr.Clear();
	DoSetText(szText);
	UpdateScrollInfo(true);
	UpdateScrollInfo(false);
	return true;
	// return Tobool(::SetWindowText(m_hWnd, szText));
}

int CEditEx::GetWindowText(LPTSTR szString, int nMaxCount)
{
	int		n = 0;

	for (int i = 0; i < (int)m_vLines.size() && n < nMaxCount; i++)
	{
		COneLine		*pLine = m_vLines[i];
		for (int k = 0; k < (int)pLine->size(); k++)
		{
			CGlyph	&glyph = (*pLine)[k];
			for (int m = 0; m < glyph.nLen; m++)
			{
				szString[n] = glyph.szGlyph[m];
				n++;
			}
		}
		if (pLine->newLineType == NLT_RN)
		{
			if (n + 1 < nMaxCount)
			{
				szString[n] = '\r';
				szString[n + 1] = '\n';
				n += 2;
			}
		}
		else if (pLine->newLineType == NLT_N)
			szString[n++] = '\n';
		else if (pLine->newLineType == NLT_R)
			szString[n++] = '\r';
	}
	szString[n] = '\0';
	return n;
}

int CEditEx::GetWindowText(tstring &str)
{
	for (int i = 0; i < (int)m_vLines.size(); i++)
	{
		COneLine		*pLine = m_vLines[i];
		for (int k = 0; k < (int)pLine->size(); k++)
		{
			CGlyph	&glyph = (*pLine)[k];
			str.append(glyph.szGlyph, glyph.nLen);
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

bool CEditEx::GetTextOfLine(int nLine, tstring &strLine)
{
	strLine = _T("");
	if (nLine < 0 || nLine >= (int)m_vLines.size())
		return false;

	COneLine		*pLine = m_vLines[nLine];

	for (int k = 0; k < (int)pLine->size(); k++)
	{
		CGlyph	&glyph = (*pLine)[k];

		strLine.append(glyph.szGlyph, glyph.nLen);
	}

	return true;
}

bool CEditEx::CanUndo()
{
	return m_undoMgr.CanUndo();
}

bool CEditEx::CanRedo()
{
	return m_undoMgr.CanRedo();
}

void CEditEx::OnCmdCopy()
{
	if (IsSelected())
	{
		tstring		str;
		SelectionToText(str);
//		if (str.size())
//			CopyTextToClipboard(str.c_str(), m_hWnd);
	}
}

// cut selected
void CEditEx::OnCmdCut()
{
	if (m_bReadOnly)
		return;

	if (!IsSelected())
		return;

	CAutoUpdate		updater(this);
	tstring			strToDel;
	int				nBegSelRow, nBegSelCol;

	SelectionToText(strToDel);

//	CopyTextToClipboard(strToDel.c_str(), m_hWnd);

	RemoveSelected(&nBegSelRow, &nBegSelCol);
	m_undoMgr.AddAction(new CEditDelAction(this, nBegSelRow, nBegSelCol, strToDel.c_str()));

	MakeCaretInSight();
}

void CEditEx::OnCmdPaste()
{
	if (m_bReadOnly)
		return;

	CAutoUpdate		updater(this);
	tstringex		str;

//	GetClipBoardText(str, m_hWnd);
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
		UpdateScrollInfo(true);
		UpdateScrollInfo(false);
	}

	MakeCaretInSight();
}

void CEditEx::OnCmdUndo()
{
	if (m_bReadOnly)
		return;

	CAutoUpdate		updater(this);

	if (m_undoMgr.CanUndo())
	{
		m_undoMgr.Undo();
		updater.SetUpdateFlag(true);
	}

	MakeCaretInSight();
}

void CEditEx::OnCmdRedo()
{
	if (m_bReadOnly)
		return;

	CAutoUpdate		updater(this);

	if (m_undoMgr.CanRedo())
	{
		m_undoMgr.Redo();
		updater.SetUpdateFlag(true);
	}

	MakeCaretInSight();
}

void CEditEx::OnCmdDelete()
{
	if (m_bReadOnly)
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
	UpdateScrollInfo(true);
	UpdateScrollInfo(false);

	MakeCaretInSight();
}

void CEditEx::SelectionToText(tstring &str)
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
				str.append(glyph.szGlyph, glyph.nLen);
			}
		}
		if (i < nEndSelRow)
			str += pLine->GetReturnStr(pLine->newLineType);
	}
}

void CEditEx::GetStrAtPos(int nRow, int nCol, tstring &str)
{
	ASSERT(IsPosValid(nRow, nCol));

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
		str.append(pLine->at(nCol).szGlyph, pLine->at(nCol).nLen);
	}
}

bool CEditEx::IsPosValid(int nRow, int nCol)
{
	if (nRow >= 0 && nRow < (int)m_vLines.size())
	{
		COneLine		*pLine = m_vLines[nRow];

		return (nCol >= 0 && nCol <= (int)pLine->size());
	}

	return false;
}

void CEditEx::SortSelectPos(int &nBegSelRow, int &nBegSelCol, int &nEndSelRow, int &nEndSelCol, bool &bSelectedEmpty)
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

void CEditEx::OnPaint(CMLGraphics *pGraph, CRect &rcClip)
{
	CRect	rc, rcClip;
	int		nBegSelRow, nEndSelRow;
	int		nBegSelCol, nEndSelCol;
	bool	bSelectedEmpty = true;

	if (m_nBegSelRow != -1)
	{
		bSelectedEmpty = false;
		SortSelectPos(nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol, bSelectedEmpty);
	}

	GetClientRect(&rc);
	if (m_dcMem.Width() != rc.Width() || m_dcMem.Height() != rc.Height())
	{
		m_dcMem.Create(rc.Width(), rc.Height(), m_window->window);
	}

	m_dcMem.FillRect(0, 0, rc.right, rc.bottom, &m_brBk);

	m_dcMem.SetFont(&m_font);

	m_dcMem.SetBgColor(m_clrBk);
	m_dcMem.SetBkMode(true);

	rcClip.SetRect(m_nMarginX, m_nMarginY, rc.right - m_nMarginX, rc.bottom - m_nMarginY);
	m_dcMem.SetClipBoundBox(rcClip);

	int		x, y, xMax, yMax;
	int		i;
	x = m_nMarginX;
	y = m_nMarginY;
	xMax = rc.right - m_nMarginX;
	yMax = rc.bottom - m_nMarginY;
	ASSERT(m_nTopVisibleLine >= 0 && m_nTopVisibleLine < (int)m_vLines.size());

	if (bSelectedEmpty)
	{
		// If no lines are selected, draw same
		for (int i = m_nTopVisibleLine; i < (int)m_vLines.size(); i++)
		{
			if (y + GetLineDx() > yMax && i > m_nTopVisibleLine)
				break;

			DrawLine(&m_dcMem, m_vLines[i], xMax, y);

			y += GetLineDx();
		}
		m_dcMem.RecoverClipBox();
		pGraph->DrawGraphics(&m_dcMem, 0, 0, rc.right, rc.bottom, 0, 0);
		return;
	}

	// draw lines before selection
	for (i = m_nTopVisibleLine; i < nBegSelRow; i++)
	{
		if (y + GetLineDx() > yMax && i > m_nTopVisibleLine)
			break;

		DrawLine(&m_dcMem, m_vLines[i], xMax, y);

		y += GetLineDx();
	}

	// draw lines at selection
	for (i = nBegSelRow; i <= nEndSelRow; i++)
	{
		if (i < m_nTopVisibleLine)
			continue;
		if (y + GetLineDx() > yMax && i > m_nTopVisibleLine)
			break;

		COneLine		*pLine = m_vLines[i];

		x = m_nMarginX - m_nScrollPosx;
		for (int k = 0; k < (int)pLine->size(); k++)
		{
			CGlyph	&glyph = (*pLine)[k];
			ASSERT(glyph.width != GLYPH_WIDTH_INVALID);
			if (x + glyph.width < m_nMarginX)
			{
				x += glyph.width;
				continue;
			}
			if (x + glyph.width > xMax)
				break;

			// if (glyph.width)
			{
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
					m_dcMem.FillRect(x, y, glyph.width, GetLineDx(), &m_brSelBg);
					m_dcMem.SetTextColor(m_clrSelText);
				}
				else
				{
					if (glyph.clrIndex >= m_vClrTable.size())
						m_dcMem.SetTextColor(m_vClrTable[0]);
					else
						m_dcMem.SetTextColor(m_vClrTable[glyph.clrIndex]);
				}
				if (glyph.szGlyph[0] != '\t')
					m_dcMem.TextOut(x, y, glyph.szGlyph, glyph.nLen);
				x += glyph.width;
			}
		}

		y += GetLineDx();
	}

//	m_dcMem.SetTextColor(m_clrText);
	for (i = nEndSelRow + 1; i < (int)m_vLines.size(); i++)
	{
		if (i < m_nTopVisibleLine)
			continue;
		if (y + GetLineDx() > yMax && i > m_nTopVisibleLine)
			break;

		DrawLine(&m_dcMem, m_vLines[i], xMax, y);

		y += GetLineDx();
	}

	m_dcMem.RecoverClipBox();
	pGraph->DrawGraphics(&m_dcMem, 0, 0, rc.right, rc.bottom, 0, 0);
}

void CEditEx::OnSize(int cx, int cy)
{
	UpdateScrollInfo(true);
	UpdateScrollInfo(false);
}

void CEditEx::NewScrollBar()
{
	CScrollBarHandler	*pSb;

	pSb = new CScrollBarHandler;
//	pSb->Init(m_hWnd, SB_HORZ);
	m_pSbHorz = pSb;

	pSb = new CScrollBarHandler;
//	pSb->Init(m_hWnd, SB_VERT);
	m_pSbVert = pSb;
}

void CEditEx::DrawLine(IMLGraphicsBase *pGraph, COneLine *pLine, int xMax, int y)
{
	int		x = m_nMarginX - m_nScrollPosx;
	BYTE	byClrLast = 0;
	pGraph->SetTextColor(m_vClrTable[0]);
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
// 		if (glyph.width == GLYPH_WIDTH_INVALID)
// 		{
// 			SIZE		size;
// 			pGraph->GetTextExtentPoint32(glyph.szGlyph, glyph.nLen, &size);
// 			glyph.width = size.cx;
// 		}
		if (x + glyph.width < m_nMarginX)
		{
			x += glyph.width;
			continue;
		}
		if (x + glyph.width > xMax)
			break;
//		if (glyph.width)
		if (glyph.szGlyph[0] != '\t')
		{
			pGraph->TextOut(x, y, glyph.szGlyph, glyph.nLen);
		}
		x += glyph.width;
	}
}

void CEditEx::UpdateWidthInfoOfLine(IMLGraphicsBase *pGraph, COneLine *pLine)
{
	int		x = 0;
	SIZE	size;
	for (int k = 0; k < (int)pLine->size(); k++)
	{
		CGlyph	&glyph = (*pLine)[k];
		if (glyph.szGlyph[0] == '\t')
		{
			// tab
			glyph.width = m_nOneCharDx * 4 - x % (m_nOneCharDx * 4);
		}
		else
		{
			pGraph->GetTextExtentPoint32(glyph.szGlyph, glyph.nLen, &size);
			glyph.width = (BYTE)size.cx;
		}
		x += glyph.width;
	}
}

bool CEditEx::DoSetText(LPCTSTR szText)
{
	IMLGraphicsBase		*pGraph;
	COneLine			*pLine = NULL;

	pGraph = GetGraphics();

	pGraph->SetFont(&m_font);

	// delete old text
	Free();

	ClearSelection();

	m_nCaretRow = 0;
	m_nCaretCol = 0;

	pLine = NewOneLine();
	m_vLines.push_back(pLine);
	while (*szText)
	{
		CGlyph		glyph;

		szText = ReadUtf8Char(glyph, szText);
		glyph.clrIndex = CLR_INDEX_FG;
		if (glyph.szGlyph[0] == '\r')
		{
			if (glyph.szGlyph[0] == '\n')
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
		else if (glyph.szGlyph[0] == '\n')
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
	}

	OnNotifyParseLine(m_vLines.size() - 1);

	for (int i = 0; i < (int)m_vLines.size(); i++)
	{
		UpdateWidthInfoOfLine(pGraph, m_vLines[i]);
	}

	ReleaseGraphics(pGraph);

	return true;
}

void CEditEx::Free()
{
	m_nCaretX = 0;
	m_nCaretY = 0;
	m_nMarginX = 2;
	m_nMarginY = 2;
	m_nTopVisibleLine = 0;
	m_nBegSelCol = m_nEndSelCol = m_nBegSelRow = m_nEndSelRow = -1;
	m_nScrollPosx = 0;

	for (int i = 0; i < (int)m_vLines.size(); i++)
	{
		delete m_vLines[i];
	}
	m_vLines.clear();
}

void CEditEx::ClearWidth()
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

void CEditEx::DrawCaret(bool bShow)
{
	if (bShow)
	{
		//if (::GetFocus() != m_hWnd)
		//	return;

		if (!m_bCaretOn)
		{
			m_bCaretOn = true;
			// CreateCaret(m_hWnd, NULL, 2, GetLineDx());
			// ShowCaret(m_hWnd);
		}
		m_nCaretY = m_nMarginY + (m_nCaretRow - m_nTopVisibleLine) * GetLineDx();
		// SetCaretPos(m_nCaretX, m_nCaretY);
	}
	else
	{
		if (m_bCaretOn)
		{
			m_bCaretOn = false;
			// HideCaret(m_hWnd);
			// DestroyCaret();
		}
	}
}

void CEditEx::MakeCaretInSight()
{
	CRect		rc;

	GetClientRect(&rc);

	// rc.DeflateRect(m_nMarginX, m_nMarginY);
// 	if (m_nCaretRow < m_nTopVisibleLine)
// 		m_nTopVisibleLine = m_nCaretRow;
// 	else
// 	{
// 		m_nCaretY = m_nMarginY + (m_nCaretRow - m_nTopVisibleLine) * GetLineDx();
// 		if (m_nCaretY >= rc.bottom - m_nMarginY)
// 		{
// 			m_nTopVisibleLine = m_nCaretRow;
// 		}
// 	}

	if (m_pSbVert->IsEnabled())
	{
		if (m_nCaretRow - m_nTopVisibleLine == m_pSbVert->GetPage())
			m_nTopVisibleLine++;
		else if (m_nTopVisibleLine - m_nCaretRow == 1)
			m_nTopVisibleLine--;
		else if (m_nCaretRow - m_nTopVisibleLine > m_pSbVert->GetPage())
		{
			m_nTopVisibleLine += (m_nCaretRow - m_nTopVisibleLine) - (m_nCaretRow - m_nTopVisibleLine) % m_pSbVert->GetPage();
		}
		else if (m_nCaretRow < m_nTopVisibleLine)
		{
			m_nTopVisibleLine -= (m_nTopVisibleLine - m_nCaretRow) - (m_nTopVisibleLine - m_nCaretRow) % m_pSbVert->GetPage();
			if (m_nCaretRow < m_nTopVisibleLine)
				m_nTopVisibleLine -= m_pSbVert->GetPage();
			if (m_nTopVisibleLine < 0)
				m_nTopVisibleLine = 0;
		}
	}

	if (m_pSbVert->IsEnabled())
	{
		if (m_nTopVisibleLine > m_pSbVert->GetMax())
			m_nTopVisibleLine = m_pSbVert->GetMax();
	}
	else
		m_nTopVisibleLine = 0;

	m_nCaretY = m_nMarginY + (m_nCaretRow - m_nTopVisibleLine) * GetLineDx();

	COneLine		*pLine;
	int				nNextCaretCharX;

	pLine = m_vLines[m_nCaretRow];
	nNextCaretCharX = m_nCaretX = m_nMarginX;
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


 	if (nNextCaretCharX - m_nScrollPosx >= rc.right - m_nMarginX)
 	{
 		// horz scroll +
 		m_nScrollPosx = nNextCaretCharX - (rc.right - m_nMarginX - 16 * m_nOneCharDx);
		if (nNextCaretCharX - m_nScrollPosx <= 0)
			m_nScrollPosx = nNextCaretCharX - rc.Width() / 2 - m_nMarginX;
		if (m_nScrollPosx > m_pSbHorz->GetMax())
			m_nScrollPosx = m_pSbHorz->GetMax();
 	}
 	if (nNextCaretCharX < m_nScrollPosx + m_nMarginX)
 	{
 		// horz scroll -
 		m_nScrollPosx = nNextCaretCharX - m_nMarginX - 16 * m_nOneCharDx;
 		if (m_nScrollPosx < 0)
 			m_nScrollPosx = 0;
 	}

 	m_nCaretX -= m_nScrollPosx;

	DrawCaret(true);

	m_pSbVert->SetScrollPos(m_nTopVisibleLine);
	m_pSbHorz->SetScrollPos(m_nScrollPosx);
}

int CEditEx::GetEndColOfLine(int nLine)
{
	ASSERT(nLine >=0 && nLine < (int)m_vLines.size());

	COneLine		*pLine;

	pLine = m_vLines[nLine];
	return pLine->size();
// 	if (pLine->size())
// 		return pLine->size() - 1;
// 	else
// 		return 0;
}

void CEditEx::SetCaretToPos(int x, int y)
{
	COneLine		*pLine;
	int				i;

	m_nCaretRow = m_nTopVisibleLine + (y - m_nMarginY) / GetLineDx();
	ASSERT(!m_vLines.empty());
	if (m_nCaretRow >= (int)m_vLines.size())
		m_nCaretRow = m_vLines.size() - 1;
	else if (m_nCaretRow < 0)
		m_nCaretRow = 0;

	if (m_pSbVert->IsEnabled() && m_pSbVert->GetPage() <= m_pSbVert->GetMax())
	{
		if (m_nCaretRow - m_nTopVisibleLine == m_pSbVert->GetPage())
			m_nTopVisibleLine++;
		else if (m_nTopVisibleLine - m_nCaretRow == 1)
			m_nTopVisibleLine--;
		else if (m_nCaretRow - m_nTopVisibleLine > m_pSbVert->GetPage())
		{
			m_nTopVisibleLine += (m_nCaretRow - m_nTopVisibleLine) - (m_nCaretRow - m_nTopVisibleLine) % m_pSbVert->GetPage();
		}
		else if (m_nCaretRow < m_nTopVisibleLine)
		{
			m_nTopVisibleLine += (m_nCaretRow - m_nTopVisibleLine) - (m_nCaretRow - m_nTopVisibleLine) % m_pSbVert->GetPage();
		}
	}

	m_nCaretY = m_nMarginY + (m_nCaretRow - m_nTopVisibleLine) * GetLineDx();

	x += m_nScrollPosx;

	pLine = m_vLines[m_nCaretRow];
	m_nCaretX = m_nMarginX;
	for (i = 0; i < (int)pLine->size(); i++)
	{
		CGlyph	&glyph = (*pLine)[i];

		ASSERT(glyph.width != GLYPH_WIDTH_INVALID);
		if (m_nCaretX + glyph.width / 2 > x)
			break;
		m_nCaretX += glyph.width;
	}
	m_nCaretCol = i;
}

void CEditEx::NextPos(int &Row, int &nCol)
{
	if (nCol >= GetEndColOfLine(Row))
	{
		if (Row < (int)m_vLines.size() - 1)
		{
			Row++;
			nCol = 0;
		}
		else
		{
			nCol = GetEndColOfLine(Row);
		}
	}
	else
	{
		nCol++;
	}
}

void CEditEx::PrevPos(int &Row, int &nCol)
{
	if (nCol > 0)
		nCol--;
	else
	{
		if (Row > 0)
		{
			Row--;
			nCol = GetEndColOfLine(Row);
		}
	}
}

void CEditEx::NextWord(int &Row, int &nCol)
{
	bool			bAlpha, bBegAlpha;
	COneLine		*pLine;

	pLine = m_vLines[Row];

	if (nCol == pLine->size())
		bBegAlpha = false;
	else
		bBegAlpha = Tobool(_istalnum((*pLine)[nCol].szGlyph[0]));

	while (1)
	{
		pLine = m_vLines[Row];

		if (nCol == pLine->size())
			bAlpha = false;
		else
			bAlpha = Tobool(_istalnum((*pLine)[nCol].szGlyph[0]));

		if (bAlpha != bBegAlpha)
			break;

		if (nCol >= (int)pLine->size())
		{
			if (Row < (int)m_vLines.size() - 1)
			{
				Row++;
				nCol = 0;
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
		}
	}
}

void CEditEx::PrevWord(int &Row, int &nCol)
{
	bool			bAlpha, bBegAlpha;
	COneLine		*pLine;

	PrevPos(Row, nCol);

	pLine = m_vLines[Row];

	if (nCol == pLine->size())
		bBegAlpha = false;
	else
		bBegAlpha = Tobool(_istalnum((*pLine)[nCol].szGlyph[0]));

	while (1)
	{
		pLine = m_vLines[Row];

		if (nCol == pLine->size())
			bAlpha = false;
		else
			bAlpha = Tobool(_istalnum((*pLine)[nCol].szGlyph[0]));

		if (bAlpha != bBegAlpha)
		{
			NextWord(Row, nCol);
			break;
		}

		if (nCol > 0)
			nCol--;
		else
		{
			if (Row > 0)
			{
				Row--;
				nCol = m_vLines[Row]->size();
			}
			else
			{
				// begin of text
				return;
			}
		}
	}
}

bool CEditEx::IsSelected()
{
	return m_nBegSelRow != -1;
}

void CEditEx::ClearSelection()
{
	m_nBegSelRow = -1;
	OnNotifySelChanged();
}

void CEditEx::RemoveSelected(int *pnBegSelRow, int *pnBegSelCol)
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
		ClearSelection();
		return;
	}

	IMLGraphicsBase		*pGraph;
	// SIZE				size;

	pGraph = GetGraphics();
	pGraph->SetFont(&m_font);


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

	ReleaseGraphics(pGraph);

	m_nCaretRow = nBegSelRow;
	m_nCaretCol = nBegSelCol;

	ClearSelection();
}

void CEditEx::RemoveStr(int nRow, int nCol, int nSize)
{
	ASSERT(nRow >= 0 && nRow < (int)m_vLines.size());

	COneLine			*pLine = NULL;
	IMLGraphicsBase		*pGraph;
	// SIZE				size;

	pGraph = GetGraphics();
	pGraph->SetFont(&m_font);

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

	ReleaseGraphics(pGraph);
}

void CEditEx::RemoveChar(int nRow, int nCol)
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

	IMLGraphicsBase		*pGraph;
	// SIZE				size;

	pGraph = GetGraphics();
	pGraph->SetFont(&m_font);

	UpdateWidthInfoOfLine(pGraph, pLine);

	ReleaseGraphics(pGraph);
}

void CEditEx::InsertChar(int nRow, int nCol, const char szInsert[])
{
	ASSERT(nRow >= 0 && nRow < (int)m_vLines.size());

	COneLine		*pLine = m_vLines[nRow];
	ASSERT(nCol >= 0 && nCol <= (int)pLine->size());

	CGlyph		glyph;

	IMLGraphicsBase		*pGraph;
	// SIZE				size;

	pGraph = GetGraphics();
	pGraph->SetFont(&m_font);

// 	if (chInsert != '\t')
// 	{
// 		pGraph->GetTextExtentPoint32(&chInsert, 1, &size);
// 		glyph.width = size.cx;
// 	}
// 	else
	ReadUtf8Char(glyph, szInsert);
	glyph.clrIndex = CLR_INDEX_FG;

	if (nCol == pLine->size())
	{
		pLine->insert(pLine->end(), glyph);
	}
	else
		pLine->insert(pLine->begin() + nCol, glyph);

	OnNotifyParseLine(nRow);

	UpdateWidthInfoOfLine(pGraph, pLine);
	ReleaseGraphics(pGraph);
}

void CEditEx::InsertStr(int nRow, int nCol, LPCTSTR szText, int &nRowNext, int &nColNext)
{
	IMLGraphicsBase		*pGraph;

	pGraph = GetGraphics();
	pGraph->SetFont(&m_font);

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

		szText = ReadUtf8Char(glyph, szText);
		glyph.clrIndex = CLR_INDEX_FG;
		if (glyph.szGlyph[0] == '\r')
		{
// 			glyph.width = 0;
// 			pLine->push_back(glyph);
			szText++;
			UpdateWidthInfoOfLine(pGraph, pLine);
			if (glyph.szGlyph[0] == '\n')
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
		else if (glyph.szGlyph[0] == '\n')
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
	}

	pLine->newLineType = nltInsertLine;

	nColNext = pLine->size();

	pLine->insert(pLine->end(), leftGlyphOfLine.begin(), leftGlyphOfLine.end());

	OnNotifyParseLine(nRowNext);

	UpdateWidthInfoOfLine(pGraph, pLine);

	ReleaseGraphics(pGraph);

}

void CEditEx::UpdateScrollInfo(bool bHorz)
{
	CRect			rc;

	GetClientRect(&rc);
	rc.DeflateRect(m_nMarginX, m_nMarginY);

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

		nPage = rc.Width() - m_nMarginX * 2;
		if (nPage <= 0)
			nPage = 1;

		if (wMax > nPage)
			m_pSbHorz->SetScrollInfo(0, wMax, nPage, m_nScrollPosx, m_nOneCharDx);
		else
		{
			m_nScrollPosx = 0;
			m_pSbHorz->DisableScrollBar();
		}
	}
	else
	{
		int		nPage;

		nPage = rc.Height() / GetLineDx();
		if (nPage <= 0)
			nPage = 2;

		if ((int)m_vLines.size() > nPage)
			m_pSbVert->SetScrollInfo(0, m_vLines.size() - 1, nPage, m_nTopVisibleLine);
		else
			m_pSbVert->DisableScrollBar();
	}
}

void CEditEx::OnSetFocus()
{
	CRect		rc;

	GetClientRect(&rc);
	rc.DeflateRect(m_nMarginX, m_nMarginY);

	if (rc.PtInRect(CPoint(m_nCaretX, m_nCaretY)))
		DrawCaret(true);
}

void CEditEx::OnKillFocus()
{
	m_bInMouseSel = false;

//	if (::GetCapture() == m_hWnd)
//	{
//		ReleaseCapture();
//	}

	DrawCaret(false);
}

void CEditEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	// bool shift = (GetKeyState(VK_SHIFT) & 0xf000) == 0xf000;
	bool shift = false;
	CAutoUpdate		updater(this);

	SetFocus();

	m_bInMouseSel = false;

	if (shift && !IsSelected())
	{
		// Select text from the current caret.
		m_nBegSelRow = m_nCaretRow;
		m_nBegSelCol = m_nCaretCol;
	}

	SetCapture();
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
		ClearSelection();
	}
}

void CEditEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	m_bInMouseSel = false;
	if (m_nBegSelRow == m_nEndSelRow && m_nBegSelCol == m_nEndSelCol)
		m_nBegSelRow = -1;

	OnNotifySelChanged();
}

void CEditEx::OnMouseMove(UINT nFlags, CPoint point)
{
	if (nFlags & MK_LBUTTON )
	{
		// bool ctrl = (GetKeyState(VK_CONTROL) & 0xf000) == 0xf000;
		// bool shift = (GetKeyState(VK_SHIFT) & 0xf000) == 0xf000;
		bool shift = false;
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

			DrawCaret(true);

			// Select text to the current caret now.
			m_nEndSelRow = m_nCaretRow;
			m_nEndSelCol = m_nCaretCol;
			return;
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
		DrawCaret(true);
	}
}

void CEditEx::OnRButtonUp(UINT nFlags, CPoint point)
{
// 	tstring		str;
// 	GetWindowText(str);
// 	DBGLOG1(_T("%s"), str.c_str());
	MLWidget::GetCursorPos(&point);
	OnContexMenu(point.x, point.y);
}

void CEditEx::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	int			nOffset;

	nOffset = nWheelDistance / 120;
	if (IsFlagSet(nMkeys, MK_SHIFT))
	{
		// horz scroll bar
		nOffset *= m_nOneCharDx;
		m_pSbHorz->SetScrollPos(m_pSbHorz->GetScrollPos() - nOffset, true);

		if (m_nScrollPosx != m_pSbHorz->GetScrollPos())
		{
			m_nScrollPosx = m_pSbHorz->GetScrollPos();
			InvalidateRect();
		}
	}
	else
	{
		m_pSbVert->SetScrollPos(m_pSbVert->GetScrollPos() - nOffset, true);

		if (m_nTopVisibleLine != m_pSbVert->GetScrollPos())
		{
			m_nTopVisibleLine = m_pSbVert->GetScrollPos();
			InvalidateRect();
		}
	}



	// WHEEL_DELTA = 120
/*	n = abs(nWheelDistance / 120) + 2;
	for (int i = 0; i < n; i++)
	{
		if (IsFlagSet(nMkeys, MK_SHIFT))
		{
			if (m_pSbHorz->IsEnabled())
			{
				if (nWheelDistance < 0)
					OnHScroll(SB_LINEDOWN, 0, NULL);
				else
					OnHScroll(SB_LINEUP, 0, NULL);
			}
		}
		else
		{
			if (m_pSbVert->IsEnabled())
			{
				if (nWheelDistance < 0)
					OnVScroll(SB_LINEDOWN, 0, NULL);
				else
					OnVScroll(SB_LINEUP, 0, NULL);
			}
		}
	}*/
}

void CEditEx::OnKeyDown(UINT nChar, UINT nFlags)
{
// 	bool ctrl = (GetKeyState(VK_CONTROL) & 0xf000) == 0xf000;
// 	bool shift = (GetKeyState(VK_SHIFT) & 0xf000) == 0xf000;
	bool ctrl = false;
	bool shift = false;

	bool			bSelKey = false;
	int				nBegSelRow, nBegSelCol;
	CAutoUpdate		updater(this);

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
				m_nCaretCol = GetEndColOfLine(m_nCaretRow);
			}
			else
			{
				// end of line
				m_nCaretCol = GetEndColOfLine(m_nCaretRow);
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
				m_nCaretCol = 0;
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
				// m_nCaretCol--;
				SetCaretToPos(m_nCaretX, m_nCaretY - GetLineDx());
			}
			break;
		}
	case VK_DOWN: // down arrow
		{
			bSelKey = true;
			if (m_nCaretRow < (int)m_vLines.size() - 1)
			{
				// m_nCaretCol++;
				SetCaretToPos(m_nCaretX, m_nCaretY + GetLineDx());
			}
			break;
		}
	case VK_PRIOR: // page up
		{
			bSelKey = true;
			if (m_nCaretRow > 0 && m_pSbVert->IsEnabled())
			{
				// m_nCaretCol++;
				SetCaretToPos(m_nCaretX, m_nCaretY - GetLineDx() * (m_pSbVert->GetPage() - 1));
				m_nTopVisibleLine -= m_pSbVert->GetPage() - 1;
				if (m_nTopVisibleLine < 0)
					m_nTopVisibleLine = 0;
			}
			break;
		}
	case VK_NEXT: // page down
		{
			bSelKey = true;
			if (m_nCaretRow < (int)m_vLines.size() && m_pSbVert->IsEnabled())
			{
				// m_nCaretCol++;
				SetCaretToPos(m_nCaretX, m_nCaretY + GetLineDx() * (m_pSbVert->GetPage() - 1));
				m_nTopVisibleLine += m_pSbVert->GetPage() - 1;
				if (m_nTopVisibleLine > m_pSbVert->GetMax())
				{
					m_nTopVisibleLine = m_pSbVert->GetMax();
				}
			}
			break;
		}
	case VK_BACK:
		{
			if (!m_bReadOnly)
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
				UpdateScrollInfo(true);
				UpdateScrollInfo(false);
			}
			break;
		}
	case VK_DELETE:
		{
			if (!m_bReadOnly)
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
			if (!m_bReadOnly)
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
				UpdateScrollInfo(true);
				UpdateScrollInfo(false);
			}
			break;
		}
	default:
		CBaseWnd::OnKeyDown(nChar, nFlags);
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
			ClearSelection();
	}

	// Where is Caret? Make it in sight
	MakeCaretInSight();
}

void CEditEx::OnChar(UINT nChar)
{
	// CStrPrintf		strp(_T("%x -"), wUniChar);
	TCHAR			wUniChar;

	if (nChar <= 127 && nChar != '\t' && !isprint(nChar))
 		return;

	wUniChar = nChar;

	CAutoUpdate		updater(this);

	if (IsSelected())
	{
		int				nBegSelRow, nBegSelCol;
		tstring			strToDel;

		SelectionToText(strToDel);
		RemoveSelected(&nBegSelRow, &nBegSelCol);
		m_undoMgr.AddAction(new CEditRepAction(this, nBegSelRow, nBegSelCol, strToDel.c_str(), (WCHAR)wUniChar));
		// InsertChar(nBegSelRow, nBegSelCol, (WCHAR)wUniChar);
	}
	else
	{
		m_undoMgr.AddAction(new CEditInsertAction(this, m_nCaretRow, m_nCaretCol, (WCHAR)wUniChar));
		// InsertChar(m_nCaretRow, m_nCaretCol, (WCHAR)wUniChar);
	}

	updater.SetUpdateFlag(true);
	NextPos(m_nCaretRow, m_nCaretCol);

	// Where is Caret? Make it in sight
	UpdateScrollInfo(true);
	MakeCaretInSight();
}

void CEditEx::OnContexMenu(int xPos, int yPos)
{
// 	if (IsSelected())
// 	{
// 		EnableMenuItem(m_hMenuContext, IDC_EX_CUT, MF_ENABLED | MF_BYCOMMAND);
// 		EnableMenuItem(m_hMenuContext, IDC_EX_COPY, MF_ENABLED | MF_BYCOMMAND);
// 	}
// 	else
// 	{
// 		EnableMenuItem(m_hMenuContext, IDC_EX_CUT, MF_GRAYED | MF_BYCOMMAND);
// 		EnableMenuItem(m_hMenuContext, IDC_EX_COPY, MF_GRAYED | MF_BYCOMMAND);
// 	}
// 
// 	if (m_undoMgr.CanUndo())
// 		EnableMenuItem(m_hMenuContext, IDC_EX_UNDO, MF_ENABLED | MF_BYCOMMAND);
// 	else
// 		EnableMenuItem(m_hMenuContext, IDC_EX_UNDO, MF_GRAYED | MF_BYCOMMAND);
// 
// 	if (m_undoMgr.CanRedo())
// 		EnableMenuItem(m_hMenuContext, IDC_EX_REDO, MF_ENABLED | MF_BYCOMMAND);
// 	else
// 		EnableMenuItem(m_hMenuContext, IDC_EX_REDO, MF_GRAYED | MF_BYCOMMAND);
// 
//	TrackPopupMenu(m_hMenuContext, TPM_RIGHTBUTTON | TPM_LEFTALIGN, xPos, yPos, 0, m_hWnd, NULL);
}

void CEditEx::OnCommand(UINT uID, UINT nNotifyCode)
{
	switch (uID)
	{
	case IDC_EX_UNDO:
		OnCmdUndo();
		break;
	case IDC_EX_REDO:
		OnCmdRedo();
		break;
	case IDC_EX_CUT:
		OnCmdCut();
		break;
	case IDC_EX_COPY:
		OnCmdCopy();
		break;
	case IDC_EX_PASTE:
		OnCmdPaste();
		break;
	case IDC_EX_DELETE:
		OnCmdDelete();
		break;
	}
}

void CEditEx::OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	if (m_pSbVert->HandleScrollCode(nSBCode, nPos))
	{
		CAutoUpdate		updater(this);

		// changes...
		m_nTopVisibleLine = m_pSbVert->GetScrollPos();

		if (m_nCaretRow - m_nTopVisibleLine >= m_pSbVert->GetPage()
			|| m_nTopVisibleLine > m_nCaretRow)
			DrawCaret(false);
		else
			DrawCaret(true);

		updater.SetUpdateFlag(true);
	}
}

void CEditEx::OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	if (m_pSbHorz->HandleScrollCode(nSBCode, nPos))
	{
		CAutoUpdate		updater(this);

		// changes...
		m_nCaretX += m_nScrollPosx - m_pSbHorz->GetScrollPos();
		m_nScrollPosx = m_pSbHorz->GetScrollPos();

		if (m_nCaretX - m_nScrollPosx - m_nMarginX >= m_pSbHorz->GetPage()
			|| m_nCaretX - m_nScrollPosx < m_nMarginX)
			DrawCaret(false);
		else
			DrawCaret(true);

		updater.SetUpdateFlag(true);
	}
}
