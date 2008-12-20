#pragma once

extern class CPlayerImp		g_Player;

#define LRC_TITLE_MAX_LEN		256

class CPlayerImp : public IPlayer
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	void OnSongChanged(LPCWSTR szFullTitle);
	void OnPlayTimeChanged(DWORD dwTimeCur);

	// Init and destroy.
	BOOL Init();
	void Quit();

	// Player Info
	int GetPlayerName(char * lpszBuffer, int nBufSize);

	// Get player skin path
	int GetCurSkinPath(char * lpszBuffer, int nBufSize);

	// Get player's proxy setting.
	int GetProxy(char * lpszBuffer, int nBufSize);

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
	WCHAR			m_szArtist[128], m_szAlbum[128], m_szTitle[128];

	WCHAR			m_szFullTitle[LRC_TITLE_MAX_LEN];

//	TCHAR			m_szSrcMedia[MAX_PATH];

};
