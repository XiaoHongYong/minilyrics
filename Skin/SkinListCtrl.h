// SkinListCtrl.h: interface for the CSkinListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINLISTCTRL_H__D3921B0E_1A2E_423B_B3CD_5E50B2E5E993__INCLUDED_)
#define AFX_SKINLISTCTRL_H__D3921B0E_1A2E_423B_B3CD_5E50B2E5E993__INCLUDED_

#pragma once

#include "../Skin/SkinScrollFrameCtrlBase.h"

class CSkinListCtrl;


class CSkinListCtrlEventNotify : public IUIObjNotify
{
public:
	CSkinListCtrlEventNotify(CUIObject *pObject, int nClickedRow, int nClickedCol) : IUIObjNotify(pObject)
		{ cmd = C_CLICK; this->nClickOnRow = nClickedRow; this->nClickOnCol = nClickedCol; }

	enum Command
	{
		C_CLICK,			// Left button click
		C_CLICK_CMD,		// Left button click at a cell, and the cell will trigger special command
		C_RBTN_CLICK,		// Right mouse button click
		C_DBL_CLICK,
		C_SEL_CHANGED,
		C_ENTER,			// Enter key pressed
		C_KEY_DELETE,		// delete key pressed
	};

	Command				cmd;
	int					nClickOnRow, nClickOnCol;

};

enum ListCtrlColType
{
	LCCT_STRING,
	LCCT_IMAGE,
};

class LC_ITEM
{
public:
	class CColHeader
	{
	public:
		CColHeader(LPCTSTR szTitle, int nWidth, bool bClickable = false) {
			strTitle = szTitle;
			this->nWidth = nWidth;
			this->bClickable = bClickable;
			colType = LCCT_STRING;
		}
		virtual ~CColHeader() { }

		virtual LC_ITEM *NewLCItem() { return new LC_ITEM(); }

		tstring			strTitle;
		int16			nWidth;
		bool			bClickable;
		ListCtrlColType	colType;
	};
public:
	LC_ITEM() { }
	virtual ~LC_ITEM() { }

	virtual bool IsClickedOn(CColHeader *pHeader, int x, int y) { return true; }
	virtual void Draw(CSkinListCtrl *pListCtrl, int nRow, int nClrNameDefault, CRect &rcItem, CRawGraph *pGraph);

public:
	tstring		strText;

};

class LC_IMAGE_ITEM : public LC_ITEM
{
public:
	class CImageColHeader : public CColHeader
	{
	public:
		CImageColHeader(LPCTSTR szTitle, int nWidth, bool bClickable = false) : CColHeader(szTitle, nWidth, bClickable) {
			colType = LCCT_IMAGE;
		}
		virtual ~CImageColHeader() { }

		virtual LC_IMAGE_ITEM *NewLCItem() { return new LC_IMAGE_ITEM(); }
	};

public:
	virtual bool IsClickedOn(CColHeader *pHeader, int x, int y);
	virtual void Draw(CSkinListCtrl *pListCtrl, int nRow, int nClrNameDefault, CRect &rcItem, CRawGraph *pGraph);

public:
	CRawImage	image;

};


class CSkinListCtrl : public CSkinScrollFrameCtrlBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinScrollFrameCtrlBase)
public:
	enum
	{
		F_SELECTED	= 1,
	};

	enum ColorName
	{
		CN_BG,
		CN_ALTER_BG,
		CN_TEXT,
		CN_SEL_BG,
		CN_SEL_TEXT,
		CN_CUSTOMIZED_START,

		CN_MAX = 255
	};

	enum HitTestArea
	{
		HTA_NONE,
		HTA_HEADER,
		HTA_HEADER_SPLIT,
		HTA_ROW,
	};

	struct LC_ROW
	{
		LC_ROW(int nImageIndex, UINT nItemData)
		{
			dwFlags = 0;

			this->nItemData = nItemData;
			this->nImageIndex = nImageIndex;
		}

		~LC_ROW()
		{
			list<LC_ITEM*>::iterator	it;
			for (it = listItems.begin(); it != listItems.end(); ++it)
				delete *it;
			listItems.clear();
		}

		list<LC_ITEM*>			listItems;
		UINT					nItemData;
		int						nImageIndex;
		DWORD					dwFlags;

		bool IsSelected() { return (dwFlags & F_SELECTED) == F_SELECTED; }
		void SetSelected(bool bSelected)
			{ if (bSelected) dwFlags |= F_SELECTED; else dwFlags &= ~F_SELECTED; }

		LC_ITEM *GetSubItem(int n)
		{
			list<LC_ITEM*>::iterator	it, itEnd;
			it = listItems.begin();
			itEnd = listItems.end();
			for (int i = 0; i < n && it != itEnd; i++, ++it)
				;
			if (it != itEnd)
				return (*it);
			return NULL;
		}

	};

public:
	CSkinListCtrl();
	virtual ~CSkinListCtrl();

	void LoadColumnWidth(LPCTSTR szProperty);
	void SaveColumnWidth(LPCTSTR szProperty);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

public:
	void SetBkColor(const MLColor &clrBk);
	void SetSelRowBkColor(const MLColor &clrBk);
	void SetFont(const CFontInfo &font);
	void SetTextColor(const MLColor &clrText);

	void SetColor(int nColorName, const MLColor &clr);
	MLColor & GetColor(int nColorName);

	virtual void AddColumn(LPCTSTR szCol, int nWidth);
	void AddImageColumn(LPCTSTR szCol, int nWidth, bool bClickable = false);

	size_t GetColumnCount() const { return m_vHeading.size(); }
	int GetColumnWidth(int nCol) const;
	BOOL SetColumnWidth(int nCol, int cx);

	void SetLineHeight(int nLineHeight);

	size_t GetItemCount() const { return m_vRows.size(); }

	UINT GetSelectedCount() const;
	int GetNextSelectedItem(int nPos = -1) const;
	bool IsRowSelected(int nRow) const;

	int GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const;
	int GetItemText(int nItem, int nSubItem, tstring &strText) const;
	virtual BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText, bool bRedraw = true);

	virtual BOOL SetItemImage(int nItem, int nSubItem, PRAW_IMAGE_DATA hImage, bool bRedraw = true);

	virtual BOOL SetItemImageIndex(int nItem, int nImageIndex, bool bRedraw = true);

	UINT GetItemData(int nItem);
	void SetItemData(int nItem, UINT nItemData);

	virtual int InsertItem(int nItem, LPCTSTR lpszItem, int nImageIndex = 0, UINT nItemData = 0, bool bRedraw = true);

	BOOL DeleteItem(int nItem, bool bRedraw = true);
	BOOL DeleteSelectedItems(bool bRedraw = true);
	BOOL DeleteAllItems(bool bRedraw = true);

	bool CanOffsetSelectedRow(bool bDown = true);
	bool OffsetAllSelectedRow(bool bDown = true);

	void SetItemSelectionState(int nIndex, bool bSelected);

	void MakeSureRowVisible(int nRow);

	void IncreaseHeight(int nValue);

	int GetContentHeight() const;

	// void HideScrollBar(bool bHorizontal, bool bVertical);

protected:
	class CPaintUpdater
	{
	public:
		CPaintUpdater(CSkinListCtrl *pHost) : m_pHost(pHost)
		{
			// m_nEndSelRow = pHost->m_nEndSelRow;
			m_nFirstVisibleRow = pHost->m_nFirstVisibleRow;
			m_bUpdate = false;
		}

		~CPaintUpdater()
		{
			if (m_pHost->m_pVertScrollBar)
			{
				if (m_pHost->m_nFirstVisibleRow != m_pHost->m_pVertScrollBar->GetScrollPos())
					m_pHost->m_pVertScrollBar->SetScrollPos(m_pHost->m_nFirstVisibleRow);
			}

			if (m_bUpdate)
				goto RET_UPDATE;
			if (m_nFirstVisibleRow != m_pHost->m_nFirstVisibleRow)
				goto RET_UPDATE;

			return;
		RET_UPDATE:
			m_pHost->Invalidate();
		}

		void SetUpdateFlag(bool bUpdate) { m_bUpdate = bUpdate; }

	protected:
		CSkinListCtrl	*m_pHost;
		bool			m_bUpdate;
		int				m_nFirstVisibleRow;

	};
	friend class CPaintUpdater;

	class CSelChangedCheck
	{
	public:
		CSelChangedCheck(CSkinListCtrl *pListCtrl) : m_pListCtrl(pListCtrl)
		{
			int		n = -1;
			while ((n = m_pListCtrl->GetNextSelectedItem(n)) != -1)
				m_vSelected.push_back(n);
		}

		bool IsSelectChanged()
		{
			int		i = 0, n = -1;
			while ((n = m_pListCtrl->GetNextSelectedItem(n)) != -1)
			{
				if (m_vSelected.size() == i || m_vSelected[i] != n)
					return true;
				i++;
			}
			if (i != m_vSelected.size())
				return true;
			return false;
		}

	public:
		CSkinListCtrl	*m_pListCtrl;
		vector<int>		m_vSelected;

	};
	friend class CSelChangedCheck;

protected:
	virtual LC_ITEM *NewLCItem(int nCol);

	void GetRowColAtPoint(POINT pt, int &row, int &col, POINT *ptAtItemCell = NULL, bool *bItemClickable = NULL);

	void SetClickCursor();

	bool ClearAllSelMark();

	int GetLinesOfPerPage() const { return m_rcContent.Height() / m_nLineHeight; }

	int GetLastVisibleRow() const;

	void ReverseSelectStatOfRow(bool bClearOldSel, int nRow);
	void SelectRangeRow(int nBeg, int nEnd);

	void RecalculateColWidth(CRawGraph *pGraph);

	void SetScrollInfo(bool bRedraw = true);

	void SetHorzScrollInfo(bool bRedraw = true);

	virtual void SendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow = -1, int nClickedCol = -1);

	HitTestArea HitTest(POINT &point, int &nColumnIndex);

	void DrawHeader(CRawGraph *pGraph);

	void InvalidateItem(int nItem);

public:
	void Draw(CRawGraph *pGraph);

	void OnKeyDown(UINT nChar, UINT nFlags);

	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDown(UINT nFlags, CPoint point);
	bool OnLButtonDblClk(UINT nFlags, CPoint point);
	bool OnRButtonDown(UINT nFlags, CPoint point);
	bool OnRButtonUp(UINT nFlags, CPoint point);
	bool OnMouseMove(UINT nFlags, CPoint point);

	void OnCreate();
	void OnSize();

	virtual void OnAdjustHue(float hue, float saturation, float luminance);

	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);

protected:
	typedef vector<LC_ROW *>				V_ROWS;
	typedef vector<LC_ITEM::CColHeader *>	V_COLUMNS;

	friend class LC_ITEM;
	friend class LC_ITEMEX;
	friend class CPopupSkinListWnd;

	bool					m_bHoverMouseSel;

	V_COLUMNS				m_vHeading;
	V_ROWS					m_vRows;

	bool					m_bDrawBorder;
	bool					m_bDrawHeader;
	bool					m_bAscending;
	bool					m_bEnableSort;
	int						m_nHeaderHeight;
	CRawPen					m_penLine;
	CSFImage				m_imageSortedHeader;
	CSFImage				m_imageHeader;
	CSFImage				m_imageAscending;
	CSFImage				m_imageDescending;
	int						m_nSortedCol;

	CSFImage				m_imageBorder;
	int						m_nBorderWidth;

	CSFImgList				m_imageList;
	int						m_imageListIconCx;
	int						m_imageListRightSpace;
	tstring					m_strImageList;

	vector<MLColor>			m_vColors;
	CSkinFontProperty		m_font;
	int						m_nBegSelRow, m_nEndSelRow;
	bool					m_bClickBtDown;

	int						m_nLineHeight, m_nLineHeightOrg;
	int						m_nXMargin;
	int						m_nFirstVisibleRow;

	// Drag to adjust the width of columns
	bool					m_bAdjustColWidth;
	int						m_nResizingCol;
	POINT					m_ptStartDrag;					// 鼠标上一次的位置
	CCursor					m_curResizeCol;
	CCursor					m_curHand;

};

#endif // !defined(AFX_SKINLISTCTRL_H__D3921B0E_1A2E_423B_B3CD_5E50B2E5E993__INCLUDED_)
