#include "stdafx.h"
#include "MPlayerApp.h"
#include "../widget/widget.h"
#include "DlgSetupPlayerPlugins.h"
#include "PreferenceDlg.h"

//////////////////////////////////////////////////////////////////////

CDlgSetupPlayerPlugins::CDlgSetupPlayerPlugins() : CBaseDialog(IDD_SETUP)
{
	m_hImageList = NULL;
}

CDlgSetupPlayerPlugins::~CDlgSetupPlayerPlugins()
{
	ImageList_Destroy(m_hImageList);
}

BOOL CDlgSetupPlayerPlugins::OnInitDialog()
{
	m_listPlayers.Attach(this, IDC_LIST_PLAYERS);
	m_listPlayers.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_wndHelper.Init(m_hWnd);

	m_wndHelper.AddAlignRightBottom(IDC_LIST_PLAYERS, false, false);
	m_wndHelper.AddAlignBottom(IDOK, true);
	m_wndHelper.AddAlignBottom(IDCANCEL, true);
	m_wndHelper.AddAlignCenter(IDOK, false);
	m_wndHelper.AddAlignCenter(IDCANCEL, false);

	LPCTSTR		szHead[] = {_T("Players"), _T("Install"), _T("Path"), _T("Result"), NULL};
	int			arrWidth[] = {190, 60, 300, 700};
	for (int i = 0; szHead[i] != NULL; i++)
		m_listPlayers.InsertColumn(i, szHead[i], LVCFMT_LEFT, arrWidth[i]);
	// m_listPlayers.LoadColumnWidth(_T("CDlgSetupPlayerPlugins_List"));

	m_hImageList = ImageList_LoadImage(GetAppInstance(), 
		MAKEINTRESOURCE(IDB_INSTALL_STAT), 16, 1, RGB(0, 0, 0), IMAGE_BITMAP, LR_DEFAULTCOLOR);

	m_listPlayers.SetImageList(m_hImageList);

	m_listPlayers.Init(&m_pluginInstaller);

	if (!CBaseDialog::OnInitDialog())
		return FALSE;

	// List All Players
	m_pluginInstaller.Init();

	m_pluginInstaller.RefreshPluginInstallationStatus();

	UpdatePlayers();

	return TRUE;
}

void CDlgSetupPlayerPlugins::UpdatePlayers()
{
	m_listPlayers.DeleteAllItems();

	CPlayerPluginInstaller::Player *players = m_pluginInstaller.GetPlayers();

	int			nInstalledPos = -1;
	for (int i = 0; players[i].szPlayerName != NULL; i++)
	{
		CPlayerPluginInstaller::Player &item = players[i];
		int nInsertPos;
		if (item.IsInstalled())
			nInsertPos = nInstalledPos + 1;
		else
			nInsertPos = m_listPlayers.GetItemCount();
		nInsertPos = m_listPlayers.InsertItem(nInsertPos, item.szPlayerName, item.GetStatusImage());
		m_listPlayers.SetItemText(nInsertPos, CPlayerPluginListCtrl::COL_PLAYER_PATH, item.pExtraInfo->strPlayerPath.c_str());
		m_listPlayers.SetItemText(nInsertPos, CPlayerPluginListCtrl::COL_INFORMATION, item.pExtraInfo->strMsg.c_str());
		m_listPlayers.SetItemData(nInsertPos, i);
		if (item.IsInstalled())
		{
			nInstalledPos = nInsertPos;
			item.pExtraInfo->status |= CPlayerPluginInstaller::S_PG_TO_INSTALL;
			m_listPlayers.SetCheck(nInsertPos, true);
		}
	}

	m_listPlayers.SetItemSelectionState(0);
}

void CDlgSetupPlayerPlugins::RefreshStatus()
{
	CPlayerPluginInstaller::Player *players = m_pluginInstaller.GetPlayers();

	for (int i = 0; i < m_listPlayers.GetItemCount(); i++)
	{
		CPlayerPluginInstaller::Player *player = m_pluginInstaller.GetPlayerByIndex(
			m_listPlayers.GetItemData(i));

		if (player && player->IsInstalled())
		{
			m_listPlayers.SetItemImage(i, player->GetStatusImage());
			m_listPlayers.SetItemText(i, CPlayerPluginListCtrl::COL_INFORMATION, player->pExtraInfo->strMsg.c_str());
		}
	}
}

void CDlgSetupPlayerPlugins::OnDestroy()
{
	// m_listPlayers.SaveColumnWidth(_T("CDlgSetupPlayerPlugins_List"));

	CBaseDialog::OnDestroy();
}

void CDlgSetupPlayerPlugins::OnSize(int cx, int cy)
{
	CBaseDialog::OnSize(cx, cy);

	m_wndHelper.OnResize();
}

void CDlgSetupPlayerPlugins::OnOK()
{
	tstring	strErr;
	if (!m_pluginInstaller.Install(strErr))
	{
		MessageOut(strErr.c_str());
		RefreshStatus();
		return;
	}

	CBaseDialog::OnOK();
}

void CDlgSetupPlayerPlugins::OnCommand(UINT uID, UINT nNotifyCode)
{
/*	switch (uID)
	{
	case IDC_INSTALL_ALL:
		{
		}
		break;
	default:
		CBaseDialog::OnCommand(uID, nNotifyCode);
	}*/
	CBaseDialog::OnCommand(uID, nNotifyCode);
}
