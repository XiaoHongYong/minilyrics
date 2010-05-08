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

#include "LyricShowObj.h"

class CLyricShowTxtObj : public CLyricShowObj, IScrollNotify
{
	UIOBJECT_CLASS_NAME_DECLARE(CLyricShowObj)
public:
	CLyricShowTxtObj();
	virtual ~CLyricShowTxtObj();

public:
	void OnKeyDown(UINT nChar, UINT nFlags);
	void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);
	void OnLyricsChanged();

	void OnCreate();
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

	void StartScrollAnimation(int nFirstVisibleLineNew);

protected:
	IScrollBar				*m_pScrollBar;
	CUIObject				*m_pObjScrollBar;

	int						m_nFirstVisibleLine;

	bool					m_bInScrolling;
	int						m_nFirstVisibleLineNew;
	int						m_yFirstVisibleLineLatest, m_nFirstVisibleLineLatest;
	int						m_yOffsetScroll;
	uint32					m_nBeginScrollTime;

	bool					m_bEnableAutoScroll;

};

#endif // !defined(AFX_LYRICSHOWTXTOBJ_H__18145744_7DB0_11D5_9E04_02608CAD9330__INCLUDED_)
