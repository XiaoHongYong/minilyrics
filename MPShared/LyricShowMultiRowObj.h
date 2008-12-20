/********************************************************************
	Created  :	2002/01/04	21:40
	FileName :	LyricShowMultiRowObj.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_LYRICSHOWMULTIROWOBJ_H__B7400FE7_78AA_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_LYRICSHOWMULTIROWOBJ_H__B7400FE7_78AA_11D5_9E04_02608CAD9330__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricShowObj.h"

class CLyricShowMultiRowObj : public CLyricShowObj  
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CLyricShowMultiRowObj();
	virtual ~CLyricShowMultiRowObj();

	void OnCreate();

	void FastDraw(CRawGraph *pGraphics, CRect *prcUpdate = NULL);

	void Invalidate();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

protected:
	bool FastestDrawFadeInOut(CRawGraph *pGraphics, CRect *prcUpdate, int y, PML_ROW pLyricRow);
	bool FastestDrawNormal(CRawGraph *pGraphics, CRect *prcUpdate, int y, PML_ROW pLyricRow);

	bool OnLyrDisplaySettings(LPCTSTR szProperty, LPCTSTR szValue);

public:
	void OnLyrDrawContextChanged();

	// 使用下面的变量记录了上一次歌词的显示位置，在新的一次显示时，
	// 可以优化显示速度
	int             m_yDrawingOld;		// 上一次歌词显示的位置
	int				m_nCurRowOld;		// 上一次显示的当前行歌词
	int				m_nAlphaOld;		// Old alpha value

	int				m_nMaxShowLines;
	bool			m_bOnlyDisplayCurLineLyr;

};

#endif // !defined(AFX_LYRICSHOWMULTIROWOBJ_H__B7400FE7_78AA_11D5_9E04_02608CAD9330__INCLUDED_)
