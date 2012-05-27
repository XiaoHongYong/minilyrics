// EditEx.h: interface for the CEditEx class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "ScrollBarHandler.h"
#include "MLMenu.h"

class CEditEx;

class IEditSyntaxParser
{
public:
	virtual ~IEditSyntaxParser() { }
	virtual void Init(CEditEx *pEdit) = 0;
	virtual void OnNotifyParseLine(int nLine) = 0;

};

class IEditNotification
{
public:
	enum Status
	{
		S_CAN_UNDO,
		S_CAN_REDO,
		S_SEL,
	};

	virtual void OnStatusChanged(Status status, bool bVal) = 0;
	virtual void OnTextChanged() = 0;

};

class CUndoAction
{
public:
	CUndoAction() {}
	virtual ~CUndoAction() {}
	virtual void Undo() = 0;
	virtual void Redo() = 0;

};

class CBatchUndoAction : public CUndoAction
{
public:
	typedef vector<CUndoAction*>		LIST_ACTIONS;

	CBatchUndoAction();
	virtual ~CBatchUndoAction();

	virtual void Undo();
	virtual void Redo();
	virtual void AddAction(CUndoAction *pAction);

	bool IsEmpty();

protected:
	LIST_ACTIONS			m_listActions;

};

class CUndoMgr
{
public:
	typedef vector<CUndoAction*>		LIST_ACTIONS;

	CUndoMgr();
	~CUndoMgr();

	void Init(CEditEx *pEdit);

	void BeginBatchAction();
	void EndBatchAction();

	virtual void AddAction(CUndoAction *pAction);

	virtual bool Undo();
	virtual bool Redo();

	virtual bool CanUndo();
	virtual bool CanRedo();

	virtual void Clear();

protected:
	void SendChangeNotifyMsg();

	void OnDetectUndoStatus();
	void OnDetectRedoStatus();

	void DoAddAction(CUndoAction *pAction);

protected:
	LIST_ACTIONS			m_listActions;
	int						m_nPosUndo;
	CEditEx					*m_pEdit;

	CBatchUndoAction		*m_pBatchUndoAction;

	bool					m_bCanUndoPrev, m_bCanRedoPrev;

};

class CEditEx : public CBaseWnd, public IScrollNotify
{
public:
	class CAutoUpdate
	{
	public:
		CAutoUpdate(CEditEx *pTarg)
		{
			m_pTarget = pTarg;
			m_bUpdate = false;
			SaveOrg();
		}
		~CAutoUpdate()
		{
			DoUpdateNow();
		}
		void SetUpdateFlag(bool bUpdate) { m_bUpdate = bUpdate; }

		void DoUpdateNow()
		{
			if (!m_pTarget)
				return;

			if (m_bUpdate)
			{
				m_bUpdate = false;
				SaveOrg();
				m_pTarget->InvalidateRect(NULL, true);
				return;
			}

			if (m_nTopVisibleLineOld != m_pTarget->m_nTopVisibleLine ||
				m_nScrollPosxOld != m_pTarget->m_nScrollPosx)
			{
				SaveOrg();
				m_pTarget->InvalidateRect(NULL, true);
				return;
			}

			if (m_nCaretRowOld != m_pTarget->m_nCaretRow)
			{
				SaveOrg();
				m_pTarget->InvalidateRect(NULL, true);
				return;
			}

			if (m_nBegSelRowOld == -1 && m_pTarget->m_nBegSelRow == -1)
			{
				SaveOrg();
				return;
			}

			if (m_nBegSelRowOld != m_pTarget->m_nBegSelRow ||
				m_nEndSelRowOld != m_pTarget->m_nEndSelRow ||
				m_nBegSelColOld != m_pTarget->m_nBegSelCol ||
				m_nEndSelColOld != m_pTarget->m_nEndSelCol)
			{
				SaveOrg();
				m_pTarget->InvalidateRect(NULL, true);
				return;
			}
		}
	protected:
		void SaveOrg()
		{
			m_nTopVisibleLineOld = m_pTarget->m_nTopVisibleLine;
			m_nScrollPosxOld = m_pTarget->m_nScrollPosx;
			m_nBegSelRowOld = m_pTarget->m_nBegSelRow;
			m_nEndSelRowOld = m_pTarget->m_nEndSelRow;
			m_nBegSelColOld = m_pTarget->m_nBegSelCol;
			m_nEndSelColOld = m_pTarget->m_nEndSelCol;
			m_nCaretRowOld = m_pTarget->m_nCaretRow;
		}

	protected:
		CEditEx		*m_pTarget;
		bool		m_bUpdate;
		int			m_nTopVisibleLineOld;
		int			m_nScrollPosxOld;
		int			m_nBegSelRowOld, m_nEndSelRowOld;
		int			m_nBegSelColOld, m_nEndSelColOld;
		int			m_nCaretRowOld;

	};

	struct CGlyph
	{
		BYTE		width;
		BYTE		clrIndex;	// color index in CEditEx color table.
		char		szGlyph[3];	// utf-8 char.
		byte		nLen;

		bool IsEqual(TCHAR ch) { return szGlyph[0] == ch; }
		bool Cmp(const char *str);
		bool iCmp(const char *str);
		bool ReverseCmp(const char *str);
		bool ReverseiCmp(const char *str);
		int Length() { return nLen; }
		void TextOut(IMLGraphicsBase *pGraph, int x, int y) { pGraph->TextOut(x, y, szGlyph, nLen); }
	};
	enum
	{
		CLR_INDEX_FG		 = 0
	};
	enum NewLineType
	{
		NLT_NONE,
		NLT_RN,
		NLT_N,
		NLT_R,
	};
	class COneLine : public vector<CGlyph>
	{
	public:
		COneLine()
		{
			newLineType = NLT_NONE;
// 			bLineUndeletable = false;
// 			wReadOnlyEndPos = 0;
		}
		int GetReturnSize()
		{
			switch (newLineType)
			{
			case NLT_N:
			case NLT_R:
				return 1;
			case NLT_RN:
				return 2;
			}
			return 0;
		}

		static LPCTSTR GetReturnStr(NewLineType nlt)
		{
			switch (nlt)
			{
			case NLT_N: return _T("\n");
			case NLT_R: return _T("\r");
			case NLT_RN: return _T("\r\n");
			}
			return _T("");
		}

		NewLineType		newLineType;
// 		WORD			wReadOnlyEndPos;	// readonly begin pos is 0.
// 		bool			bLineUndeletable;	// unable to delete this line.
	};

	// typedef vector<CGlyph>		COneLine;
	typedef vector<COneLine*>	CVLines;

public:
	CEditEx();
	virtual ~CEditEx();

	BOOL Create(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	virtual bool OnCreate();
	virtual void OnDestroy();

	virtual void SetEditSyntaxParser(IEditSyntaxParser *pEditSyntaxParser) { m_pEditSyntaxParser = pEditSyntaxParser; }

	virtual void SetEditNotification(IEditNotification *pEditNotification) { m_pEditNotification = pEditNotification; }

	void SetSel(int nBeg, int nEnd);
	void ReplaceSel(LPCTSTR szText);
	void RemoveSel();

	void SetCaret(int nRow, int nCol);

	int GetLength();

	void SetFont(CMLFont &font, const MLColor &clrText);
	void SetBgColor(const MLColor &clr);
	void SetSelColor(const MLColor &clrSelText, const MLColor &clrSelBg);

	bool SetWindowText(LPCTSTR szText);

	int GetWindowText(LPTSTR szString, int nMaxCount);
	int GetWindowText(tstring &str);

	bool GetTextOfLine(int nLine, tstring &strLine);

	void BeginBatchUndo() { m_undoMgr.BeginBatchAction(); }
	void EndBatchUndo() { m_undoMgr.EndBatchAction(); }

	bool IsSelected();

	bool CanUndo();
	bool CanRedo();

	void OnCmdCopy();
	void OnCmdCut();
	void OnCmdPaste();
	void OnCmdRedo();
	void OnCmdUndo();
	void OnCmdDelete();

public:
	// interface for expanding
	virtual COneLine *NewOneLine() { return new COneLine; }

protected:
	virtual void NewScrollBar();

	void DrawLine(IMLGraphicsBase *pGraph, COneLine *pLine, int xMax, int y);

	bool DoSetText(LPCTSTR szText);
	void UpdateWidthInfoOfLine(IMLGraphicsBase *pGraph, COneLine *pLine);

	void Free();
	void ClearWidth();

	int GetLineDx() { return (m_nLineSpace + m_font.GetHeight()); }

	void DrawCaret(bool bShow);

	void MakeCaretInSight();

	int GetEndColOfLine(int nLine);

	void SetCaretToPos(int x, int y);

	void NextPos(int &Row, int &nCol);
	void PrevPos(int &Row, int &nCol);
	void NextWord(int &Row, int &nCol);
	void PrevWord(int &Row, int &nCol);

	void ClearSelection();

	void RemoveSelected(int *pnBegSelRow = NULL, int *pnBegSelCol = NULL);
	void RemoveStr(int nRow, int nCol, int nSize);
	void RemoveChar(int nRow, int nCol);

	void InsertChar(int nRow, int nCol, const char szInsert[]);
	void InsertStr(int nRow, int nCol, LPCTSTR szText, int &nRowNext, int &nColNext);

	void UpdateScrollInfo(bool bHorz);

	void SelectionToText(tstring &str);

	void GetStrAtPos(int nRow, int nCol, tstring &str);

	bool IsPosValid(int nRow, int nCol);

	void SortSelectPos(int &nBegSelRow, int &nBegSelCol, int &nEndSelRow, int &nEndSelCol, bool &bSelectedEmpty);

protected:
	void OnPaint(CMLGraphics *pGraph, CRect &rcClip);
	void OnSize(int cx, int cy);

	virtual void OnSetFocus();
	virtual void OnKillFocus();

	virtual void OnLButtonUp(UINT nFlags, CPoint point);
	virtual void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnRButtonUp(UINT nFlags, CPoint point);
	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);
	virtual void OnKeyDown(UINT nChar, UINT nFlags);
	virtual void OnChar(UINT nChar);

	virtual void OnContexMenu(int xPos, int yPos);
	virtual void OnCommand(UINT uID, UINT nNotifyCode);

	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);

	void OnNotifyParseLine(int nLine)
	{
		if (m_pEditSyntaxParser)
			m_pEditSyntaxParser->OnNotifyParseLine(nLine);
	}
	void OnNotifySelChanged()
	{
		bool	bSelected = m_nBegSelRow != -1;
		if (m_bPrevSelectedStatus != bSelected)
		{
			m_bPrevSelectedStatus = bSelected;
			if (m_pEditNotification)
				m_pEditNotification->OnStatusChanged(IEditNotification::S_SEL, bSelected);
		}
	}

	friend class CAutoUpdate;
	friend class CEditInsertAction;
	friend class CEditDelAction;
	friend class CUndoMgr;
protected:
	CVLines				m_vLines;

	// scroll bar info
	IScrollBar			*m_pSbHorz, *m_pSbVert;
	// CScrollBarHandler	m_sihHorz, m_sihVert;
	// SCROLLINFO			m_siHorz, m_siVert;
	int					m_nTopVisibleLine;
	int					m_nScrollPosx;

	bool				m_bCaretOn;
	int					m_nCaretX, m_nCaretY;
	int					m_nCaretRow, m_nCaretCol;

	int					m_nMarginX, m_nMarginY;
	int					m_nLineSpace;

	CMLFont				m_font;
	MLColor				m_clrBk;
	MLColor				m_clrSelText, m_clrSelBg;
	CMLBrush			m_brSelBg, m_brBk;

	vector<MLColor>		m_vClrTable;

	int					m_nBegSelRow, m_nEndSelRow;
	int					m_nBegSelCol, m_nEndSelCol;
	bool				m_bInMouseSel;		// is mouse select text?

	bool				m_bReadOnly;
	int					m_nOneCharDx;

	// undo
	CUndoMgr			m_undoMgr;

	CMemGraphics		m_dcMem;

	// for win9x, process MBCS input...
	char				m_szMbcsA[2];		// first half part of MBCS char.
	bool				m_bMbcsA;
	bool				m_bWin9x;

	CMLMenu				m_menu;

	IEditSyntaxParser	*m_pEditSyntaxParser;
	IEditNotification	*m_pEditNotification;
	bool				m_bPrevSelectedStatus;

};
