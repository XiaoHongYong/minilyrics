
#pragma once

// using STL
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#include <vector>
using namespace std;

class CMLWndHelper  
{
	struct ChildWndProp
	{
		HWND		hWnd;
		HWND		hWndRelation;
		BOOL		bAlignRight;
		BOOL		bAlignBottom;
		int			nMarginX, nMarginY;

		BOOL		bSizeXFix, bSizeYFix;
		int			nX, nY;
		int			nCx, nCy;
	};
	struct ChildAlignCenter
	{
		HWND		hWnd;
		bool		bVert;
		int			nOffsetToCenter;
	};

public:
	CMLWndHelper();
	virtual ~CMLWndHelper();

	void Init(CBaseWnd *pWndParent) { m_pWndParent = pWndParent; }

	void Add(int nChildID, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, int nChildIDRelation = 0);
	void AddAlignCenter(int nChildID, bool bAlignVert);
	void AddAlignRight(int nChildID, bool bSizeXFixed);
	void AddAlignBottom(int nChildID, bool bSizeYFixed);
	void AddAlignRightBottom(int nChildID, bool bSizeXFixed, bool bSizeYFixed);

	void OnResize();

protected:
	void Add(HWND hWndChild, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, HWND hChildRelation = NULL);
	void AddAlignCenter(HWND hWndChild, bool bAlignVert);

protected:
	CBaseWnd					*m_pWndParent;

	vector<ChildWndProp>		m_vChildWnd;
	vector<ChildAlignCenter>	m_vAlignCenterWnds;

};
