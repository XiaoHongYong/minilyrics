#include "stdafx.h"
#include "MPlayerApp.h"
#include "../widget/widget.h"
#include "DlgSetupPlayerPlugins.h"
#include "PreferenceDlg.h"
#include "UACTool.h"

TOLOCAL_SECTION("MiniLyrics setup")

#define INDEX_BUILT_IN_PLUGIN		-1

bool g_bFirstSetup = true;

enum SetupCmdType
{
	SCT_CHECK,
	SCT_INSTALL,
	SCT_BR_PATH,
	SCT_PATH,
};

struct SetupPlayerCtrl
{
	int		nCheckBt, nInstallBt, nPlayerNameId;
	int		nPathId, nBrPathId, nSeperatorLineId;

	void SetVisible(CBaseWnd *pWnd, bool bVisible)
	{
		int nShow = bVisible ? SW_SHOW : SW_HIDE;
		pWnd->ShowDlgItem(nCheckBt, SW_HIDE);
		pWnd->ShowDlgItem(nInstallBt, nShow);
		pWnd->ShowDlgItem(nPlayerNameId, nShow);
		pWnd->ShowDlgItem(nPathId, nShow);
		pWnd->ShowDlgItem(nBrPathId, nShow);
		pWnd->ShowDlgItem(nSeperatorLineId, nShow);
		if (g_bFirstSetup && bVisible)
		{
			pWnd->ShowDlgItem(nCheckBt, SW_SHOW);
			pWnd->ShowDlgItem(nInstallBt, SW_HIDE);
			pWnd->ShowDlgItem(nPlayerNameId, SW_HIDE);
		}
	}

	void SetVisibleForMoreBtn(CBaseWnd *pWnd)
	{
		pWnd->ShowDlgItem(nCheckBt, SW_HIDE);
		pWnd->ShowDlgItem(nInstallBt, SW_SHOW);
		pWnd->ShowDlgItem(nPlayerNameId, SW_SHOW);
		pWnd->ShowDlgItem(nPathId, SW_HIDE);
		pWnd->ShowDlgItem(nBrPathId, SW_HIDE);
		pWnd->ShowDlgItem(nSeperatorLineId, SW_SHOW);
	}
};

SetupPlayerCtrl	__vSetupPlayerCtrl[] = {
	{ IDC_C_PLAYER1, IDC_INSTALL1, IDC_PLAYER1, IDC_E_PLAYER1, IDC_BR_PLAYER1, IDC_SL1 },
	{ IDC_C_PLAYER2, IDC_INSTALL2, IDC_PLAYER2, IDC_E_PLAYER2, IDC_BR_PLAYER2, IDC_SL2 },
	{ IDC_C_PLAYER3, IDC_INSTALL3, IDC_PLAYER3, IDC_E_PLAYER3, IDC_BR_PLAYER3, IDC_SL3 },
	{ IDC_C_PLAYER4, IDC_INSTALL4, IDC_PLAYER4, IDC_E_PLAYER4, IDC_BR_PLAYER4, IDC_SL4 },
	{ IDC_C_PLAYER5, IDC_INSTALL5, IDC_PLAYER5, IDC_E_PLAYER5, IDC_BR_PLAYER5, IDC_SL5 },
	{ IDC_C_PLAYER6, IDC_INSTALL6, IDC_PLAYER6, IDC_E_PLAYER6, IDC_BR_PLAYER6, IDC_SL6 },
	{ IDC_C_PLAYER7, IDC_INSTALL7, IDC_PLAYER7, IDC_E_PLAYER7, IDC_BR_PLAYER7, IDC_SL7 },
	{ IDC_C_PLAYER8, IDC_INSTALL8, IDC_PLAYER8, IDC_E_PLAYER8, IDC_BR_PLAYER8, IDC_SL8 },
};

#define COUNT_PER_PAGE CountOf(__vSetupPlayerCtrl)

int CmdId2Index(int nId, SetupCmdType &type)
{
	for (int i = 0; i < CountOf(__vSetupPlayerCtrl); i++)
	{
		SetupPlayerCtrl &ctrl = __vSetupPlayerCtrl[i];
		if (nId == ctrl.nCheckBt)
		{
			type = SCT_CHECK;
			return i;
		}
		else if (nId == ctrl.nInstallBt)
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

BOOL ShowElevatedDialog(CBaseWnd *pWnd)
{
	tstring	str = TOLOCAL_T("To install MiniLyrics plugin, you will be asked to run MiniLyrics as administrator.");
	str += _T("\n\n");
	str += TOLOCAL_T("Do you want to restart MiniLyrics to continue the setup?");
	int nRet = pWnd->MessageOut(str.c_str(), MB_ICONINFORMATION | MB_OKCANCEL);
	if (nRet == IDOK)
	{
		TCHAR szPath[MAX_PATH];
		::GetModuleFileName(NULL, szPath, MAX_PATH);
		BOOL bSuccess = RunElevated(pWnd->GetHandle(), szPath);
		if (!bSuccess)
		{
			int nErr = GetLastError();
			str = TOLOCAL_T("Failed to elevate the privilege of MiniLyrics.");
			str += _T("\n");
			str += OSError(nErr).Description();
			pWnd->MessageOut(str.c_str(), MB_ICONSTOP | MB_OK);
		}
		else
			PostQuitMessage(0);

		return TRUE;
	}
	else
		return FALSE;
}

//////////////////////////////////////////////////////////////////////

CDlgSetupPlayerPlugins::CDlgSetupPlayerPlugins(bool bFirstSetup) : CBaseDialog(IDD_SETUP)
{
	g_bFirstSetup = bFirstSetup;
	m_bShowAllSupportedPlayers = false;
	m_nCurPage = 0;
}

CDlgSetupPlayerPlugins::~CDlgSetupPlayerPlugins()
{
}

BOOL CDlgSetupPlayerPlugins::OnInitDialog()
{
	if (!CBaseDialog::OnInitDialog())
		return FALSE;

	SetTopmost(true);

	if (!g_bFirstSetup)
	{
		ShowDlgItem(IDOK, SW_HIDE);
		SetDlgItemText(IDCANCEL, TOLOCAL_T("Close"));
		// ShowDlgItem(IDCANCEL, SW_HIDE);
	}

	m_installer.Init();

	// Show only installed players.
	for (int i = 0; ; i++)
	{
		CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(i);
		if (!p)
			break;

		if (p->dwInstallFlags != 0 && p->IsInstalled())
		{
			p->SetToInstallPlugin(true);
			tstring strError;
			p->CheckPluginInstalled(strError);
			m_vPlayerItems.push_back(i);
		}
	}

	if (m_vPlayerItems.empty())
		ShowAllSupportedPlayers();

	UpdatePlayersToView();

	return TRUE;
}

void CDlgSetupPlayerPlugins::OnSize(int cx, int cy)
{
	CBaseDialog::OnSize(cx, cy);
}

void CDlgSetupPlayerPlugins::OnOK()
{
	int nInstalledCount = 0;
	bool bHasError = false;
	for (uint i = 0; i < m_vPlayerItems.size(); i++)
	{
		CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(m_vPlayerItems[i]);
		if (!p)
			continue;

		if (p->IsToInstallPlugin())
		{
			if (!p->CheckPlayerInstalled(false))
			{
				if (!BrowsePlayerFolder(p))
					return;
			}

			tstring strErr;
			int err;
			if (p->Install(strErr, err))
			{
				p->SavePlayerPath();
				nInstalledCount++;
			}
			else
			{
				bHasError = true;
				MessageOut(strErr.c_str(), MB_ICONINFORMATION);

				if (err == ERROR_ACCESS_DENIED)
					ShowElevatedDialog(this);
			}
		}
	}

	if (!bHasError)
	{
		if (nInstalledCount > 0)
		{
			MessageOut(TOLOCAL_T("MiniLyrics plug-ins were installed successfully."), MB_ICONINFORMATION);
			CBaseDialog::OnOK();
		}
		else
			MessageOut(TOLOCAL_T("Please choose at least one player."), MB_ICONINFORMATION);
	}
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
			if (n == m_vPlayerItems.size())
			{
				// Clicked "More..." button
				if (!m_bShowAllSupportedPlayers && type == SCT_INSTALL)
					ShowAllSupportedPlayers();
				return;
			}
			else if (n >= (int)m_vPlayerItems.size())
				return;

			if (m_vPlayerItems[n] == INDEX_BUILT_IN_PLUGIN)
			{
				// Built-in plugins
				if (type == SCT_INSTALL)
				{
					CStrPrintf	str(_T("%s\n\n%s"),
						TOLOCAL_T("The players below are built-in supported."),
						m_strBuiltInPlayers.c_str());
					MessageOut(str.c_str(), MB_ICONINFORMATION | MB_OK);
				}
				return;
			}

			CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(m_vPlayerItems[n]);
			ASSERT(p);
			if (!p)
				return;

			// Get player path
			SetupPlayerCtrl &ctrl = __vSetupPlayerCtrl[n % COUNT_PER_PAGE];
			int nPathId = ctrl.nPathId;
			GetDlgItemText(nPathId, p->pExtraInfo->strPlayerPath);

			switch (type)
			{
			case SCT_CHECK:
				OnClickedCheckBtn(p, ctrl.nCheckBt);
				break;
			case SCT_INSTALL:
				OnClickedInstallBtn(p);
				SetDlgItemText(nPathId, p->pExtraInfo->strPlayerPath.c_str());
				break;
			case SCT_BR_PATH:
				OnClickedBrowsePathBtn(p);
				SetDlgItemText(nPathId, p->pExtraInfo->strPlayerPath.c_str());
				p->SetToInstallPlugin(p->IsInstalled());
				CheckButton(ctrl.nCheckBt, p->IsInstalled());
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

void CDlgSetupPlayerPlugins::OnClickedCheckBtn(CPlayerPluginInstaller::Player *p, int nCheckBtnId)
{
	if (!p->IsInstalled())
		p->DetectPlayerPath();

	bool bChecked = IsButtonChecked(nCheckBtnId);
	if (bChecked)
	{
		if (!p->CheckPlayerInstalled(false))
		{
			if (BrowsePlayerFolder(p))
			{
				p->SetToInstallPlugin(true);
				UpdatePlayersToView();
			}
			else
			{
				bChecked = false;
				CheckButton(nCheckBtnId, false);
				return;
			}
		}
	}
	p->SetToInstallPlugin(bChecked);
}

void CDlgSetupPlayerPlugins::OnClickedInstallBtn(CPlayerPluginInstaller::Player *p)
{
	ASSERT(p);

	m_installer.RefreshRunningPlayers();

	// Is player installed?
	if (!p->IsInstalled())
	{
		if (!BrowsePlayerFolder(p))
			return;
	}
	else if (p->IsPluginInstalled())
	{
		// Uninstall plugin
		tstring strMsg = CStrPrintf(TOLOCAL_T("Are you sure that you want to uninstall MiniLyrics plugin for %s?"), p->szPlayerName).c_str();
		if (MessageOut(strMsg.c_str(), MB_ICONINFORMATION | MB_OKCANCEL) != IDOK)
			return;

		tstring strErr;
		if (!p->Uninstall(strErr))
		{
			tstring strMsg = CStrPrintf(TOLOCAL_T("Failed to uninstall MiniLyrics plugin for %s"), p->szPlayerName).c_str();
			strMsg += _T("\n\n");
			strMsg += strErr;
			MessageOut(strMsg.c_str(), MB_ICONWARNING | MB_OK);
		}
		else
		{
			p->CheckPluginInstalled(strErr);
			UpdatePlayersToView();
		}
		return;
	}

	// Is player running?
	if (p->IsPlayerRunning())
	{
		tstring strMsg = TOLOCAL_T("The player is running.");
		strMsg += _T("\n");
		strMsg += p->GetPlayerExeFile();
		strMsg += _T("\n\n");
		strMsg += TOLOCAL_T("Please close it, then click 'OK' to continue.");
		MessageOut(strMsg.c_str(), MB_ICONINFORMATION | MB_OK);
	}

	tstring strErr;
	int err;
	if (!p->Install(strErr, err))
	{
		MessageOut(strErr.c_str(), MB_ICONINFORMATION);
		if (err == ERROR_ACCESS_DENIED)
			ShowElevatedDialog(this);
		return;
	}

	p->SavePlayerPath();

	MessageOut(CStrPrintf(TOLOCAL_T("MiniLyrics plugin was installed for %s."), p->szPlayerName).c_str(),
		MB_ICONINFORMATION | MB_OK);

	UpdatePlayersToView();
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
		MessageOut(TOLOCAL_T("Can't detect the player installed in the specified folder."), MB_OK | MB_ICONINFORMATION);

	UpdatePlayersToView();
}

void CDlgSetupPlayerPlugins::OnPathEditChanged(CPlayerPluginInstaller::Player *p)
{

}

bool CDlgSetupPlayerPlugins::BrowsePlayerFolder(CPlayerPluginInstaller::Player *p)
{
	tstring strMsg = CStrPrintf(TOLOCAL_T("MiniLyrics can't detect the path of %s."), p->szPlayerName).c_str();
	strMsg += _T("\n\n");
	strMsg += TOLOCAL_T("Do you want to specify its installation path manually?");
	if (MessageOut(strMsg.c_str(), MB_ICONINFORMATION | MB_OKCANCEL) != IDOK)
		return false;

	CFolderDialog	dlg;

	dlg.SetInitFolder(GetProgramFilesPath().c_str());
	if (!dlg.DoBrowse(this))
		return false;

	p->pExtraInfo->strPlayerPath = dlg.GetFolder();
	if (!p->CheckPlayerInstalled(false))
	{
		MessageOut(CStrPrintf(TOLOCAL_T("Sorry, the folder you browsed does NOT have file: %s"), p->szExeName).c_str());
		return false;
	}

	return true;
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

		if (p->dwInstallFlags == 0)
		{
			if (m_strBuiltInPlayers.size())
				m_strBuiltInPlayers += _T(", ");
			m_strBuiltInPlayers += p->szPlayerName;
		}
		else if (!p->IsInstalled())
			m_vPlayerItems.push_back(i);
	}

	m_vPlayerItems.push_back(INDEX_BUILT_IN_PLUGIN);

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
		SetupPlayerCtrl &ctrl = __vSetupPlayerCtrl[i % COUNT_PER_PAGE];

		if (m_vPlayerItems[i] == INDEX_BUILT_IN_PLUGIN)
		{
			SetDlgItemText(ctrl.nInstallBt, TOLOCAL_T("&Built-in Plugins"));
			SetDlgItemText(ctrl.nPlayerNameId, TOLOCAL_T("Supported players:"));
			SetDlgItemText(ctrl.nPathId, m_strBuiltInPlayers.c_str());
			ctrl.SetVisibleForMoreBtn(this);
			ShowDlgItem(ctrl.nPathId, SW_SHOW);
		}
		else
		{
			CPlayerPluginInstaller::Player *p = m_installer.GetPlayerByIndex(m_vPlayerItems[i]);
			ASSERT(p != NULL);

			CheckButton(ctrl.nCheckBt, p->IsToInstallPlugin());
			SetDlgItemText(ctrl.nCheckBt, CStrPrintf(TOLOCAL_T("Support %s:"), p->szPlayerName).c_str());
			SetDlgItemText(ctrl.nInstallBt, p->IsPluginInstalled() ? TOLOCAL_T("Uninstall") : TOLOCAL_T("Install"));
			SetDlgItemText(ctrl.nPlayerNameId, p->szPlayerName);
			SetDlgItemText(ctrl.nPathId, p->pExtraInfo->strPlayerPath.c_str());
			ctrl.SetVisible(this, true);
		}
	}
	if (nEndItem >= (m_nCurPage + 1) * COUNT_PER_PAGE)
		return;

	int nEndCtrlIndex = nEndItem % COUNT_PER_PAGE;
	if (!m_bShowAllSupportedPlayers
		&& nEndCtrlIndex < COUNT_PER_PAGE)
	{
		// Show "More ..." button
		SetupPlayerCtrl &ctrl = __vSetupPlayerCtrl[nEndCtrlIndex];
		SetDlgItemText(ctrl.nInstallBt, TOLOCAL_T("&More..."));
		SetDlgItemText(ctrl.nPlayerNameId, TOLOCAL_T("More players..."));
		nEndCtrlIndex++;
		ctrl.SetVisibleForMoreBtn(this);
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
