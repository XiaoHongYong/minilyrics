// SkinListCtrlEx.cpp: implementation of the CSkinListCtrlEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Helper.h"
#include "SkinListCtrl.h"
#include "SkinListCtrlEx.h"


void LC_ITEMEX::Draw(CSkinListCtrl *pListCtrl, int nRow, int nClrNameDefault, CRect &rcItem, CRawGraph *pGraph)
{
	int			nPos = 0, nLen, i = 0;
	CRect		rc = rcItem;
	SIZE		size;
	int			nClrName;

	while (nPos < strText.size())
	{
		if (i >= vTextColor.size())
		{
			nLen = strText.size() - nPos;
			if (nRow == pListCtrl->m_nPlayingRow)
				nClrName = CSkinListCtrl::CN_PLAYING_TEXT;
			else
				nClrName = nClrNameDefault;
		}
		else
		{
			nLen = vTextColor[i].nCountOfText;
			if (nPos + nLen > strText.size())
				nLen = strText.size() - nPos;
			nClrName = vTextColor[i].nClrIndex;
		}

		pGraph->SetTextColor(pListCtrl->GetColor(nClrName));

		if (pListCtrl->m_font.IsOutlined())
		{
			pGraph->DrawTextOutlined(strText.c_str() + nPos, nLen, &rc, 
				pListCtrl->GetColor(nClrName), 
				pListCtrl->m_font.GetColorOutlined(), DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
		}
		else
			pGraph->DrawText(strText.c_str() + nPos, nLen, &rc, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);

		pGraph->GetTextExtentPoint32(strText.c_str() + nPos, nLen, &size);

		nPos += nLen;
		rc.left += size.cx;
		i++;
	}
}


UIOBJECT_CLASS_NAME_IMP(CSkinListCtrlEx, _T("ListCtrlEx"))


CSkinListCtrlEx::CSkinListCtrlEx()
{
}


CSkinListCtrlEx::~CSkinListCtrlEx()
{
}


BOOL CSkinListCtrlEx::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nItem];
		LC_ITEMEX		*item = (LC_ITEMEX*)row->GetSubItem(nSubItem);
		if (item)
		{
			item->strText = lpszText;
			item->vTextColor.clear();
			return TRUE;
		}
	}

	return FALSE;
}


int CSkinListCtrlEx::InsertItem(int nItem, LPCTSTR lpszItem, int nImageIndex, UINT nItemData, bool bRedraw)
{
	LC_ITEMEX::VecTextColor	vTextColor;

	return InsertItem(nItem, lpszItem, vTextColor, nImageIndex, nItemData, bRedraw);
}


BOOL CSkinListCtrlEx::SetItemTextEx(int nItem, int nSubItem, LPCTSTR lpszText, const LC_ITEMEX::VecTextColor & vTextColor)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nItem];
		LC_ITEMEX		*item = (LC_ITEMEX*)row->GetSubItem(nSubItem);
		if (item)
		{
			item->strText = lpszText;
			item->vTextColor = vTextColor;
			return TRUE;
		}
	}

	return FALSE;
}


int CSkinListCtrlEx::InsertItem(int nItem, LPCTSTR lpszItem, const LC_ITEMEX::VecTextColor & vTextColor, int nImageIndex, UINT nItemData, bool bRedraw)
{
	LC_ROW		*row = new LC_ROW;
	LC_ITEMEX		*item = NULL;

	row->nItemData = nItemData;
	row->nImageIndex = nImageIndex;

	item = new LC_ITEMEX;
	item->strText = lpszItem;
	item->vTextColor = vTextColor;
	row->listItems.push_back(item);

	// alloc space for the new item column
	for (int i = 1; i < (int)m_vHeading.size(); i++)
	{
		item = new LC_ITEMEX;
		row->listItems.push_back(item);
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

	if (bRedraw && nItem >= m_nFirstVisibleRow && nItem <= m_nFirstVisibleRow + m_rcObj.Height() / m_nLineHeight)
		Invalidate();

	return nItem;
}
