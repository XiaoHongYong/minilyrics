
#pragma once

#include "SkinScrollFrameCtrlBase.h"

class CSkinEditCtrl;

//
// Editor syntax (color) parser
//
class IEditSyntaxParser
{
public:
	virtual ~IEditSyntaxParser() { }
	virtual void Init(CSkinEditCtrl *pEdit) = 0;
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

	enum SpecialKey
	{
		SK_ENTER,
		SK_CTRL_ENTER,
		SK_SHIFT_ENTER,
		SK_ESCAPE,
		SK_TAB,
	};

	virtual void OnStatusChanged(Status status, bool bVal) { }
	virtual void OnTextChanged() { }

	virtual void OnSpecialKey(SpecialKey key) { }

};

#define CARET_EXTRA_EXPAND		2

class CSkinCaret
{
public:
	CSkinCaret()
	{
		m_bOldDrawed = false;
		m_rcCaret.SetRect(0, 0, 0, 0);
		m_bOn = false;
		m_clrCaret.set(RGB(255, 255, 255));
		m_xOffset = 0;
		m_yOffset = 0;
	}

	virtual ~CSkinCaret() { }

	bool IsOn() { return m_bOn; }

	bool IsDrawed() { return m_bOldDrawed; }

	virtual void DrawFlash(CRawGraph *pGraph)
	{
		if (!m_bOn)
			return;

		pGraph->FillRectXOR(&m_rcCaret, m_clrCaret);
		m_bOldDrawed = !m_bOldDrawed;
	}

	virtual void Draw(CRawGraph *pGraph)
	{
		if (!m_bOn)
			return;

		if (m_bOldDrawed)
			pGraph->FillRectXOR(&m_rcCaret, m_clrCaret);
	}

	virtual void ShowCaret(int x, int y, int nWidth, int nHeight)
	{
		m_bOn = true;
		x += m_xOffset;
		y += m_yOffset;
		m_rcCaret.SetRect(x, y, x + nWidth, y + nHeight + CARET_EXTRA_EXPAND);
	}

	void HideCaret()
	{
		m_bOn = false;
		m_bOldDrawed = false;
	}

	void SetOffset(int xOffset, int yOffset)
	{
		if (xOffset != m_xOffset || m_yOffset != yOffset)
		{
			m_rcCaret.OffsetRect(xOffset - m_xOffset, yOffset - m_yOffset);
			m_xOffset = xOffset;
			m_yOffset = yOffset;
		}
	}

	const CRect & GetUpdateRect() { return m_rcCaret; }

protected:
	bool			m_bOn;
	MLColor			m_clrCaret;

	CRect			m_rcCaret;

	bool			m_bOldDrawed;
	int				m_xOffset, m_yOffset;

};


class CSkinEditCtrl : public CSkinScrollFrameCtrlBase, public IUndoMgrNotify
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinScrollFrameCtrlBase)
public:
	enum ColorName
	{
		CN_BG,
		CN_TEXT,
		CN_SEL_BG,
		CN_SEL_TEXT,
		CN_EDIT_LINE_BG,
		CN_EDIT_LINE_TEXT,
		CN_CUSTOMIZED_START,

		CN_MAX = 255
	};

	enum
	{
		TIMER_DURATION_CARET		= 600
	};

public:
	class CAutoUpdate
	{
	public:
		CAutoUpdate(CSkinEditCtrl *pTarg)
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

			if (m_pTarget->IsInBatchAction())
				return;

			if (m_bUpdate)
			{
				m_bUpdate = false;
				SaveOrg();
				m_pTarget->Invalidate();
				return;
			}

			if (m_nTopVisibleLineOld != m_pTarget->m_nTopVisibleLine ||
				m_nScrollPosxOld != m_pTarget->m_nScrollPosx)
			{
				SaveOrg();
				m_pTarget->Invalidate();
				return;
			}

			if (m_nCaretRowOld != m_pTarget->m_nCaretRow)
			{
				SaveOrg();
				m_pTarget->Invalidate();
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
				m_pTarget->Invalidate();
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
		CSkinEditCtrl*m_pTarget;
		bool		m_bUpdate;
		int			m_nTopVisibleLineOld;
		int			m_nScrollPosxOld;
		int			m_nBegSelRowOld, m_nEndSelRowOld;
		int			m_nBegSelColOld, m_nEndSelColOld;
		int			m_nCaretRowOld;

	};

	friend class CAutoUpdate;

	class CAutoBatchUndo
	{
	public:
		CAutoBatchUndo(CSkinEditCtrl *pTarg) : m_update(pTarg)
		{
			m_pTarget = pTarg;

			m_pTarget->m_undoMgr.BeginBatchAction();
		}

		~CAutoBatchUndo()
		{
			EndBatchUndo();
		}

		void EndBatchUndo()
		{
			m_pTarget->m_undoMgr.EndBatchAction();
			m_pTarget->UpdateScrollInfo(true);
			m_pTarget->UpdateScrollInfo(false);
			m_pTarget->MakeCaretInSight();
			m_update.SetUpdateFlag(true);
			m_update.DoUpdateNow();
		}

	protected:
		CSkinEditCtrl		*m_pTarget;
		CAutoUpdate			m_update;

	};

	friend class CAutoBatchUndo;

	struct CGlyph
	{
		BYTE		width;
		BYTE		clrIndex;	// color index in CSkinEditCtrl color table.
		StringIterator::CharType	chGlyph;

		bool IsEqual(TCHAR ch) { return chGlyph == ch; }
		bool Cmp(const TCHAR *str) { return chGlyph == *str; }
		bool iCmp(const TCHAR *str);
		bool ReverseCmp(const TCHAR *str) { return chGlyph == *str; }
		bool ReverseiCmp(const TCHAR *str);
		int Length() { return 1; }

		// WCHAR		chGlyph;
		// bool Cmp(const TCHAR *str) { return chGlyph == *str; }
		// bool iCmp(const TCHAR *str);
		// bool ReverseCmp(const TCHAR *str) { return chGlyph == *str; }
		// bool ReverseiCmp(const TCHAR *str);
		// int Length() { return 1; }
		// void TextOut(CRawGraph *pGraph, int x, int y) { pGraph->TextOut(x, y, &chGlyph, 1); }
	};

	enum Style
	{
		S_MULTILINE			= 1,			// Multiple line editor
		S_READ_ONLY			= 1 << 0x1,		// The text can't be modified if this flag is set.
		S_PASSWORD			= 1 << 0x2,
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
	};

	typedef vector<COneLine*>	CVLines;

public:
	CSkinEditCtrl();
	virtual ~CSkinEditCtrl();

	//
	// Editor APIs
	//

	void SetEditSyntaxParser(IEditSyntaxParser *pEditSyntaxParser) { m_pEditSyntaxParser = pEditSyntaxParser; }

	void SetEditNotification(IEditNotification *pEditNotification) { m_pEditNotification = pEditNotification; }

	void ReplaceSel(LPCTSTR szText);
	void RemoveSel();

	void SetSel(int nBeg, int nEnd);
	void SetSel(int nBegLine, int nBegCol, int nEndLine, int nEndCol);
	void SetSelOfLine(int nLine, int nCol, int nNumOfSel);

	void SetCaret(int nRow, int nCol);
	void GetCaret(int &nLine, int &nCol) { nLine = m_nCaretRow; nCol = m_nCaretCol; }

	void OnFontChanged();
	void SetSelColor(const MLColor &clrSelText, const MLColor &clrSelBg)
	{ SetColor(CN_SEL_TEXT, clrSelText); SetColor(CN_SEL_BG, clrSelBg); }

	void SetStyle(uint32 nStyles) { m_nEditorStyles = nStyles; }
	uint32 GetStyle() const { return m_nEditorStyles; }

	virtual void SetText(LPCTSTR szText);

	int GetText(tstring &str);

	void SelectionToText(tstring &str);

	int GetLength();

	size_t GetLineCount() const { return m_vLines.size(); }

	bool GetTextOfLine(int nLine, tstring &strLine);
	bool SetTextOfLine(int nLine, LPCTSTR szLine);

	bool IsSelected();

	bool CanUndo();
	bool CanRedo();

	bool IsInBatchAction() const { return m_undoMgr.IsInBatchAction(); }

	void OnCmdCopy();
	void OnCmdCut();
	void OnCmdPaste();
	void OnCmdRedo();
	void OnCmdUndo();
	virtual void OnCmdDelete();

public:
	virtual COneLine *NewOneLine() { return new COneLine; }

protected:
	virtual int GetFontHeight() { return m_font.GetHeight(); }

	virtual MLColor & GetColor(int nColorName);
	virtual void SetColor(int nColorName, const MLColor & clr);

	// Alpha of background, used to implement translucent effect.
	virtual byte GetBgAlpha();
	virtual CRawGraph *GetMemGraphics()
	{
		CRawGraph		*pGraph = m_pContainer->GetMemGraph();
		pGraph->SetFont(m_font.GetFont());

		return pGraph;
	}

	void UpdateRectToScreen(const CRect *rc);

	void FillGraph(CRawGraph *pGraph, const CRect *rc, int nColorName);
	void FillGraph(CRawGraph *pGraph, int x, int y, int nWidth, int nHeight, int nColorName)
		{ CRect		rc(x, y, x + nWidth, y + nHeight); FillGraph(pGraph, &rc, nColorName); }
	void FillSelectedLineBg(CRawGraph *pGraph, int x, int y, COneLine *pLine, int nBegSelCol, int nEndSelCol);

	void DrawLine(CRawGraph *pGraph, COneLine *pLine, int x, int xMax, int y);
	void UpdateWidthInfoOfLine(CRawGraph *pGraph, COneLine *pLine);

	bool DoSetText(LPCTSTR szText);

	void Clear();
	void ClearWidth();

	int GetLineDy() { return (m_nLineSpace + GetFontHeight()); }

	virtual void ShowCaret(int x, int y, int nWidth, int nHeight);
	virtual void HideCaret();
	virtual int GetCaretLineHomePos() { return 0; }
	void ShowCaret();
	void MakeCaretInSight();
	void SetCaretToPos(int x, int y);

	int GetEndColPosOfLine(int nLine);

	void NextPos(int &Row, int &nCol);
	void PrevPos(int &Row, int &nCol);
	void NextWord(int &Row, int &nCol);
	void PrevWord(int &Row, int &nCol);

	void SelectNone();
	void RemoveSelected(int *pnBegSelRow = NULL, int *pnBegSelCol = NULL);
	void RemoveStr(int nRow, int nCol, int nSize);
	void RemoveChar(int nRow, int nCol);

	void InsertChar(int nRow, int nCol, WCHAR chInsert);
	void InsertStr(int nRow, int nCol, LPCTSTR szText, int &nRowNext, int &nColNext);

	void UpdateScrollInfo(bool bHorz = true, bool bVert = true);

	void GetStrAtPos(int nRow, int nCol, tstring &str);

	bool IsPosValid(int nRow, int nCol);

	void SortSelectPos(int &nBegSelRow, int &nBegSelCol, int &nEndSelRow, int &nEndSelCol, bool &bSelectedEmpty);

	bool IsReadOnly() { return IsFlagSet(m_nEditorStyles, S_READ_ONLY); }
	bool IsSingleLine() { return !IsFlagSet(m_nEditorStyles, S_MULTILINE); }
	bool IsPassword() { return IsFlagSet(m_nEditorStyles, S_PASSWORD); }

public:
	//
	// CUIObject messages to draw or handle user input, etc.
	//

	virtual void OnCreate();

	void Draw(CRawGraph *pGraph);

	// rc is the destination rectangle to draw the edit
	void OnSize(int cx, int cy);

	virtual void OnSetFocus();
	virtual void OnKillFocus();

	virtual bool OnLButtonUp(UINT nFlags, CPoint point);
	virtual bool OnLButtonDown(UINT nFlags, CPoint point);
	virtual bool OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual bool OnMouseMove(UINT nFlags, CPoint point);
	virtual bool OnRButtonUp(UINT nFlags, CPoint point);
	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);
	virtual void OnKeyDown(UINT nChar, UINT nFlags);
	virtual void OnChar(UINT nChar);

	virtual void OnContexMenu(int xPos, int yPos);
	virtual bool OnCommand(UINT uID);

	virtual void OnTimer(int nId);

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

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	virtual tstring &GetText();

#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	//
	// IUndoMgrNotify interface
	//
	virtual void OnStatusChanged(Status status, bool bVal);
	virtual void OnAction(Action action);

	friend class CAutoUpdate;
	friend class CEditInsertAction;
	friend class CEditDelAction;

protected:
	uint32				m_nEditorStyles;

	CVLines				m_vLines;

	// scroll bar info
	int					m_nTopVisibleLine;
	int					m_nScrollPosx;

	int					m_nLineSpace;

	// Selected text
	int					m_nBegSelRow, m_nEndSelRow;
	int					m_nBegSelCol, m_nEndSelCol;
	bool				m_bInMouseSel;		// is mouse select text?

	int					m_nOneCharDx;

	// undo
	CUndoMgr			m_undoMgr;

	// for win9x, process MBCS input...
	char				m_szMbcsA[2];		// first half part of MBCS char.
	bool				m_bMbcsA;
	bool				m_bWin9x;

	IEditSyntaxParser	*m_pEditSyntaxParser;
	IEditNotification	*m_pEditNotification;
	bool				m_bPrevSelectedStatus;

	// Caret
	int					m_nIDTimerCaret;
	CSkinCaret			m_caret;
	int					m_nCaretX, m_nCaretY;
	int					m_nCaretRow, m_nCaretCol;
	int					m_nCaretMaxXLatest;

	// Cursor
	CCursor				m_cursor;

	// Font and colors
	CSkinFontProperty	m_font;
	int					m_xMargin, m_yMargin;
	vector<MLColor>		m_vClrTable;

};
