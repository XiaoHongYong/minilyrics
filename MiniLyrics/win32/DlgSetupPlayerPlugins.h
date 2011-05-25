
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
	void OnClickedInstallBtn(CPlayerPluginInstaller::Player *p);
	void OnClickedBrowsePathBtn(CPlayerPluginInstaller::Player *p);
	void OnPathEditChanged(CPlayerPluginInstaller::Player *p);

	void ShowAllSupportedPlayers();

	void UpdatePlayersToView();
	int GetMaxPagesCount();

protected:
	CWndHelper				m_wndHelper;

	bool					m_bShowAllSupportedPlayers;
	int						m_nCurPage;
	vector<int>				m_vPlayerItems;

	CPlayerPluginInstaller	m_installer;

};
