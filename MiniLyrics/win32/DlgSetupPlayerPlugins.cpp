#include "stdafx.h"
#include "MPlayerApp.h"
#include "../widget/widget.h"
#include "DlgSetupPlayerPlugins.h"
#include "PreferenceDlg.h"

enum SetupCmdType
{
	SCT_INSTALL,
	SCT_BR_PATH,
	SCT_PATH,
};

struct SetupPlayerCtrl
{
	int		nInstallBt, nPlayerNameId, nPathId, nBrPathId, nSeperatorLineId;

	void SetVisible(CBaseWnd *pWnd, bool bVisible)
	{
		int nShow = bVisible ? SW_SHOW : SW_HIDE;
		pWnd->ShowDlgItem(nInstallBt, nShow);
		pWnd->ShowDlgItem(nPlayerNameId, nShow);
		pWnd->ShowDlgItem(nPathId, nShow);
		pWnd->ShowDlgItem(nBrPathId, nShow);
		pWnd->ShowDlgItem(nSeperatorLineId, nShow);
	}
};

SetupPlayerCtrl	__vSetupPlayerCtrl[] = {
	{ IDC_INSTALL1, IDC_PLAYER1, IDC_E_PLAYER1, IDC_BR_PLAYER1, IDC_SL1 },
	{ IDC_INSTALL2, IDC_PLAYER2, IDC_E_PLAYER2, IDC_BR_PLAYER2, IDC_SL2 },
	{ IDC_INSTALL3, IDC_PLAYER3, IDC_E_PLAYER3, IDC_BR_PLAYER3, IDC_SL3 },
	{ IDC_INSTALL4, IDC_PLAYER4, IDC_E_PLAYER4, IDC_BR_PLAYER4, IDC_SL4 },
	{ IDC_INSTALL5, IDC_PLAYER5, IDC_E_PLAYER5, IDC_BR_PLAYER5, IDC_SL5 },
	{ IDC_INSTALL6, IDC_PLAYER6, IDC_E_PLAYER6, IDC_BR_PLAYER6, IDC_SL6 },
	{ IDC_INSTALL7, IDC_PLAYER7, IDC_E_PLAYER7, IDC_BR_PLAYER7, IDC_SL7 },
	{ IDC_INSTALL8, IDC_PLAYER8, IDC_E_PLAYER8, IDC_BR_PLAYER8, IDC_SL8 },
};

#define COUNT_PER_PAGE CountOf(__vSetupPlayerCtrl)

int CmdId2Index(int nId, SetupCmdType &type)
{
	for (int i = 0; i < CountOf(__vSetupPlayerCtrl); i++)
	{
		SetupPlayerCtrl &ctrl = __vSetupPlayerCtrl[i];
		if (nId == ctrl.nInstallBt)
		{
			type = SCT_INSTALL;
			return i;
		}
		else if (nId == ctrl.nPathId)
		{
			type = SCT_PATH;
			return i;
		}
		else if (nId == ctrl.nBrPathId)
		{
			type = SCT_BR_PATH;
			return i;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////

CDlgSetupPlayerPlugins::CDlgSetupPlayerPlugins() : CBaseDialog(IDD_SETUP)
{
	m_bShowAllSupportedPlayers = false;
	m_nCurPage = 0;
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
	switch (uID)
	{
	case IDC_PREV:
		if (m_nCurPage > 0)
		{
			m_nCurPage--;
			UpdatePlayersToView();
		}
		break;
	case IDC_NEXT:
		if (m_nCurPage < GetMaxPagesCount())
		{
			m_nCurPage++;
			UpdatePlayersToView();
		}
		break;
	default:
		{
			SetupCmdType type;
			int n = CmdId2Index(uID, type);
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
				if (!m_bShowAllSupportedPlayers)
					ShowAllSupportedPlayers();
				return;
			}

			CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(m_vPlayerItems[n]);
			ASSERT(p);
			if (!p)
				return;

			// Get player path
			GetDlgItemText(__vSetupPlayerCtrl[n % COUNT_PER_PAGE].nPathId, p->pExtraInfo->strPlayerPath);

			switch (type)
			{
			case SCT_INSTALL:
				OnClickedInstallBtn(p);
				break;
			case SCT_BR_PATH:
				OnClickedBrowsePathBtn(p);
				break;
			case SCT_PATH:
				OnPathEditChanged(p);
				break;
			default:
				ASSERT(0);
				break;
			}
		}
	}
}

void CDlgSetupPlayerPlugins::OnClickedInstallBtn(CPlayerPluginInstaller::Player *p)
{
	ASSERT(p);

	m_installer.RefreshRunningPlayers();

	// Is player installed?
	if (!p->IsInstalled())
	{
		tstring strMsg = TOLOCAL_T("MiniLyrics can't detect the player's path.");
		strMsg += _T("\n");
		strMsg += TOLOCAL_T("Do you want to specify the player's installation path manually?");
		if (MessageOut(strMsg.c_str(), MB_ICONINFORMATION | MB_OKCANCEL) == IDOK)
		{
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
}

void CDlgSetupPlayerPlugins::OnClickedBrowsePathBtn(CPlayerPluginInstaller::Player *p)
{
	// Browse media player folder
	CFolderDialog	dlg;

	dlg.SetInitFolder(p->pExtraInfo->strPlayerPath.c_str());
	if (!dlg.DoBrowse(this))
		return;

	p->pExtraInfo->strPlayerPath = dlg.GetFolder();
	if (p->CheckPlayerInstalled(false))
	{
		tstring str;
		if (!p->CheckPluginInstalled(str))
			p->SetToInstallPlugin(true);
	}
	else
		MessageOut(_T("Can't detect the player installed in the specified folder."), MB_OK | MB_ICONINFORMATION);
}

void CDlgSetupPlayerPlugins::OnPathEditChanged(CPlayerPluginInstaller::Player *p)
{

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
	EnableDlgItem(IDC_PREV, m_nCurPage != 0);
	EnableDlgItem(IDC_NEXT, m_nCurPage < GetMaxPagesCount() - 1);

	uint nEndItem = (m_nCurPage + 1) * COUNT_PER_PAGE;
	if (nEndItem > m_vPlayerItems.size())
		nEndItem = m_vPlayerItems.size();

	for (uint i = m_nCurPage * COUNT_PER_PAGE; i < nEndItem; i++)
	{
		SetupPlayerCtrl &ctrl = __vSetupPlayerCtrl[i - m_nCurPage * COUNT_PER_PAGE];
		CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(m_vPlayerItems[i]);
		ASSERT(p != NULL);

		SetDlgItemText(ctrl.nInstallBt, p->IsPluginInstalled() ? TOLOCAL_T("Uninstall") : TOLOCAL_T("Install"));
		SetDlgItemText(ctrl.nPlayerNameId, p->szPlayerName);
		SetDlgItemText(ctrl.nPathId, p->pExtraInfo->strPlayerPath.c_str());
		ctrl.SetVisible(this, true);
	}
	if (nEndItem % COUNT_PER_PAGE == 0 && nEndItem < m_vPlayerItems.size())
		return;

	int nEndCtrlIndex = nEndItem % COUNT_PER_PAGE;
	if (!m_bShowAllSupportedPlayers
		&& nEndCtrlIndex < COUNT_PER_PAGE)
	{
		// Show "More ..." button
		SetupPlayerCtrl &ctrl = __vSetupPlayerCtrl[nEndCtrlIndex];
		SetDlgItemText(ctrl.nInstallBt, TOLOCAL_T("&More ..."));
		SetDlgItemText(ctrl.nPlayerNameId, TOLOCAL_T("More players ..."));
		nEndCtrlIndex++;
		ctrl.SetVisible(this, true);
		ShowDlgItem(ctrl.nPathId, SW_HIDE);
		ShowDlgItem(ctrl.nBrPathId, SW_HIDE);
	}

	// Set other items invisible.
	for (int i = nEndCtrlIndex; i < COUNT_PER_PAGE; i++)
	{
		SetupPlayerCtrl &ctrl = __vSetupPlayerCtrl[i];
		ctrl.SetVisible(this, false);
	}
}

int CDlgSetupPlayerPlugins::GetMaxPagesCount()
{
	int nMaxItems = m_vPlayerItems.size();
	if (!m_bShowAllSupportedPlayers)
		nMaxItems++;

	return (nMaxItems + COUNT_PER_PAGE - 1) / COUNT_PER_PAGE;
}
