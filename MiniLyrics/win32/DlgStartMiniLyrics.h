
#pragma once

#include "../widget/widget.h"
#include "PlayerPluginInstaller.h"

class CDlgStartMiniLyrics : public CBaseDialog  
{
public:
	CDlgStartMiniLyrics();
	virtual ~CDlgStartMiniLyrics();

	virtual BOOL OnInitDialog();

	void OnCommand(UINT uID, UINT nNotifyCode);
	void OnTimer(UINT nIDEvent);

	static void OnPlayerStarted();

protected:
	struct PlayerItem
	{
		HWND			hWndPluginService;		// PluginService (MiniLyrics) for player
		tstring			strPlayerName, strPlayerExe;
		int				nIndexOfPlayerInstaller;

		PlayerItem(LPCTSTR szPlayerName, HWND hWndPluginService, LPCTSTR szPlayerExe) {
			strPlayerName = szPlayerName;
			strPlayerExe = szPlayerExe;
			this->hWndPluginService = hWndPluginService;
			nIndexOfPlayerInstaller = 0;
		}

		PlayerItem(LPCTSTR szPlayerName, int nIndexOfPlayerInstaller) {
			hWndPluginService = NULL;
			strPlayerName = szPlayerName;
			this->nIndexOfPlayerInstaller = nIndexOfPlayerInstaller;
		}
	};
	typedef vector<PlayerItem>	VecPlayerItems;

	void OnClickPlayer(CPlayerPluginInstaller::Player *p);

	void ShowInstalledPlayers();
	void ShowAllSupportedPlayers();

	void UpdatePlayersToView();

	int GetMaxPagesCount();

protected:
	bool					m_bShowInstalledPlayers;
	bool					m_bShowAllSupportedPlayers;

	VecPlayerItems			m_vPlayerItems;
	int						m_nCurPage;

	CPlayerPluginInstaller	m_installer;

};
