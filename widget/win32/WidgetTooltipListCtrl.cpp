#include "StdAfx.h"
#include "WidgetTooltipListCtrl.h"

#ifndef LVM_SETINFOTIP
#define  LVM_SETINFOTIP         (LVM_FIRST + 173)
typedef struct tagLVSETINFOTIP
{
	UINT cbSize;
	DWORD dwFlags;
	LPWSTR pszText;
	int iItem;
	int iSubItem;
} LVSETINFOTIP, *PLVSETINFOTIP;

#define ListView_SetInfoTip(hwndLV, plvInfoTip)\
	(BOOL)SNDMSG((hwndLV), LVM_SETINFOTIP, (WPARAM)0, (LPARAM)plvInfoTip)

#define LVS_EX_INFOTIP 0x00000400  

#endif // LVM_SETINFOTIP

CWidgetTooltipListCtrl::CWidgetTooltipListCtrl()
{
}

CWidgetTooltipListCtrl::~CWidgetTooltipListCtrl()
{

}

BOOL CWidgetTooltipListCtrl::Attach(CBaseWnd *pWndParent, int nCtrlID)
{
	if (!CWidgetListCtrl::Attach(pWndParent, nCtrlID))
		return FALSE;

	SetExtendedStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT);

	return TRUE;
}

int CWidgetTooltipListCtrl::InsertItem(int nItem, LPCTSTR lpszItem)
{
	int nRet = CWidgetListCtrl::InsertItem(nItem, lpszItem);

	SetToolTip(nRet, 0, lpszItem);

	return nRet;
}

int CWidgetTooltipListCtrl::InsertItem(int nItem, LPCTSTR lpszItem, int nImage)
{
	int nRet = CWidgetListCtrl::InsertItem(nItem, lpszItem, nImage);

	SetToolTip(nRet, 0, lpszItem);

	return nRet;
}

BOOL CWidgetTooltipListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
	BOOL bRet = CWidgetListCtrl::SetItemText(nItem, nSubItem, lpszText);

	SetToolTip(nItem, nSubItem, lpszText);

	return bRet;
}

void CWidgetTooltipListCtrl::SetToolTip(int nItem, int nSubItem, LPCTSTR lpszText)
{
	LVSETINFOTIP		tip;
	tip.cbSize = sizeof(tip);
	tip.dwFlags = 0;
	tip.pszText = (LPWSTR)lpszText;
	tip.iItem = nItem;
	tip.iSubItem = nSubItem;

	// ASSERT(GetWindowLong(m_hWnd, GWL_EXSTYLE) & LVS_EX_INFOTIP);

	ListView_SetInfoTip(m_hWnd, &tip);
}
