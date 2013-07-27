// SkinListCtrl.h: interface for the CSkinListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINLISTCTRL_H__D3921B0E_1A2E_423B_B3CD_5E50B2E5E993__INCLUDED_)
#define AFX_SKINLISTCTRL_H__D3921B0E_1A2E_423B_B3CD_5E50B2E5E993__INCLUDED_

#pragma once

class CSkinListCtrl;

#include "SkinListView.h"

class CSkinListCtrl : public CSkinListView, public IListViewDataSource
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinListView)
public:
	class Item
	{
	public:
		virtual ~Item() { }
	};

	class ItemString : public Item
	{
	public:
		ItemString() { }
		ItemString(LPCTSTR str) { text = str; }
		tstring			text;
	};

	class ItemImage : public Item
	{
	public:
		ItemImage() { }
		ItemImage(PRAW_IMAGE_DATA hImage) { image.Attach(hImage); }
		CRawImage		image;
	};

	struct Row
	{
		Row(int nImageIndex, UINT nItemData)
		{
			dwFlags = 0;

			this->nItemData = nItemData;
			this->nImageIndex = nImageIndex;
		}

		~Row()
		{
			vector<Item*>::iterator	it;
			for (it = vItems.begin(); it != vItems.end(); ++it)
				delete (*it);
			vItems.clear();
		}

		enum
		{
			F_SELECTED	= 1,
		};

		vector<Item *>			vItems;
		UINT					nItemData;
		int						nImageIndex;
		DWORD					dwFlags;

		bool IsSelected() { return (dwFlags & F_SELECTED) == F_SELECTED; }
		void SetSelected(bool bSelected)
		{ if (bSelected) dwFlags |= F_SELECTED; else dwFlags &= ~F_SELECTED; }

		Item *GetSubItem(int n)
		{
			if (n >= (int)vItems.size() || n < 0)
				return NULL;
			return vItems[n];
		}

	};

public:
	CSkinListCtrl();
	virtual ~CSkinListCtrl();

	virtual void AddImageColumn(LPCTSTR szCol, int nWidth, bool bClickable = false) { AddColumn(szCol, nWidth, CColHeader::TYPE_IMAGE, bClickable); }

	int GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const;
	int GetItemText(int nItem, int nSubItem, tstring &strText) const;
	virtual bool SetItemText(int nItem, int nSubItem, LPCTSTR lpszText, bool bRedraw = true);

	virtual bool SetItemImage(int nItem, int nSubItem, PRAW_IMAGE_DATA hImage, bool bRedraw = true);

	virtual int InsertItem(int nItem, LPCTSTR lpszItem, int nImageIndex = 0, UINT nItemData = 0, bool bRedraw = true);
	size_t GetItemCount() const { return m_vRows.size(); }

	bool DeleteItem(int nItem, bool bRedraw = true);
	bool DeleteSelectedItems(bool bRedraw = true);
	bool DeleteAllItems(bool bRedraw = true);

	bool CanOffsetSelectedRow(bool bDown = true);
	bool OffsetAllSelectedRow(bool bDown = true);

	UINT GetItemData(int nItem);
	void SetItemData(int nItem, UINT nItemData);

	//
	// Methods for IListViewDataSource
	//

	virtual int GetRowCount() const;

	virtual bool IsRowSelected(int row) const;
	virtual void SetRowSelectionState(int nIndex, bool bSelected);

	virtual int GetItemImageIndex(int row);
	virtual bool SetItemImageIndex(int nItem, int nImageIndex, bool bRedraw = true);

	virtual LPCTSTR GetCellText(int row, int col) const;
	virtual CRawImage *GetCellImage(int row, int col) const;

protected:
	Item *NewItem(int nCol);

protected:
	typedef vector<Row *>				V_ROWS;

	friend class LC_ITEM;
	friend class LC_ITEMEX;
	friend class CPopupSkinListWnd;

	V_ROWS					m_vRows;

};

#endif // !defined(AFX_SKINLISTCTRL_H__D3921B0E_1A2E_423B_B3CD_5E50B2E5E993__INCLUDED_)
