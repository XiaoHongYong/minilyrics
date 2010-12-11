
#pragma once

#include "WidgetCtrlBase.h"

class CWidgetTabCtrl : public CWidgetCtrlBase
{
public:
	CWidgetTabCtrl();
	virtual ~CWidgetTabCtrl();

public:
	int GetItemCount() const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_GETITEMCOUNT, 0, 0L); }
	BOOL GetItem(int nItem, TCITEM* pTabCtrlItem) const
		{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_GETITEM, nItem, (LPARAM)pTabCtrlItem); }
	BOOL SetItem(int nItem, TCITEM* pTabCtrlItem)
		{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_SETITEM, nItem, (LPARAM)pTabCtrlItem); }

	int GetCurSel() const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_GETCURSEL, 0, 0L); }
	int SetCurSel(int nItem)
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_SETCURSEL, nItem, 0L); }
	int GetCurFocus() const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, TCM_GETCURFOCUS, 0, 0L); }
	void SetCurFocus(int nItem)
		{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, TCM_SETCURFOCUS, (WPARAM) nItem, 0); }

	BOOL InsertItem(int nItem, TCITEM* pTabCtrlItem)
		{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_INSERTITEM, nItem, (LPARAM)pTabCtrlItem); }
	BOOL InsertItem(int nItem, LPCTSTR lpszItem)
		{ ASSERT(::IsWindow(m_hWnd)); return InsertItem(TCIF_TEXT, nItem, lpszItem, 0, 0); }
	BOOL InsertItem(int nItem, LPCTSTR lpszItem, int nImage)
		{ ASSERT(::IsWindow(m_hWnd)); return InsertItem(TCIF_TEXT|TCIF_IMAGE, nItem, lpszItem, nImage, 0); }

	BOOL InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, int nImage, LPARAM lParam);

	BOOL DeleteItem(int nItem)
		{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_DELETEITEM, nItem, 0L); }
	BOOL DeleteAllItems()
		{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, TCM_DELETEALLITEMS, 0, 0L); }

};

