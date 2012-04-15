// LyricShowSingleRowObj.h: interface for the CLyricShowSingleRowObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LYRICSHOWSINGLEROWOBJ_H__5DC5BCC2_135B_11D6_B478_FFFFFF000000__INCLUDED_)
#define AFX_LYRICSHOWSINGLEROWOBJ_H__5DC5BCC2_135B_11D6_B478_FFFFFF000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricShowObj.h"

class CLyricShowSingleRowObj : public CLyricShowObj  
{
public:
	CLyricShowSingleRowObj();
	virtual ~CLyricShowSingleRowObj();

public:
	void FastDraw(CRawGraph *pGraphics, CRect *prcUpdate = NULL);
	int GetLyricRightRowAlignPos(CRawGraph *pGraphics, LyricsLine *pLyricRow);
	int GetLyricLeftRowAlignPos(CRawGraph *pGraphics, LyricsLine *pLyricRow);

	void DrawCurLineFadeInNextLine(CRawGraph *pGraphics, LyricsLine *pRowCur, LyricsLine *pRowNext, int x, int y);

	void GetCurLineFadeOutColor(LyricsLine *pRow, MLColor clrIn, MLColor clrOut, MLColor &color);
	void GetNextLineFadeInColor(LyricsLine *pRow, MLColor clrIn, MLColor clrOut, MLColor &color);

public:
	static LPCTSTR ClassName() { return ms_szClassName; }
	BOOL IsKindOf(LPCTSTR szClassName);
	LPCTSTR GetClassName();
	static LPCTSTR		ms_szClassName;

protected:
	virtual int GetAutoHeightLines() { return 1; }

};

#endif // !defined(AFX_LYRICSHOWSINGLEROWOBJ_H__5DC5BCC2_135B_11D6_B478_FFFFFF000000__INCLUDED_)
