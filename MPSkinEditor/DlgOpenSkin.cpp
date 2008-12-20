// DlgOpenSkin.cpp : implementation file
//

#include "stdafx.h"
#include "mpskineditor.h"
#include "DlgOpenSkin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgOpenSkin dialog


CDlgOpenSkin::CDlgOpenSkin(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgOpenSkin::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgOpenSkin)
	m_strSkinsRootDir = g_Profile.GetProfileString(_T("SkinsRootDir"), _T(""));
	m_strInfo = _T("");
	//}}AFX_DATA_INIT
}


void CDlgOpenSkin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgOpenSkin)
	DDX_Control(pDX, IDC_L_SKINS, m_lbSkins);
	DDX_Text(pDX, IDC_E_SKIN_ROOT_DIR, m_strSkinsRootDir);
	DDX_Text(pDX, IDC_E_INFO, m_strInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgOpenSkin, CDialog)
	//{{AFX_MSG_MAP(CDlgOpenSkin)
	ON_BN_CLICKED(IDC_BR_SKIN_ROOT_DIR, OnBrSkinRootDir)
	ON_LBN_DBLCLK(IDC_L_SKINS, OnDblclkLSkins)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgOpenSkin message handlers

BOOL CDlgOpenSkin::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (m_strSkinsRootDir.IsEmpty())
	{
		m_strSkinsRootDir = GetWorkingFolder();
		m_strSkinsRootDir += _T("skins\\");
	}
	g_skinFactory.SetSkinsRootDir(m_strSkinsRootDir);

	vector<tstring>		vSkins;

	g_skinFactory.EnumAllSkins(vSkins);

	if (vSkins.empty())
	{
		g_skinFactory.SetSkinFileName(_T("minilyrics.xml"));
		g_skinFactory.EnumAllSkins(vSkins);
		if (vSkins.empty())
			g_skinFactory.SetSkinFileName(_T("ZikiPlayer.xml"));
	}

	for (int i = 0; i < vSkins.size(); i++)
	{
		m_lbSkins.AddString(vSkins[i].c_str());
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgOpenSkin::OnOK() 
{
	if (m_lbSkins.GetCount() == 0)
		return;

	int	nSel = m_lbSkins.GetCurSel();
	if (nSel == -1)
		return;

	CString		str;
	m_lbSkins.GetText(nSel, str);
	m_strSelSkin = (LPCTSTR)str;

	CDialog::OnOK();
}

void CDlgOpenSkin::OnBrSkinRootDir() 
{
	TCHAR		szFolder[MAX_PATH];

	_tcscpy_safe(szFolder, CountOf(szFolder), g_Profile.GetProfileString(_T("SkinsRootDir"), _T("")));
	if (!BrowserForFolder(m_hWnd, _T("Browse skins root dir:"), szFolder))
		return;

	m_strSkinsRootDir = szFolder;
	SetDlgItemText(IDC_E_SKIN_ROOT_DIR, m_strSkinsRootDir);
	g_Profile.WriteProfileString(_T("SkinsRootDir"), m_strSkinsRootDir);

	g_skinFactory.SetSkinsRootDir(m_strSkinsRootDir);

	vector<tstring>		vSkins;
	g_skinFactory.EnumAllSkins(vSkins);

	for (int i = 0; i < vSkins.size(); i++)
	{
		m_lbSkins.AddString(vSkins[i].c_str());
	}
}

void CDlgOpenSkin::OnDblclkLSkins() 
{
	OnOK();
}
