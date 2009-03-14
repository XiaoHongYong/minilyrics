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

	HWND GetMainWnd();

	// get player state and song info
	DWORD GetPlayerState();

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
			return 1;
		else
			return IPlayer::IsFeatureSupported(feature);
	}

	void OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan);

	void OnMediaChanged();

public:
	PLAYER_STATE	m_bPlayerState;
	DWORD			m_dwLength;

};
