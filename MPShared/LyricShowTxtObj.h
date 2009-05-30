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
public:
	void Invalidate();

	CLyricShowTxtObj();
	virtual ~CLyricShowTxtObj();

public:
	LPCTSTR GetClassName();
	void OnKeyDown(UINT nChar, UINT nFlags);
	void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);
	void OnLyricsChanged();

	void OnCreate();
	void OnSize(int cx, int cy);

	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }

protected:
	IScrollBar				*m_pScrollBar;
	CUIObject				*m_pObjScrollBar;

	int						m_nRowCur;			// 当前歌词行

	CMLBrush				m_brCurRow;		// 当前编辑歌词行的背景色

	int						m_nCurRowOld;		// 上一次显示的当前行歌词

public:
	void OnPlayTimeChangedUpdate();

	static LPCTSTR ClassName() { return ms_szClassName; }
	BOOL IsKindOf(LPCTSTR szClassName);

protected:
	void FastDraw(CRawGraph *pGraphics, CRect *prcUpdate = NULL);

	void OnLyrDrawContextChanged();
	void SetScrollInfo();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

protected:

	static LPCTSTR		ms_szClassName;

};

#endif // !defined(AFX_LYRICSHOWTXTOBJ_H__18145744_7DB0_11D5_9E04_02608CAD9330__INCLUDED_)
