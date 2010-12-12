#pragma once

#include "../MLPluginLib/MiniLyricsPlugin.h"
#include "../MLPluginLib/TinyBaseWnd.h"

class CMLPlayerMgr : public CTinyBaseWnd
{
public:
	CMLPlayerMgr() : m_hWndCurPlugin(NULL) { }

	bool Create();

	void OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds);

	void OnPlayerInfoReturned(PlayerInfo *playerInfo);

	void OnPlayerDisconnected();

	void QueryRunningPlayerInfo();

	HWND GetCurPluginServiceWnd() { return m_hWndCurPlugin; }

	void SetCurPlayer(int nIndex);

	bool IsValid() { return m_hWnd != NULL; }

public:
	typedef vector<PlayerInfo>	ListPlayerInfo;
	ListPlayerInfo				m_listPlayers;
	HWND						m_hWndCurPlugin;

};

extern CMLPlayerMgr			g_playerMgr;
