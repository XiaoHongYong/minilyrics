#pragma once

#include "../MLPluginLib/MiniLyricsPlugin.h"
#include "../MLPluginLib/TinyBaseWnd.h"
#include "../MLPlayerPluginAgentLib/IPlayerPluginAgent.h"

HWND GetMLSericeWindow();

bool ProcessMLCmdLine(LPCTSTR szCmdLine, StartApp &startApp);

class CMLPlayerMgr : public CTinyBaseWnd
{
public:
	typedef vector<PlayerInfo>	ListPlayerInfo;

	CMLPlayerMgr() : m_hWndCurPlugin(NULL) { }

	bool Create();

	bool ConnectToPlayer();

	bool OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds);

	void OnPlayerInfoReturned(PlayerInfo *playerInfo);

	void OnPlayerDisconnected();

	void QueryRunningPlayerInfo();
	const ListPlayerInfo &GetRunningPlayers() const { return m_listPlayers; }

	HWND GetCurPluginServiceWnd() { return m_hWndCurPlugin; }

	bool ConnectToPlayer(HWND hWndPluginService);

	bool IsValid() { return m_hWnd != NULL; }

protected:
	ListPlayerInfo				m_listPlayers;
	HWND						m_hWndCurPlugin;
	VecPlayerPluginAgent		m_vPlayerPluginAgent;

};

extern CMLPlayerMgr			g_playerMgr;
