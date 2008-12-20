#pragma once

#include "WidgetCtrlBase.h"

class CWidgetColorButton : public CWidgetCtrlBase
{
public:
	CWidgetColorButton();
	~CWidgetColorButton();

	BOOL Attach(CBaseWnd *pWndParent, int nCtrlID);

	void SetColor(const MLColor &clr);
	void GetColor(MLColor &clr);

public:
	void OnPaint(IMLGraphicsBase *pGraph);

	void OnLButtonUp(UINT nFlags, CPoint point);

protected:
	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	WNDPROC			m_pWndProcOrg;
	MLColor			m_clrBg;

};
