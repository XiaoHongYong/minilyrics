/********************************************************************
	Created  :	2002/01/04	21:43
	FileName :	WndDrag.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_WNDDRAG_H__54E078FE_AFB3_4369_8EF6_5E00313640DC__INCLUDED_)
#define AFX_WNDDRAG_H__54E078FE_AFB3_4369_8EF6_5E00313640DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBaseWnd;

class CWndDrag  
{
public:
	typedef struct WndCloseTo
	{
		CBaseWnd *pWnd;
		tstring	strClass;
		tstring	strWndName;
	}WndCloseTo;

	vector<WndCloseTo>	m_vWndCloseTo;

public:
	CWndDrag();
	virtual ~CWndDrag();

	void Init(CBaseWnd *pWnd);

	BOOL IsDragging() { return m_bDragWnd; }
	void OnDrag(UINT fwKeys, POINT pt);

	void EnableDrag(BOOL bEnable) { m_bEnableDrag = bEnable; }
	void EnableAutoCloseto(BOOL bEnable) { m_bDragAutoCloseto = bEnable; }

	void AddWndCloseto(CBaseWnd *pWnd, LPCTSTR szWndClass, LPCTSTR szWndName);

	void BeforeTrackMoveWith(CBaseWnd *pWndChain[], int nCount, CBaseWnd *pWndToTrack);
	void TrackMoveWith(CBaseWnd *pWnd, int x, int y);

protected:
	virtual void SetWindowPosSafely(int xOld, int yOld, int nOffsetx, int nOffsety);
	virtual BOOL AutoCloseToWindows(int &nOffx, int &nOffy, BOOL bMoveWindow = TRUE);

protected:
    CBaseWnd		*m_pWnd;			// 父窗口

	BOOL			m_bDragWnd;		// dragging window?

	CPoint			m_ptDragOld;			// drag begin pos

    BOOL            m_bDragAutoCloseto;

	BOOL			m_bSticked;		// 粘住了目标窗口？
	CBaseWnd		*m_pWndToTrack;	// 粘住的目标窗口
	CPoint			m_ptWndTrack;	// 粘住的目标窗口的左上角

	BOOL			m_bEnableDrag;

	// vector<tstring>		*m_pvWndClassCloseTo;
    // LPTSTR          *m_arrszWndClassCloseTo;
};

#endif // !defined(AFX_WNDDRAG_H__54E078FE_AFB3_4369_8EF6_5E00313640DC__INCLUDED_)
