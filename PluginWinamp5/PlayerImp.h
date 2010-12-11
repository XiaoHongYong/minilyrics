#pragma once

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

	virtual void SwitchToEmbeddedSkin();
	virtual void EndEmbeddedSkin();

public:
	void SkinOfPlayerChanged();

	void PrepareCurSkinInfo(PlayerSkinInfo &skinInfo);

	int HasVideo();

	HWND GetAttachedParentWnd();

	BOOL IsWinamp5() { return m_bWinamp5; }
	DWORD GetPlayPos();

public:
	bool				m_bWinamp5;
	HWND				m_hWndPluginParent;
	embedWindowState	*m_embeddedWndState;

	bool				m_bWorking;
	DWORD				m_dwShoutcastTitleChangedPos;
	PLAYER_STATE		m_state;

};
