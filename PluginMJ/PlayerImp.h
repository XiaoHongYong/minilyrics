#pragma once

#include "../MLPluginLib/IPlayer.h"
#include "../MLPluginLib/MLPlugin.h"
extern class CPlayerImp		g_Player;

#define LRC_TITLE_MAX_LEN		256

class CPlayerImp : public IPlayer
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	void OnSongChanged(LPCWSTR szFullTitle, LPCWSTR szSrcMedia);
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

	DWORD GetMediaLength();

	DWORD GetPlayPos();
	// int HasVideo();

	// control player
	BOOL Play();
	BOOL Pause();
	BOOL PlayPause();
	BOOL Stop();
	BOOL Prev();
	BOOL Next();
	BOOL SeekTo(int nMsPos);

	virtual bool IsUcs2Support()
	{
		return true;
	}


	int GetArtistW(WCHAR * lpszBuffer, int nBufSize);
	int GetTitleW(WCHAR * lpszBuffer, int nBufSize);
	int GetAlbumW(WCHAR * lpszBuffer, int nBufSize);
	int GetFullTitleW(WCHAR * lpszBuffer, int nBufSize);
	int GetSourceMediaW(WCHAR * lpszSourceMedia, int nBufSize);

	void SetTimeUpdateFreq(int nFreq);

	void OnMusicChangeCheck();
	void OnPlayTimeChangeCheck();

public:
	IMJAutomationPtr	m_pMJ;
	WCHAR				m_szFullTitle[LRC_TITLE_MAX_LEN];
	WCHAR				m_szSrcMedia[MAX_PATH];
	DWORD				m_dwSongLength;

	DWORD				m_nPlaybackPosBeg;
	DWORD				m_dwTimeBeg;

};
