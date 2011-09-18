#include "StdAfx.h"
#include "Skin.h"
#include "SkinLinearContainer.h"

UIOBJECT_CLASS_NAME_IMP(CSkinLinearContainer, _T("LinearContainer"))

//////////////////////////////////////////////////////////////////////

CSkinLinearContainer::CSkinLinearContainer()
{
	m_bVertical = true;
	m_nSeparatorThickness = 0;
}

CSkinLinearContainer::~CSkinLinearContainer()
{
}

void CSkinLinearContainer::RecalculateUIObjSizePos(CUIObject *pObj)
{
	m_pContainer->RecalculateUIObjSizePos(this);
}

void CSkinLinearContainer::OnMeasureSizeByContent()
{
	FORMULA_VAR 	vars[] = {
		{'w', m_rcObj.Width()},
		{'h', m_rcObj.Height()},
		{0, 0}
	};

	uint requestedHeight = 0;
	uint requestedWidth = 0;
	for (uint i = 0; i < m_vUIObjs.size(); i++)
	{
		CUIObject *p = m_vUIObjs[i];
		if (!p->IsVisible())
			continue;

		p->OnMeasureSizePos(vars);
		if (p->GetWeight() > 0)
			DBGLOG2(_T("Parent LinearContainer has wrap content property, the weight of child can't take effect: %s, %d."), p->m_strName.c_str(), p->GetWeight());

		if (m_bVertical)
		{
			requestedHeight += p->m_rcObj.Height();
			requestedHeight += p->GetMargin().top + p->GetMargin().bottom;

			int w = p->m_rcObj.Width() + p->GetMargin().left + p->GetMargin().right;
			if (w > (int)requestedWidth)
				requestedWidth = w;
		}
		else
		{
			requestedWidth += p->m_rcObj.Width();
			requestedWidth += p->GetMargin().left + p->GetMargin().right;

			int h = p->m_rcObj.Height() + p->GetMargin().top + p->GetMargin().bottom;
			if (h > (int)requestedHeight)
				requestedHeight = h;
		}
	}

	m_rcObj.right = m_rcObj.left + requestedWidth;
	m_rcObj.bottom = m_rcObj.top + requestedHeight;
}

void CSkinLinearContainer::OnSize()
{
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

	int size;
	if (m_bVertical)
		size = m_rcObj.Height() - totalFixedSize;
	else
		size = m_rcObj.Width() - totalFixedSize;

	ZoomWeightable(vItems, size);

	ResizeFromItemsToUIObjs(vItems, m_vUIObjs.begin(), m_vUIObjs.end(), 
		m_bVertical ? m_rcContent.top : m_rcContent.left);
}

BOOL CSkinLinearContainer::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinContainer::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("Orientation")))
		m_bVertical = IsPropertyName(szValue, _T("Vertical"));
	else if (IsPropertyName(szProperty, _T("SeparatorThickness")))
		m_nSeparatorThickness = _ttoi(szValue);
	else
		return FALSE;

	return TRUE;
}

void CSkinLinearContainer::ZoomWeightable(VecItems &vItems, int nSize)
{
	if (vItems.size() == 0)
		return;

	int totalWeight = 0;
	int sizeToAlloc = nSize;
	for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it)
	{
		Item &item = *it;
		if (item.weight > 0)
		{
			totalWeight += item.weight;
			sizeToAlloc -= item.minSize;
		}
		else
			sizeToAlloc -= item.size;
	}

	for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it)
	{
		Item &item = *it;
		if (item.weight > 0)
		{
			item.size = item.minSize + sizeToAlloc * item.weight / totalWeight;
			if (item.size < 0)
				item.size = 0;
		}
	}
}

void CSkinLinearContainer::UIObjsToItems(VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, VecItems &vItems, int &totalMinSize, int &totalSize, int &totalFixedSize)
{
	totalMinSize = 0;
	totalFixedSize = 0;
	totalSize = 0;

	for (VecUIObjects::iterator it = itBeg; it != itEnd; ++it)
	{
		CUIObject *p = *it;
		if (!p->IsVisible())
			continue;

		totalFixedSize += m_nSeparatorThickness;

		// Calculate totalFixedSize
		if (m_bVertical)
		{
			if (p->IsFixedHeight())
			{
				totalFixedSize += p->m_rcObj.Height() + p->GetMargin().top + p->GetMargin().bottom;
				continue;
			}
		}
		else
		{
			if (p->IsFixedWidth())
			{
				totalFixedSize += p->m_rcObj.Width() + p->GetMargin().left + p->GetMargin().right;
				continue;
			}
		}

		Item item;

		item.weight = p->GetWeight();
		if (m_bVertical)
		{
			item.minSize = p->GetMinHeight();
			item.size = p->m_rcObj.Height();
			totalFixedSize += p->GetMargin().top + p->GetMargin().bottom;
		}
		else
		{
			item.minSize = p->GetMinWidth();
			item.size = p->m_rcObj.Width();
			totalFixedSize += p->GetMargin().left + p->GetMargin().right;
		}

		totalMinSize += item.minSize;
		totalSize += item.size;

		vItems.push_back(item);
	}

	// Decrease the extra item.
	totalFixedSize -= m_nSeparatorThickness;
}

void CSkinLinearContainer::ResizeFromItemsToUIObjs(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos)
{
	// Calculate size position for every UIObject
	if (m_bVertical)
		ResizeFromItemsToUIObjsForVert(vItems, itBeg, itEnd, startPos);
	else
		ResizeFromItemsToUIObjsForHorz(vItems, itBeg, itEnd, startPos);
}

void CSkinLinearContainer::ResizeFromItemsToUIObjsForVert(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos)
{
	// Calculate size position for every UIObject
	int pos = startPos;
	VecItems::iterator itItem = vItems.begin();
	for (VecUIObjects::iterator it = itBeg; it != itEnd; ++it)
	{
		CUIObject *p = *it;
		if (!p->IsVisible())
			continue;

		// Calculate height
		int height;
		if (p->IsFixedHeight())
			height = p->m_rcObj.Height();
		else
		{
			if (itItem == vItems.end())
			{
				ASSERT(0);
				DBGLOG0(_T("vItems shall NOT end now."));
				break;
			}
			Item &item = *itItem;
			height = item.size;
			++itItem;
			// Set the size value back to UIObject
			p->m_formHeight.SetFormula(height);
		}
		p->m_rcObj.top = pos + p->GetMargin().top;
		p->m_rcObj.bottom = p->m_rcObj.top + height;
		pos = p->m_rcObj.bottom + p->GetMargin().bottom;

		// Calculate width
		int width;
		if (p->GetLayoutParams() & LAYOUT_WIDTH_MATCH_PARENT)
			width = m_rcObj.Width() - p->GetMargin().left - p->GetMargin().right;
		else
			width = p->m_rcObj.Width();

		// Calculate left and right
		if (p->GetLayoutParams() & LAYOUT_ALIN_RIGHT)
		{
			p->m_rcObj.right = m_rcObj.right - p->GetMargin().right;
			p->m_rcObj.left = p->m_rcObj.right - width - p->GetMargin().left;
		}
		else if (p->GetLayoutParams() & LAYOUT_ALIN_HORZ_CENTER)
		{
			p->m_rcObj.left = (m_rcObj.left + m_rcObj.right - width) / 2;
			p->m_rcObj.right += p->m_rcObj.left + width;
		}
		else
		{
			p->m_rcObj.left = m_rcObj.left + p->GetMargin().right;
			p->m_rcObj.right = m_rcObj.left + width;
		}

		p->OnSize();

		pos += m_nSeparatorThickness;
	}
}

void CSkinLinearContainer::ResizeFromItemsToUIObjsForHorz(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos)
{
	// Calculate size position for every UIObject
	int pos = startPos;
	VecItems::iterator itItem = vItems.begin();
	for (VecUIObjects::iterator it = itBeg; it != itEnd && itItem != vItems.end(); ++it)
	{
		CUIObject *p = *it;
		if (!p->IsVisible())
			continue;

		// Calculate width
		int width;
		if (p->IsFixedWidth())
			width = p->m_rcObj.Width();
		else
		{
			Item &item = *itItem;
			width = item.size;
			++itItem;
			// Set the size value back to UIObject
			p->m_formWidth.SetFormula(width);
		}
		p->m_rcObj.left = pos + p->GetMargin().left;
		p->m_rcObj.right = p->m_rcObj.left + width;
		pos = p->m_rcObj.right + p->GetMargin().right;

		// Calculate height
		int height;
		if (p->GetLayoutParams() & LAYOUT_HEIGHT_MATCH_PARENT)
			height = m_rcObj.Height() - p->GetMargin().top - p->GetMargin().bottom;
		else
			height = p->m_rcObj.Height();

		// Calculate left and right
		if (p->GetLayoutParams() & LAYOUT_ALIN_BOTTOM)
		{
			p->m_rcObj.bottom = m_rcObj.bottom - p->GetMargin().bottom;
			p->m_rcObj.top = p->m_rcObj.bottom - height - p->GetMargin().top;
		}
		else if (p->GetLayoutParams() & LAYOUT_ALIN_VERT_CENTER)
		{
			p->m_rcObj.top = (m_rcObj.top + m_rcObj.bottom - height) / 2;
			p->m_rcObj.bottom += p->m_rcObj.top + height;
		}
		else
		{
			p->m_rcObj.top = m_rcObj.top + p->GetMargin().bottom;
			p->m_rcObj.bottom = m_rcObj.top + height;
		}

		p->OnSize();

		pos += m_nSeparatorThickness;
	}
}

void CSkinLinearContainer::OnSetChildVisible(CUIObject *pChild, bool bVisible, bool bRedraw)
{
	// Recalculate the size position.
	m_pContainer->RecalculateUIObjSizePos(this);
}

#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

IMPLEMENT_CPPUNIT_TEST_REG(CSkinLinearContainer)

class CTestCaseCSkinLinearContainer : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCSkinLinearContainer);
	CPPUNIT_TEST(TestZoomFromWeightable);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestZoomFromWeightable()
	{
		CSkinLinearContainer::VecItems items;
		CSkinLinearContainer::Item item;

		item.Set(80, 100, 0);
		items.push_back(item);

		item.Set(50, 50, 1);
		items.push_back(item);

		item.Set(50, 50, 0);
		items.push_back(item);

		// Zoom out
		CSkinLinearContainer::ZoomWeightable(items, 250);
		CPPUNIT_ASSERT(items[0].size == 100);
		CPPUNIT_ASSERT(items[1].size = 100);
		CPPUNIT_ASSERT(items[2].size = 50);

		// Zoom to minimum
		CSkinLinearContainer::ZoomWeightable(items, 200);
		CPPUNIT_ASSERT(items[0].size == 100);
		CPPUNIT_ASSERT(items[1].size = 50);
		CPPUNIT_ASSERT(items[2].size = 50);

		// Zoom to less than minimum
		CSkinLinearContainer::ZoomWeightable(items, 100);
		CPPUNIT_ASSERT(items[0].size == 100);
		CPPUNIT_ASSERT(items[1].size = 50);
		CPPUNIT_ASSERT(items[2].size = 50);
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCSkinLinearContainer);

#endif // _CPPUNIT_TEST
