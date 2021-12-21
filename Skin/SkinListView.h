// SkinListCtrl.h: interface for the CSkinListView class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../Skin/SkinScrollFrameCtrlBase.h"

class CSkinListView;

class CColHeader
{
public:
	CColHeader(LPCTSTR szTitle, int nWidth, int colType, bool bClickable = false) {
		this->strTitle = szTitle;
		this->nWidth = nWidth;
		this->bClickable = bClickable;
		this->colType = colType;
	}
	virtual ~CColHeader() { }

	enum
	{
		TYPE_TEXT = 1,
		TYPE_IMAGE = 2,
		TYPE_NEXT = 5,
	};

	tstring			strTitle;
	int16			nWidth;
	bool			bClickable;
	int				colType;

};

class IListViewDataSource
{
public:
	IListViewDataSource() { m_pView = NULL; }
	virtual ~IListViewDataSource() { }

	virtual void SetView(CSkinListView *pView) { m_pView = pView; }

	virtual int GetRowCount() const = 0;

	virtual bool IsRowSelected(int row) const = 0;
	virtual void SetRowSelectionState(int nIndex, bool bSelected) = 0;

	virtual int GetItemImageIndex(int row) { return -1; }
	virtual bool SetItemImageIndex(int nItem, int nImageIndex, bool bRedraw = true) { return false; }

	virtual LPCTSTR GetCellText(int row, int col) = 0;
	virtual CRawImage *GetCellImage(int row, int col) = 0;

protected:
	CSkinListView *m_pView;

};

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

class CSkinListView : public CSkinScrollFrameCtrlBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinScrollFrameCtrlBase)
public:
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

public:
	CSkinListView();
	virtual ~CSkinListView();

	void SetDataSource(IListViewDataSource *pDataSource);
	
	void NotifyDataChanged();

	void LoadColumnWidth(LPCTSTR szProperty);
	void SaveColumnWidth(LPCTSTR szProperty);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	void SetBkColor(const MLColor &clrBk);
	void SetSelRowBkColor(const MLColor &clrBk);
	void SetFont(const CFontInfo &font);
	void SetTextColor(const MLColor &clrText);

	void SetColor(int nColorName, const MLColor &clr);
	MLColor & GetColor(int nColorName);

	virtual void AddColumn(LPCTSTR szCol, int nWidth, int colType = CColHeader::TYPE_TEXT, bool bClickable = false);

	size_t GetColumnCount() const { return m_vHeading.size(); }
	int GetColumnWidth(int nCol) const;
	bool SetColumnWidth(int nCol, int cx);

	void SetLineHeight(int nLineHeight);

	UINT GetSelectedCount() const;
	int GetNextSelectedItem(int nPos = -1) const;

	void SetItemSelectionState(int nIndex, bool bSelected);
	void ClearAllSelMark();

	void MakeSureRowVisible(int nRow);

	void IncreaseHeight(int nValue);

	int GetContentHeight() const;

	int GetRowCount() const;

protected:
	class CPaintUpdater
	{
	public:
		CPaintUpdater(CSkinListView *pHost) : m_pHost(pHost)
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
		CSkinListView	*m_pHost;
		bool			m_bUpdate;
		int				m_nFirstVisibleRow;

	};
	friend class CPaintUpdater;

	class CSelChangedCheck
	{
	public:
		CSelChangedCheck(CSkinListView *pListCtrl) : m_pListCtrl(pListCtrl)
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
		CSkinListView	*m_pListCtrl;
		vector<int>		m_vSelected;

	};
	friend class CSelChangedCheck;

protected:
	void GetRowColAtPoint(POINT pt, int &row, int &col, POINT *ptAtItemCell = NULL, bool *bItemClickable = NULL);

	void SetClickCursor();

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

	virtual bool IsClickedOn(int row, int col, CColHeader *pHeader, int x, int y);

	virtual void DrawCell(int row, int col, RECT &rcCell, CRawGraph *pGraph, MLColor &clrText);
	virtual void DrawCellText(LPCTSTR text, RECT &rcCell, CRawGraph *pGraph, MLColor &clrText);
	virtual void DrawCellImage(CRawImage &image, RECT &rcCell, CRawGraph *pGraph);

public:
	void Draw(CRawGraph *pGraph);

	void OnKeyDown(UINT nChar, UINT nFlags);

	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDown(UINT nFlags, CPoint point);
	bool OnLButtonDblClk(UINT nFlags, CPoint point);
	bool OnRButtonDown(UINT nFlags, CPoint point);
	bool OnRButtonUp(UINT nFlags, CPoint point);
	bool OnMouseDrag(CPoint point);
	bool OnMouseMove(CPoint point);

	void OnCreate();
	void OnSize();

	virtual void OnAdjustHue(float hue, float saturation, float luminance);

	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);

protected:
	typedef vector<CColHeader *>	V_COLUMNS;

	friend class LC_ITEMEX;
	friend class CPopupSkinListWnd;
	friend class CListViewDataSource;

	bool					m_bHoverMouseSel;

	IListViewDataSource		*m_dataSource;
	V_COLUMNS				m_vHeading;

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
