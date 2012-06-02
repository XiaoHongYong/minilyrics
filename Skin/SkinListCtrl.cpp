// SkinListCtrl.cpp: implementation of the CSkinListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinListCtrl.h"
#include "SkinFrameCtrl.h"


void LC_ITEM::Draw(CSkinListCtrl *pListCtrl, int nRow, int nClrNameDefault, CRect &rcItem, CRawGraph *pGraph)
{
	if (pListCtrl->m_font.IsOutlined())
	{
		pGraph->DrawTextOutlined(strText.c_str(), strText.size(), &rcItem, 
			pListCtrl->GetColor(nClrNameDefault), 
			pListCtrl->m_font.GetColorOutlined(), DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
	}
	else
	{
		pGraph->SetTextColor(pListCtrl->GetColor(nClrNameDefault));
		pGraph->DrawText(strText.c_str(), strText.size(), &rcItem, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
	}
}

bool LC_IMAGE_ITEM::IsClickedOn(CColHeader *pHeader, int x, int y)
{
	return x >= 0 && x < image.m_cx && y >= 0 && y < image.m_cy;
}

void LC_IMAGE_ITEM::Draw(CSkinListCtrl *pListCtrl, int nRow, int nClrNameDefault, CRect &rcItem, CRawGraph *pGraph)
{
	if (image.IsValid())
	{
		CRawGraph::CClipBoxAutoRecovery	cbar(pGraph);

		pGraph->SetClipBoundBox(rcItem);

		int		yDest = rcItem.top + (rcItem.Height() - image.height()) / 2;

		image.Blt(pGraph, rcItem.left, yDest);
	}
	else
		LC_ITEM::Draw(pListCtrl, nRow, nClrNameDefault, rcItem, pGraph);
}

//////////////////////////////////////////////////////////////////////////

#define SZ_CUSTOMIZED_CLR	_T("CustomizedColor")

UIOBJECT_CLASS_NAME_IMP(CSkinListCtrl, _T("ListCtrl"))

CSkinListCtrl::CSkinListCtrl()
{
	m_bHorzScrollBar = m_bVertScrollBar = true;
	m_bEnableBorder = true;

	m_vColors.resize(CN_CUSTOMIZED_START);

	m_nXMargin = 2;
	m_bDrawHeader = false;
	m_bEnableSort = false;

	m_nMsgWant = UO_MSG_WANT_RBUTTON | UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_KEY | UO_MSG_WANT_MOUSEWHEEL;

	// SetBkColor(MLColor(GetSysColor(COLOR_WINDOW)));
	SetBkColor(MLColor(RGB(192, 192, 192)));
	SetSelRowBkColor(MLColor(RGB(128, 128, 128)));
	m_vColors[CN_SEL_TEXT].set(RGB(255, 255, 255));

	m_nLineHeightOrg = -1;
	m_nLineHeight = 16;

	m_imageListIconCx = 16;
	m_imageListRightSpace = 4;

	m_bDrawBorder = false;
	m_bDrawHeader = false;
	m_nHeaderHeight = 16;
	MLColor clrPen(RGB(128, 128, 128));
	m_penLine.CreateSolidPen(1, clrPen);
	m_nSortedCol = 0;
	m_bAscending = false;
	m_bAdjustColWidth = false;

	m_nBorderWidth = 0;

	m_bHoverMouseSel = false;
	m_bClickBtDown = false;

	m_nFirstVisibleRow = 0;
	m_nBegSelRow = -1;
	m_nEndSelRow = -1;
}


CSkinListCtrl::~CSkinListCtrl()
{
	for (int i = 0; i < (int)m_vRows.size(); i++)
	{
		LC_ROW		*row = m_vRows[i];
		delete row;
	}

	// Delete all column header
	for (size_t i = 0; i < m_vHeading.size(); i++)
		delete m_vHeading[i];
}

void CSkinListCtrl::LoadColumnWidth(LPCTSTR szProperty)
{
	VecString vWidth;
	StrSplit(g_Profile.GetProfileString(szProperty, _T("")), ',', vWidth);
	for (uint i = 0; i < vWidth.size() && i < m_vHeading.size(); i++)
	{
		if (vWidth[i].empty())
			continue;
		SetColumnWidth(i, _ttoi(vWidth[i].c_str()));
	}
}

void CSkinListCtrl::SaveColumnWidth(LPCTSTR szProperty)
{
	VecInt	vWidth;
	for (size_t i = 0; i < GetColumnCount(); i++)
		vWidth.push_back(GetColumnWidth(i));
	tstring str = StrJoin(vWidth.begin(), vWidth.end(), _T("%d"), _T(","));
	g_Profile.WriteProfileString(szProperty, str.c_str());
}

void CSkinListCtrl::OnCreate()
{
	m_font.OnCreate(m_pSkin);

	CSkinScrollFrameCtrlBase::OnCreate();

	m_nLineHeight = m_font.GetHeight() + 4;
	if (m_nLineHeight < m_nLineHeightOrg)
		m_nLineHeight = m_nLineHeightOrg;

	// if in Editor mode, don't create scrollbar
	if (m_pSkin->IsInEditorMode())
		return;

	TCHAR		szCurFile[MAX_PATH];
	if (m_pSkin->GetSkinFactory()->GetResourceMgr()->GetResourcePathName(
		szCurFile, CountOf(szCurFile), _T("list_ctrl_resize_col.cur")))
		m_curResizeCol.LoadCursorFromFile(szCurFile);
}

BOOL CSkinListCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (IsPropertyName(szProperty, _T("BgColor")))
	{
		V_Str	vColor;
		StrSplit(szValue, ',', vColor);
		TrimStr(vColor);
		if (vColor.size() > 0)
			GetColorValue(GetColor(CN_BG), vColor[0].c_str());
		if (vColor.size() > 1)
			GetColorValue(GetColor(CN_ALTER_BG), vColor[1].c_str());
		else
			GetColor(CN_ALTER_BG) = GetColor(CN_BG);

		m_clrBg = GetColor(CN_BG);
		m_bDrawBg = true;
		return true;
	}

	if (CSkinScrollFrameCtrlBase::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("SelBgColor")) == 0)
	{
		GetColorValue(GetColor(CN_SEL_BG), szValue);
	}
	else if (_tcsicmp(szProperty, _T("SelTextColor")) == 0)
	{
		GetColorValue(GetColor(CN_SEL_TEXT), szValue);
	}
	else if (*szProperty == 'C' && _tcsnicmp(szProperty, SZ_CUSTOMIZED_CLR, _tcslen(SZ_CUSTOMIZED_CLR)) == 0)
	{
		MLColor		clr;
		int			nColorName = CN_CUSTOMIZED_START + _ttoi(szValue + _tcslen(SZ_CUSTOMIZED_CLR));

		GetColorValue(clr, szValue);
		SetColor(nColorName, clr);
	}
	else if (m_font.SetProperty(szProperty, szValue))
		return true;
	else if (_tcsicmp(szProperty, _T("ImageList")) == 0)
	{
		m_strImageList = szValue;
		m_imageList.Destroy();
		m_imageList.Load(m_pSkin->GetSkinFactory(), szValue, 
			m_imageListIconCx);
	}
	else if (_tcsicmp(szProperty, _T("ImageListIconCx")) == 0)
	{
		m_imageListIconCx = _ttoi(szValue);
		m_imageList.SetIconCx(m_imageListIconCx);
	}
	else if (IsPropertyName(szProperty, _T("DrawHeader")))
		m_bDrawHeader = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("DrawBorder")))
		m_bDrawBorder = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("EnableSort")))
		m_bEnableSort = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("HeaderHeight")))
		m_nHeaderHeight = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("LineHeight")))
	{
		m_nLineHeightOrg = _ttoi(szValue);
		if (m_nLineHeight < m_nLineHeightOrg)
			m_nLineHeight = m_nLineHeightOrg;
	}
	else if (IsPropertyName(szProperty, _T("XMargin")))
		m_nXMargin = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("LineColor")))
	{
		MLColor clrPen(RGB(128, 128, 128));
		GetColorValue(clrPen, szValue);
		m_penLine.CreateSolidPen(1, clrPen);
	}
	else if (IsPropertyName(szProperty, _T("ImageSortedHeader")))
		m_imageSortedHeader.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	else if (IsPropertyName(szProperty, _T("ImageHeader")))
		m_imageHeader.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	else if (IsPropertyName(szProperty, _T("ImageSortFlag")))
	{
		m_imageAscending.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imageAscending.m_cx /= 2;
		m_imageDescending.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		m_imageDescending.m_x = m_imageDescending.m_cx / 2;
		m_imageDescending.m_cx /= 2;
	}
	else if (IsPropertyName(szProperty, _T("ImageBorder")))
		m_imageBorder.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	else if (IsPropertyName(szProperty, _T("BorderWidth")))
		m_nBorderWidth = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("MouseHoverSel")))
		m_bHoverMouseSel = IsTRUE(szValue);
	else
		return false;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinListCtrl::EnumProperties(CUIObjProperties &listProperties)
{
	CSkinScrollFrameCtrlBase::EnumProperties(listProperties);

	listProperties.AddPropColor(_T("BgColor"), GetColor(CN_BG));
	listProperties.AddPropColor(_T("SelBgColor"), GetColor(CN_SEL_BG));
	listProperties.AddPropColor(_T("SelTextColor"), GetColor(CN_SEL_TEXT));
	m_font.EnumProperties(listProperties);
	listProperties.AddPropImageFile(_T("ImageList"), m_strImageList.c_str());

	for (vector<MLColor>::size_type i = CN_CUSTOMIZED_START; i < m_vColors.size(); i++)
	{
		listProperties.AddPropColor(CStrPrintf(_T("%s%d"), SZ_CUSTOMIZED_CLR, i - CN_CUSTOMIZED_START).c_str(),
			GetColor(i));
	}
}
#endif // _SKIN_EDITOR_

void CSkinListCtrl::OnSize()
{
	CSkinScrollFrameCtrlBase::OnSize();

	SetScrollInfo(false);
	SetHorzScrollInfo(false);

	if (m_vHeading.size() == 0)
		return;

	int		w = m_rcContent.Width() - m_nXMargin * 2;

	if (m_vHeading.size() == 1)
	{
		m_vHeading[0]->nWidth = w;
	}
	else
	{
		int		i;
		int		n = 0;
		for (i = 0; i < (int)m_vHeading.size(); i++)
		{
			n += m_vHeading[i]->nWidth;
		}
		if (n < w)
		{
			m_vHeading[m_vHeading.size() - 1]->nWidth += w - n;
		}
	}

	if (GetLinesOfPerPage() >= (int)m_vRows.size())
		m_nFirstVisibleRow = 0;
}

void CSkinListCtrl::OnAdjustHue(float hue, float saturation, float luminance)
{
	CSkinScrollFrameCtrlBase::OnAdjustHue(hue, saturation, luminance);

	for (size_t i = 0; i < m_vColors.size(); i++)
		m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_vColors[i]);

	m_font.OnAdjustHue(m_pSkin, hue, saturation, luminance);
}

void CSkinListCtrl::OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	if (m_vRows.size() == 0)
		return;

	ASSERT(nPos >= 0 && nPos < (int)m_vRows.size());
	if (nPos >= 0 && nPos < (int)m_vRows.size())
	{
		m_nFirstVisibleRow = nPos;
		Invalidate();
	}
}


void CSkinListCtrl::OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	Invalidate();
}


void CSkinListCtrl::SetBkColor(const MLColor &clrBk)
{
	GetColor(CN_BG) = clrBk;
}


void CSkinListCtrl::SetSelRowBkColor(const MLColor &clrBk)
{
	GetColor(CN_SEL_BG) = clrBk;
}


void CSkinListCtrl::SetFont(const CFontInfo &font)
{
	m_font.Create(font);
	m_nLineHeight = m_font.GetHeight() + 4;
	if (m_nLineHeight < m_nLineHeightOrg)
		m_nLineHeight = m_nLineHeightOrg;
}


void CSkinListCtrl::SetTextColor(const MLColor &clrText)
{
	tstring		strColor;

	ColorToStr(clrText, strColor);
	m_font.SetProperty(_T("TextColor"), strColor.c_str());
}

void CSkinListCtrl::SetColor(int nColorName, const MLColor &clr)
{
	ASSERT(m_vColors.size() >= CN_CUSTOMIZED_START);
	if (nColorName >= (int)m_vColors.size())
	{
		if (nColorName > CN_MAX)
			return;
		m_vColors.resize(nColorName + 1);
	}
	else if (nColorName == CN_TEXT)
	{
		tstring		strColor;

		ColorToStr(clr, strColor);
		m_font.SetProperty(_T("TextColor"), strColor.c_str());
	}

	m_vColors[nColorName] = clr;
}


MLColor & CSkinListCtrl::GetColor(int nColorName)
{
	ASSERT(m_vColors.size() >= CN_CUSTOMIZED_START);
	if (nColorName < 0 || nColorName >= (int)m_vColors.size())
	{
		ASSERT(0 && "Invalid color name ID");
		return m_vColors[0];
	}
	else if (nColorName == CN_TEXT)
		m_vColors[CN_TEXT] = m_font.GetTextColor(m_bEnable);

	return m_vColors[nColorName];
}


void CSkinListCtrl::AddColumn(LPCTSTR szCol, int nWidth)
{
	m_vHeading.push_back(new LC_ITEM::CColHeader(szCol, nWidth));

	// Alloc column for existed rows.
	for (int i = 0; i < (int)m_vRows.size(); i++)
	{
		LC_ROW *row = m_vRows[i];
		row->listItems.push_back(NewLCItem(m_vHeading.size() - 1));
	}

	SetHorzScrollInfo(false);
}

void CSkinListCtrl::AddImageColumn(LPCTSTR szCol, int nWidth, bool bClickable)
{
	m_vHeading.push_back(new LC_IMAGE_ITEM::CImageColHeader(szCol, nWidth, bClickable));

	SetHorzScrollInfo(false);
}

int CSkinListCtrl::GetColumnWidth(int nCol) const
{
	ASSERT(nCol >= 0 && nCol < (int)m_vHeading.size());
	if (nCol >= 0 && nCol < (int)m_vHeading.size())
		return m_vHeading[nCol]->nWidth;
	else
		return -1;
}


BOOL CSkinListCtrl::SetColumnWidth(int nCol, int cx)
{
	ASSERT(nCol >= 0 && nCol < (int)m_vHeading.size());
	if (nCol >= 0 && nCol < (int)m_vHeading.size())
	{
		m_vHeading[nCol]->nWidth = cx;
		return true;
	}
	else
		return false;
}

void CSkinListCtrl::SetLineHeight(int nLineHeight)
{
	m_nLineHeightOrg = nLineHeight;
	if (m_nLineHeight < m_nLineHeightOrg)
		m_nLineHeight = m_nLineHeightOrg;
}

UINT CSkinListCtrl::GetSelectedCount() const
{
	int		nSelectedCount = 0;

	for (int i = 0; i < (int)m_vRows.size(); i++)
	{
		LC_ROW		*row = m_vRows[i];

		if (row->IsSelected())
			nSelectedCount++;
	}
	return nSelectedCount;
}


int CSkinListCtrl::GetNextSelectedItem(int nPos) const
{
	if (nPos < -1)
		nPos = -1;
	for (int i = nPos + 1; i < (int)m_vRows.size(); i++)
	{
		LC_ROW		*row = m_vRows[i];

		if (row->IsSelected())
			return i;
	}

	return -1;
}


bool CSkinListCtrl::IsRowSelected(int nRow) const
{
	if (nRow >= 0 && nRow <= (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nRow];
		return row->IsSelected();
	}

	return false;
}


int CSkinListCtrl::GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nItem];
		LC_ITEM		*item = row->GetSubItem(nSubItem);
		if (item)
		{
			_tcscpy_safe(lpszText, nLen, item->strText.c_str());
			return item->strText.size();
		}
	}

	EmptyStr(lpszText);

	return -1;
}


int CSkinListCtrl::GetItemText(int nItem, int nSubItem, tstring &strText) const
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nItem];
		LC_ITEM		*item = row->GetSubItem(nSubItem);
		if (item)
		{
			strText = item->strText.c_str();
			return strText.size();
		}
	}

	strText = _T("");

	return -1;
}


BOOL CSkinListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText, bool bRedraw)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nItem];
		LC_ITEM		*item = row->GetSubItem(nSubItem);
		if (item)
		{
			item->strText = lpszText;
			if (bRedraw)
				InvalidateItem(nItem);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CSkinListCtrl::SetItemImage(int nItem, int nSubItem, PRAW_IMAGE_DATA hImage, bool bRedraw)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nItem];
		LC_ITEM		*item = row->GetSubItem(nSubItem);
		if (item)
		{
			if (m_vHeading[nSubItem]->colType == LCCT_IMAGE)
			{
				LC_IMAGE_ITEM	*pImageItem = (LC_IMAGE_ITEM*)item;
				pImageItem->image.Attach(hImage);
				if (bRedraw)
					InvalidateItem(nItem);
			}
			else
				ASSERT(0 && "This Column should be Image Column, but NOT.");
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CSkinListCtrl::SetItemImageIndex(int nItem, int nImageIndex, bool bRedraw)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nItem];
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
		LC_ROW		*row = m_vRows[nItem];
		return row->nItemData;
	}

	return -1;
}


void CSkinListCtrl::SetItemData(int nItem, UINT nItemData)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nItem];
		row->nItemData = nItemData;
	}
}


int CSkinListCtrl::InsertItem(int nItem, LPCTSTR lpszItem, int nImageIndex, UINT nItemData, bool bRedraw)
{
	ASSERT(m_vHeading.size() > 0);
	if (m_vHeading.size() == 0)
		return -1;

	LC_ROW		*row = new LC_ROW(nImageIndex, nItemData);

	LC_ITEM *item = NewLCItem(0);
	item->strText = lpszItem;
	row->listItems.push_back(item);

	// alloc space for the new row
	for (int i = 1; i < (int)m_vHeading.size(); i++)
	{
		item = NewLCItem(i);
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

	if (bRedraw && nItem <= GetLastVisibleRow())
		Invalidate();

	return nItem;
}


BOOL CSkinListCtrl::DeleteItem(int nItem, bool bRedraw)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nItem];
		m_vRows.erase(m_vRows.begin() + nItem);
		delete row;

		if (bRedraw)
			Invalidate();

		return TRUE;
	}
	else
		return FALSE;
}


BOOL CSkinListCtrl::DeleteSelectedItems(bool bRedraw)
{
	for (int i = (int)m_vRows.size() - 1; i >= 0; i--)
	{
		LC_ROW		*row = m_vRows[i];
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


BOOL CSkinListCtrl::DeleteAllItems(bool bRedraw)
{
	for (int i = 0; i < (int)m_vRows.size(); i++)
	{
		LC_ROW		*row = m_vRows[i];
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

void CSkinListCtrl::SetItemSelectionState(int nIndex, bool bSelected)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vRows.size());
	if (nIndex < 0 || nIndex >= (int)m_vRows.size())
		return;

	m_nBegSelRow = m_nEndSelRow = nIndex;

	CPaintUpdater		updater(this);
	CSelChangedCheck	selChecher(this);

	LC_ROW		*row = m_vRows[nIndex];
	row->SetSelected(bSelected);

	if (selChecher.IsSelectChanged())
	{
		updater.SetUpdateFlag(true);
		SendNotifyEvent(CSkinListCtrlEventNotify::C_SEL_CHANGED);
	}
}

void CSkinListCtrl::MakeSureRowVisible(int nRow)
{
	if (nRow >= m_nFirstVisibleRow && nRow <= GetLastVisibleRow())
		return;

	if (nRow < m_nFirstVisibleRow)
		m_nFirstVisibleRow = nRow;
	else
	{
		m_nFirstVisibleRow = nRow - (GetLinesOfPerPage() - 1);
		if (m_nFirstVisibleRow < 0)
			m_nFirstVisibleRow = 0;
	}

	if (m_pVertScrollBar)
		m_pVertScrollBar->SetScrollPos(m_nFirstVisibleRow);
}

void CSkinListCtrl::IncreaseHeight(int nValue)
{
	if (nValue == 0)
		return;

	m_formHeight.Increase(nValue);
	m_pContainer->RecalculateUIObjSizePos(this);
}

int CSkinListCtrl::GetContentHeight() const
{
	int		nHeight = 0;
	
	if (m_bDrawHeader)
		nHeight += m_nHeaderHeight;

	nHeight += m_nLineHeight * m_vRows.size();
	return nHeight;
}

LC_ITEM *CSkinListCtrl::NewLCItem(int nCol)
{
	ASSERT(nCol >= 0 && nCol < (int)m_vHeading.size());
	return m_vHeading[nCol]->NewLCItem();
}

void CSkinListCtrl::GetRowColAtPoint(POINT pt, int &row, int &col, POINT *ptAtItemCell, bool *bItemClickable)
{
	row = col = -1;
	if (bItemClickable)
		*bItemClickable = false;

	// Col
	pt.x -= m_rcContent.left + m_nXMargin;

	if (pt.x >= 0 && pt.x <= m_rcContent.Width())
	{
		if (m_pHorzScrollBar && m_pHorzScrollBar->IsEnabled())
			pt.x += m_pHorzScrollBar->GetScrollPos();
		for (uint i = 0; i < m_vHeading.size(); i++)
		{
			if (pt.x < m_vHeading[i]->nWidth)
			{
				if (ptAtItemCell)
					ptAtItemCell->x = pt.x;
				col = i;
			}
			pt.x -= m_vHeading[i]->nWidth;
		}
	}

	// Row
	pt.y -= m_rcContent.top;
	if (m_bDrawHeader)
		pt.y -= m_nHeaderHeight;

	if (pt.y >= 0 && pt.y <= m_rcContent.Height())
	{
		int		n = m_nFirstVisibleRow + pt.y / m_nLineHeight;
		if (ptAtItemCell)
			ptAtItemCell->y = pt.y % m_nLineHeight;
		if (n < (int)m_vRows.size())
			row = n;
	}

	if (bItemClickable && col != -1 && row != -1
		&& m_vHeading[col]->bClickable)
	{
		LC_ITEM		*item = m_vRows[row]->GetSubItem(col);
		if (item && item->IsClickedOn(m_vHeading[col], ptAtItemCell->x, ptAtItemCell->y))
			*bItemClickable = true;
	}
}

void CSkinListCtrl::SetClickCursor()
{
	if (!m_curHand.IsValid())
		m_curHand.LoadStdCursor(CCursor::C_HAND);

	MLWidget::SetCursor(m_curHand);
}

bool CSkinListCtrl::ClearAllSelMark()
{
	bool bRet = false;

	for (int i = 0; i < (int)m_vRows.size(); i++)
	{
		LC_ROW		*row = m_vRows[i];
		if (row->IsSelected())
		{
			bRet = true;
			row->SetSelected(false);
		}
	}
	return bRet;
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
			LC_ROW		*row = m_vRows[i];
			if (row->IsSelected())
			{
				LC_ROW	*rowTemp;
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
			LC_ROW		*row = m_vRows[i];
			if (row->IsSelected())
			{
				LC_ROW	*rowTemp;
				rowTemp = m_vRows[i - 1];
				m_vRows[i - 1] = m_vRows[i];
				m_vRows[i] = rowTemp;
			}
		}
	}

	Invalidate();
	return true;
}


int CSkinListCtrl::GetLastVisibleRow() const
{
	int		n;
	n = m_nFirstVisibleRow + GetLinesOfPerPage() - 1;
	if (n > (int)m_vRows.size() - 1)
		n = (int)m_vRows.size() - 1;
	return n;
}


void CSkinListCtrl::ReverseSelectStatOfRow(bool bClearOldSel, int nRow)
{
	ASSERT(nRow >= -1 && nRow < (int)m_vRows.size());
	if (bClearOldSel)
	{
		ClearAllSelMark();
	}

	if (nRow != -1)
	{
		LC_ROW		*row = m_vRows[nRow];
		if (row->IsSelected())
		{
			row->SetSelected(false);
		}
		else
		{
			row->SetSelected(true);
		}
	}
}


void CSkinListCtrl::SelectRangeRow(int nBeg, int nEnd)
{
	if (nEnd == -1)
		return;

	ASSERT(nEnd >= 0 && nEnd < (int)m_vRows.size());
	ASSERT(nBeg >= -1 && nBeg < (int)m_vRows.size());

	ClearAllSelMark();

	if (nBeg == -1)
		nBeg = 0;

	m_nBegSelRow = nBeg;
	m_nEndSelRow = nEnd;

	if (nBeg > nEnd)
	{
		int		n = nBeg;
		nBeg = nEnd;
		nEnd = n;
	}
	for (; nBeg <= nEnd; nBeg++)
	{
		LC_ROW		*row = m_vRows[nBeg];
		row->SetSelected(true);
	}
}


void CSkinListCtrl::RecalculateColWidth(CRawGraph *pGraph)
{
/*
	int		i;

	ASSERT(m_vHeading.size() == 2);
	if (m_vHeading.size() != 2)
		return;

	// Should recalculate column width?
	for (i = 0; i < (int)m_vHeading.size(); i++)
	{
		if (m_vHeading[i].nWidth == -1)
			break;
	}
	if (i == m_vHeading.size())
		return;

	SIZE	size;
	int		nWidthMax = 0;
	for (i = 0; i < (int)m_vRows.size(); i++)
	{
		int			x = rc.left;
		LC_ROW		*row = m_vRows[i];
		LC_ITEM		*pItem = row->GetSubItem(1);

		if (pGraph->GetTextExtentPoint32(pItem->strText.c_str(), pItem->strText.size(), &size))
		{
			if (nWidthMax < size.cx)
				nWidthMax = size.cx;
		}
	}

	m_vHeading[1].nWidth = nWidthMax;
	m_vHeading[0].nWidth = 
*/
}


void CSkinListCtrl::SetScrollInfo(bool bRedraw)
{
	int		nPage = GetLinesOfPerPage() - 1;
	if (nPage < 1)
		nPage = 1;
	if (m_pVertScrollBar)
		m_pVertScrollBar->SetScrollInfo(0, m_vRows.size(), nPage, m_nFirstVisibleRow, 1, bRedraw);
}

void CSkinListCtrl::SetHorzScrollInfo(bool bRedraw)
{
	int		nWidth = 0;

	for (size_t i = 0; i < m_vHeading.size(); i++)
		nWidth += m_vHeading[i]->nWidth;

	if (m_pHorzScrollBar)
		m_pHorzScrollBar->SetScrollInfo(0, nWidth, m_rcContent.Width(), m_pHorzScrollBar->GetScrollPos(), 10, bRedraw);
}


void CSkinListCtrl::SendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol)
{
	CSkinListCtrlEventNotify	event(this, nClickedRow, nClickedCol);

	event.cmd = cmd;
	m_pSkin->DispatchUIObjNotify(&event);
}

CSkinListCtrl::HitTestArea CSkinListCtrl::HitTest(POINT &point, int &nColumnIndex)
{
	nColumnIndex = 0;

	if (!IsPtIn(point))
		return HTA_NONE;

	if (m_bDrawHeader && point.y - m_rcContent.top < m_nHeaderHeight)
	{
		int		x = point.x - m_rcContent.left;
		if (m_pHorzScrollBar)
			x += m_pHorzScrollBar->GetScrollPos();

		for (size_t i = 0; i < m_vHeading.size(); i++)
		{
			if (m_vHeading[i]->nWidth == 0)
				continue;

			if (x > m_vHeading[i]->nWidth)
			{
				x -= m_vHeading[i]->nWidth;
				continue;
			}

			int		RESIZE_RANGE = 5;

			if (m_vHeading[i]->nWidth < 10)
				RESIZE_RANGE = 2;

			nColumnIndex = (int)i;

			if (x <= RESIZE_RANGE)
			{
				// Near to next column
				if (x * 2 > m_vHeading[i]->nWidth)
					return HTA_HEADER_SPLIT;

				if (nColumnIndex == 0)
					return HTA_HEADER;

				nColumnIndex = (int)i - 1;
				return HTA_HEADER_SPLIT;
			}
			else if (m_vHeading[i]->nWidth - x <= RESIZE_RANGE)
			{
				return HTA_HEADER_SPLIT;
			}
			else
				return HTA_HEADER;
		}

		return HTA_HEADER;
	}

	return HTA_ROW;
}

void CSkinListCtrl::DrawHeader(CRawGraph *pGraph)
{
	int		x = m_rcContent.left;
	int		y = m_rcContent.top;

	if (m_pHorzScrollBar)
		x -= m_pHorzScrollBar->GetScrollPos();

	for (int i = 0; i < (int)m_vHeading.size(); i++)
	{
		LC_ITEM::CColHeader	*pColHeader = m_vHeading[i];
		if (pColHeader->nWidth <= 0)
			continue;

		if (x + pColHeader->nWidth < m_rcContent.left)
		{
			x += pColHeader->nWidth;
			continue;
		}

		CSFImage	*pImageHeader = NULL, *pImageSortFlag = NULL;
		if (m_nSortedCol == i)
		{
			if (m_bAscending)
				pImageSortFlag = &m_imageAscending;
			else
				pImageSortFlag = &m_imageDescending;
			pImageHeader = &m_imageSortedHeader;
		}
		else
			pImageHeader = &m_imageHeader;

		// Draw background image.
		if (pImageHeader->IsValid())
			pImageHeader->StretchBlt(pGraph, x, y, pColHeader->nWidth, m_nHeaderHeight);

		// Sort flag
		if (pImageSortFlag && pImageSortFlag->IsValid())
			pImageSortFlag->Blt(pGraph, x + pColHeader->nWidth - 2 - pImageSortFlag->m_cx,
			y + (m_nHeaderHeight - pImageSortFlag->height()) / 2);

		// Draw Head Text
		CRect		rc(x + 3, y + 1, x + pColHeader->nWidth - 5, y + m_nHeaderHeight - 2);
		pGraph->DrawText(pColHeader->strTitle.c_str(), pColHeader->strTitle.size(), &rc, DT_VCENTER);

		x += pColHeader->nWidth;
		if (x > m_rcContent.right)
			break;

		// Draw line
		pGraph->Line(x - 1, y, x - 1, y + m_nHeaderHeight);
	}

	// Draw background image.
	if (m_imageHeader.IsValid() && x < m_rcContent.right)
		m_imageHeader.StretchBlt(pGraph, x, y, m_rcContent.right - x, m_nHeaderHeight);

	pGraph->Line(m_rcContent.left, y + m_nHeaderHeight - 1, m_rcContent.right, y + m_nHeaderHeight - 1);
}

void CSkinListCtrl::InvalidateItem(int nItem)
{
	if (nItem >= m_nFirstVisibleRow && nItem <= GetLastVisibleRow())
		Invalidate();
}

void CSkinListCtrl::Draw(CRawGraph *pGraph)
{
	CSkinScrollFrameCtrlBase::Draw(pGraph);

	CRect	rc, rcItem;
	int		nImageCx = 0;

	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	if (m_imageList.IsValid())
		nImageCx = m_imageList.GetIconCx();

	pGraph->SetTextColor(m_font.GetTextColor(m_bEnable));

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraph);
	pGraph->SetClipBoundBox(m_rcContent);

	pGraph->SetPen(m_penLine);

	// Draw background color
	if (m_bUseParentBg)
		m_pContainer->RedrawBackground(pGraph, m_rcContent);
	else
	{
		int			alpha;
		MLColor		clrBg1, clrBg2;

		if (m_bTranslucencyWithSkin)
			alpha = int(m_pSkin->m_nCurTranslucencyAlpha);
		else
			alpha = 255;

		clrBg1 = GetColor(CN_BG);
		clrBg1.setAlpha(alpha);

		clrBg2 = GetColor(CN_ALTER_BG);
		clrBg2.setAlpha(alpha);

		rcItem = m_rcContent;
		if (m_bDrawHeader)
			rcItem.top += m_nHeaderHeight;
		rcItem.bottom = rcItem.top + m_nLineHeight;
		bool		bFlag = true;
		while (rcItem.top < m_rcContent.bottom)
		{
			if (bFlag)
				pGraph->FillRect(&rcItem, clrBg1);
			else
				pGraph->FillRect(&rcItem, clrBg2);
			bFlag = !bFlag;
			rcItem.top = rcItem.bottom;
			rcItem.bottom += m_nLineHeight;
		}
	}

	rc = m_rcContent;
	rc.left += m_nXMargin;
	rc.right -= m_nXMargin;

	if (m_bDrawHeader)
	{
		DrawHeader(pGraph);
		rc.top += m_nHeaderHeight;
	}

	if (m_nFirstVisibleRow >= (int)m_vRows.size())
		m_nFirstVisibleRow = (int)m_vRows.size() - rc.Height() / m_nLineHeight;
	if (m_nFirstVisibleRow < 0)
		m_nFirstVisibleRow = 0;

	int		y = rc.top;
	int		nClrName;
	for (int i = m_nFirstVisibleRow; i < (int)m_vRows.size(); i++)
	{
		LC_ROW		*row = m_vRows[i];
		int			x = rc.left;
		if (m_pHorzScrollBar)
			x -= m_pHorzScrollBar->GetScrollPos();

		if (row->IsSelected())
		{
			CRect		rc(m_rcContent.left, y, m_rcContent.left + m_rcContent.Width(), y + m_nLineHeight);
			int			alpha;
			MLColor		clr = GetColor(CN_SEL_BG);

			if (m_bTranslucencyWithSkin)
				alpha = (int)(m_pSkin->m_nCurTranslucencyAlpha);
			else
				alpha = 255;

			clr.setAlpha(alpha);

			pGraph->FillRect(&rc, clr);
			nClrName = CN_SEL_TEXT;
		}
		else
			nClrName = CN_TEXT;

		list<LC_ITEM*>::iterator	it;
		int						k = 0;
		it = row->listItems.begin();
		rcItem.left = x;
		rcItem.top = y;
		rcItem.bottom = y + m_nLineHeight;
		for (; it != row->listItems.end() && k < (int)m_vHeading.size(); it++, k++)
		{
			LC_ITEM		*item = *it;
			rcItem.right = rcItem.left + m_vHeading[k]->nWidth;
			if (it == row->listItems.begin() && nImageCx > 0)
			{
				// Draw icon.
				if (nImageCx > 0 && rcItem.Width() > 0)
				{
					if (row->nImageIndex >= 0)
						m_imageList.Draw(pGraph, row->nImageIndex, x, y + (m_nLineHeight - m_imageList.GetIconCy()) / 2);
					rcItem.left += nImageCx + m_imageListRightSpace;
				}
			}
			if (rcItem.right < m_rcContent.left)
			{
				rcItem.left = rcItem.right;
				continue;
			}

			if (rcItem.right > rc.right)
				rcItem.right = rc.right;

			item->Draw(this, i, nClrName, rcItem, pGraph);
			if (rcItem.right >= rc.right)
				break;
			rcItem.left = rcItem.right;
		}
		y += m_nLineHeight;
		if (y >= rc.bottom)
			break;
	}

	// Draw Border
	if (m_bDrawBorder)
	{
		pGraph->Rectangle(m_rcContent.left, m_rcContent.top, m_rcContent.Width(), m_rcContent.Height());
	}
}


bool CSkinListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (CSkinScrollFrameCtrlBase::OnLButtonDown(nFlags, point))
		return true;

	int			nColumnIndex;
	HitTestArea	hta = HitTest(point, nColumnIndex);
	if (hta == HTA_HEADER_SPLIT)
	{
		// Drag to resize column width.
		m_pSkin->SetCaptureMouse(this);

		m_nResizingCol = nColumnIndex;
		m_ptStartDrag = point;
		m_bAdjustColWidth = true;
		MLWidget::SetCursor(m_curResizeCol);
		return true;
	}

	m_bClickBtDown = true;

	CPaintUpdater		updater(this);
	CSelChangedCheck	selChecher(this);

	POINT ptAtItemCell;
	int nClickedRow = -1, nClickedCol = -1;
	bool bItemClickable;
	GetRowColAtPoint(point, nClickedRow, nClickedCol, &ptAtItemCell, &bItemClickable);
	if (bItemClickable)
	{
		SetClickCursor();
		return true;
	}

	m_nEndSelRow = nClickedRow;
	if (nFlags & MK_SHIFT)
		SelectRangeRow(m_nBegSelRow, m_nEndSelRow);
	else
	{
		m_nBegSelRow = m_nEndSelRow;
		ReverseSelectStatOfRow((nFlags & MK_CONTROL) != MK_CONTROL, m_nEndSelRow);
	}

	if (m_nEndSelRow != -1)
		MakeSureRowVisible(m_nEndSelRow);

	if (selChecher.IsSelectChanged())
	{
		updater.SetUpdateFlag(true);
		SendNotifyEvent(CSkinListCtrlEventNotify::C_SEL_CHANGED, nClickedRow, nClickedCol);
	}

	return true;
}


bool CSkinListCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (CSkinScrollFrameCtrlBase::OnRButtonDown(nFlags, point))
		return true;

	int			nColumnIndex;
	HitTestArea	hta = HitTest(point, nColumnIndex);
	if (hta != HTA_ROW)
		return true;

	int nClickedRow = -1, nClickedCol = -1;
	GetRowColAtPoint(point, nClickedRow, nClickedCol);
	if (nClickedRow != -1)
	{
		if (IsRowSelected(nClickedRow))
			return true;
	}

	CPaintUpdater		updater(this);
	CSelChangedCheck	selChecher(this);

	m_nEndSelRow = nClickedRow;
	m_nBegSelRow = m_nEndSelRow;
	ReverseSelectStatOfRow((nFlags & MK_CONTROL) != MK_CONTROL, m_nEndSelRow);

	if (m_nEndSelRow != -1)
		MakeSureRowVisible(m_nEndSelRow);

	if (selChecher.IsSelectChanged())
	{
		updater.SetUpdateFlag(true);
		SendNotifyEvent(CSkinListCtrlEventNotify::C_SEL_CHANGED, nClickedRow, nClickedCol);
	}

	return false;
}


bool CSkinListCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (CSkinScrollFrameCtrlBase::OnRButtonUp(nFlags, point))
		return true;

	SendNotifyEvent(CSkinListCtrlEventNotify::C_RBTN_CLICK);

	return false;
}


bool CSkinListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (CSkinScrollFrameCtrlBase::OnLButtonDblClk(nFlags, point))
		return true;

	CPaintUpdater		updater(this);
	CSelChangedCheck	selChecher(this);

	int nClickedRow = -1, nClickedCol = -1;
	GetRowColAtPoint(point, nClickedRow, nClickedCol);
	m_nEndSelRow = nClickedRow;

	m_nBegSelRow = m_nEndSelRow;
	ReverseSelectStatOfRow((nFlags & MK_CONTROL) != MK_CONTROL, m_nEndSelRow);

	updater.SetUpdateFlag(selChecher.IsSelectChanged());

	if (m_nEndSelRow != -1)
		MakeSureRowVisible(m_nEndSelRow);

	SendNotifyEvent(CSkinListCtrlEventNotify::C_DBL_CLICK, nClickedRow, nClickedCol);

	return true;
}


void CSkinListCtrl::OnKeyDown(UINT nChar, UINT nFlags)
{
	if (m_vRows.empty())
		return;

	if (nChar == VK_RETURN)
	{
		SendNotifyEvent(CSkinListCtrlEventNotify::C_ENTER);
		return;
	}
	else if (nChar == VK_DELETE)
	{
		SendNotifyEvent(CSkinListCtrlEventNotify::C_KEY_DELETE);
		return;
	}
	else if (nChar == 'A')
	{
		bool		bCtrl = IsKeyPressed(VK_CONTROL);

		if (bCtrl)
		{
			// select all
			CSelChangedCheck	selChecher(this);

			SelectRangeRow(0, m_vRows.size() - 1);
			if (selChecher.IsSelectChanged())
				Invalidate();
		}
		return;
	}
	else if (nChar == 'I')
	{
		bool		bCtrl = IsKeyPressed(VK_CONTROL);

		if (bCtrl)
		{
			// invert selection
			CSelChangedCheck	selChecher(this);

			for (int i = 0; i < (int)m_vRows.size(); i++)
			{
				LC_ROW		*row = m_vRows[i];
				row->SetSelected(!row->IsSelected());
			}
			Invalidate();
		}
		return;
	}

	bool		bShift = IsKeyPressed(VK_SHIFT);

	if (nChar != VK_UP && nChar != VK_DOWN && nChar != VK_PRIOR && nChar != VK_NEXT
		&& nChar != VK_HOME && nChar != VK_END)
		return;

	CPaintUpdater		updater(this);
	CSelChangedCheck	selChecher(this);

	switch (nChar)
	{
	case VK_UP:
		{
// 			bool		bAlt = IsKeyPressed(VK_MENU);
// 			if (bAlt)
// 			{
// 				updater.SetUpdateFlag(OffsetAllSelectedRow(false));
// 				return;
// 			}

			if (m_nEndSelRow == -1)
				m_nEndSelRow = m_nFirstVisibleRow;
			else if (m_nEndSelRow > 0)
			{
				m_nEndSelRow--;
			}

			if (bShift)
				SelectRangeRow(m_nBegSelRow, m_nEndSelRow);
			else
			{
				m_nBegSelRow = m_nEndSelRow;
				ReverseSelectStatOfRow(true, m_nEndSelRow);
			}

			MakeSureRowVisible(m_nEndSelRow);
		}
		break;
	case VK_DOWN:
		{
// 			bool		bAlt = IsKeyPressed(VK_MENU);
// 			if (bAlt)
// 			{
// 				updater.SetUpdateFlag(OffsetAllSelectedRow(true));
// 				return;
// 			}

			if (m_nEndSelRow == -1)
				m_nEndSelRow = m_nFirstVisibleRow;
			else if (m_nEndSelRow < (int)m_vRows.size() - 1)
				m_nEndSelRow++;

			if (bShift)
				SelectRangeRow(m_nBegSelRow, m_nEndSelRow);
			else
			{
				m_nBegSelRow = m_nEndSelRow;
				ReverseSelectStatOfRow(true, m_nEndSelRow);
			}

			MakeSureRowVisible(m_nEndSelRow);
		}
		break;
	case VK_PRIOR:
		{
			if (m_nEndSelRow == -1)
				m_nEndSelRow = m_nFirstVisibleRow;
			else if (m_nEndSelRow > 0)
			{
				if (m_nEndSelRow != m_nFirstVisibleRow)
					m_nEndSelRow = m_nFirstVisibleRow;
				else
				{
					m_nEndSelRow = m_nFirstVisibleRow - (GetLinesOfPerPage() - 1);
					if (m_nEndSelRow == m_nFirstVisibleRow)
						m_nEndSelRow--;
					if (m_nEndSelRow < 0)
						m_nEndSelRow = 0;
				}
			}

			if (bShift)
				SelectRangeRow(m_nBegSelRow, m_nEndSelRow);
			else
			{
				m_nBegSelRow = m_nEndSelRow;
				ReverseSelectStatOfRow(true, m_nEndSelRow);
			}

			MakeSureRowVisible(m_nEndSelRow);
		}
		break;
	case VK_NEXT:
		{
			if (m_nEndSelRow == -1)
				m_nEndSelRow = m_nFirstVisibleRow;
			else if (m_nEndSelRow < (int)m_vRows.size() - 1)
			{
				if (m_nEndSelRow != GetLastVisibleRow())
					m_nEndSelRow = GetLastVisibleRow();
				else
				{
					m_nEndSelRow = GetLastVisibleRow() + GetLinesOfPerPage() - 1;
					if (m_nEndSelRow == GetLastVisibleRow())
						m_nEndSelRow++;
					if (m_nEndSelRow >= (int)m_vRows.size())
						m_nEndSelRow = m_vRows.size() - 1;
					if (m_nEndSelRow < 0)
						m_nEndSelRow = 0;
				}
			}

			if (bShift)
				SelectRangeRow(m_nBegSelRow, m_nEndSelRow);
			else
			{
				m_nBegSelRow = m_nEndSelRow;
				ReverseSelectStatOfRow(true, m_nEndSelRow);
			}

			MakeSureRowVisible(m_nEndSelRow);
		}
		break;
	case VK_HOME:
		{
			m_nEndSelRow = 0;
			m_nFirstVisibleRow = 0;

			if (bShift)
				SelectRangeRow(m_nBegSelRow, m_nEndSelRow);
			else
			{
				m_nBegSelRow = m_nEndSelRow;
				ReverseSelectStatOfRow(true, m_nEndSelRow);
			}
		}
		break;
	case VK_END:
		{
			m_nEndSelRow = m_vRows.size() - 1;

			if (bShift)
				SelectRangeRow(m_nBegSelRow, m_nEndSelRow);
			else
			{
				m_nBegSelRow = m_nEndSelRow;
				ReverseSelectStatOfRow(true, m_nEndSelRow);
			}

			MakeSureRowVisible(m_nEndSelRow);
		}
		break;
	}

	if (selChecher.IsSelectChanged())
	{
		updater.SetUpdateFlag(true);
		SendNotifyEvent(CSkinListCtrlEventNotify::C_SEL_CHANGED);
	}
}


bool CSkinListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bAdjustColWidth)
	{
		m_pSkin->ReleaseCaptureMouse(this);

		m_bAdjustColWidth = false;
		MLWidget::SetCursor(m_curResizeCol);
		return true;
	}

	if (CSkinScrollFrameCtrlBase::OnLButtonUp(nFlags, point))
		return true;

	Invalidate();

	if (m_bClickBtDown)
	{
		m_bClickBtDown = false;

		POINT ptAtItemCell;
		int nClickedRow = -1, nClickedCol = -1;
		bool bItemClickable;
		GetRowColAtPoint(point, nClickedRow, nClickedCol, &ptAtItemCell, &bItemClickable);
		if (bItemClickable)
		{
			SetClickCursor();

			CSkinListCtrlEventNotify	event(this, nClickedRow, nClickedCol);
			event.cmd = CSkinListCtrlEventNotify::C_CLICK_CMD;
			m_pSkin->DispatchUIObjNotify(&event);
		}
		else
			SendNotifyEvent(CSkinListCtrlEventNotify::C_CLICK, nClickedRow, nClickedCol);
	}

	return true;
}

bool CSkinListCtrl::OnMouseDrag(CPoint point)
{
	if (m_bAdjustColWidth && m_nResizingCol < (int)m_vHeading.size())
	{
		MLWidget::SetCursor(m_curResizeCol);

		int		nOffset = point.x - m_ptStartDrag.x;
		if (nOffset + m_vHeading[m_nResizingCol]->nWidth >= 0)
		{
		}
		else if (m_vHeading[m_nResizingCol]->nWidth > 0)
		{
			nOffset = - m_vHeading[m_nResizingCol]->nWidth;
		}
		else
			nOffset = 0;

		if (nOffset != 0)
		{
			m_vHeading[m_nResizingCol]->nWidth += nOffset;
			m_ptStartDrag.x += nOffset;
			SetHorzScrollInfo(true);
			Invalidate();
		}

		return true;
	}
	else
		return CSkinScrollFrameCtrlBase::OnMouseDrag(point);
}

bool CSkinListCtrl::OnMouseMove(CPoint point)
{
	if (CSkinScrollFrameCtrlBase::OnMouseMove(point))
		return true;

	int			nColumnIndex;
	HitTestArea	hta = HitTest(point, nColumnIndex);
	if (hta == HTA_HEADER_SPLIT)
	{
		// Set cursor of resize column width.
		MLWidget::SetCursor(m_curResizeCol);
	}
	else if (hta == HTA_ROW && m_bHoverMouseSel)
	{
		// Mouse hover to change selected.
		CSelChangedCheck	selChecher(this);

		int nClickedRow = -1, nClickedCol = -1;
		GetRowColAtPoint(point, nClickedRow, nClickedCol, NULL);
		m_nBegSelRow = m_nEndSelRow = nClickedRow;
		SelectRangeRow(m_nBegSelRow, m_nEndSelRow);

		if (selChecher.IsSelectChanged())
		{
			SendNotifyEvent(CSkinListCtrlEventNotify::C_SEL_CHANGED);
			Invalidate();
		}
	}
	else
	{
		int nClickedRow = -1, nClickedCol = -1;
		POINT ptAtItemCell;
		bool bItemClickable;
		GetRowColAtPoint(point, nClickedRow, nClickedCol, &ptAtItemCell, &bItemClickable);
		if (bItemClickable)
			SetClickCursor();
	}

	return true;
}
