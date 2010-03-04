#include "StdAfx.h"
#include "MLSkin.h"
#include "PopupSkinListWnd.h"

CPopupSkinListWnd::CPopupSkinListWnd()
{
	m_nCurSel = -1;
	m_pListCtrl = NULL;
}

CPopupSkinListWnd::~CPopupSkinListWnd()
{

}

void CPopupSkinListWnd::OnLoadWndSizePos()
{
}

void CPopupSkinListWnd::OnSkinLoaded()
{
	m_pListCtrl = (CSkinListCtrl *)GetUIObjectByClassName(CSkinListCtrl::ClassName());
	ASSERT(m_pListCtrl);
	if (!m_pListCtrl)
		return;

	RegisterUIObjNotifyHandler(m_pListCtrl->m_nId, this);

	m_pListCtrl->InsertColumn(_T("col1"), 100);

	for (int i = 0; i < (int)m_vItems.size(); i++)
	{
		Item &item = m_vItems[i];
		m_pListCtrl->InsertItem(i, item.strText.c_str(), item.nImage, item.nData, false);
	}

	if (m_nCurSel != -1)
		m_pListCtrl->SetItemSelectionState(m_nCurSel, true);

	int		nContentHeight = m_pListCtrl->GetContentHeight();

	if (m_pListCtrl->m_rcObj.Height() != nContentHeight)
	{
		CRect	rcOrg;
		GetWindowRect(&rcOrg);

		CRect	rcNew = rcOrg;
		rcNew.bottom = rcNew.bottom - m_pListCtrl->m_rcObj.Height() + nContentHeight;

		MoveWindow(rcNew, FALSE);
	}
}

void CPopupSkinListWnd::OnUIObjNotify(IUIObjNotify *pNotify)
{
	if (!m_pPopupSkinWndNotify)
		return;

	ASSERT(m_pListCtrl);
	ASSERT(pNotify->IsKindOf(CSkinListCtrl::ClassName()));

	CSkinListCtrlEventNotify *pListCtrlNotify = (CSkinListCtrlEventNotify*)pNotify;
	if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_CLICK)
	{
		m_nCurSel = m_pListCtrl->GetNextSelectedItem();
		m_pPopupSkinWndNotify->PopupSkinWndOnSelected();
		PostDestroy();
	}
}

int CPopupSkinListWnd::GetCount() const
{
	return (int)m_vItems.size();
}

int CPopupSkinListWnd::GetCurSel()
{
	return m_nCurSel;
}

void CPopupSkinListWnd::SetCurSel(int nIndex)
{
	if (nIndex >= m_vItems.size())
		return;

	m_nCurSel = nIndex;
}

int CPopupSkinListWnd::AddString(LPCTSTR szStr)
{
	Item	item(szStr);
	m_vItems.push_back(item);
	return m_vItems.size() - 1;
}

int CPopupSkinListWnd::InsertString(int nIndex, LPCTSTR lpszString)
{
	Item	item(lpszString);
	if (nIndex >= m_vItems.size() || nIndex < 0)
	{
		m_vItems.push_back(item);
		return m_vItems.size() - 1;
	}
	else
	{
		m_vItems.insert(m_vItems.begin() + nIndex, item);
		return nIndex;
	}
}

void CPopupSkinListWnd::SetItemData(int nIndex, UINT nItemData)
{
	if (nIndex < 0 || nIndex >= m_vItems.size())
		return;
	m_vItems[nIndex].nData = nItemData;
}

UINT CPopupSkinListWnd::GetItemData(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_vItems.size())
		return 0;
	return m_vItems[nIndex].nData;
}

UINT CPopupSkinListWnd::GetItemText(int nIndex, tstring &str)
{
	str.clear();
	if (nIndex < 0 || nIndex >= m_vItems.size())
		return 0;

	str = m_vItems[nIndex].strText;

	return str.size();
}

bool CPopupSkinListWnd::DeleteString(UINT nIndex)
{
	if (nIndex < 0 || nIndex >= m_vItems.size())
		return false;

	m_vItems.erase(m_vItems.begin() + nIndex);

	if (m_nCurSel > nIndex)
		m_nCurSel--;

	return true;
}

void CPopupSkinListWnd::ResetContent()
{
	m_vItems.clear();
}

int CPopupSkinListWnd::FindStringExact(int nIndexStart, LPCTSTR lpszFind) const
{
	ASSERT(nIndexStart >= -1);
	for (int i = nIndexStart + 1; i < (int)m_vItems.size(); i++)
	{
		const Item	&item = m_vItems[i];
		if (_tcscmp(item.strText.c_str(), lpszFind) == 0)
			return i;
	}

	return -1;
}

int CPopupSkinListWnd::SelectString(int nStartAfter, LPCTSTR lpszString)
{
	m_nCurSel = FindStringExact(nStartAfter, lpszString);

	return m_nCurSel;
}

void CPopupSkinListWnd::GetSelItemText(tstring &str)
{
	if (m_nCurSel >= 0 && m_nCurSel < (int)m_vItems.size())
		str = m_vItems[m_nCurSel].strText;
}
