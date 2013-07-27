// SkinListCtrl.h: interface for the CSkinListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__SKINLISTCTRLEX__H__)
#define __SKINLISTCTRLEX__H__

#pragma once


class CSkinListCtrlEx;


class CSkinListCtrlEx : public CSkinListCtrl
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinListCtrl)
public:
	enum
	{
		TYPE_TEXT_EX	= CColHeader::TYPE_NEXT + 1
	};

	class ItemStringEx : public CSkinListCtrl::ItemString
	{
	public:
		struct TextColor
		{
			TextColor(byte _nCountOfText, byte _nClrIndex) : nCountOfText(_nCountOfText), nClrIndex(_nClrIndex) { }
			byte		nCountOfText;
			byte		nClrIndex;
		};
		class VecTextColor : public vector<TextColor>
		{
		public:
			void Add(byte _nCountOfText, byte _nClrIndex)
			{
				push_back(TextColor(_nCountOfText, _nClrIndex));
			}

		};

		VecTextColor		vTextColor;

	};

public:
	virtual Item *NewItem(int col);

	virtual void AddColumn(LPCTSTR szCol, int nWidth);

	virtual bool SetItemText(int nItem, int nSubItem, LPCTSTR lpszText, bool bRedraw = true);
	virtual int InsertItem(int nItem, LPCTSTR lpszItem, int nImageIndex = 0, UINT nItemData = 0, bool bRedraw = true);

	virtual void DrawCell(int row, int col, RECT &rcCell, CRawGraph *pGraph, MLColor &clrText);

	bool SetItemTextEx(int nItem, int nSubItem, LPCTSTR lpszText, const ItemStringEx::VecTextColor & vTextColor);
	int InsertItem(int nItem, LPCTSTR lpszItem, const ItemStringEx::VecTextColor & vTextColor, int nImageIndex = 0, UINT nItemData = 0, bool bRedraw = false);


	void DrawCellTextEx(ItemStringEx *item, RECT &rcItem, CRawGraph *pGraph, MLColor &clrText);

};

#endif // !defined(__SKINLISTCTRLEX__H__)
