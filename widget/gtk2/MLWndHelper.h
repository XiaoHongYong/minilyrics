// WndHelper.h: interface for the CWndHelper class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MLWNDHELPER_INC_
#define _MLWNDHELPER_INC_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// using STL
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#include <vector>
using namespace std;

class CMLWndHelper  
{
	struct ChildWndProp
	{
		CBaseWnd	*pWnd;
		CBaseWnd	*pWndRelation;
		BOOL		bAlignRight;
		BOOL		bAlignBottom;
		int			nMarginX, nMarginY;

		BOOL		bSizeXFix, bSizeYFix;
		int			nX, nY;
		int			nCx, nCy;
	};
	struct ChildAlignCenter
	{
		CBaseWnd	*pWnd;
		bool		bVert;
		int			nOffsetToCenter;
	};

public:
	CMLWndHelper();
	virtual ~CMLWndHelper();

	void Init(CBaseWnd *pWndParent) { m_pWndParent = pWndParent; }

	void Add(CBaseWnd *pWnd, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, CBaseWnd *pWndRelation = NULL);
	void AddAlignCenter(CBaseWnd *pWnd, bool bAlignVert);

	void Add(int nChildID, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, CBaseWnd *pWndRelation = NULL);
	void AddAlignCenter(int nChildID, bool bAlignVert);
	void AddAlignRight(int nChildID, bool bSizeXFixed);
	void AddAlignBottom(int nChildID, bool bSizeYFixed);
	void AddAlignRightBottom(int nChildID, bool bSizeXFixed, bool bSizeYFixed);

	void OnResize();

protected:
	CBaseWnd					*m_pWndParent;

	vector<ChildWndProp>		m_vChildWnd;
	vector<ChildAlignCenter>	m_vAlignCenterWnds;

};

#endif // _MLWNDHELPER_INC_
