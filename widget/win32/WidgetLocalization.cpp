// LanguageTool.cpp: implementation of the CLanguageTool class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Widget.h"
#include "WidgetLocalization.h"

#ifdef _WINCE_

#ifndef DEFAULT_GUI_FONT
#define DEFAULT_GUI_FONT  SYSTEM_FONT
#endif

#endif

/////////////////////////////////////////////////////////////////////////////////
// BEGIN OF CLangListCtrlHelper
CLangTreeCtrlHelper::CLangTreeCtrlHelper()
{
	m_hWnd = NULL;
}

CLangTreeCtrlHelper::~CLangTreeCtrlHelper()
{
}

void CLangTreeCtrlHelper::Init(CBaseWnd *pWndParent, UINT nIDTreeCtrl)
{
    HTREEITEM       hItem;

	assert(pWndParent);

	m_hWnd = ::GetDlgItem(pWndParent->GetHandle(), nIDTreeCtrl);
	m_vStrItem.erase(m_vStrItem.begin(), m_vStrItem.end());

    hItem = TreeView_GetRoot(m_hWnd);

	if (hItem)
		InitBrotherAndSubTree(hItem);
}


void CLangTreeCtrlHelper::Destroy()
{
	m_hWnd = NULL;
	m_vStrItem.erase(m_vStrItem.begin(), m_vStrItem.end());
}


void CLangTreeCtrlHelper::InitBrotherAndSubTree(HTREEITEM hItem)
{
	assert(m_hWnd);

    HTREEITEM       hChild;
    TVITEM			tvi;
	TCHAR			szText[256];
   
    tvi.mask = TVIF_TEXT;
	tvi.pszText = szText;
	tvi.cchTextMax = CountOf(szText);

    while (hItem)
    {
		tvi.hItem = hItem;
		if (TreeView_GetItem(m_hWnd, &tvi))
		{
			m_vStrItem.push_back(tvi.pszText);
		}

		// 取得子树
		hChild = TreeView_GetChild(m_hWnd, hItem);
		if (hChild)
			InitBrotherAndSubTree(hChild);

        hItem = TreeView_GetNextSibling(m_hWnd, hItem);
    }
}

void CLangTreeCtrlHelper::ToLocal()
{
	assert(m_hWnd);

    HTREEITEM       hItem;
	int				nIndex;

    hItem = TreeView_GetRoot(m_hWnd);

	nIndex = 0;
	if (hItem)
		BrotherAndSubTreeToLocal(hItem, nIndex);
}

void CLangTreeCtrlHelper::BrotherAndSubTreeToLocal(HTREEITEM hItem, int &nIndex)
{
	assert(m_hWnd);

    HTREEITEM       hChild;
    TVITEM			tvi;
   
    tvi.mask = TVIF_TEXT;

    while (hItem)
    {
		assert((UINT)nIndex < m_vStrItem.size());

		tvi.hItem = hItem;
		tvi.pszText = (LPTSTR)_TL(m_vStrItem[nIndex].c_str());
		tvi.cchTextMax = _tcslen(tvi.pszText);

		if (TreeView_SetItem(m_hWnd, &tvi))
		{
			nIndex++;
		}

		// 取得子树
		hChild = TreeView_GetChild(m_hWnd, hItem);
		if (hChild)
			BrotherAndSubTreeToLocal(hChild, nIndex);

        hItem = TreeView_GetNextSibling(m_hWnd, hItem);
    }
}

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

void CLangListCtrlHelper::Init(HWND hWnd)
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
		LvCol.pszText = (LPTSTR)_TL(m_vStrHeader[i].c_str());

		if (!ListView_SetColumn(m_hWnd, i, &LvCol))
			return;
		
		m_vStrHeader.push_back(LvCol.pszText);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// BEGIN OF CLangDialogHelper
CLangDialogHelper::CLangDialogHelper()
{
	m_hWndParent = NULL;
}

CLangDialogHelper::~CLangDialogHelper()
{
}

void CLangDialogHelper::AddDlgItem(int nID)
{
	assert(m_hWndParent);

	HWND		hWndItem;

	hWndItem = GetDlgItem(m_hWndParent, nID);
	assert(hWndItem);

	AddDlgItem(hWndItem);
}

void CLangDialogHelper::AddDlgItem(HWND hWndItem)
{
	assert(m_hWndParent);
	assert(hWndItem);

	TCHAR		szWndText[256];
	DlgItem		Item;
	
	Item.bLoacalText = TRUE;

	if (GetClassName(hWndItem, szWndText, CountOf(szWndText)))
	{
		if (_tcscmp(szWndText, _T("SysListView32")) == 0)
		{
			CLangListCtrlHelper	ListHelper;

			ListHelper.Init(hWndItem);

			m_vListCtrlHelper.push_back(ListHelper);
			Item.bLoacalText = FALSE;
		}
	}

	if (GetWindowText(hWndItem, szWndText, CountOf(szWndText)) == 0)
	{
		Item.bLoacalText = FALSE;
	}

	Item.hWnd = hWndItem;
	Item.strInit = szWndText;

	m_vDlgItem.push_back(Item);
}

void CLangDialogHelper::ToLocal()
{
	assert(m_hWndParent);

	// 设置窗口的字体
	SendMessage(m_hWndParent, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), TRUE);


	if (!m_strParent.empty())
		SetWindowText(m_hWndParent, _TL(m_strParent.c_str()));

	vector<DlgItem>::iterator		it;

	for (it = m_vDlgItem.begin(); it != m_vDlgItem.end(); it++)
	{
		// 转换文字
		if ((*it).bLoacalText)
			SetWindowText((*it).hWnd, _TL((*it).strInit.c_str()));

		// 设置字体
		SendMessage((*it).hWnd, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), TRUE);
	}

	// 本地化语言ListView控件
	vector<CLangListCtrlHelper>::iterator		itList;
	for (itList = m_vListCtrlHelper.begin(); itList != m_vListCtrlHelper.end(); itList++)
	{
		(*itList).ToLocal();
	}
}

void CLangDialogHelper::Init(HWND hWnd)
{
	m_hWndParent = hWnd;

	TCHAR		szWndText[256];

	GetWindowText(m_hWndParent, szWndText, CountOf(szWndText));
	m_strParent = szWndText;

	m_vDlgItem.erase(m_vDlgItem.begin(), m_vDlgItem.end());
	m_vListCtrlHelper.erase(m_vListCtrlHelper.begin(), m_vListCtrlHelper.end());
}

void CLangDialogHelper::InitAddAll(HWND hWndParent)
{
	Init(hWndParent);

	HWND		hWndItem;

	// hWndItem = FindWindowEx(m_hWndParent, NULL, NULL, NULL);
	hWndItem = GetWindow(m_hWndParent, GW_CHILD);

	while (hWndItem)
	{
		AddDlgItem(hWndItem);
		// hWndItem = FindWindowEx(m_hWndParent, hWndItem, NULL, NULL);
		hWndItem = GetWindow(hWndItem, GW_HWNDNEXT);
	}
}

void CLangDialogHelper::RemoveDlgItem(int nID)
{
	assert(m_hWndParent);

	HWND		hWndItem;

	hWndItem = GetDlgItem(m_hWndParent, nID);
	assert(hWndItem);

	RemoveDlgItem(hWndItem);
}

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

void CLangDialogHelper::Destroy()
{
	m_hWndParent = NULL;
	m_vDlgItem.erase(m_vDlgItem.begin(), m_vDlgItem.end());
	m_vListCtrlHelper.erase(m_vListCtrlHelper.begin(), m_vListCtrlHelper.end());
}
