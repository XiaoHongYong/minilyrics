
#pragma once

#include "../widget/widget.h"
#include "PlayerPluginListCtrl.h"
#include "PlayerPluginInstaller.h"

class CDlgSetupPlayerPlugins : public CBaseDialog  
{
public:
	CDlgSetupPlayerPlugins();
	virtual ~CDlgSetupPlayerPlugins();

	virtual BOOL OnInitDialog();
	void OnDestroy();

	void OnSize(int cx, int cy);

	void OnOK();
	void OnCommand(UINT uID, UINT nNotifyCode);

protected:
	void RefreshStatus();

	void UpdatePlayers();

protected:
	CPlayerPluginListCtrl	m_listPlayers;
	CPlayerPluginInstaller	m_pluginInstaller;
	HIMAGELIST				m_hImageList;
	CWndHelper				m_wndHelper;

};
