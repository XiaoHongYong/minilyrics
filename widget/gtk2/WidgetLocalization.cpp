// WidgetLocalization.cpp: implementation of the CWidgetLocalization class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLMenu.h"
#include "BaseWnd.h"
#include "WidgetLocalization.h"



BOOL ToLocalMenu(CMLMenu *pMenu)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
CLangTreeCtrlHelper::CLangTreeCtrlHelper()
{

}

CLangTreeCtrlHelper::~CLangTreeCtrlHelper()
{

}

void CLangTreeCtrlHelper::Init(CBaseWnd *pWndParent, UINT nIDTreeCtrl)
{

}

void CLangTreeCtrlHelper::Destroy()
{

}

void CLangTreeCtrlHelper::ToLocal()
{

}

/*

/////////////////////////////////////////////////////////////////////////////////
// BEGIN OF CLangListCtrlHelper
CLangListCtrlHelper::CLangListCtrlHelper()
{
	m_hWnd = NULL;
}

CLangListCtrlHelper::~CLangListCtrlHelper()
{
	m_vStrHeader.erase(m_vStrHeader.begin(), m_vStrHeader.end());
}

void CLangListCtrlHelper::Init(finder.GetCurName() hWnd)
{
	int			i;
	LVCOLUMN	LvCol;
	TCHAR		szBuffer[256];

	assert(hWnd);
	m_hWnd = hWnd;

	LvCol.cchTextMax = CountOf(szBuffer);
	LvCol.mask = LVCF_TEXT;
	LvCol.pszText = szBuffer;

	for (i = 0; ; i++)
	{
		if (!ListView_GetColumn(m_hWnd, i, &LvCol))
			return;
		
		m_vStrHeader.push_back(LvCol.pszText);
	}
}

void CLangListCtrlHelper::ToLocal()
{
	int			i, nCount;
	LVCOLUMN	LvCol;

	assert(m_hWnd);

	nCount = m_vStrHeader.size();
	LvCol.mask = LVCF_TEXT;

	for (i = 0; i < nCount; i++)
	{
		LvCol.cchTextMax = m_vStrHeader[i].size();
		LvCol.pszText = (LPTSTR)TOLOCAL(m_vStrHeader[i].c_str());

		if (!ListView_SetColumn(m_hWnd, i, &LvCol))
			return;
		
		m_vStrHeader.push_back(LvCol.pszText);
	}
}
*/

/////////////////////////////////////////////////////////////////////////////////
// BEGIN OF CLangDialogHelper
CLangDialogHelper::CLangDialogHelper()
{
//	m_hWndParent = NULL;
}

CLangDialogHelper::~CLangDialogHelper()
{
}

void CLangDialogHelper::AddDlgItem(int nID)
{
// 	assert(m_hWndParent);
// 
// 	finder.GetCurName()		hWndItem;
// 
// 	hWndItem = GetDlgItem(m_hWndParent, nID);
// 	assert(hWndItem);
// 
// 	AddDlgItem(hWndItem);
}

// void CLangDialogHelper::AddDlgItem(finder.GetCurName() hWndItem)
// {
// 	assert(m_hWndParent);
// 	assert(hWndItem);
// 
// 	TCHAR		szWndText[256];
// 	DlgItem		Item;
// 	
// 	Item.bLoacalText = TRUE;
// 
// 	if (GetClassName(hWndItem, szWndText, CountOf(szWndText)))
// 	{
// 		if (_tcscmp(szWndText, _T("SysListView32")) == 0)
// 		{
// 			CLangListCtrlHelper	ListHelper;
// 
// 			ListHelper.Init(hWndItem);
// 
// 			m_vListCtrlHelper.push_back(ListHelper);
// 			Item.bLoacalText = FALSE;
// 		}
// 	}
// 
// 	if (GetWindowText(hWndItem, szWndText, CountOf(szWndText)) == 0)
// 	{
// 		Item.bLoacalText = FALSE;
// 	}
// 
// 	Item.hWnd = hWndItem;
// 	Item.strInit = szWndText;
// 
// 	m_vDlgItem.push_back(Item);
// }

void CLangDialogHelper::ToLocal()
{
// 	assert(m_hWndParent);
// 
// 	// 设置窗口的字体
// 	SendMessage(m_hWndParent, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), TRUE);
// 
// 
// 	if (!m_strParent.empty())
// 		SetWindowText(m_hWndParent, TOLOCAL(m_strParent.c_str()));
// 
// 	vector<DlgItem>::iterator		it;
// 
// 	for (it = m_vDlgItem.begin(); it != m_vDlgItem.end(); it++)
// 	{
// 		// 转换文字
// 		if ((*it).bLoacalText)
// 			SetWindowText((*it).hWnd, TOLOCAL((*it).strInit.c_str()));
// 
// 		// 设置字体
// 		SendMessage((*it).hWnd, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), TRUE);
// 	}
// 
// 	// 本地化语言ListView控件
// 	vector<CLangListCtrlHelper>::iterator		itList;
// 	for (itList = m_vListCtrlHelper.begin(); itList != m_vListCtrlHelper.end(); itList++)
// 	{
// 		(*itList).ToLocal();
// 	}
}

void CLangDialogHelper::Init(CBaseWnd *pWnd)
{
// 	m_hWndParent = hWnd;
// 
// 	TCHAR		szWndText[256];
// 
// 	GetWindowText(m_hWndParent, szWndText, CountOf(szWndText));
// 	m_strParent = szWndText;
// 
// 	m_vDlgItem.erase(m_vDlgItem.begin(), m_vDlgItem.end());
// 	m_vListCtrlHelper.erase(m_vListCtrlHelper.begin(), m_vListCtrlHelper.end());
}

void CLangDialogHelper::InitAddAll(CBaseWnd *pWndParent)
{
// 	Init(hWndParent);
// 
// 	HWND		hWndItem;
// 
// 	hWndItem = FindWindowEx(m_hWndParent, NULL, NULL, NULL);
// 
// 	while (hWndItem)
// 	{
// 		AddDlgItem(hWndItem);
// 		hWndItem = FindWindowEx(m_hWndParent, hWndItem, NULL, NULL);
// 	}
}

void CLangDialogHelper::RemoveDlgItem(int nID)
{
// 	assert(m_hWndParent);
// 
// 	HWND		hWndItem;
// 
// 	hWndItem = GetDlgItem(m_hWndParent, nID);
// 	assert(hWndItem);
// 
// 	RemoveDlgItem(hWndItem);
}
/*

void CLangDialogHelper::RemoveDlgItem(HWND hWndItem)
{
	assert(m_hWndParent);

	vector<DlgItem>::iterator		it;

	// 删除普通控件
	for (it = m_vDlgItem.begin(); it != m_vDlgItem.end(); it++)
	{
		if ((*it).hWnd == hWndItem)
		{
			// 转换文字
			SetWindowText((*it).hWnd, (*it).strInit.c_str());

			(*it).bLoacalText = FALSE;
			return;
		}
	}

	// 删除ListView控件
	vector<CLangListCtrlHelper>::iterator		itList;
	for (itList = m_vListCtrlHelper.begin(); itList != m_vListCtrlHelper.end(); itList++)
	{
		if ((*itList).m_hWnd == hWndItem)
		{
			m_vListCtrlHelper.erase(itList);
			return;
		}
	}

	assert(0);
}
*/

void CLangDialogHelper::Destroy()
{
// 	m_hWndParent = NULL;
// 	m_vDlgItem.erase(m_vDlgItem.begin(), m_vDlgItem.end());
// 	m_vListCtrlHelper.erase(m_vListCtrlHelper.begin(), m_vListCtrlHelper.end());
}

