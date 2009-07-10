#include "StdAfx.h"
#include "WidgetCtrlBase.h"

BOOL CWidgetCtrlBase::Attach(CBaseWnd *pWndParent, int nCtrlID)
{
	ASSERT(pWndParent);
	m_pWndParent = pWndParent;
	m_nID = nCtrlID;

	m_window = pWndParent->GetDlgItemHandle(nCtrlID);
	ASSERT(m_window);

	return m_window != NULL;
}
