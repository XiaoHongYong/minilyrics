#include "StdAfx.h"
#include "Skin.h"
#include "SkinLinearContainer.h"
#include "SkinResizableLinearContainer.h"

UIOBJECT_CLASS_NAME_IMP(CSkinResizableLinearContainer, _T("ResizableLinearContainer"))

//////////////////////////////////////////////////////////////////////

CSkinResizableLinearContainer::CSkinResizableLinearContainer()
{
	m_bDragWnd = false;

	m_cursorWE.LoadStdCursor(CCursor::C_SIZEWE);
	m_cursorNS.LoadStdCursor(CCursor::C_SIZENS);

	m_nThicknessOfResizer = 3;
}

CSkinResizableLinearContainer::~CSkinResizableLinearContainer()
{
}

bool CSkinResizableLinearContainer::OnMouseDrag(CPoint point)
{
	if (m_bDragWnd)
	{
		SetResizerCursor();

		// Start to resizer drag now
		bool bDone = false;
		if (m_bVertical)
		{
			int offset = point.y - m_ptDragOld.y;
			DragResizer(offset);
			m_ptDragOld.y += offset;
		}
		else
		{
			int offset = point.x - m_ptDragOld.x;
			DragResizer(offset);
			m_ptDragOld.x += offset;
		}

		return true;
	}

	int nResizerArea = HitTestResizer(point);
	if (nResizerArea != -1)
		SetResizerCursor();

	return CSkinLinearContainer::OnMouseDrag(point);
}

bool CSkinResizableLinearContainer::OnMouseMove(CPoint point) 
{
	int nResizerArea = HitTestResizer(point);
	if (nResizerArea != -1)
		SetResizerCursor();

	return CSkinLinearContainer::OnMouseMove(point);
}

bool CSkinResizableLinearContainer::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_nCurrentResizerArea = HitTestResizer(point);
	if (m_nCurrentResizerArea == -1)
		return CSkinLinearContainer::OnLButtonDown(nFlags, point);

	m_bDragWnd = true;
	m_ptDragOld = point;
	m_pSkin->SetCaptureMouse(this);

	SetResizerCursor();

	return true;
}

bool CSkinResizableLinearContainer::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!m_bDragWnd)
		return CSkinLinearContainer::OnLButtonUp(nFlags, point);

	m_bDragWnd = false;
	m_pSkin->ReleaseCaptureMouse(this);

	SetResizerCursor();

	return true;
}

BOOL CSkinResizableLinearContainer::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinLinearContainer::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("ResizerThickness")))
		m_nThicknessOfResizer = _ttoi(szValue);
	else
		return FALSE;

	return TRUE;
}

int CSkinResizableLinearContainer::HitTestResizer(CPoint pt)
{
	if (!m_rcObj.PtInRect(pt))
		return -1;

	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject *p = m_vUIObjs[i];

		if (m_bVertical)
		{
			if (pt.y < p->m_rcObj.top + p->GetMargin().top)
				return i - 1;
			else if (pt.y < p->m_rcObj.bottom + p->GetMargin().bottom)
				return -1;
		}
		else
		{
			if (pt.x < p->m_rcObj.left + p->GetMargin().left)
				return i - 1;
			else if (pt.x < p->m_rcObj.right + p->GetMargin().right)
				return -1;
		}
	}

	return -1;
}

void CSkinResizableLinearContainer::SetResizerCursor()
{
	if (m_bVertical)
		MLWidget::SetCursor(m_cursorNS);
	else
		MLWidget::SetCursor(m_cursorWE);
}

void CSkinResizableLinearContainer::DragResizer(int &offset)
{
	ASSERT(m_nCurrentResizerArea != -1);
	if (m_nCurrentResizerArea < 0 || m_nCurrentResizerArea >= (int)m_vUIObjs.size())
	{
		offset = 0;
		return;
	}

	// Check whether and how much we can resize.
	int totalMinSize = 0, totalFixedSize = 0;
	VecItems vItemsStart;
	int totalSizeStart = 0;
	UIObjsToItems(m_vUIObjs.begin(), m_vUIObjs.begin() + m_nCurrentResizerArea + 1, vItemsStart, totalMinSize, totalSizeStart, totalFixedSize);
	int offsetMin = totalMinSize - totalSizeStart;

	VecItems vItemsEnd;
	int totalSizeEnd = 0;
	UIObjsToItems(m_vUIObjs.begin() + m_nCurrentResizerArea + 1, m_vUIObjs.end(), vItemsEnd, totalMinSize, totalSizeEnd, totalFixedSize);
	int offsetMax = totalSizeEnd - totalMinSize;

	if (offset < offsetMin)
		offset = offsetMin;
	else if (offset > offsetMax)
		offset = offsetMax;

	if (offset == 0)
		return;

	// Drag to top or left
	ZoomFromRight(vItemsStart, totalSizeStart + offset);

	// Drag to top or left
	ZoomFromLeft(vItemsEnd, totalSizeEnd - offset);

	vItemsStart.insert(vItemsStart.end(), vItemsEnd.begin(), vItemsEnd.end());

	ResizeFromItemsToUIObjs(vItemsStart, m_vUIObjs.begin(), m_vUIObjs.end(), 
		m_bVertical ? m_rcContent.top : m_rcContent.left);

	Invalidate();
}

void CSkinResizableLinearContainer::OnCreate()
{
	CSkinLinearContainer::OnCreate();

	int totalMinSize = 0, totalFixedSize = 0, totalSizeStart = 0;
	VecItems vItems;
	UIObjsToItems(m_vUIObjs.begin(), m_vUIObjs.end(), vItems, totalMinSize, totalSizeStart, totalFixedSize);

	// Reload sizes of children
	VecString vSize;
	StrSplit(GetProfileString(_T("ChildrenSize"), _T("")).c_str(), ',', vSize);
	if (vSize.size() == vItems.size())
	{
		int i = 0;
		for (VecItems::iterator it = vItems.begin();
			i < (int)vSize.size() && it != vItems.end(); i++, ++it)
		{
			if (vSize[i].empty())
				continue;
			int size = _ttoi(vSize[i].c_str());
			(*it).size = size;
		}

		ResizeFromItemsToUIObjs(vItems, m_vUIObjs.begin(), m_vUIObjs.end(), 
			m_bVertical ? m_rcContent.top : m_rcContent.left);
	}
}

void CSkinResizableLinearContainer::OnDestroy()
{
	int totalMinSize = 0, totalFixedSize = 0, totalSizeStart = 0;
	VecItems vItems;
	UIObjsToItems(m_vUIObjs.begin(), m_vUIObjs.end(), vItems, totalMinSize, totalSizeStart, totalFixedSize);

	// Save sizes of children
	CStrPrintf	strFormat;
	tstring		strSizes;
	for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it)
	{
		if (strSizes.size())
			strSizes += ',';
		strFormat.printf(_T("%d"), (*it).size);
		strSizes += strFormat.c_str();
	}

	WriteProfileString(_T("ChildrenSize"), strSizes.c_str());

	CSkinLinearContainer::OnDestroy();
}

void CSkinResizableLinearContainer::OnSize()
{
	CSkinLinearContainer::OnSize();
	return;

	CUIObject::OnSize();

	FORMULA_VAR 	vars[] = {
		{'w', m_rcObj.Width()},
		{'h', m_rcObj.Height()},
		{0, 0}
	};

	// Measure child size position
	for (uint i = 0; i < m_vUIObjs.size(); i++)
	{
		CUIObject *p = m_vUIObjs[i];
		if (!p->IsVisible())
			continue;

		p->OnMeasureSizePos(vars);
	}

	VecItems vItems;
	int totalMinSize = 0, totalFixedSize = 0, totalSize = 0;
	UIObjsToItems(m_vUIObjs.begin(), m_vUIObjs.end(), vItems, totalMinSize, totalSize, totalFixedSize);

	uint totalReqSize = 0;
	uint totalMinSizeOfHasWeight = 0;
	uint totalReqSizeOfHasWeight = 0;
	for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it)
	{
		Item &item = *it;
		totalReqSize += item.size;
		if (item.weight > 0)
		{
			totalMinSizeOfHasWeight += item.minSize;
			totalReqSizeOfHasWeight += item.size;
		}
	}

	int nSize = m_rcObj.Height() - totalFixedSize;
	int nSizeToDecrease = totalReqSize - nSize;
	if (nSizeToDecrease > 0)
	{
		// The requested height is bigger than LinearCtrl height. We have to decrease it.
		if ((int)(totalReqSizeOfHasWeight - totalMinSizeOfHasWeight) < nSizeToDecrease)
			ZoomAll(vItems, nSize);
		else
			ZoomWeightable(vItems, nSize);
	}
	else
		ZoomWeightable(vItems, nSize);

	ResizeFromItemsToUIObjs(vItems, m_vUIObjs.begin(), m_vUIObjs.end(), 
		m_bVertical ? m_rcContent.top : m_rcContent.left);
}

void CSkinResizableLinearContainer::ZoomFromRight(VecItems &vItems, int nSize)
{
	if (vItems.size() == 0)
		return;

	int sizeTotal = 0;
	for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it)
	{
		Item &item = *it;
		sizeTotal += item.size;
	}

	int sizeDecrease = sizeTotal - nSize;
	if (sizeDecrease <= 0)
	{
		// Zoom in rightest
		vItems.back().size += -sizeDecrease;
		return;
	}

	// Zoom out one by one from right
	for (VecItems::reverse_iterator it = vItems.rbegin(); it != vItems.rend(); ++it)
	{
		Item &item = *it;
		if (sizeDecrease <= item.size - item.minSize)
		{
			// Zoom out this item can end the zoom out
			item.size -= sizeDecrease;
			break;
		}

		sizeDecrease -= item.size - item.minSize;
		item.size = item.minSize;
	}
}

void CSkinResizableLinearContainer::ZoomFromLeft(VecItems &vItems, int nSize)
{
	if (vItems.size() == 0)
		return;

	int sizeTotal = 0;
	for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it)
	{
		Item &item = *it;
		sizeTotal += item.size;
	}

	int sizeDecrease = sizeTotal - nSize;
	if (sizeDecrease <= 0)
	{
		// Zoom in left one
		vItems.front().size += -sizeDecrease;
		return;
	}

	// Zoom out one by one from left
	for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it)
	{
		Item &item = *it;
		if (sizeDecrease <= item.size - item.minSize)
		{
			// Zoom out this item can end the zoom out
			item.size -= sizeDecrease;
			break;
		}

		sizeDecrease -= item.size - item.minSize;
		item.size = item.minSize;
	}
}

void CSkinResizableLinearContainer::ZoomAll(VecItems &vItems, int nSize)
{
	if (vItems.size() == 0)
		return;

	int sizeToAlloc = nSize;
	int totalMinSize = 0;
	for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it)
	{
		Item &item = *it;

		sizeToAlloc -= item.minSize;

		if (item.weight == 0)
			totalMinSize += item.minSize;
	}
	ASSERT(totalMinSize > 0);

	for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it)
	{
		Item &item = *it;
		if (item.weight > 0)
			item.size = item.minSize;
		else if (item.minSize == 0)
			item.size = 10;
		else
			item.size = item.minSize + sizeToAlloc * item.minSize / totalMinSize;
	}
}
