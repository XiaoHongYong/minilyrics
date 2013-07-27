// SkinListCtrlEx.cpp: implementation of the CSkinListCtrlEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Helper.h"
#include "SkinListCtrlEx.h"


UIOBJECT_CLASS_NAME_IMP(CSkinListCtrlEx, _T("ListCtrlEx"))

CSkinListCtrl::Item *CSkinListCtrlEx::NewItem(int col)
{
	int colType = m_vHeading[col]->colType;
	if (colType == TYPE_TEXT_EX)
		return new ItemStringEx();
	else
		return CSkinListCtrl::NewItem(col);
}

void CSkinListCtrlEx::AddColumn(LPCTSTR szCol, int nWidth)
{
	CSkinListCtrl::AddColumn(szCol, nWidth, TYPE_TEXT_EX);
}

bool CSkinListCtrlEx::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText, bool bRedraw)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		Row		*row = m_vRows[nItem];
		ItemStringEx	*item = (ItemStringEx*)row->GetSubItem(nSubItem);
		if (item)
		{
			item->text = lpszText;
			item->vTextColor.clear();
			if (bRedraw)
				InvalidateItem(nItem);
			return TRUE;
		}
	}

	return FALSE;
}


int CSkinListCtrlEx::InsertItem(int nItem, LPCTSTR lpszItem, int nImageIndex, UINT nItemData, bool bRedraw)
{
	ItemStringEx::VecTextColor	vTextColor;

	return InsertItem(nItem, lpszItem, vTextColor, nImageIndex, nItemData, bRedraw);
}


bool CSkinListCtrlEx::SetItemTextEx(int nItem, int nSubItem, LPCTSTR lpszText, const ItemStringEx::VecTextColor & vTextColor)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		Row		*row = m_vRows[nItem];
		ItemStringEx		*item = (ItemStringEx*)row->GetSubItem(nSubItem);
		if (item)
		{
			item->text = lpszText;
			item->vTextColor = vTextColor;
			return TRUE;
		}
	}

	return FALSE;
}


int CSkinListCtrlEx::InsertItem(int nItem, LPCTSTR lpszItem, const ItemStringEx::VecTextColor & vTextColor, int nImageIndex, UINT nItemData, bool bRedraw)
{
	int	n = CSkinListCtrl::InsertItem(nItem, lpszItem, nImageIndex, nItemData, false);

	if (n >= 0 && n < (int)m_vRows.size())
	{
		ItemStringEx		*item = (ItemStringEx *)(m_vRows[n]->vItems.front());
		item->vTextColor = vTextColor;
	}

	if (bRedraw)
	{
		SetScrollInfo(true);

		if (nItem >= m_nFirstVisibleRow && nItem <= m_nFirstVisibleRow + m_rcObj.Height() / m_nLineHeight)
			Invalidate();
	}

	return nItem;
}

void CSkinListCtrlEx::DrawCell(int row, int col, RECT &rcCell, CRawGraph *pGraph, MLColor &clrText)
{
	int colType = m_vHeading[col]->colType;
	if (colType == TYPE_TEXT_EX)
	{
		Row *pRow = m_vRows[row];
		return DrawCellTextEx((ItemStringEx*) pRow->vItems[col], rcCell, pGraph, clrText);
	}
	else
		return CSkinListCtrl::DrawCell(row, col, rcCell, pGraph, clrText);
}

void CSkinListCtrlEx::DrawCellTextEx(ItemStringEx *item, RECT &rcItem, CRawGraph *pGraph, MLColor &clrText)
{
	tstring::size_type			nPos = 0, nLen, i = 0;
	CRect		rc = rcItem;
	SIZE		size;

	while (nPos < item->text.size())
	{
		MLColor clr;
		if (i >= item->vTextColor.size())
		{
			nLen = item->text.size() - nPos;
			clr = clrText;
		}
		else
		{
			nLen = item->vTextColor[i].nCountOfText;
			if (nPos + nLen > item->text.size())
				nLen = item->text.size() - nPos;
			clr = GetColor(item->vTextColor[i].nClrIndex);
		}

		pGraph->SetTextColor(clr);

		if (m_font.IsOutlined())
		{
			pGraph->DrawTextOutlined(item->text.c_str() + nPos, nLen, &rc, 
				clr, 
				m_font.GetColorOutlined(), DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
		}
		else
			pGraph->DrawText(item->text.c_str() + nPos, nLen, &rc, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);

		pGraph->GetTextExtentPoint32(item->text.c_str() + nPos, nLen, &size);

		nPos += nLen;
		rc.left += size.cx;
		i++;
	}
}

