#pragma once

#include "../MLPluginLib/IPlayer.h"
extern class CPlayerImp		g_Player;

void WmpPluginInit(HINSTANCE hInstance);
void WmpPluginQuit();

class CPlayerImp : public IPlayer
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	virtual void OnGetInfo(PlayerInfo &playerInfo);

	virtual void OnConnect();
	virtual void OnConnectedDone();
	virtual void OnDisconnect();

	// control player
	virtual void Play();
	virtual void Pause();
	virtual void PlayPause();
	virtual void Stop();
	virtual void Prev();
	virtual void Next();
	virtual void SeekTo(int nMsPos);

public:
	// get player state and song info
	PLAYER_STATE GetPlayerState();

	DWORD GetPlayPos();

	void OnMediaChanged();

	void SetWMPCore(IWMPCore *pCore);
	void ReleaseWMPCore();

	int GetProperty(const WCHAR *szProperty, WCHAR *szValue, int nLen);

protected:
	CComPtr<IWMPCore>		m_pCore;
	CComPtr<IWMPControls>	m_pControl;

};
