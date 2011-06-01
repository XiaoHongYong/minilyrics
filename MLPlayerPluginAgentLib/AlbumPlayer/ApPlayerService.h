#pragma once

#include "IPlayerPluginAgent.h"

namespace AlbumPlayerAgent
{

class CPlayerService : public CPlayerServiceBase
{
public:
	CPlayerService();
	virtual ~CPlayerService();

public:
	virtual void OnGetInfo(PlayerInfo &playerInfo);

	virtual void OnConnected();
	virtual void OnDisconnect();

	virtual void Play();
	virtual void Pause();
	virtual void PlayPause();
	virtual void Stop();
	virtual void Prev();
	virtual void Next();
	virtual void SeekTo(int nMsPos);

	virtual void SwitchToEmbeddedSkin();
	virtual void EndEmbeddedSkin();

protected:
	void OnCopyDataCheckSongTitle(COPYDATASTRUCT *pCopyDataStruct);
	void SendAPCopyDataMsg(LPCSTR szXML);

	void OnPlayTimeChanged(DWORD dwTimeCur);

	void OnPlayTimeChangeCheck();

	void OnTimer(int nTimerID);
	LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	DWORD			m_dwTimeBeg;
	HWND			m_hwndPlayer;

	int				m_nPos, m_nPosNotify;

	PLAYER_STATE	m_state;
	PlayerSkinInfo	m_skinInfo;

};

class CPlayerPluginAgent : public IPlayerPluginAgent
{
public:
	CPlayerPluginAgent();

	virtual bool IsPlayerRunning();

};

}
