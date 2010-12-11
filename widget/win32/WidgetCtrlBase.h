// WidgetTreeCtrl.h: interface for the CListBox class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../../WidgetLite/win32/BaseWnd.h"

class CWidgetCtrlBase : public CBaseWnd
{
public:
	BOOL Attach(CBaseWnd *pWndParent, int nCtrlID);

	BOOL Attach(HWND hWnd) { ASSERT(::IsWindow(hWnd)); m_hWnd = hWnd; return hWnd != NULL; }
	BOOL Attach(CBaseWnd *pWnd) { ASSERT(::IsWindow(pWnd->GetHandle())); m_hWnd = pWnd->GetHandle(); return pWnd != NULL; }

};
