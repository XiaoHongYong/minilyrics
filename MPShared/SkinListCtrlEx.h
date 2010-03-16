// SkinListCtrl.h: interface for the CSkinListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__SKINLISTCTRLEX__H__)
#define __SKINLISTCTRLEX__H__

#pragma once


class CSkinListCtrlEx;


class LC_ITEMEX : public LC_ITEM
{
public:
	class CExColHeader : public CColHeader
	{
	public:
		CExColHeader(LPCTSTR szTitle, int nWidth) : CColHeader(szTitle, nWidth) {
			colType = LCCT_STRING;
		}

		virtual LC_ITEMEX *NewLCItem() { return new LC_ITEMEX(); }
	};

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

public:
	LC_ITEMEX() { }
	virtual ~LC_ITEMEX() { }

	virtual void Draw(CSkinListCtrl *pListCtrl, int nRow, int nClrNameDefault, CRect &rcItem, CRawGraph *pGraph);

public:
	VecTextColor		vTextColor;

};

class CSkinListCtrlEx : public CSkinListCtrl
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinListCtrl)
public:
	CSkinListCtrlEx();
	virtual ~CSkinListCtrlEx();

public:
	virtual void AddColumn(LPCTSTR szCol, int nWidth);

	virtual BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText, bool bRedraw = true);
	virtual int InsertItem(int nItem, LPCTSTR lpszItem, int nImageIndex = 0, UINT nItemData = 0, bool bRedraw = true);

	BOOL SetItemTextEx(int nItem, int nSubItem, LPCTSTR lpszText, const LC_ITEMEX::VecTextColor & vTextColor);
	int InsertItem(int nItem, LPCTSTR lpszItem, const LC_ITEMEX::VecTextColor & vTextColor, int nImageIndex = 0, UINT nItemData = 0, bool bRedraw = false);

public:

};

#endif // !defined(__SKINLISTCTRLEX__H__)
