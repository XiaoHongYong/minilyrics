// WidgetTabCtrl.cpp: implementation of the CWidgetTabCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WidgetTabCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWidgetTabCtrl::CWidgetTabCtrl()
{
	m_hWnd = NULL;
}

CWidgetTabCtrl::~CWidgetTabCtrl()
{

}

BOOL CWidgetTabCtrl::InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem,
	int nImage, LPARAM lParam)
{
	ASSERT(::IsWindow(m_hWnd));

	TCITEM item;
	item.mask = nMask;
	item.iImage = nImage;
	item.lParam = lParam;
	item.pszText = (LPTSTR) lpszItem;

	return (BOOL) ::SendMessage(m_hWnd, TCM_INSERTITEM, nItem, (LPARAM) &item);
}
