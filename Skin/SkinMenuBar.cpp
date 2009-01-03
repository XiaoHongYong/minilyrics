// SkinMenuBar.cpp: implementation of the CSkinMenuBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLSkin.h"
#include "SkinMenuBar.h"

#define SPACE_SUBMENUS		(m_rcObj.Height() / 2)

UIOBJECT_CLASS_NAME_IMP(CSkinMenuBar, _T("MenuBar"))

CSkinMenuBar::CSkinMenuBar()
{
	m_nMsgWant = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON;

	m_bLBtDown = false;
	m_bHover = false;

	m_nSelSubMenu = -1;

	m_bOutlinedHover = false;
	m_bOutlinedPressed = false;

	m_clrHover.set(0);
	m_clrPressed.set(0);

	// m_bpm = BPM_ALPHA_BLEND;
/*	SubMenu		item;

	item.strText = _T("File");
	m_vSubMenus.push_back(item);

	item.strText = _T("Edit");
	m_vSubMenus.push_back(item);

	item.strText = _T("View");
	m_vSubMenus.push_back(item);

	item.strText = _T("Tool");
	m_vSubMenus.push_back(item);*/

	m_pMenu = NULL;
}


CSkinMenuBar::~CSkinMenuBar()
{

}


void CSkinMenuBar::OnCreate()
{
	CUIObject::OnCreate();

	m_font.OnCreate(m_pSkin);

	ASSERT(m_pMenu);
	if (m_pMenu)
	{
		m_pMenu->SetModeless(true);

		for (int i = 0; i < 255; i++)
		{
			SubMenu		item;

			if (m_pMenu->GetMenuItemInfo(i, item.strText, item.nID, item.bHasSubMenu, true))
			{
				m_vSubMenus.push_back(item);
			}
		}
	}
}


void CSkinMenuBar::Draw(CRawGraph *pGraph)
{
	int			x = 0;
	int			y = 0;
	int			nSpaceSubMenus = SPACE_SUBMENUS;
	CRect		rcItem;

	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	rcItem.top = m_rcObj.top;
	rcItem.bottom = m_rcObj.bottom;
	x = m_rcObj.left;

	for (int i = 0; i < m_vSubMenus.size(); i++)
	{
		SubMenu		&item = m_vSubMenus[i];
		MLColor		clrText, clrOutlined;
		SIZE		size;
		bool		bOutlined;

		pGraph->GetTextExtentPoint32(item.strText.c_str(), item.strText.size(), &size);
		item.nWidth = size.cx + nSpaceSubMenus * 2;

		if (x + item.nWidth > m_rcObj.right)
			break;

		rcItem.left = x;
		rcItem.right = x + item.nWidth;

// 		if (i % 2 == 0)
// 		{
// 			CMLBrush	br;
// 			br.CreateSolidBrush(MLColor(0));
// 			pGraph->FillRect(rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height(), &br);
// 		}

		if (i == m_nSelSubMenu && m_bLBtDown)
		{
			clrText = m_clrPressed;
			clrOutlined = m_clrOutlinedPressed;
			bOutlined = m_bOutlinedPressed;
		}
		else if (i == m_nSelSubMenu && m_bHover)
		{
			clrText = m_clrHover;
			clrOutlined = m_clrOutlinedHover;
			bOutlined = m_bOutlinedHover;
		}
		else
		{
			clrText = m_font.GetTextColor();
			clrOutlined = m_font.GetColorOutlined();
			bOutlined = m_font.IsOutlined();
		}

		if (bOutlined)
			pGraph->DrawTextOutlined(item.strText.c_str(), item.strText.size(), 
				&rcItem, clrText, clrOutlined, DT_VCENTER | DT_CENTER);
		else
		{
			pGraph->SetTextColor(clrText);
			pGraph->DrawText(item.strText.c_str(), item.strText.size(), 
			&rcItem, DT_VCENTER | DT_CENTER);
		}

		x += m_vSubMenus[i].nWidth;
	}
}


bool CSkinMenuBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_rcObj.PtInRect(point))
		return false;

	m_nSelSubMenu = HitTestSubMenu(point);
	if (m_nSelSubMenu == -1)
		return false;

	m_bLBtDown = TRUE;

	if (m_bHover)
		m_bHover = false;
	else
	{
		// 捕捉鼠标输入
		m_pSkin->SetCaptureMouse(this);
	}

	Invalidate();

	// Popup sel menu item
	if (m_vSubMenus[m_nSelSubMenu].bHasSubMenu)
		TrackPopupMenu();
	else
		m_pSkin->OnCommand(m_vSubMenus[m_nSelSubMenu].nID, 0);

	return true;
}


bool CSkinMenuBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	// 释放鼠标输入
	m_pSkin->ReleaseCaptureMouse(this);

	if (!m_bLBtDown)
		return true;

	m_bLBtDown = FALSE;

	Invalidate();

	return true;
}


bool CSkinMenuBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (HitTestSubMenu(point) == -1)
		return false;

	return true;
}


bool CSkinMenuBar::OnMouseMove(UINT nFlags, CPoint point)
{
	int			nSelSubMenuNew;

	nSelSubMenuNew = HitTestSubMenu(point);
	if (nSelSubMenuNew == -1 && !m_bLBtDown && !m_bHover)
		return false;

	if (IsPtIn(point))
	{
		if (!m_bLBtDown && (nFlags & MK_LBUTTON))
		{
			m_bLBtDown = TRUE;

			if (m_bHover)
				m_bHover = false;
			else
			{
				// 捕捉鼠标输入
				m_pSkin->SetCaptureMouse(this);
			}

			m_nSelSubMenu = nSelSubMenuNew;

			Invalidate();

			// Popup sel menu item
			if (m_nSelSubMenu != -1 && m_vSubMenus[m_nSelSubMenu].bHasSubMenu)
				TrackPopupMenu();
		}
		else if (!m_bLBtDown && !m_bHover)
		{
			m_bHover = true;

			// 捕捉鼠标输入
			m_pSkin->SetCaptureMouse(this);

			m_nSelSubMenu = nSelSubMenuNew;

			Invalidate();
		}
		else if (m_bHover)
		{
			if (m_nSelSubMenu != nSelSubMenuNew)
			{
				m_nSelSubMenu = nSelSubMenuNew;
				Invalidate();
			}
		}
		else if (m_bLBtDown)
		{
			if (m_nSelSubMenu != nSelSubMenuNew)
			{
				m_nSelSubMenu = nSelSubMenuNew;
				Invalidate();

				// Popup new menu
				if (m_nSelSubMenu != -1 && m_vSubMenus[m_nSelSubMenu].bHasSubMenu)
					TrackPopupMenu();
			}
		}
	}
	else
	{
		if (m_bLBtDown)
		{
			m_bLBtDown = FALSE;

			// 释放鼠标输入
			m_pSkin->ReleaseCaptureMouse(this);

			m_nSelSubMenu = nSelSubMenuNew;

			Invalidate();
		}
		else if (m_bHover)
		{
			m_bHover = false;

			// 释放鼠标输入
			m_pSkin->ReleaseCaptureMouse(this);

			m_nSelSubMenu = nSelSubMenuNew;

			Invalidate();
		}
	}

	return true;
}


BOOL CSkinMenuBar::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("Menu")))
	{
		m_strMenu = szValue;
		m_pSkin->GetSkinFactory()->LoadMenu(m_pSkin, &m_pMenu, szValue);
	}
	else if (m_font.SetProperty(szProperty, szValue))
		return TRUE;
	else if (IsPropertyName(szProperty, _T("TextColorHover")))
		GetColorValue(m_clrHover, szValue);
	else if (IsPropertyName(szProperty, _T("TextOutlinedColorHover")))
	{
		GetColorValue(m_clrOutlinedHover, szValue);
		m_bOutlinedHover = true;
	}
	else if (IsPropertyName(szProperty, _T("TextColorPressed")))
		GetColorValue(m_clrPressed, szValue);
	else if (IsPropertyName(szProperty, _T("TextOutlinedColorPressed")))
	{
		GetColorValue(m_clrOutlinedPressed, szValue);
		m_bOutlinedPressed = true;
	}
	else
		return FALSE;

	return TRUE;
}


void CSkinMenuBar::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);
}


int CSkinMenuBar::HitTestSubMenu(const POINT point)
{
	int			x = m_rcObj.left;

	if (point.x < m_rcObj.left)
		return -1;

	for (int i = 0; i < m_vSubMenus.size(); i++)
	{
		x += m_vSubMenus[i].nWidth;
		if (x > m_rcObj.right)
			return -1;

		if (point.x < x)
			return i;
	}

	return -1;
}

int CSkinMenuBar::GetItemX(int n)
{
	int			x = m_rcObj.left;

	if (n >= m_vSubMenus.size())
		n = m_vSubMenus.size() - 1;
	for (int i = 0; i < n; i++)
	{
		x += m_vSubMenus[i].nWidth;
		if (x > m_rcObj.right)
			return x;
	}

	return x;
}

void CSkinMenuBar::TrackPopupMenu()
{
	if (m_nSelSubMenu == -1)
		return;

	CPoint	pt;

	pt.x = GetItemX(m_nSelSubMenu);
	pt.y = m_rcObj.bottom;

	m_pSkin->ClientToScreen(pt);

	if (m_pMenu)
		m_pMenu->TrackPopupSubMenu(pt.x, pt.y, m_nSelSubMenu, m_pSkin);
}
