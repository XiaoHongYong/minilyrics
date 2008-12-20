// DlgTree.cpp : implementation file
//

#include "stdafx.h"
#include "debugtracer.h"
#include "DlgTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgTree dialog


CDlgTree::CDlgTree(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTree::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgTree)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgTree::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgTree)
	DDX_Control(pDX, IDC_TREE, m_tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgTree, CDialog)
	//{{AFX_MSG_MAP(CDlgTree)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgTree message handlers

HTREEITEM FindTreeItemByData(CTreeCtrl &tree, DWORD dwData, HTREEITEM hBegItem = TVI_ROOT)
{
	TVITEM item;
	item.hItem = hBegItem;
	item.mask = TVIF_PARAM;
	if (::SendMessage(tree.m_hWnd, TVM_GETITEM, 0, (LPARAM)&item) && (DWORD)item.lParam == dwData)
	{
		return hBegItem;
	}

	hBegItem = tree.GetChildItem(hBegItem);

	while (hBegItem)
	{
		HTREEITEM		hTemp;
		hTemp = FindTreeItemByData(tree, dwData, hBegItem);
		if (hTemp)
			return hTemp;

		hBegItem = tree.GetNextSiblingItem(hBegItem);
	}
	return NULL;
}

void CDlgTree::AddTree(DWORD dwTreeId, DWORD dwParentId, DWORD CurId, LPCTSTR szName)
{
	if (dwTreeId != m_dwTreeId)
	{
		m_tree.DeleteAllItems();
		m_dwTreeId = dwTreeId;
	}

	HTREEITEM	hParent, hCur;

	hParent = FindTreeItemByData(m_tree, dwParentId, m_tree.GetRootItem());
	if (hParent == NULL)
	{
		// Create new?
		m_tree.DeleteAllItems();
		hCur = m_tree.InsertItem(szName);
	}
	else
	{
		hCur = m_tree.InsertItem(szName, hParent);
	}
	m_tree.SetItemData(hCur, CurId);
}
