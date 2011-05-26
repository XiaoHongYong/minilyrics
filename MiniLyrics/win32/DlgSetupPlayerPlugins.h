
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

	void OnOK();
	void OnCommand(UINT uID, UINT nNotifyCode);

protected:
	void OnClickedCheckBtn(CPlayerPluginInstaller::Player *p, int nCheckBtnId);
	void OnClickedInstallBtn(CPlayerPluginInstaller::Player *p);
	void OnClickedBrowsePathBtn(CPlayerPluginInstaller::Player *p);
	void OnPathEditChanged(CPlayerPluginInstaller::Player *p);

	bool BrowsePlayerFolder(CPlayerPluginInstaller::Player *p);

	void ShowAllSupportedPlayers();

	void UpdatePlayersToView();
	int GetMaxPagesCount();

protected:
	bool					m_bShowAllSupportedPlayers;
	int						m_nCurPage;
	vector<int>				m_vPlayerItems;

	tstring					m_strBuiltInPlayers;

	CPlayerPluginInstaller	m_installer;

};
