#include "StdAfx.h"
#include "WidgetCtrlBase.h"

BOOL CWidgetCtrlBase::Attach(CBaseWnd *pWndParent, int nCtrlID)
{
	ASSERT(pWndParent);

	m_hWnd = ::GetDlgItem(pWndParent->GetHandle(), nCtrlID);

	return m_hWnd != NULL;
}
