// ToolTipWnd.h: interface for the CToolTipWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOOLTIPWND_H__4D602F6E_4E58_4921_ACDE_3E8DBA83DBBE__INCLUDED_)
#define AFX_TOOLTIPWND_H__4D602F6E_4E58_4921_ACDE_3E8DBA83DBBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseWnd.h"

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

#endif // !defined(AFX_TOOLTIPWND_H__4D602F6E_4E58_4921_ACDE_3E8DBA83DBBE__INCLUDED_)
