#ifndef _ML_GEN_CTRL_INC_
#define _ML_GEN_CTRL_INC_

#include "WidgetCtrlBase.h"
#include "ScrollBarHandler.h"

typedef unsigned int uint;

class CWidgetLable : public CWidgetCtrlBase
{
public:
	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

};

class CWidgetButton : public CWidgetCtrlBase
{
public:
	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

protected:

};

class CWidgetToggleButton : public CWidgetCtrlBase
{
public:
	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	bool IsChecked();

	void SetCheck(bool bChecked);

};

class CWidgetCheckButton : public CWidgetToggleButton
{
public:
	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

};

class CWidgetRadioButton : public CWidgetToggleButton
{
public:
	bool Create(LPCTSTR szText, CBaseWnd *pWndParent, CWidgetRadioButton *pGroupStart, int nID = ID_UNDEFINE);

};


class CWidgetFrame : public CWidgetCtrlBase
{
public:
	CWidgetFrame();

	virtual bool Create(LPCTSTR szText, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

};

class CWidgetBox : public CWidgetCtrlBase
{
public:
	CWidgetBox();

	virtual bool Create(bool bHorz, bool bHomogeneous, int nSpacing, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	// pack 
	void PackStart(CBaseWnd *pWnd, bool bExpand, bool bFill, uint nPadding);

	void PackEnd(CBaseWnd *pWnd, bool bExpand, bool bFill, uint nPadding);

protected:
	bool			m_bHorz;
	bool			m_bHomogeneous;
	bool			m_nSpacing;

};

class CWidgetScrollBar : public CBaseWnd, public IScrollBar, IScrollNotify
{
public:
	CWidgetScrollBar();
	~CWidgetScrollBar();

	virtual BOOL Create(int x, int y, int nWidth, int nHeight, bool bVert, CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	void SetScrollInfo(int nMin, int nMax, int nPage, int nPos = 0, int nLine = 1, bool bRedraw = true);
	int SetScrollPos(int nPos, bool bRedraw = true);

	int GetScrollPos() const;
	int GetPage() const { return m_sbHandler.GetPage(); }
	int GetMin() const { return m_sbHandler.GetMin(); }
	int GetMax() const { return m_sbHandler.GetMax(); }
	virtual int GetID() const { return 0; }

	void SetScrollNotify(IScrollNotify *pNofity) { m_pScrollNotify = pNofity; }

	virtual bool IsEnabled() const { return m_sbHandler.IsEnabled(); }
	virtual void DisableScrollBar() { m_sbHandler.DisableScrollBar(); }
	virtual bool HandleScrollCode(UINT nSBCode, int nPos);

	// IScrollNotify, // for use of CBaseWnd::WM_VSCROLL
	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);

protected:
	CScrollBarHandler	m_sbHandler;
	IScrollNotify		*m_pScrollNotify;

};


#endif // _ML_GEN_CTRL_INC_
