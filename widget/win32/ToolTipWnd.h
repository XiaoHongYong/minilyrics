
#pragma once

#include "Widget.h"

class CToolTipWnd : public CBaseWnd  
{
public:
	void RelayEvent(LPMSG lpMsg);
	void FillInToolInfo(TOOLINFO& ti, HWND hWnd, UINT nIDTool) const;
	void DelTool(HWND hWnd, UINT nIDTool);
	BOOL AddTool(HWND hWnd, LPCTSTR szText = LPSTR_TEXTCALLBACK, LPCRECT lpRectTool = NULL, UINT nIDTool = 0);
	BOOL Create(HWND hWndParent, DWORD dwStyle = 0);
	CToolTipWnd();
	virtual ~CToolTipWnd();

};

