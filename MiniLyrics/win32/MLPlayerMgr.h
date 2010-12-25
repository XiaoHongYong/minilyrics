#pragma once

#include "../MLPluginLib/MiniLyricsPlugin.h"
#include "../MLPluginLib/TinyBaseWnd.h"
#include "../MLPlayerPluginAgentLib/IPlayerPluginAgent.h"

HWND GetMLSericeWindow();

bool ProcessMLCmdLine(LPCTSTR szCmdLine);

bool MLConnectToPlayer();

class CMLPlayerMgr : public CTinyBaseWnd
{
public:
	CMLPlayerMgr() : m_hWndCurPlugin(NULL) { }

	bool Create();

	bool OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds);

	void OnPlayerInfoReturned(PlayerInfo *playerInfo);

	void OnPlayerDisconnected();

	void QueryRunningPlayerInfo();

	HWND GetCurPluginServiceWnd() { return m_hWndCurPlugin; }

	void SetCurPlayer(int nIndex);

	bool IsValid() { return m_hWnd != NULL; }

	// void RegisterPluginAgent(IPlayerPluginAgent *pluginAgent) { m_vPlayerPluginAgent.push_back(pluginAgent); }

public:
	typedef vector<PlayerInfo>	ListPlayerInfo;
	ListPlayerInfo				m_listPlayers;
	HWND						m_hWndCurPlugin;
	VecPlayerPluginAgent		m_vPlayerPluginAgent;

};

extern CMLPlayerMgr			g_playerMgr;
