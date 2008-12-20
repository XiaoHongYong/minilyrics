// DlgChooseIDs.cpp : implementation file
//

#include "stdafx.h"
#include "mpskineditor.h"
#include "DlgChooseIDs.h"
#include "../MPShared/MLCmd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgChooseIDs dialog


CDlgChooseIDs::CDlgChooseIDs(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgChooseIDs::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgChooseIDs)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgChooseIDs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgChooseIDs)
	DDX_Control(pDX, IDC_LIST, m_listIDs);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgChooseIDs, CDialog)
	//{{AFX_MSG_MAP(CDlgChooseIDs)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgChooseIDs message handlers

void CDlgChooseIDs::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnOK();
	
	*pResult = 0;
}

void CDlgChooseIDs::OnOK() 
{
	TCHAR		szText[256] = _T("");

	POSITION	pos = m_listIDs.GetFirstSelectedItemPosition();
	if (pos)
	{
		int n = m_listIDs.GetNextSelectedItem(pos);
		m_listIDs.GetItemText(n, 0, szText, CountOf(szText));
		m_strID = szText;
	}
	else
		return;
	
	CDialog::OnOK();
}

BOOL CDlgChooseIDs::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_listIDs.InsertColumn(0, _T("ID"), LVCFMT_LEFT, 150, 250);
	m_listIDs.InsertColumn(0, _T("Tool tip"), LVCFMT_LEFT, 150, 250);
	m_listIDs.SetExtendedStyle(m_listIDs.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	for (int i =0; g_StringIdMap[i].szId != NULL; i++)
	{
		int		n = m_listIDs.InsertItem(m_listIDs.GetItemCount(), g_StringIdMap[i].szId, 0);
		if (g_StringIdMap[i].szToolTip)
			m_listIDs.SetItemText(n, 1, g_StringIdMap[i].szToolTip);
		if (_tcsicmp(g_StringIdMap[i].szId, m_strID.c_str()) == 0)
			m_listIDs.SetItemState(n, LVIS_SELECTED, LVIS_SELECTED);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
