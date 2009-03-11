#pragma once

extern class CPlayerImp		g_Player;

class CPlayerImp : public IPlayer
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	// Init and destroy.
	BOOL Init();
	void Quit();

	// Player Info
	int GetPlayerName(char * lpszBuffer, int nBufSize);

	// Get player skin path
	int GetCurSkinPath(char * lpszBuffer, int nBufSize);

	HWND GetMainWnd();

	// get player state and song info
	DWORD GetPlayerState();

	DWORD GetMediaLength();

	DWORD GetPlayPos();
	int HasVideo();

	// control player
	BOOL Play();
	BOOL Pause();
	BOOL PlayPause();
	BOOL Stop();
	BOOL Prev();
	BOOL Next();
	BOOL SeekTo(int nMsPos);

	virtual int IsFeatureSupported(PlayerFeature feature)
	{
		if (feature == PF_MinimizeRestoreWithPlayer)
			return PR_OK;
		else
			return IPlayer::IsFeatureSupported(feature);
	}

	virtual HWND GetAttachedParentWnd(RECT *prc);
	virtual void OnNotifySkinLoaded();
	virtual void OnNotifyDetachedFromParentWnd();
	virtual int GetEmbeddedSkinName(TCHAR * lpszBuffer, int nBufSize);

	virtual int GetSkinThemeColor(SKIN_CLR_NAME scn, COLORREF *clrValue);

	virtual void OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan);

	BOOL IsWinamp5();

public:
	DWORD			m_dwPlayerVer;
	HWND			m_hwndPluginParent;

	TCHAR			m_szPlayerName[64];

	bool			m_bWorking;
	DWORD			m_dwShoutcastTitleChangedPos;
	PLAYER_STATE	m_state;

};
