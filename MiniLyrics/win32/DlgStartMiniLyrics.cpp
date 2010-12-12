#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgStartMiniLyrics.h"
#include "MLPlayerMgr.h"

//////////////////////////////////////////////////////////////////////

CDlgStartMiniLyrics::CDlgStartMiniLyrics() : CBaseDialog(IDD_START_MINILYRICS)
{
	m_hImageList = NULL;
}

CDlgStartMiniLyrics::~CDlgStartMiniLyrics()
{
	ImageList_Destroy(m_hImageList);
}

BOOL CDlgStartMiniLyrics::OnInitDialog()
{
	m_listPlayers.Attach(this, IDC_LIST_PLAYERS);
	m_listPlayers.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_wndHelper.Init(m_hWnd);

	m_wndHelper.AddAlignRightBottom(IDC_LIST_PLAYERS, false, false);
	m_wndHelper.AddAlignBottom(IDOK, true);
	m_wndHelper.AddAlignBottom(IDCANCEL, true);
	m_wndHelper.AddAlignCenter(IDOK, false);
	m_wndHelper.AddAlignCenter(IDCANCEL, false);

	LPCTSTR		szHead[] = {_T("Players"), _T("Result"), _T("Path"), NULL};
	int			arrWidth[] = {190, 400, 300};
	for (int i = 0; szHead[i] != NULL; i++)
		m_listPlayers.InsertColumn(i, szHead[i], LVCFMT_LEFT, arrWidth[i]);

	m_hImageList = ImageList_LoadImage(GetAppInstance(), 
		MAKEINTRESOURCE(IDB_INSTALL_STAT), 16, 1, RGB(0, 0, 0), IMAGE_BITMAP, LR_DEFAULTCOLOR);

	m_listPlayers.SetImageList(m_hImageList);

	if (!CBaseDialog::OnInitDialog())
		return FALSE;

	g_playerMgr.QueryRunningPlayerInfo();

	for (CMLPlayerMgr::ListPlayerInfo::iterator it = g_playerMgr.m_listPlayers.begin();
		it != g_playerMgr.m_listPlayers.end(); ++it)
	{
		PlayerInfo &info = *it;
		m_listPlayers.InsertItem(m_listPlayers.GetItemCount(), info.playerName, 0);
	}

	m_listPlayers.SetItemSelectionState(0, true);

	return TRUE;
}

void CDlgStartMiniLyrics::OnSize(int cx, int cy)
{
	CBaseDialog::OnSize(cx, cy);

	m_wndHelper.OnResize();
}

void CDlgStartMiniLyrics::OnOK()
{
	int nSel = m_listPlayers.GetNextSelectedItem();
	if (nSel == -1)
		return;

	g_playerMgr.SetCurPlayer(nSel);

	CBaseDialog::OnOK();
}
