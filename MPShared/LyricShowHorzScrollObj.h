// LyricShowHorzScrollObj.h: interface for the CLyricShowHorzScrollObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LYRICSHOWHORZSCROLLOBJ_H__9D736B74_72C3_4869_857C_C10D4AE717B4__INCLUDED_)
#define AFX_LYRICSHOWHORZSCROLLOBJ_H__9D736B74_72C3_4869_857C_C10D4AE717B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricShowObj.h"

class CLyricShowHorzScrollObj : public CLyricShowObj  
{
public:
	CLyricShowHorzScrollObj();
	virtual ~CLyricShowHorzScrollObj();

public:
	//	void UpdateDraw(DWORD dwTimeElapsed);
	void FastDraw(CRawGraph *pGraphics, CRect *prcUpdate = NULL);
	int GetLyricRightRowAlignPos(CRawGraph *pGraphics, PML_ROW pLyricRow);
	int GetLyricLeftRowAlignPos(CRawGraph *pGraphics, PML_ROW pLyricRow);

	void OnLyrDrawContextChanged();

	static LPCTSTR ClassName() { return ms_szClassName; }
	BOOL IsKindOf(LPCTSTR szClassName);
	LPCTSTR GetClassName();

protected:
	void CalculateAccelerate();

public:
	static LPCTSTR		ms_szClassName;

	//
	// lyrics move accelerately ...
	// int				m_nV0, m_nA;
	vector<double>		m_vV, m_vA;

};

#endif // !defined(AFX_LYRICSHOWHORZSCROLLOBJ_H__9D736B74_72C3_4869_857C_C10D4AE717B4__INCLUDED_)
