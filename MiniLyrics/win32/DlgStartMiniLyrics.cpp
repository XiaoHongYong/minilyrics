#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgStartMiniLyrics.h"
#include "MLPlayerMgr.h"
#include "ProcessEnum.h"

TOLOCAL_SECTION("MiniLyrics setup")

struct PlayerCtrl
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

PlayerCtrl	__vPlayerCtrl[] = {
	{ IDC_PLAYER1, IDC_PLAYER_INFO1, IDC_SL1 },
	{ IDC_PLAYER2, IDC_PLAYER_INFO2, IDC_SL2 },
	{ IDC_PLAYER3, IDC_PLAYER_INFO3, IDC_SL3 },
	{ IDC_PLAYER4, IDC_PLAYER_INFO4, IDC_SL4 },
	{ IDC_PLAYER5, IDC_PLAYER_INFO5, IDC_SL5 },
	{ IDC_PLAYER6, IDC_PLAYER_INFO6, IDC_SL6 },
	{ IDC_PLAYER7, IDC_PLAYER_INFO7, IDC_SL7 },
	{ IDC_PLAYER8, IDC_PLAYER_INFO8, IDC_SL8 },
};

#define COUNT_PER_PAGE CountOf(__vPlayerCtrl)

int BtnId2Index(int nId)
{
	for (int i = 0; i < CountOf(__vPlayerCtrl); i++)
	{
		if (nId == __vPlayerCtrl[i].nStartBt)
			return i;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////

CDlgStartMiniLyrics::CDlgStartMiniLyrics() : CBaseDialog(IDD_START_MINILYRICS)
{
	m_bShowInstalledPlayers = false;
	m_bShowAllSupportedPlayers = false;
	m_nCurPage = 0;
}

CDlgStartMiniLyrics::~CDlgStartMiniLyrics()
{
}

BOOL CDlgStartMiniLyrics::OnInitDialog()
{
	m_wndHelper.Init(m_hWnd);

	m_wndHelper.AddAlignRightBottom(IDC_LIST_PLAYERS, false, false);
	m_wndHelper.AddAlignRightBottom(IDC_PREV, true, true);
	m_wndHelper.AddAlignRightBottom(IDC_NEXT, true, true);

	if (!CBaseDialog::OnInitDialog())
		return FALSE;

	// Show running players
	CProcessEnum	processEnum;
	g_playerMgr.QueryRunningPlayerInfo();
	CMLPlayerMgr::ListPlayerInfo listPlayers = g_playerMgr.GetRunningPlayers();
	for (CMLPlayerMgr::ListPlayerInfo::iterator it = listPlayers.begin(); it != listPlayers.end(); ++it)
	{
		PlayerInfo &info = *it;

		DWORD dwProcessId = 0;
		GetWindowThreadProcessId(info.hWndPlayer, &dwProcessId);
		tstring strFile = processEnum.GetProcessFileName(dwProcessId);
		m_vPlayerItems.push_back(PlayerItem(info.playerName, info.hWndPluginService, strFile.c_str()));
	}

	if (m_vPlayerItems.empty())
		ShowInstalledPlayers();

	UpdatePlayersToView();

	return TRUE;
}

void CDlgStartMiniLyrics::OnSize(int cx, int cy)
{
	CBaseDialog::OnSize(cx, cy);

	m_wndHelper.OnResize();
}

void CDlgStartMiniLyrics::OnCommand(UINT uID, UINT nNotifyCode)
{
	if (uID == IDC_PREV)
	{
		if (m_nCurPage > 0)
		{
			m_nCurPage--;
			UpdatePlayersToView();
		}
	}
	else if (uID == IDC_NEXT)
	{
		if (m_nCurPage < GetMaxPagesCount())
		{
			m_nCurPage++;
			UpdatePlayersToView();
		}
	}
	else
	{
		int n = BtnId2Index(uID);
		if (n == -1)
		{
			CBaseDialog::OnCommand(uID, nNotifyCode);
			return;
		}

		n += m_nCurPage * COUNT_PER_PAGE;
		ASSERT((uint)n <= m_vPlayerItems.size());
		if (n == m_vPlayerItems.size())
		{
			// Clicked "More..." button
			if (!m_bShowInstalledPlayers)
				ShowInstalledPlayers();
			else if (!m_bShowAllSupportedPlayers)
				ShowAllSupportedPlayers();
			return;
		}

		PlayerItem &item = m_vPlayerItems[n];
		if (item.hWndPluginService != NULL)
		{
			g_playerMgr.ConnectToPlayer(item.hWndPluginService);
			OnOK();
		}
		else
		{
			CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(item.nIndexOfPlayerInstaller);
			if (!p)
				return;

			OnClickPlayer(p);
		}
	}
}

void CDlgStartMiniLyrics::OnClickPlayer(CPlayerPluginInstaller::Player *p)
{
	ASSERT(p);

	m_installer.RefreshRunningPlayers();

	// Is player installed?
	if (!p->IsInstalled())
	{
		tstring strMsg = TOLOCAL_T("MiniLyrics can't detect the player's path.");
		strMsg += _T("\n");
		strMsg += TOLOCAL_T("Do you want to specify the player's installation path manually?");
		if (MessageOut(strMsg.c_str(), MB_ICONINFORMATION | MB_OKCANCEL) != IDOK)
			return;

		CFolderDialog	dlg;

		dlg.SetInitFolder(GetProgramFilesPath().c_str());
		if (!dlg.DoBrowse(this))
			return;

		p->pExtraInfo->strPlayerPath = dlg.GetFolder();
		if (!p->CheckPlayerInstalled(false))
		{
			MessageOut(CStrPrintf(TOLOCAL_T("Sorry, the folder you browsed do NOT have file: %s"), p->szExeName).c_str());
			return;
		}
	}

	// Is plugin installed?
	tstring strError;
	if (!p->CheckPluginInstalled(strError) || !p->IsPluginInstalled())
	{
		tstring strMsg = strError;
		strMsg += _T("\n\n");
		strMsg += TOLOCAL_T("MiniLyrics plugin for this player was NOT installed properly.");
		strMsg += _T("\n");
		strMsg += TOLOCAL_T("Do you want to install the plugin for this player again?");
		if (MessageOut(strMsg.c_str(), MB_ICONINFORMATION | MB_OKCANCEL) == IDOK)
		{
			if (p->IsPlayerRunning())
			{
				strMsg = TOLOCAL_T("The player is running.");
				strMsg += _T("\n");
				strMsg += p->GetPlayerExeFile();
				strMsg += _T("\n\n");
				strMsg += TOLOCAL_T("Please close it, then click 'OK' to continue.");
				MessageOut(strMsg.c_str(), MB_ICONINFORMATION | MB_OK);
			}

			tstring strErr;
			if (!p->Install(strErr))
			{
				MessageOut(strErr.c_str(), MB_ICONINFORMATION);
				return;
			}
		}
	}

	// Is Player running?
	if (!p->IsPlayerRunning())
	{
		// Run this player
		tstring strPlayerExeFile = p->GetPlayerExeFile();
		if (strPlayerExeFile.size())
		{
			ExecuteCmdAndWait(strPlayerExeFile.c_str(), 0, NULL);
		}
		else
			MessageOut(_T("MiniLyrics can't get the player program, please start it manually."));
		return;
	}

	// Check whether Plugin Service is running?
	m_installer.RefreshRunningPluginService();
	if (!p->IsPluginServiceRunning())
	{
		// Restart player or install it again.
	}

	// Player may just start running again, so query running player info again
	g_playerMgr.QueryRunningPlayerInfo();
	if (g_playerMgr.ConnectToPlayer(p->pExtraInfo->hWndPluginService))
	{
		// Connected to the player. So, just close this dialog
		OnOK();
	}
	else
	{
		tstring strMsg = _T("MiniLyrics plugin for this player may NOT being installed properly.");
		strMsg += _T("\n");
		strMsg += TOLOCAL_T("Do you want to install the plugin for this player again?");
		if (MessageOut(strMsg.c_str(), MB_ICONINFORMATION | MB_OKCANCEL) == IDOK)
		{
			tstring strErr;
			if (!p->Install(strErr))
			{
				tstring strMsg = strErr;
				strMsg += _T("\n");
				strMsg += TOLOCAL_T("Failed to install the plugin for this player.");
				strMsg += _T("\n");
				strMsg += TOLOCAL_T("Please close the player, and click 'Install' again.");
				MessageOut(strMsg.c_str(), MB_ICONINFORMATION);
			}
		}
	}
}

void CDlgStartMiniLyrics::ShowInstalledPlayers()
{
	// Show installed players
	m_installer.Init();
	ASSERT(!m_bShowInstalledPlayers);
	m_bShowInstalledPlayers = true;
	for (int i = 0; ; i++)
	{
		CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(i);
		if (!p)
			break;

		if (p->IsInstalled())
			m_vPlayerItems.push_back(PlayerItem(p->szPlayerName, i));
	}

	if (m_vPlayerItems.empty())
		ShowAllSupportedPlayers();

	UpdatePlayersToView();
}

void CDlgStartMiniLyrics::ShowAllSupportedPlayers()
{
	// Show only NOT installed players
	m_bShowAllSupportedPlayers = true;
	for (int i = 0; ; i++)
	{
		CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(i);
		if (!p)
			break;

		if (!p->IsInstalled())
			m_vPlayerItems.push_back(PlayerItem(p->szPlayerName, i));
	}

	UpdatePlayersToView();
}

void CDlgStartMiniLyrics::UpdatePlayersToView()
{
	EnableDlgItem(IDC_PREV, m_nCurPage != 0);
	EnableDlgItem(IDC_NEXT, m_nCurPage < GetMaxPagesCount() - 1);

	uint nEndItem = (m_nCurPage + 1) * COUNT_PER_PAGE;
	if (nEndItem > m_vPlayerItems.size())
		nEndItem = m_vPlayerItems.size();

	for (uint i = m_nCurPage * COUNT_PER_PAGE; i < nEndItem; i++)
	{
		PlayerCtrl &ctrl = __vPlayerCtrl[i - m_nCurPage * COUNT_PER_PAGE];
		PlayerItem &item = m_vPlayerItems[i];

		SetDlgItemText(ctrl.nStartBt, item.strPlayerName.c_str());
		SetDlgItemText(ctrl.nInfoId, item.strPlayerExe.c_str());

		if (item.hWndPluginService == NULL)
		{
			CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(item.nIndexOfPlayerInstaller);
			if (p)
				SetDlgItemText(ctrl.nInfoId, p->GetPlayerExeFile().c_str());
		}
		ctrl.SetVisible(this, true);
	}
	if (nEndItem % COUNT_PER_PAGE == 0 && nEndItem < m_vPlayerItems.size())
		return;

	int nEndCtrlIndex = nEndItem % COUNT_PER_PAGE;
	if ((!m_bShowAllSupportedPlayers || !m_bShowInstalledPlayers)
		&& nEndCtrlIndex < COUNT_PER_PAGE)
	{
		// Show "More ..." button
		PlayerCtrl &ctrl = __vPlayerCtrl[nEndCtrlIndex];
		SetDlgItemText(ctrl.nStartBt, TOLOCAL_T("&More ..."));
		if (!m_bShowInstalledPlayers)
			SetDlgItemText(ctrl.nInfoId, TOLOCAL_T("Show more players installed on your PC"));
		else if (!m_bShowAllSupportedPlayers)
			SetDlgItemText(ctrl.nInfoId, TOLOCAL_T("Show more players that MiniLyrics supports"));
		nEndCtrlIndex++;
		ctrl.SetVisible(this, true);
	}

	// Set other items invisible.
	for (int i = nEndCtrlIndex; i < COUNT_PER_PAGE; i++)
	{
		PlayerCtrl &ctrl = __vPlayerCtrl[i];
		ctrl.SetVisible(this, false);
	}
}

int CDlgStartMiniLyrics::GetMaxPagesCount()
{
	int nMaxItems = m_vPlayerItems.size();
	if (!m_bShowAllSupportedPlayers || !m_bShowInstalledPlayers)
		nMaxItems++;

	return (nMaxItems + COUNT_PER_PAGE - 1) / COUNT_PER_PAGE;
}
