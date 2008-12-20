// WndHelper.h: interface for the CWndHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WNDHELPER_H__45A76D39_017B_41E8_A838_312A5F92D1AF__INCLUDED_)
#define AFX_WNDHELPER_H__45A76D39_017B_41E8_A838_312A5F92D1AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// using STL
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#include <vector>
using namespace std;

class CWndHelper  
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
	CWndHelper();
	virtual ~CWndHelper();

	void Init(HWND	hWndParent) { m_hWndParent = hWndParent; }

	void Add(HWND hWnd, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, HWND hWndRelation = NULL);
	void AddAlignCenter(HWND hWnd, bool bAlignVert);

	void Add(int nChildID, BOOL bAlignRight, BOOL bAlignBottom, BOOL bSizeXFix, BOOL bSizeYFix, HWND hWndRelation = NULL);
	void AddAlignCenter(int nChildID, bool bAlignVert);
	void AddAlignRight(int nChildID, bool bSizeXFixed);
	void AddAlignBottom(int nChildID, bool bSizeYFixed);
	void AddAlignRightBottom(int nChildID, bool bSizeXFixed, bool bSizeYFixed);

	void OnResize();

protected:
	HWND		m_hWndParent;

	vector<ChildWndProp>		m_vChildWnd;
	vector<ChildAlignCenter>	m_vAlignCenterWnds;

};

#endif // !defined(AFX_WNDHELPER_H__45A76D39_017B_41E8_A838_312A5F92D1AF__INCLUDED_)
