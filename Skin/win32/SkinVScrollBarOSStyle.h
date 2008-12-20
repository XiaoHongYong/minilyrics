// SkinVScrollBarOSStyle.h: interface for the CSkinVScrollBarOSStyle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINVSCROLLBAROSSTYLE_H__122A87BD_1DC8_4EA7_A35E_F85FC32AB87B__INCLUDED_)
#define AFX_SKINVSCROLLBAROSSTYLE_H__122A87BD_1DC8_4EA7_A35E_F85FC32AB87B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UIObject.h"
#include "../widget/win32/CommonWidget.h"

class CSkinScrollBarOSStyleBase : public CUIObject, public IScrollBar, public IScrollNotify
{
public:
	CSkinScrollBarOSStyleBase();
	virtual ~CSkinScrollBarOSStyleBase();

	void SetScrollInfo(int nMin, int nMax, int nPage, int nPos = 0, int nLine = 1, bool bRedraw = true)
	{
		m_scrollbar.SetScrollInfo(nMin, nMax, nPage, nPos, nLine, bRedraw);
	}
	int SetScrollPos(int nPos, bool bRedraw = true)
	{
		return m_scrollbar.SetScrollPos(nPos, bRedraw);
	}

	int GetScrollPos() const { return m_scrollbar.GetScrollPos(); }
	int GetPage() const { return m_scrollbar.GetPage(); }
	int GetMin() const { return m_scrollbar.GetMin(); }
	int GetMax() const { return m_scrollbar.GetMax(); }
	virtual int GetID() const { return m_nId; }

	void SetScrollNotify(IScrollNotify *pNofity) { m_pScrollNotify = pNofity; }

	virtual bool IsEnabled() const { return m_scrollbar.IsEnabled(); }
	virtual void DisableScrollBar() { m_scrollbar.DisableScrollBar(); }
	virtual bool HandleScrollCode(UINT nSBCode, int nPos) { return m_scrollbar.HandleScrollCode(nSBCode, nPos); }

	// IScrollNotify
	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);

	virtual void OnSize(int cx, int cy);
	virtual void OnSetFocus();

	void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);

protected:
	// ª˘±æ…Ë÷√
	CWidgetScrollBar	m_scrollbar;
	IScrollNotify		*m_pScrollNotify;

};

class CSkinVScrollBarOSStyle : public CSkinScrollBarOSStyleBase
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	virtual void OnCreate();

};

class CSkinHScrollBarOSStyle : public CSkinScrollBarOSStyleBase
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	virtual void OnCreate();

};

#endif // !defined(AFX_SKINVSCROLLBAROSSTYLE_H__122A87BD_1DC8_4EA7_A35E_F85FC32AB87B__INCLUDED_)
