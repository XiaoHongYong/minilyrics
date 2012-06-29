#pragma once

#include "../../MLPluginLib/MiniLyricsPlugin.h"
#include "../../MacPluginiTunes/MsgPipe.h"

class CMLPlayerMgr
{
public:
	typedef vector<PlayerInfo>	ListPlayerInfo;

	bool Init(int argc, char *argv[]);
	void Quit();

	bool ConnectToPlayer();

	void OnPlayerInfoReturned(PlayerInfo *playerInfo);

	void OnPlayerDisconnected();

	bool IsValid() { return true; }

	void SendMessageToPlayer(MessageToPlayer message, LPVOID param, size_t sizeParam);

protected:
	void QueryInfo();
	bool OnMsg(uint32 msg, LPCVOID lpData, size_t lenData);
	static LPVOID ThreadMsg(LPVOID param);

protected:
	CMsgPipe					m_msgPipeOfPlayer;
	CMsgPipe					m_msgPipeOfMiniLyrics;

};

extern CMLPlayerMgr			g_playerMgr;
