// SkinContainerBar.cpp: implementation of the CSkinComboBox class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinComboBox.h"

UIOBJECT_CLASS_NAME_IMP(CSkinComboBox, _T("ComboBox"))

CSkinComboBox::CSkinComboBox()
{
}

CSkinComboBox::~CSkinComboBox()
{
}

void CSkinComboBox::OnCreate()
{
	CSkinTextButton::OnCreate();

	m_popupListWnd.AddColumn(_T(""), 100);
}

BOOL CSkinComboBox::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinTextButton::SetProperty(szProperty, szValue))
		return TRUE;

	return TRUE;
}

void CSkinComboBox::ButtonDownAction()
{
	CSkinTextButton::ButtonDownAction();

	m_pSkin->ReleaseCaptureMouse(this);

	CRect	rcPopup = m_rcObj;

	rcPopup.DeflateRect(m_rcPadding.left, m_rcPadding.right);

	m_pSkin->ClientToScreen(rcPopup);

	// Popup list window
	m_popupListWnd.Create(m_pSkin->GetSkinFactory(), _T("ComboBoxPopup.xml"),
		m_pSkin, this, rcPopup);
}

void CSkinComboBox::ButtonUpAction()
{
}

void CSkinComboBox::PopupSkinWndOnSelected()
{
	tstring		str;
	if (GetSelItemText(str))
	{
		SetProperty(SZ_PN_TEXT, str.c_str());
		Invalidate();

		CSkinComboBoxEventNotify	event(this);

		event.cmd = CSkinComboBoxEventNotify::C_SEL_CHANGED;
		m_pSkin->DispatchUIObjNotify(&event);
	}
}

void CSkinComboBox::PopupSkinWndOnDestroy()
{

}

// Interface for combo box.
int CSkinComboBox::GetCount() const
{
	return m_popupListWnd.GetItemCount();
}

int CSkinComboBox::GetCurSel()
{
	return m_popupListWnd.GetCurSel();
}

void CSkinComboBox::SetCurSel(int nIndex)
{
	m_popupListWnd.SetCurSel(nIndex);

	tstring		str;
	if (GetSelItemText(str))
		SetProperty(SZ_PN_TEXT, str.c_str());
}

int CSkinComboBox::AddString(LPCTSTR szStr)
{
	return m_popupListWnd.InsertItem(m_popupListWnd.GetItemCount(), szStr);
}

int CSkinComboBox::InsertString(int nIndex, LPCTSTR lpszString)
{
	return m_popupListWnd.InsertItem(nIndex, lpszString);
}

void CSkinComboBox::SetItemData(int nIndex, UINT nItemData)
{
	m_popupListWnd.SetItemData(nIndex, nItemData);
}

UINT CSkinComboBox::GetItemData(int nIndex)
{
	return m_popupListWnd.GetItemData(nIndex);
}

UINT CSkinComboBox::GetItemText(int nIndex, tstring &str)
{
	str = m_popupListWnd.GetItemText(nIndex, 0);
	return str.size();
}

bool CSkinComboBox::DeleteString(UINT nIndex)
{
	return Tobool(m_popupListWnd.DeleteItem(nIndex));
}

void CSkinComboBox::ResetContent()
{
	m_popupListWnd.DeleteAllItems();

	SetProperty(SZ_PN_TEXT, _T(""));
}

bool CSkinComboBox::GetSelItemText(tstring &str)
{
	int n = m_popupListWnd.GetCurSel();
	if (n == -1)
		return false;

	str = m_popupListWnd.GetItemText(n, 0);
	return true;
}
