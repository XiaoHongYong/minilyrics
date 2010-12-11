#pragma once

#include "CommonWidget.h"

class CWidgetComboBox : public CWidgetCtrlBase
{
public:
	CWidgetComboBox() { }
	virtual ~CWidgetComboBox();

	int GetCount() const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_GETCOUNT, 0, 0); }
	int GetCurSel() const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_GETCURSEL, 0, 0); }
	int SetCurSel(int nSelect)
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_SETCURSEL, nSelect, 0); }
	DWORD GetEditSel() const
		{ ASSERT(::IsWindow(m_hWnd)); return ::SendMessage(m_hWnd, CB_GETEDITSEL, 0, 0); }
	BOOL LimitText(int nMaxChars)
		{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, CB_LIMITTEXT, nMaxChars, 0); }
	BOOL SetEditSel(int nStartChar, int nEndChar)
		{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, CB_SETEDITSEL, 0, MAKELONG(nStartChar, nEndChar)); }
	DWORD GetItemData(int nIndex) const
		{ ASSERT(::IsWindow(m_hWnd)); return ::SendMessage(m_hWnd, CB_GETITEMDATA, nIndex, 0); }
	int SetItemData(int nIndex, DWORD dwItemData)
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_SETITEMDATA, nIndex, (LPARAM)dwItemData); }
	void* GetItemDataPtr(int nIndex) const
		{ ASSERT(::IsWindow(m_hWnd)); return (LPVOID)GetItemData(nIndex); }
	int SetItemDataPtr(int nIndex, void* pData)
		{ ASSERT(::IsWindow(m_hWnd)); return SetItemData(nIndex, (DWORD)(LPVOID)pData); }
	int GetLBText(int nIndex, LPTSTR lpszText) const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_GETLBTEXT, nIndex, (LPARAM)lpszText); }
	int GetLBTextLen(int nIndex) const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_GETLBTEXTLEN, nIndex, 0); }
	void ShowDropDown(BOOL bShowIt)
		{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, CB_SHOWDROPDOWN, bShowIt, 0); }
	int AddString(LPCTSTR lpszString)
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_ADDSTRING, 0, (LPARAM)lpszString); }
	int DeleteString(UINT nIndex)
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_DELETESTRING, nIndex, 0);}
	int InsertString(int nIndex, LPCTSTR lpszString)
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_INSERTSTRING, nIndex, (LPARAM)lpszString); }
	void ResetContent()
		{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, CB_RESETCONTENT, 0, 0); }
	int FindString(int nStartAfter, LPCTSTR lpszString) const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_FINDSTRING, nStartAfter,
			(LPARAM)lpszString); }
	int SelectString(int nStartAfter, LPCTSTR lpszString)
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_SELECTSTRING,
			nStartAfter, (LPARAM)lpszString); }
	void Clear()
		{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, WM_CLEAR, 0, 0); }
	void Copy()
		{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, WM_COPY, 0, 0); }
	void Cut()
		{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, WM_CUT, 0, 0); }
	void Paste()
		{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, WM_PASTE, 0, 0); }
	int SetItemHeight(int nIndex, UINT cyItemHeight)
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_SETITEMHEIGHT, nIndex, MAKELONG(cyItemHeight, 0)); }
	int GetItemHeight(int nIndex) const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_GETITEMHEIGHT, nIndex, 0L); }
	int FindStringExact(int nIndexStart, LPCTSTR lpszFind) const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_FINDSTRINGEXACT, nIndexStart, (LPARAM)lpszFind); }
	int SetExtendedUI(BOOL bExtended )
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_SETEXTENDEDUI, bExtended, 0L); }
	BOOL GetExtendedUI() const
		{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, CB_GETEXTENDEDUI, 0, 0L); }
	void GetDroppedControlRect(LPRECT lprect) const
		{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, CB_GETDROPPEDCONTROLRECT, 0, (DWORD)lprect); }
	BOOL GetDroppedState() const
		{ ASSERT(::IsWindow(m_hWnd)); return (BOOL)::SendMessage(m_hWnd, CB_GETDROPPEDSTATE, 0, 0L); }
	LCID GetLocale() const
		{ ASSERT(::IsWindow(m_hWnd)); return (LCID)::SendMessage(m_hWnd, CB_GETLOCALE, 0, 0); }
	LCID SetLocale(LCID nNewLocale)
		{ ASSERT(::IsWindow(m_hWnd)); return (LCID)::SendMessage(m_hWnd, CB_SETLOCALE, (WPARAM)nNewLocale, 0); }
	int GetTopIndex() const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_GETTOPINDEX, 0, 0); }
	int SetTopIndex(int nIndex)
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_SETTOPINDEX, nIndex, 0); }
	int InitStorage(int nItems, UINT nBytes)
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_INITSTORAGE, (WPARAM)nItems, nBytes); }
	void SetHorizontalExtent(UINT nExtent)
		{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, CB_SETHORIZONTALEXTENT, nExtent, 0); }
	UINT GetHorizontalExtent() const
		{ ASSERT(::IsWindow(m_hWnd)); return (UINT)::SendMessage(m_hWnd, CB_GETHORIZONTALEXTENT, 0, 0); }
	int SetDroppedWidth(UINT nWidth)
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_SETDROPPEDWIDTH, nWidth, 0); }
	int GetDroppedWidth() const
		{ ASSERT(::IsWindow(m_hWnd)); return (int)::SendMessage(m_hWnd, CB_GETDROPPEDWIDTH, 0, 0); }

	void GetSelItemText(tstring &str);
	
};
