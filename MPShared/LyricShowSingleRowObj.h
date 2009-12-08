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
	int GetLyricRightRowAlignPos(CRawGraph *pGraphics, PML_ROW pLyricRow);
	int GetLyricLeftRowAlignPos(CRawGraph *pGraphics, PML_ROW pLyricRow);

	void DrawCurLineFadeInNextLine(CRawGraph *pGraphics, PML_ROW pRowCur, PML_ROW pRowNext, int x, int y);

	void GetCurLineFadeOutColor(PML_ROW pRow, MLColor clrIn, MLColor clrOut, MLColor &color);
	void GetNextLineFadeInColor(PML_ROW pRow, MLColor clrIn, MLColor clrOut, MLColor &color);

public:
	static LPCTSTR ClassName() { return ms_szClassName; }
	BOOL IsKindOf(LPCTSTR szClassName);
	LPCTSTR GetClassName();
	static LPCTSTR		ms_szClassName;

protected:
	virtual int GetAutoHeightLines() { return 1; }

};

#endif // !defined(AFX_LYRICSHOWSINGLEROWOBJ_H__5DC5BCC2_135B_11D6_B478_FFFFFF000000__INCLUDED_)
