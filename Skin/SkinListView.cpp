// SkinListCtrl.cpp: implementation of the CSkinListView class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinListView.h"
#include "SkinFrameCtrl.h"


//////////////////////////////////////////////////////////////////////////

#define SZ_CUSTOMIZED_CLR	_T("CustomizedColor")

UIOBJECT_CLASS_NAME_IMP(CSkinListView, _T("ListView"))

CSkinListView::CSkinListView()
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

	m_dataSource = NULL;
}

CSkinListView::~CSkinListView()
{
}

void CSkinListView::SetDataSource(IListViewDataSource *pDataSource)
{
	m_dataSource = pDataSource;
	if (pDataSource != NULL)
		pDataSource->SetView(this);
}

void CSkinListView::NotifyDataChanged()
{
	SetScrollInfo();
	Invalidate();
}

void CSkinListView::LoadColumnWidth(LPCTSTR szProperty)
{
	VecString vWidth;
	StrSplit(g_Profile.GetString(szProperty, _T("")), ',', vWidth);
	for (uint i = 0; i < vWidth.size() && i < m_vHeading.size(); i++)
	{
		if (vWidth[i].empty())
			continue;
		SetColumnWidth(i, _ttoi(vWidth[i].c_str()));
	}
}

void CSkinListView::SaveColumnWidth(LPCTSTR szProperty)
{
	VecInt	vWidth;
	for (size_t i = 0; i < GetColumnCount(); i++)
		vWidth.push_back(GetColumnWidth(i));
	tstring str = StrJoin(vWidth.begin(), vWidth.end(), _T("%d"), _T(","));
	g_Profile.WriteString(szProperty, str.c_str());
}

void CSkinListView::OnCreate()
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

BOOL CSkinListView::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
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
		m_BgType = BG_COLOR;
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
void CSkinListView::EnumProperties(CUIObjProperties &listProperties)
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

void CSkinListView::OnSize()
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

	if (GetLinesOfPerPage() >= (int)GetRowCount())
		m_nFirstVisibleRow = 0;
}

void CSkinListView::OnAdjustHue(float hue, float saturation, float luminance)
{
	CSkinScrollFrameCtrlBase::OnAdjustHue(hue, saturation, luminance);

	for (size_t i = 0; i < m_vColors.size(); i++)
		m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_vColors[i]);

	m_font.OnAdjustHue(m_pSkin, hue, saturation, luminance);
}

void CSkinListView::OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	if (GetRowCount() == 0)
		return;

	ASSERT(nPos >= 0 && nPos < (int)GetRowCount());
	if (nPos >= 0 && nPos < (int)GetRowCount())
	{
		m_nFirstVisibleRow = nPos;
		Invalidate();
	}
}


void CSkinListView::OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	Invalidate();
}


void CSkinListView::SetBkColor(const MLColor &clrBk)
{
	GetColor(CN_BG) = clrBk;
}


void CSkinListView::SetSelRowBkColor(const MLColor &clrBk)
{
	GetColor(CN_SEL_BG) = clrBk;
}


void CSkinListView::SetFont(const CFontInfo &font)
{
	m_font.Create(font);
	m_nLineHeight = m_font.GetHeight() + 4;
	if (m_nLineHeight < m_nLineHeightOrg)
		m_nLineHeight = m_nLineHeightOrg;
}


void CSkinListView::SetTextColor(const MLColor &clrText)
{
	m_font.SetProperty(_T("TextColor"), ColorToStr(clrText).c_str());
}

void CSkinListView::SetColor(int nColorName, const MLColor &clr)
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
		m_font.SetProperty(_T("TextColor"), ColorToStr(clr).c_str());
	}

	m_vColors[nColorName] = clr;
}


MLColor & CSkinListView::GetColor(int nColorName)
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

void CSkinListView::AddColumn(LPCTSTR szCol, int nWidth, int colType, bool bClickable)
{
	m_vHeading.push_back(new CColHeader(szCol, nWidth, colType, bClickable));

	SetHorzScrollInfo(false);
}

int CSkinListView::GetColumnWidth(int nCol) const
{
	ASSERT(nCol >= 0 && nCol < (int)m_vHeading.size());
	if (nCol >= 0 && nCol < (int)m_vHeading.size())
		return m_vHeading[nCol]->nWidth;
	else
		return -1;
}


bool CSkinListView::SetColumnWidth(int nCol, int cx)
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

void CSkinListView::SetLineHeight(int nLineHeight)
{
	m_nLineHeightOrg = nLineHeight;
	if (m_nLineHeight < m_nLineHeightOrg)
		m_nLineHeight = m_nLineHeightOrg;
}

UINT CSkinListView::GetSelectedCount() const
{
	int		nSelectedCount = 0;

	for (int i = 0; i < (int)GetRowCount(); i++)
	{
		if (m_dataSource->IsRowSelected(i))
			nSelectedCount++;
	}
	return nSelectedCount;
}

int CSkinListView::GetNextSelectedItem(int nPos) const
{
	if (nPos < -1)
		nPos = -1;
	for (int i = nPos + 1; i < (int)GetRowCount(); i++)
	{
		if (m_dataSource->IsRowSelected(i))
			return i;
	}

	return -1;
}

void CSkinListView::SetItemSelectionState(int nIndex, bool bSelected)
{
	ASSERT(nIndex >= 0 && nIndex < (int)GetRowCount());
	if (nIndex < 0 || nIndex >= (int)GetRowCount())
		return;

	m_nBegSelRow = m_nEndSelRow = nIndex;

	CPaintUpdater		updater(this);
	CSelChangedCheck	selChecher(this);

	m_dataSource->SetRowSelectionState(nIndex, bSelected);

	if (selChecher.IsSelectChanged())
	{
		updater.SetUpdateFlag(true);
		SendNotifyEvent(CSkinListCtrlEventNotify::C_SEL_CHANGED);
	}
}

void CSkinListView::MakeSureRowVisible(int nRow)
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

void CSkinListView::IncreaseHeight(int nValue)
{
	if (nValue == 0)
		return;

	m_formHeight.Increase(nValue);
	m_pContainer->RecalculateUIObjSizePos(this);
}

int CSkinListView::GetContentHeight() const
{
	int		nHeight = 0;
	
	if (m_bDrawHeader)
		nHeight += m_nHeaderHeight;

	nHeight += m_nLineHeight * GetRowCount();
	return nHeight;
}

void CSkinListView::GetRowColAtPoint(POINT pt, int &row, int &col, POINT *ptAtItemCell, bool *bItemClickable)
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
		if (n < (int)GetRowCount())
			row = n;
	}

	if (bItemClickable && col != -1 && row != -1 && ptAtItemCell != NULL
		&& m_vHeading[col]->bClickable)
	{
		if (IsClickedOn(row, col, m_vHeading[col], ptAtItemCell->x, ptAtItemCell->y))
			*bItemClickable = true;
	}
}

void CSkinListView::SetClickCursor()
{
	if (!m_curHand.IsValid())
		m_curHand.LoadStdCursor(CCursor::C_HAND);

	MLWidget::SetCursor(m_curHand);
}

void CSkinListView::ClearAllSelMark()
{
	for (int i = 0; i < (int)GetRowCount(); i++)
	{
		if (m_dataSource->IsRowSelected(i))
			m_dataSource->SetRowSelectionState(i, false);
	}
}

int CSkinListView::GetLastVisibleRow() const
{
	int		n;
	n = m_nFirstVisibleRow + GetLinesOfPerPage() - 1;
	if (n > (int)GetRowCount() - 1)
		n = (int)GetRowCount() - 1;
	return n;
}


void CSkinListView::ReverseSelectStatOfRow(bool bClearOldSel, int nRow)
{
	ASSERT(nRow >= -1 && nRow < (int)GetRowCount());
	if (bClearOldSel)
	{
		ClearAllSelMark();
	}

	if (nRow != -1)
	{
		m_dataSource->SetRowSelectionState(nRow, !m_dataSource->IsRowSelected(nRow));
	}
}

void CSkinListView::SelectRangeRow(int nBeg, int nEnd)
{
	if (nEnd == -1)
		return;

	ASSERT(nEnd >= 0 && nEnd < (int)GetRowCount());
	ASSERT(nBeg >= -1 && nBeg < (int)GetRowCount());

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
		m_dataSource->SetRowSelectionState(nBeg, true);
	}
}


void CSkinListView::RecalculateColWidth(CRawGraph *pGraph)
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
	for (i = 0; i < (int)GetRowCount(); i++)
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


void CSkinListView::SetScrollInfo(bool bRedraw)
{
	int		nPage = GetLinesOfPerPage() - 1;
	if (nPage < 1)
		nPage = 1;
	if (m_pVertScrollBar)
		m_pVertScrollBar->SetScrollInfo(0, GetRowCount(), nPage, m_nFirstVisibleRow, 1, bRedraw);
}

void CSkinListView::SetHorzScrollInfo(bool bRedraw)
{
	int		nWidth = 0;

	for (size_t i = 0; i < m_vHeading.size(); i++)
		nWidth += m_vHeading[i]->nWidth;

	if (m_pHorzScrollBar)
		m_pHorzScrollBar->SetScrollInfo(0, nWidth, m_rcContent.Width(), m_pHorzScrollBar->GetScrollPos(), 10, bRedraw);
}


void CSkinListView::SendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol)
{
	CSkinListCtrlEventNotify	event(this, nClickedRow, nClickedCol);

	event.cmd = cmd;
	m_pSkin->DispatchUIObjNotify(&event);
}

CSkinListView::HitTestArea CSkinListView::HitTest(POINT &point, int &nColumnIndex)
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

void CSkinListView::DrawHeader(CRawGraph *pGraph)
{
	int		x = m_rcContent.left;
	int		y = m_rcContent.top;

	if (m_pHorzScrollBar)
		x -= m_pHorzScrollBar->GetScrollPos();

	for (int i = 0; i < (int)m_vHeading.size(); i++)
	{
		CColHeader	*pColHeader = m_vHeading[i];
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

void CSkinListView::InvalidateItem(int nItem)
{
	if (nItem >= m_nFirstVisibleRow && nItem <= GetLastVisibleRow())
		Invalidate();
}

bool CSkinListView::IsClickedOn(int row, int col, CColHeader *pHeader, int x, int y)
{
	CColHeader *header = m_vHeading[col];
	if (header->colType == CColHeader::TYPE_IMAGE)
	{
		CRawImage *image = m_dataSource->GetCellImage(row, col);
		if (image != NULL)
			return x >= 0 && x < image->m_cx && y >= 0 && y < image->m_cy;
	}
	else
		return true;

	return false;
}

void CSkinListView::DrawCell(int row, int col, RECT &rcCell, CRawGraph *pGraph, MLColor &clrText)
{
	int colType = m_vHeading[col]->colType;
	if (colType == CColHeader::TYPE_TEXT)
	{
		LPCTSTR text = m_dataSource->GetCellText(row, col);
		if (text != NULL)
			DrawCellText(text, rcCell, pGraph, clrText);
	}
	else if (colType == CColHeader::TYPE_IMAGE)
	{
		CRawImage *image = m_dataSource->GetCellImage(row, col);
		if (image != NULL)
			DrawCellImage(*image, rcCell, pGraph);
	}
	else
	{
		ASSERT(0 && "Undefined column type.");
	}
}

void CSkinListView::DrawCellText(LPCTSTR text, RECT &rcCell, CRawGraph *pGraph, MLColor &clrText)
{
	if (m_font.IsOutlined())
	{
		pGraph->DrawTextOutlined(text, -1, &rcCell, 
			clrText,
			m_font.GetColorOutlined(), DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
	}
	else
	{
		pGraph->SetTextColor(clrText);
		pGraph->DrawText(text, -1, &rcCell, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
	}
}

void CSkinListView::DrawCellImage(CRawImage &image, RECT &rcCell, CRawGraph *pGraph)
{
	if (image.IsValid())
	{
		CRawGraph::CClipBoxAutoRecovery	cbar(pGraph);

		pGraph->SetClipBoundBox(rcCell);

		int		yDest = rcCell.top + (rcCell.bottom - rcCell.top - image.height()) / 2;

		image.Blt(pGraph, rcCell.left, yDest);
	}
}

void CSkinListView::Draw(CRawGraph *pGraph)
{
	CSkinScrollFrameCtrlBase::Draw(pGraph);

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
	if (IsUseParentBg())
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

		CRect rcItem = m_rcContent;
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

	CRect rc = m_rcContent;
	rc.left += m_nXMargin;
	rc.right -= m_nXMargin;

	if (m_bDrawHeader)
	{
		DrawHeader(pGraph);
		rc.top += m_nHeaderHeight;
	}

	if (m_nFirstVisibleRow >= (int)GetRowCount())
		m_nFirstVisibleRow = (int)GetRowCount() - rc.Height() / m_nLineHeight;
	if (m_nFirstVisibleRow < 0)
		m_nFirstVisibleRow = 0;

	int		y = rc.top;
	int		nClrName;
	for (int i = m_nFirstVisibleRow; i < (int)GetRowCount(); i++)
	{
		int			x = rc.left;
		if (m_pHorzScrollBar)
			x -= m_pHorzScrollBar->GetScrollPos();

		if (m_dataSource->IsRowSelected(i))
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

		CRect rcItem;
		rcItem.left = x;
		rcItem.top = y;
		rcItem.bottom = y + m_nLineHeight;
		for (int k = 0; k < (int)m_vHeading.size(); k++)
		{
			rcItem.right = rcItem.left + m_vHeading[k]->nWidth;
			if (k == 0 && nImageCx > 0)
			{
				// Draw icon.
				if (nImageCx > 0 && rcItem.Width() > 0)
				{
					int nImageIndex = m_dataSource->GetItemImageIndex(i);
					if (nImageIndex >= 0)
						m_imageList.Draw(pGraph, nImageIndex, x, y + (m_nLineHeight - m_imageList.GetIconCy()) / 2);
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

			DrawCell(i, k, rcItem, pGraph, GetColor(nClrName));
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


bool CSkinListView::OnLButtonDown(UINT nFlags, CPoint point)
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


bool CSkinListView::OnRButtonDown(UINT nFlags, CPoint point)
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
		if (m_dataSource->IsRowSelected(nClickedRow))
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


bool CSkinListView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (CSkinScrollFrameCtrlBase::OnRButtonUp(nFlags, point))
		return true;

	SendNotifyEvent(CSkinListCtrlEventNotify::C_RBTN_CLICK);

	return false;
}


bool CSkinListView::OnLButtonDblClk(UINT nFlags, CPoint point)
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


void CSkinListView::OnKeyDown(UINT nChar, UINT nFlags)
{
	if (GetRowCount() == 0)
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
		bool		bCtrl = IsModifierKeyPressed(MK_CONTROL, nFlags);

		if (bCtrl)
		{
			// select all
			CSelChangedCheck	selChecher(this);

			SelectRangeRow(0, GetRowCount() - 1);
			if (selChecher.IsSelectChanged())
				Invalidate();
		}
		return;
	}
	else if (nChar == 'I')
	{
		bool		bCtrl = IsModifierKeyPressed(MK_CONTROL, nFlags);

		if (bCtrl)
		{
			// invert selection
			CSelChangedCheck	selChecher(this);

			for (int i = 0; i < (int)GetRowCount(); i++)
			{
				m_dataSource->SetRowSelectionState(i, !m_dataSource->IsRowSelected(i));
			}
			Invalidate();
		}
		return;
	}

	bool		bShift = IsModifierKeyPressed(MK_SHIFT, nFlags);

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
			else if (m_nEndSelRow < (int)GetRowCount() - 1)
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
			else if (m_nEndSelRow < (int)GetRowCount() - 1)
			{
				if (m_nEndSelRow != GetLastVisibleRow())
					m_nEndSelRow = GetLastVisibleRow();
				else
				{
					m_nEndSelRow = GetLastVisibleRow() + GetLinesOfPerPage() - 1;
					if (m_nEndSelRow == GetLastVisibleRow())
						m_nEndSelRow++;
					if (m_nEndSelRow >= (int)GetRowCount())
						m_nEndSelRow = GetRowCount() - 1;
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
			m_nEndSelRow = GetRowCount() - 1;

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


bool CSkinListView::OnLButtonUp(UINT nFlags, CPoint point)
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

bool CSkinListView::OnMouseDrag(CPoint point)
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

bool CSkinListView::OnMouseMove(CPoint point)
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

int CSkinListView::GetRowCount() const
{
	if (m_dataSource != NULL)
		return m_dataSource->GetRowCount();
	else
		return 0;
}
