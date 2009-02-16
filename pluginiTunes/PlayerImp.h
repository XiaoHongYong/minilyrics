#pragma once

#include "../MLPluginLib/IPlayer.h"
#include "iTunesCOMInterface.h"
#include "iTunesVisualAPI.h"

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

	//	BOOL GetCurSong(LPTSTR szTitle, int nSizeTitle, LPTSTR szArtist, int nSizeArtist);
	int GetFullTitleW(WCHAR *lpszBuffer, int nBufSize);
	int GetArtistW(WCHAR *lpszBuffer, int nBufSize);
	int GetAlbumW(WCHAR *lpszBuffer, int nBufSize);
	int GetTitleW(WCHAR *lpszBuffer, int nBufSize);
	int GetSourceMediaW(WCHAR *lpszSourceMedia, int nBufSize);

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

	virtual HWND GetAttachedParentWnd(RECT *prc);
	virtual void OnNotifySkinLoaded();
	virtual void OnNotifyDetachedFromParentWnd();
	virtual int GetEmbeddedSkinName(TCHAR * lpszBuffer, int nBufSize);

	virtual int GetSkinThemeColor(SKIN_CLR_NAME scn, COLORREF *clrValue);

	virtual void SetTimeUpdateFreq(int nFreq);

	virtual bool IsUcs2Support()
	{
		return true;
	}

public:
	void OniTunesChangeVisualWnd(HWND hWnd, const Rect *rc);

	HWND			m_hWndiTunesVisual;
	Rect			m_rciTunesVisual;

protected:
	FUNPlayerService	m_funPlayerService;

public:
	WCHAR			m_szFullTitle[LRC_TITLE_MAX_LEN];
	DWORD			m_dwPos;
	WCHAR			m_szArtist[128], m_szAlbum[128], m_szTitle[128];

	WCHAR			m_szSrcMedia[MAX_PATH];
	WCHAR			m_szFileName[MAX_PATH];

	int				m_nMediaLength;

	PLAYER_STATE	m_bPlayerState;
	DWORD			m_dwBegTickCount;

};
