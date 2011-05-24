#include "stdafx.h"
#include "MPlayerApp.h"
#include "../widget/widget.h"
#include "DlgSetupPlayerPlugins.h"
#include "PreferenceDlg.h"

struct SetupPlayerCtrl
{
	int		nStartBt, nInfoId, nSeperatorLineId;

	void SetVisible(CBaseWnd *pWnd, bool bVisible)
	{
		int nShow = bVisible ? SW_SHOW : SW_HIDE;
		pWnd->ShowDlgItem(nStartBt, nShow);
		pWnd->ShowDlgItem(nInfoId, nShow);
		pWnd->ShowDlgItem(nSeperatorLineId, nShow);
	}
};

SetupPlayerCtrl	__vSetupPlayerCtrl[] = {
	{ IDC_PLAYER1, IDC_PLAYER_INFO1, IDC_SL1 },
	{ IDC_PLAYER2, IDC_PLAYER_INFO2, IDC_SL2 },
	{ IDC_PLAYER3, IDC_PLAYER_INFO3, IDC_SL3 },
	{ IDC_PLAYER4, IDC_PLAYER_INFO4, IDC_SL4 },
	{ IDC_PLAYER5, IDC_PLAYER_INFO5, IDC_SL5 },
	{ IDC_PLAYER6, IDC_PLAYER_INFO6, IDC_SL6 },
	{ IDC_PLAYER7, IDC_PLAYER_INFO7, IDC_SL7 },
};

#define COUNT_PER_PAGE CountOf(__vSetupPlayerCtrl)

//////////////////////////////////////////////////////////////////////

CDlgSetupPlayerPlugins::CDlgSetupPlayerPlugins() : CBaseDialog(IDD_SETUP)
{
	m_bShowAllSupportedPlayers = false;
}

CDlgSetupPlayerPlugins::~CDlgSetupPlayerPlugins()
{
}

BOOL CDlgSetupPlayerPlugins::OnInitDialog()
{
	m_wndHelper.Init(m_hWnd);

	if (!CBaseDialog::OnInitDialog())
		return FALSE;

	m_installer.Init();

	// Show only installed players.
	for (int i = 0; ; i++)
	{
		CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(i);
		if (!p)
			break;

		if (p->IsInstalled())
			m_vPlayerItems.push_back(i);
	}

	if (m_vPlayerItems.empty())
		ShowAllSupportedPlayers();

	UpdatePlayersToView();

	return TRUE;
}

void CDlgSetupPlayerPlugins::OnSize(int cx, int cy)
{
	CBaseDialog::OnSize(cx, cy);

	m_wndHelper.OnResize();
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

		
			// Browse media player folder
			CFolderDialog	dlg;
			tstring			strPath;

			GetItemText(nSel, COL_PLAYER_PATH, strPath);

			dlg.SetInitFolder(strPath.c_str());
			if (!dlg.DoBrowse(this))
				return;

			player->pExtraInfo->strPlayerExeFile = dlg.GetFolder();
			if (player->CheckPlayerInstalled(false))
			{
				player->RefreshPluginInstallationStatus(player->pExtraInfo->strMsg);
				player->pExtraInfo->status |= CPlayerPluginInstaller::S_PG_TO_INSTALL;
			}
			else
			{
				player->pExtraInfo->status &= ~(CPlayerPluginInstaller::S_PG_TO_INSTALL | CPlayerPluginInstaller::S_PG_INSTALLED);
				MessageOut(_T("Can't detect the player installed in the specified folder."), MB_OK | MB_ICONINFORMATION, SZ_APP_NAME);
			}

			m_btnPlayerPath.SetWindowText(player->pExtraInfo->strPlayerExeFile.c_str());
			SetItemText(nSel, COL_PLAYER_PATH, player->pExtraInfo->strPlayerExeFile.c_str());
			SetItemImage(nSel, player->GetStatusImage());
			m_listCkInstall[nSel]->SetCheck(player->IsToInstallPlugin());

	}*/
	CBaseDialog::OnCommand(uID, nNotifyCode);
}

void CDlgSetupPlayerPlugins::ShowAllSupportedPlayers()
{
	// Show only NOT installed players
	m_bShowAllSupportedPlayers = true;
	for (int i = 0; ; i++)
	{
		CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(i);
		if (!p)
			break;

		if (!p->IsInstalled())
			m_vPlayerItems.push_back(i);
	}

	UpdatePlayersToView();
}

void CDlgSetupPlayerPlugins::UpdatePlayersToView()
{

}
