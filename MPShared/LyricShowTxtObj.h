/********************************************************************
	Created  :	2002/01/04	21:40
	FileName :	LyricShowTxtObj.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_LYRICSHOWTXTOBJ_H__18145744_7DB0_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_LYRICSHOWTXTOBJ_H__18145744_7DB0_11D5_9E04_02608CAD9330__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricShowMultiRowObj.h"

class CLyricShowTxtObj : public CLyricShowMultiRowObj, IScrollNotify
{
	UIOBJECT_CLASS_NAME_DECLARE(CLyricShowMultiRowObj)
public:
	CLyricShowTxtObj();
	virtual ~CLyricShowTxtObj();

public:
	void OnKeyDown(UINT nChar, UINT nFlags);
	void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);
	void OnLyricsChanged();

	void OnCreate();
	void OnEvent(const IEvent *pEvent);
	void OnSize(int cx, int cy);

	void OnTimer(int nId);

	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }

	void OnPlayTimeChangedUpdate();

protected:
	void OnLyrDrawContextChanged();

	void FastDraw(CRawGraph *pGraphics, CRect *prcUpdate = NULL);

	void SetScrollInfo();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	int GetLinesPerPage();

	void StartScrollAnimation(int nCurLineNew);

protected:
	IScrollBar				*m_pScrollBar;
	CUIObject				*m_pObjScrollBar;

	int						m_nCurLine;

	bool					m_bInScrollingToNewLine;
	int						m_nCurLineNew;
	int						m_yCurLineLatest, m_nCurLineLatest;
	int						m_yOffsetScroll;
	uint32					m_nBeginScrollTime;

	bool					m_bInVerticalScrollingMode;

};

#endif // !defined(AFX_LYRICSHOWTXTOBJ_H__18145744_7DB0_11D5_9E04_02608CAD9330__INCLUDED_)
