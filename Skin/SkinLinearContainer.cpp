#include "StdAfx.h"
#include "Skin.h"
#include "SkinLinearContainer.h"

UIOBJECT_CLASS_NAME_IMP(CSkinLinearContainer, _T("LinearContainer"))

//////////////////////////////////////////////////////////////////////

CSkinLinearContainer::CSkinLinearContainer()
{
	m_bVertical = true;
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

	uint totalHeightMin = 0;
	uint totalWidthMin = 0;
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
			totalHeightMin += p->GetMinHeight();
			totalHeightMin += p->GetMargin().top + p->GetMargin().bottom;
		}
		else
		{
			totalWidthMin += p->GetMinWidth();
			totalWidthMin += p->GetMargin().left + p->GetMargin().right;
		}
	}
}

void CSkinLinearContainer::OnSize()
{
	if (m_bVertical)
		OnSizeForVert();
	else
		OnSizeForHorz();
}

void CSkinLinearContainer::OnSizeForVert()
{
	FORMULA_VAR 	vars[] = {
		{'w', m_rcObj.Width()},
		{'h', m_rcObj.Height()},
		{0, 0}
	};

	// Calculate total weight and total fixed height.
	uint totalFixedHeight = 0;
	uint totalWeight = 0;
	for (uint i = 0; i < m_vUIObjs.size(); i++)
	{
		CUIObject *p = m_vUIObjs[i];
		if (!p->IsVisible())
			continue;

		p->OnMeasureSizePos(vars);
		if (p->GetWeight() > 0)
			totalWeight += p->GetWeight();
		else
			totalFixedHeight += p->GetMinHeight();
		totalFixedHeight += p->GetMargin().top + p->GetMargin().bottom;
	}

	// Calculate size position for every UIObject
	int y =  m_rcObj.top;
	for (uint i = 0; i < m_vUIObjs.size(); i++)
	{
		CUIObject *p = m_vUIObjs[i];
		if (!p->IsVisible())
			continue;

		// Calculate height
		int height;
		if (p->GetWeight() != 0)
		{
			height = (m_rcObj.Height() - totalFixedHeight) * p->GetWeight() / totalWeight;
			if (height < p->GetMinHeight())
			{
				totalFixedHeight -= (p->GetMinHeight() - height);
				height = p->GetMinHeight();
			}
		}
		else
			height = p->GetMinHeight();
		p->m_rcObj.top = y + p->GetMargin().top;
		p->m_rcObj.bottom = p->m_rcObj.top + height;
		y = p->m_rcObj.bottom + p->GetMargin().bottom;

		// Calculate width
		int width;
		if (p->GetLayoutParams() & LAYOUT_WIDTH_MATCH_PARENT)
			width = m_rcObj.Width() - p->GetMargin().left - p->GetMargin().right;
		else
		{
			width = p->GetMinWidth();
			if (width > m_rcObj.Width() - p->GetMargin().left - p->GetMargin().right)
				width = m_rcObj.Width() - p->GetMargin().left - p->GetMargin().right;
		}

		// Calculate left and right
		if (p->GetLayoutParams() & LAYOUT_ALIN_RIGHT)
		{
			p->m_rcObj.right = m_rcObj.right - p->GetMargin().right;
			p->m_rcObj.left = m_rcObj.right - p->GetMinWidth();
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
	}
}

void CSkinLinearContainer::OnSizeForHorz()
{

}

BOOL CSkinLinearContainer::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinContainer::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("Orientation")))
		m_bVertical = IsPropertyName(szValue, _T("Vertical"));
	else
		return FALSE;

	return TRUE;
}
