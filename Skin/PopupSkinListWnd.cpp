#include "StdAfx.h"
#include "Skin.h"
#include "PopupSkinListWnd.h"

CPopupSkinListWnd::CPopupSkinListWnd()
{
	m_nCurSel = -1;
	m_pListCtrl = NULL;
	m_nLineHeight = 0;

	m_pListCtrl = &m_listCtrlData;
}

CPopupSkinListWnd::~CPopupSkinListWnd()
{

}

void CPopupSkinListWnd::OnLoadWndSizePos()
{
}

void CPopupSkinListWnd::OnSkinLoaded()
{
	CSkinListCtrl *pListCtrl = (CSkinListCtrl *)GetUIObjectByClassName(CSkinListCtrl::ClassName());
	ASSERT(pListCtrl);
	if (!pListCtrl)
		return;

	m_pListCtrl = pListCtrl;

	m_pListCtrl->m_imageListRightSpace = 14;

	m_pListCtrl->m_vHeading = m_listCtrlData.m_vHeading;
	m_pListCtrl->m_vRows = m_listCtrlData.m_vRows;

	m_listCtrlData.m_vHeading.clear();
	m_listCtrlData.m_vRows.clear();

	if (m_nLineHeight > 0)
		m_pListCtrl->SetLineHeight(m_nLineHeight);

	if (m_nCurSel != -1)
		m_pListCtrl->SelectRangeRow(m_nCurSel, m_nCurSel);

	int		nContentHeight = m_pListCtrl->GetContentHeight();
	CRect	rcOrg;
	GetWindowRect(&rcOrg);

	CRect	rcNew = rcOrg;
	CRect	rcMonitor;
	GetMonitorRestrictRect(rcOrg, rcMonitor);

	if (m_pListCtrl->m_rcObj.Height() != nContentHeight)
	{
		rcNew.bottom = rcNew.bottom - m_pListCtrl->m_rcObj.Height() + nContentHeight;

		if (rcNew.bottom > rcMonitor.bottom)
		{
			rcNew.top -= rcNew.bottom - rcMonitor.bottom;
			if (rcNew.top < rcMonitor.top)
				rcNew.top = rcMonitor.top;
			rcNew.bottom = rcMonitor.bottom;
		}
	}

	if (rcNew.right > rcMonitor.right)
	{
		rcNew.left -= rcNew.right - rcMonitor.right;
		rcNew.right = rcMonitor.right;
	}

	if (rcNew != rcOrg)
		MoveWindow(rcNew, FALSE);

	CPopupSkinWnd::OnSkinLoaded();
}

void CPopupSkinListWnd::CloseSkin()
{
	if (m_bSkinOpened && m_pListCtrl)
	{
		m_listCtrlData.m_vHeading = m_pListCtrl->m_vHeading;
		m_listCtrlData.m_vRows = m_pListCtrl->m_vRows;

		m_pListCtrl->m_vHeading.clear();
		m_pListCtrl->m_vRows.clear();

		m_pListCtrl = &m_listCtrlData;
	}

	CPopupSkinWnd::CloseSkin();
}

void CPopupSkinListWnd::OnUIObjNotify(IUIObjNotify *pNotify)
{
	if (!m_pPopupSkinWndNotify)
		return;

	ASSERT(m_pListCtrl);
	ASSERT(pNotify->IsKindOf(CSkinListCtrl::ClassName()));

	CSkinListCtrlEventNotify *pListCtrlNotify = (CSkinListCtrlEventNotify*)pNotify;
	if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_CLICK
		|| pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_ENTER)
	{
		SetCurSel(m_pListCtrl->GetNextSelectedItem());
		m_pPopupSkinWndNotify->PopupSkinWndOnSelected();
		PostDestroy();
	}
}

int CPopupSkinListWnd::GetCurSel()
{
	return m_nCurSel;
}

int CPopupSkinListWnd::GetItemCount() const
{
	if (m_pListCtrl)
		return m_pListCtrl->GetItemCount();
	else
	{
		ASSERT(0);
		return 0;
	}
}

void CPopupSkinListWnd::SetCurSel(int nIndex)
{
	if (nIndex >= (int)GetItemCount())
		return;

	ASSERT(m_pListCtrl);
	if (!m_pListCtrl)
		return;

	if (m_nCurSel != -1)
		m_pListCtrl->SetItemImageIndex(m_nCurSel, 0);

	m_nCurSel = nIndex;

	if (m_nCurSel != -1)
		m_pListCtrl->SetItemImageIndex(m_nCurSel, 1);
}

BOOL CPopupSkinListWnd::DeleteItem(int nItem)
{
	ASSERT(m_pListCtrl);
	if (m_pListCtrl)
		return m_pListCtrl->DeleteItem(nItem, false);
	else
		return FALSE;
}

BOOL CPopupSkinListWnd::DeleteAllItems()
{
	ASSERT(m_pListCtrl);
	if (m_pListCtrl)
		return m_pListCtrl->DeleteAllItems(false);
	else
		return FALSE;
}

int CPopupSkinListWnd::InsertItem(int nItem, LPCTSTR lpszItem, int nImageIndex, UINT nItemData)
{
	ASSERT(m_pListCtrl);
	if (m_pListCtrl)
		return m_pListCtrl->InsertItem(nItem, lpszItem, nImageIndex, nItemData, false);
	else
		return -1;
}

BOOL CPopupSkinListWnd::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
	ASSERT(m_pListCtrl);
	if (m_pListCtrl)
		return m_pListCtrl->SetItemText(nItem, nSubItem, lpszText);
	else
		return FALSE;
}

BOOL CPopupSkinListWnd::SetItemImage(int nItem, int nSubItem, PRAW_IMAGE_DATA hImage)
{
	ASSERT(m_pListCtrl);
	if (m_pListCtrl)
		return m_pListCtrl->SetItemImage(nItem, nSubItem, hImage);
	else
		return FALSE;
}

tstring CPopupSkinListWnd::GetItemText(int nItem, int nSubItem) const
{
	tstring	str;
	ASSERT(m_pListCtrl);
	if (m_pListCtrl)
		m_pListCtrl->GetItemText(nItem, nSubItem, str);
	return str;
}

void CPopupSkinListWnd::SetItemData(int nItem, UINT nItemData)
{
	ASSERT(m_pListCtrl);
	if (m_pListCtrl)
		m_pListCtrl->SetItemData(nItem, nItemData);
}

UINT CPopupSkinListWnd::GetItemData(int nItem)
{
	ASSERT(m_pListCtrl);
	if (m_pListCtrl)
		return m_pListCtrl->GetItemData(nItem);
	else
		return 0;
}
