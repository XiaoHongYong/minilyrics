#pragma once

//
// CSkinContainerCtrl is the super class of CSkinListCtrl or CSkinEditCtrl.
//
class CSkinScrollFrameCtrlBase : public CSkinContainer, public IScrollNotify
{
public:
	CSkinScrollFrameCtrlBase();
	virtual ~CSkinScrollFrameCtrlBase();

	void OnCreate();

	void OnSize();

	virtual void OnKillFocus();
	virtual void OnSetFocus();

	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }

protected:
	void CreateFrameCtrl();

	void CreateVertScrollbar();
	void CreateHorzScrollbar();

protected:
	bool					m_bEnableBorder;
	bool					m_bHorzScrollBar, m_bVertScrollBar;
	int						m_nVertScrollBarId, m_nHorzScrollBarId;		// If this is set, don't create dynamic scroll bar control.
	IScrollBar				*m_pVertScrollBar, *m_pHorzScrollBar;
	CUIObject				*m_pObjVertScrollBar, *m_pObjHorzScrollBar;
	CUIObject				*m_pObjFrame;

	// The size and position of control area
	int						m_nWidthVertScrollBar, m_nHeightHorzScrollBar;

};
