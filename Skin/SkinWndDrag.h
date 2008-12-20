// SkinWndDrag.h: interface for the CSkinWndDrag class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINWNDDRAG_H__A02E79B7_9596_4BAB_AAAE_6173A4201D75__INCLUDED_)
#define AFX_SKINWNDDRAG_H__A02E79B7_9596_4BAB_AAAE_6173A4201D75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ISkinWndDragHost
{
public:
	virtual void GetWndDragAutoCloseTo(vector<CBaseWnd *> &vWnd) = 0;
	virtual void GetWndDragTrackMove(vector<CBaseWnd *> &vWnd) = 0;

};

class CSkinWndDrag : public CWndDrag
{
public:
	CSkinWndDrag();
	virtual ~CSkinWndDrag();

	void Init(CBaseWnd *pWnd, ISkinWndDragHost *pSkinWndDragHost);

	void OnDrag(UINT fwKeys, POINT pt);

	virtual BOOL AutoCloseToWindows(int &nOffx, int &nOffy, BOOL bMoveWindow);

protected:
	void SetWindowPosSafely(int xOld, int yOld, int nOffsetx, int nOffsety);

protected:
	ISkinWndDragHost	*m_pSkinWndDragHost;
	vector<CBaseWnd *>	m_vStickedWnds;

};

class CSkinWndResizer : public CWndResizer
{
public:
	CSkinWndResizer();
	virtual ~CSkinWndResizer();

	void Init(CBaseWnd *pWnd, ISkinWndDragHost *pSkinWndDragHost);

	virtual void AutoCloseToWindows(int &nOffx, int &nOffy);

protected:
	ISkinWndDragHost	*m_pSkinWndDragHost;

};

#endif // !defined(AFX_SKINWNDDRAG_H__A02E79B7_9596_4BAB_AAAE_6173A4201D75__INCLUDED_)
