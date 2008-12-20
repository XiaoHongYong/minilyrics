#pragma once

#include "../MLPluginLib/IPlayer.h"
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

	HWND GetMainWnd();

	// get player state and song info
	DWORD GetPlayerState();

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

	virtual bool IsUcs2Support()
	{
		return true;
	}

	virtual void SetTimeUpdateFreq(int nFreq);

	int GetFullTitleW(WCHAR * lpszBuffer, int nBufSize);
	int GetArtistW(WCHAR * lpszBuffer, int nBufSize);
	int GetAlbumW(WCHAR * lpszBuffer, int nBufSize);
	int GetTitleW(WCHAR * lpszBuffer, int nBufSize);
	int GetSourceMediaW(WCHAR * lpszSourceMedia, int nBufSize);

public:

	WCHAR			m_szFullTitle[LRC_TITLE_MAX_LEN];
	// WCHAR			m_szArtist[128], m_szAlbum[128], m_szTitle[128];
	DWORD			m_dwPos;

	WCHAR			m_szSrcMedia[MAX_PATH];

public:
	// for wmplayer
	void OnMediaChanged();

	void SetWMPCore(IWMPCore *pCore);
	void ReleaseWMPCore();

	int GetProperty(const WCHAR *szProperty, WCHAR *szValue, int nLen);

protected:
	CComPtr<IWMPCore>		m_pCore;
	CComPtr<IWMPControls>	m_pControl;
	DWORD			m_dwSongLength;

};
