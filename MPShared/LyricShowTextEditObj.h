// LyricShowTextEditObj.h: interface for the CLyricShowTextEditObj class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CLyricShowTextEditObj;

class CLyrEditSyntaxParser : public IEditSyntaxParser
{
public:
	enum LYR_CLR
	{
		C_LOWLIGHT		= CSkinEditCtrl::CN_CUSTOMIZED_START,
		C_TAG,
		C_HILIGHT,
	};

	CLyrEditSyntaxParser();
	virtual ~CLyrEditSyntaxParser();

	virtual void Init(CSkinEditCtrl *pEdit);
	virtual void OnNotifyParseLine(int nLine);

public:
	CLyricShowTextEditObj	*m_pLyrEdit;

};

class CLyricShowTextEditObj : public CSkinEditCtrl, public IEditNotification, public IEventHandler
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinEditCtrl)
public:
	CLyricShowTextEditObj();
	virtual ~CLyricShowTextEditObj();

	class CLyrOneLine : public CSkinEditCtrl::COneLine
	{
	public:
		CLyrOneLine()
		{
			nBegTime = 0, nEndTime = 0;
			bTimeLine = false;
			bTempBegTime = true;
			bTempEndTime = true;
		}
		int			nBegTime, nEndTime;
		bool		bTempBegTime, bTempEndTime;
		bool		bTimeLine;
	};

public:
	enum FindTextFlag
	{
		FTF_DOWN		= 0x1,			// search continue from selection
		FTF_MATCH_CASE	= 0x1 << 1,		// case sensitive
		FTF_WHOLEWORD	= 0x1 << 2,		// 
	};

	void OnKeyDown(UINT nChar, UINT nFlags);

	void AutoVScrollDown(int nDownLine);

	bool FindText(LPCTSTR szText, DWORD dwFlags);

	CLyrOneLine *GetLine(int nLine);

	// for lyrics draw
	void FastDraw(CRawGraph *pGraphics, CRect *prcUpdate = NULL);
	void ReDraw(CRawGraph *pGraphics);
	void OnPlayTimeChangedUpdate();
	void DrawLineGradual(CRawGraph *pGraph, COneLine *pLine, int x, int xMax, int y);

	void SetColorTheme(MLColor &clrHilight, MLColor &clrLowlight, MLColor &clrTag, MLColor &clrEditLineBg);

	bool IsTextChanged();

public:
	virtual void OnCmdDelete();

	virtual COneLine *NewOneLine() { return new CLyrOneLine; }

public:
	//
	// Find and replace dialog
	//
	void ShowFindDialog(bool bFind = true);
	bool FindNext(bool bSearchDown = true);

protected:
	virtual int GetCaretLineHomePos();

public:
	virtual void OnCreate();
	virtual void OnDestroy();

	virtual void OnEvent(const IEvent *pEvent);

	virtual bool OnCustomCommand(int nId);

	virtual void OnSaveLyrics();

	virtual void Draw(CRawGraph *pGraph);

public:
	void UpdateLyricsProperties(bool bRedraw = false);
	void OnLyricsChanged();

public:
	virtual void OnStatusChanged(IEditNotification::Status status, bool bVal);
	virtual void OnTextChanged();

	virtual void OnTimer(int nId);

protected:
	void UpdateTextFontColor();

	void SyncTimeTag(bool bMoveToNextLine);

	void AdjustSyncTimeOfSelected(bool bIncreaseTime);

	bool				m_bDiscardLyrChangeEvent;
	int					m_nTimerIDUpdateLyrics;

	enum FindReplaceStatus
	{
		FRS_NONE,
		FRS_FIND,
		FRS_REPLACE,
	};

protected:
	CLyrEditSyntaxParser	m_lyrEditSyntaxParser;

	FindReplaceStatus		m_findReplaceStatus;
	int						CID_MATCHCASE, CID_FIND_PREV, CID_FIND_NEXT, CID_E_FIND;
	int						CID_REPLACE, CID_REPLACE_ALL, CID_E_REPLACE;
	int						CID_E_ARTIST, CID_E_TITLE, CID_E_ALBUM, CID_E_BY;
	INT						CID_E_OFFSET, CID_E_MEDIA_LENGTH;

	// 
	int					m_nTopVisibleLineOld;
	int					m_nCurPosLineOld;
	int					m_nDrawYOfCurPosLine;
	int					m_nXOfCurPosLineOld;

};
