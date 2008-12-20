// HyperLinkCtrl.h: interface for the CHyperLinkCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HYPERLINKCTRL_H__16709A1C_FC71_447E_96B6_E295286C4FFC__INCLUDED_)
#define AFX_HYPERLINKCTRL_H__16709A1C_FC71_447E_96B6_E295286C4FFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CHyperLinkCtrl : public CWidgetCtrlBase
{
public:
	CHyperLinkCtrl();
	~CHyperLinkCtrl();

	BOOL Attach(CBaseWnd *pParentWnd, UINT uChildId, LPCTSTR szUrl);

	void OnPaint(IMLGraphicsBase *pGraph);

	void OnMouseMove(UINT nFlags, CPoint point);

	void OnLButtonDown(UINT nFlags, CPoint point);

	void OnLButtonUp(UINT nFlags, CPoint point);

protected:
	BOOL			m_bVisited;
	tstring			m_strUrl;

};

#endif // !defined(AFX_HYPERLINKCTRL_H__16709A1C_FC71_447E_96B6_E295286C4FFC__INCLUDED_)
