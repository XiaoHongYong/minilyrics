// SkinListCtrlEx.cpp: implementation of the CSkinListCtrlEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Helper.h"
#include "SkinListCtrl.h"
#include "SkinListCtrlEx.h"


void LC_ITEMEX::Draw(CSkinListCtrl *pListCtrl, int nRow, int nClrNameDefault, CRect &rcItem, CRawGraph *pGraph)
{
	tstring::size_type			nPos = 0, nLen, i = 0;
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

void CSkinListCtrlEx::AddColumn(LPCTSTR szCol, int nWidth)
{
	m_vHeading.push_back(new LC_ITEMEX::CExColHeader(szCol, nWidth));

	SetHorzScrollInfo(false);
}

BOOL CSkinListCtrlEx::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText, bool bRedraw)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nItem];
		LC_ITEMEX	*item = (LC_ITEMEX*)row->GetSubItem(nSubItem);
		if (item)
		{
			item->strText = lpszText;
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
	int	n = CSkinListCtrl::InsertItem(nItem, lpszItem, nImageIndex, nItemData, false);

	if (n >= 0 && n < m_vRows.size())
	{
		LC_ITEMEX		*item = (LC_ITEMEX *)(m_vRows[n]->listItems.front());
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
