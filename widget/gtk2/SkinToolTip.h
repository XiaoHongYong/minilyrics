// ToolTipWnd.h: interface for the CToolTipWnd class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SKIN_TOOL_TOP_INC_
#define _SKIN_TOOL_TOP_INC_

#include "BaseWnd.h"

class CSkinToolTip
{
public:
	CSkinToolTip();
	virtual ~CSkinToolTip();

	// void RelayEvent(LPMSG lpMsg);
	void DelTool(UINT nIDTool);
	BOOL AddTool(LPCTSTR szText, LPCRECT lpRectTool = NULL, UINT nIDTool = 0);
	BOOL Create(CBaseWnd *pWndParent);

	bool IsValid();
	bool IsWindow();

	void Destroy();

};

#endif // !defined(AFX_TOOLTIPWND_H__4D602F6E_4E58_4921_ACDE_3E8DBA83DBBE__INCLUDED_)
