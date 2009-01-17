#pragma once

extern class CPlayerImp		g_Player;

#define LRC_TITLE_MAX_LEN		256

class CPlayerImp : public IPlayer
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	void OnSongChanged();

	// Init and destroy.
	BOOL Init();
	void Quit();

	// Player Info
	int GetPlayerName(char * lpszBuffer, int nBufSize);

	HWND GetMainWnd();

	// get player state and song info
	DWORD GetPlayerState();

	int GetFullTitleW(WCHAR * lpszBuffer, int nBufSize);
	int GetArtistW(WCHAR * lpszBuffer, int nBufSize);
	int GetAlbumW(WCHAR * lpszBuffer, int nBufSize);
	int GetTitleW(WCHAR * lpszBuffer, int nBufSize);
	int GetSourceMediaW(WCHAR * lpszSourceMedia, int nBufSize);

	DWORD GetMediaLength();

	DWORD GetPlayPos();

	virtual int HasVideo();

	// control player
	BOOL Play();
	BOOL Pause();
	BOOL PlayPause();
	BOOL Stop();
	BOOL Prev();
	BOOL Next();
	BOOL SeekTo(int nMsPos);

	void SetTimeUpdateFreq(int nFreq);

	virtual bool IsUcs2Support()
	{
		return true;
	}

public:
	void NotifyPlayerStatChanged(PLAYER_STATE newStat);

	void MediaChangedCheck();

public:
	wstring			m_strArtist, m_strAlbum, m_strTitle, m_strFullTitle, m_strSrcMedia;

	PLAYER_STATE	m_stat;

	intf_thread_t	*m_p_intf;

};
