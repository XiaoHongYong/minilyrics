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

	void DelTool(UINT nIDTool);
	BOOL AddTool(LPCTSTR szText = LPSTR_TEXTCALLBACK, LPCRECT lpRectTool = NULL, UINT nIDTool = 0);
	BOOL Create(CBaseWnd *pWndParent);

	bool IsValid() { return m_hWnd != NULL; }

	void Destroy();
	
protected:
	LPVOID		m_hWnd;

};

#endif // !defined(AFX_TOOLTIPWND_H__4D602F6E_4E58_4921_ACDE_3E8DBA83DBBE__INCLUDED_)
