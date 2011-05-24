
#pragma once

#include "../widget/widget.h"
#include "PlayerPluginInstaller.h"

class CDlgSetupPlayerPlugins : public CBaseDialog  
{
public:
	CDlgSetupPlayerPlugins();
	virtual ~CDlgSetupPlayerPlugins();

	virtual BOOL OnInitDialog();

	void OnSize(int cx, int cy);

	void OnCommand(UINT uID, UINT nNotifyCode);

protected:
	void ShowAllSupportedPlayers();

	void UpdatePlayersToView();

protected:
	CPlayerPluginInstaller	m_installer;
	CWndHelper				m_wndHelper;

	bool					m_bShowAllSupportedPlayers;
	vector<int>				m_vPlayerItems;

};
