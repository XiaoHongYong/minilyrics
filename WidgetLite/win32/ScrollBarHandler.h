// ScrollBarHandler.h: interface for the CScrollBarHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCROLLBARHANDLER_H__5D3E5098_074F_4ECD_A670_4030CB63F97A__INCLUDED_)
#define AFX_SCROLLBARHANDLER_H__5D3E5098_074F_4ECD_A670_4030CB63F97A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBaseWnd;

class CScrollBarHandler : public IScrollBar  
{
public:
	CScrollBarHandler();
	virtual ~CScrollBarHandler();

	virtual void SetScrollInfo(int nMin, int nMax, int nPage, int nPos, int nLine = 1, bool bRedraw = true);
	virtual int SetScrollPos(int nPos, bool bRedraw = true);

	virtual int GetScrollPos() const;
	virtual int GetPage() const{ return m_si.nPage; }
	virtual int GetMin() const{ return m_si.nMin; }
	virtual int GetMax() const;
	virtual int GetID() const;

	// For CSkinVScrollBar, it will send the scroll notify event to pNotify, or it will send to parent.
	virtual void SetScrollNotify(IScrollNotify *pNofity) { }

	virtual bool IsEnabled() const { return !m_bDisabled; }
	virtual void DisableScrollBar();

	// For windows scroll bar, IScrollBar need to handle scroll codes and set scroll pos of the scroll bar.
	// This was done in WM_VSCROLL or WM_HSCROLL message handler.
	// If pos is changed, return true.
	bool HandleScrollCode(UINT nSBCode, int nPos);

	void Init(CBaseWnd *pWnd, int nBar = SB_VERT);

	bool IsVertBar() { return m_nScrollBar == SB_VERT; }

protected:
	HWND			m_hWnd;
	SCROLLINFO		m_si;
	int				m_nScrollBar;
	bool			m_bDisabled;
	int				m_nOneline;

};

#endif // !defined(AFX_SCROLLBARHANDLER_H__5D3E5098_074F_4ECD_A670_4030CB63F97A__INCLUDED_)
