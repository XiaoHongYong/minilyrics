// SkinListCtrl.cpp: implementation of the CSkinListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinListCtrl.h"
#include "SkinFrameCtrl.h"


//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinListCtrl, _T("ListCtrl"))

CSkinListCtrl::CSkinListCtrl()
{
	SetDataSource(this);
}


CSkinListCtrl::~CSkinListCtrl()
{
	for (int i = 0; i < (int)m_vRows.size(); i++)
	{
		Row *row = m_vRows[i];
		delete row;
	}
}

int CSkinListCtrl::GetRowCount() const
{
	return (int)m_vRows.size();
}

bool CSkinListCtrl::IsRowSelected(int nRow) const
{
	if (nRow >= 0 && nRow <= (int)m_vRows.size())
	{
		Row	*row = m_vRows[nRow];
		return row->IsSelected();
	}

	return false;
}


void CSkinListCtrl::AddColumn(LPCTSTR szCol, int nWidth, int colType, bool bClickable)
{
	CSkinListView::AddColumn(szCol, nWidth, colType, bClickable);

	int col = GetColumnCount() - 1;

	for (uint i = 0; i < m_vRows.size(); i++)
	{
		Row *row = m_vRows[i];
		row->vItems.push_back(NewItem(col));
	}
}

int CSkinListCtrl::GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	LPCTSTR szText = GetCellText(nItem, nSubItem);
	if (szText != NULL)
	{
		_tcscpy_safe(lpszText, nLen, szText);
		return (int)_tcslen(szText);
	}

	EmptyStr(lpszText);

	return -1;
}


int CSkinListCtrl::GetItemText(int nItem, int nSubItem, tstring &strText)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	LPCTSTR szText = GetCellText(nItem, nSubItem);
	if (szText != NULL)
	{
		strText = szText;
		return strText.size();
	}

	strText = _T("");

	return -1;
}


bool CSkinListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText, bool bRedraw)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		Row *row = m_vRows[nItem];
		if (m_vHeading[nSubItem]->colType == CColHeader::TYPE_TEXT)
		{
			ItemString *item = (ItemString *)row->vItems[nSubItem];
			item->text = lpszText;
			if (bRedraw)
				InvalidateItem(nItem);
			return TRUE;
		}
	}

	return FALSE;
}

bool CSkinListCtrl::SetItemImage(int nItem, int nSubItem, PRAW_IMAGE_DATA hImage, bool bRedraw)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		Row		*row = m_vRows[nItem];
		if (m_vHeading[nSubItem]->colType == CColHeader::TYPE_IMAGE)
		{
			ItemImage *item = (ItemImage *)row->vItems[nSubItem];
			item->image.Attach(hImage);
			if (bRedraw)
				InvalidateItem(nItem);
			return TRUE;
		}
	}

	return FALSE;
}

int CSkinListCtrl::InsertItem(int nItem, LPCTSTR lpszItem, int nImageIndex, UINT nItemData, bool bRedraw)
{
	ASSERT(m_vHeading.size() > 0);
	if (m_vHeading.size() == 0)
		return -1;

	Row	*row = new Row(nImageIndex, nItemData);

	// alloc space for the new row
	for (int i = 0; i < (int)m_vHeading.size(); i++)
	{
		row->vItems.push_back(NewItem(i));
	}

	if (m_vHeading[0]->colType == CColHeader::TYPE_TEXT)
	{
		ItemString *itemStr = (ItemString *)row->vItems[0];
		itemStr->text = lpszItem;
	}

	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		m_vRows.insert(m_vRows.begin() + nItem, row);
	}
	else
	{
		m_vRows.push_back(row);
		nItem = m_vRows.size() - 1;
	}

	SetScrollInfo(bRedraw);

	if (bRedraw && nItem <= GetLastVisibleRow())
		Invalidate();

	return nItem;
}


bool CSkinListCtrl::DeleteItem(int nItem, bool bRedraw)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		Row		*row = m_vRows[nItem];
		m_vRows.erase(m_vRows.begin() + nItem);
		delete row;

		if (bRedraw)
			Invalidate();

		return TRUE;
	}
	else
		return FALSE;
}


bool CSkinListCtrl::DeleteSelectedItems(bool bRedraw)
{
	for (int i = (int)m_vRows.size() - 1; i >= 0; i--)
	{
		Row		*row = m_vRows[i];
		if (row->IsSelected())
		{
			delete row;
			m_vRows.erase(m_vRows.begin() + i);
		}
	}

	if (bRedraw)
		Invalidate();

	return TRUE;
}


bool CSkinListCtrl::DeleteAllItems(bool bRedraw)
{
	for (int i = 0; i < (int)m_vRows.size(); i++)
	{
		Row		*row = m_vRows[i];
		delete row;
	}
	m_vRows.clear();

	m_nEndSelRow = -1;
	m_nBegSelRow = -1;

	if (bRedraw)
		Invalidate();

	if (m_pVertScrollBar)
	{
		m_pVertScrollBar->SetScrollPos(0);
		m_pVertScrollBar->DisableScrollBar();
	}

// 	if (m_pHorzScrollBar)
// 	{
// 		m_pHorzScrollBar->SetScrollPos(0);
// 		m_pHorzScrollBar->DisableScrollBar();
// 	}

	return TRUE;
}

void CSkinListCtrl::SetRowSelectionState(int nIndex, bool bSelected)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vRows.size());
	if (nIndex < 0 || nIndex >= (int)m_vRows.size())
		return;

	Row		*row = m_vRows[nIndex];
	row->SetSelected(bSelected);
}

int CSkinListCtrl::GetItemImageIndex(int row)
{
	ASSERT(row >= 0 && row < m_vRows.size());
	if (row < 0 || row >= (int)m_vRows.size())
		return -1;

	return m_vRows[row]->nImageIndex;
}

bool CSkinListCtrl::SetItemImageIndex(int nItem, int nImageIndex, bool bRedraw)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		Row		*row = m_vRows[nItem];
		row->nImageIndex = nImageIndex;
		if (bRedraw)
			InvalidateItem(nItem);
		return TRUE;
	}

	return FALSE;
}

UINT CSkinListCtrl::GetItemData(int nItem)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		Row		*row = m_vRows[nItem];
		return row->nItemData;
	}

	return -1;
}

void CSkinListCtrl::SetItemData(int nItem, UINT nItemData)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		Row		*row = m_vRows[nItem];
		row->nItemData = nItemData;
	}
}

LPCTSTR CSkinListCtrl::GetCellText(int row, int col)
{
	ASSERT(row >= 0 && row < m_vRows.size());
	if (row < 0 || row >= (int)m_vRows.size())
		return _T("");

	Row *pRow = m_vRows[row];
	if (col < 0 || col >= (int)pRow->vItems.size())
		return _T("");

	ItemString *item = (ItemString*) pRow->vItems[col];
	return item->text.c_str();
}

CRawImage *CSkinListCtrl::GetCellImage(int row, int col)
{
	ASSERT(row >= 0 && row < m_vRows.size());
	if (row < 0 || row >= (int)m_vRows.size())
		return NULL;

	Row *pRow = m_vRows[row];
	if (col < 0 || col >= (int)pRow->vItems.size())
		return NULL;

	ItemImage *item = (ItemImage*) pRow->vItems[col];
	return &item->image;
}

CSkinListCtrl::Item *CSkinListCtrl::NewItem(int nCol)
{
	ASSERT(nCol >= 0 && nCol < (int)m_vHeading.size());
	int type = m_vHeading[nCol]->colType;
	if (type == CColHeader::TYPE_TEXT)
		return new ItemString();
	else if (type == CColHeader::TYPE_IMAGE)
		return new ItemImage();
	else
	{
		ASSERT(0 && "Undefined column type.");
		return NULL;
	}
}

bool CSkinListCtrl::CanOffsetSelectedRow(bool bDown)
{
	if (m_vRows.size() == 0)
		return false;

	if (GetNextSelectedItem() == -1)
		return false;

	if (bDown)
	{
		if (m_vRows.back()->IsSelected())
			return false;
	}
	else
	{
		// move up
		if (m_vRows.front()->IsSelected())
			return false;
	}

	return true;
}


bool CSkinListCtrl::OffsetAllSelectedRow(bool bDown)
{
	if (m_vRows.size() == 0)
		return false;

	if (bDown)
	{
		if (m_vRows.back()->IsSelected())
			return false;

		for (int i = (int)m_vRows.size() - 1; i >= 0; i--)
		{
			Row		*row = m_vRows[i];
			if (row->IsSelected())
			{
				Row	*rowTemp;
				rowTemp = m_vRows[i + 1];
				m_vRows[i + 1] = m_vRows[i];
				m_vRows[i] = rowTemp;
			}
		}
	}
	else
	{
		// move up
		if (m_vRows.front()->IsSelected())
			return false;

		for (int i = 0; i < (int)m_vRows.size(); i++)
		{
			Row		*row = m_vRows[i];
			if (row->IsSelected())
			{
				Row	*rowTemp;
				rowTemp = m_vRows[i - 1];
				m_vRows[i - 1] = m_vRows[i];
				m_vRows[i] = rowTemp;
			}
		}
	}

	Invalidate();
	return true;
}
