#include "StdAfx.h"
#include "WidgetListBox.h"

int CWidgetListBox::GetCount() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_GETCOUNT, 0, 0);
}

int CWidgetListBox::GetCurSel() 
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_GETCURSEL, 0, 0);
}

int CWidgetListBox::SetCurSel(int nSelect)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_SETCURSEL, nSelect, 0);
}

// retrieve from a list box the width, in pixels, by which the list box can 
// be scrolled horizontally (the scrollable width) if the list box has a 
// horizontal scroll bar. 
int CWidgetListBox::GetHorizontalExtent() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_GETHORIZONTALEXTENT, 0, 0);
}

void CWidgetListBox::SetHorizontalExtent(int cxExtent)
{
	ASSERT(::IsWindow(m_hWnd));
	::SendMessage(m_hWnd, LB_SETHORIZONTALEXTENT, cxExtent, 0);
}

int CWidgetListBox::GetSelCount() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_GETSELCOUNT, 0, 0);
}

void CWidgetListBox::GetText(int nIndex, LPTSTR lpszBuffer) const
{
	ASSERT(::IsWindow(m_hWnd));
	::SendMessage(m_hWnd, LB_GETTEXT, nIndex, (LPARAM)lpszBuffer);
}

void CWidgetListBox::GetText(int nIndex, tstring &str) const
{
	TCHAR		szBuffer[512];
	::SendMessage(m_hWnd, LB_GETTEXT, nIndex, (LPARAM)szBuffer);
	str = szBuffer;
}

int CWidgetListBox::GetTextLen(int nIndex) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_GETTEXTLEN, nIndex, 0);
}

int CWidgetListBox::FindStringExact(int nIndexStart, LPCTSTR lpszFind) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_FINDSTRINGEXACT, nIndexStart, (LPARAM)lpszFind);
}

int CWidgetListBox::AddString(LPCTSTR lpszItem)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_ADDSTRING, 0, (LPARAM)lpszItem);
}

int CWidgetListBox::DeleteString(UINT nIndex)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_DELETESTRING, nIndex, 0);
}

int CWidgetListBox::InsertString(int nIndex, LPCTSTR lpszItem)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_INSERTSTRING, nIndex, (LPARAM)lpszItem);
}

int CWidgetListBox::SetItemData(int nIndex, DWORD dwItemData)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_SETITEMDATA, nIndex, (LPARAM)dwItemData);
}

DWORD CWidgetListBox::GetItemData(int nIndex) const
{
	ASSERT(::IsWindow(m_hWnd));
	return ::SendMessage(m_hWnd, LB_GETITEMDATA, nIndex, 0);
}

void CWidgetListBox::ResetContent()
{
	ASSERT(::IsWindow(m_hWnd));
	::SendMessage(m_hWnd, LB_RESETCONTENT, 0, 0);
}

int CWidgetListBox::FindString(int nStartAfter, LPCTSTR lpszItem) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_FINDSTRING,
		nStartAfter, (LPARAM)lpszItem);
}

int CWidgetListBox::SelectString(int nStartAfter, LPCTSTR lpszItem)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LB_SELECTSTRING,
		nStartAfter, (LPARAM)lpszItem);
}
