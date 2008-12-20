#pragma once


extern class CPlayerImp		g_Player;

#define LRC_TITLE_MAX_LEN		256

class CPlayerImp : public IPlayer
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	void OnSongChanged(LPCTSTR szFullTitle, LPCTSTR szMediaSrc);
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
	int GetFullTitle(char * lpszBuffer, int nBufSize);
	int GetTitle(char * lpszBuffer, int nBufSize);
	int GetArtist(char * lpszBuffer, int nBufSize);
	int GetAlbum(char * lpszBuffer, int nBufSize);
	int GetSourceMedia(char * lpszSourceMedia, int nBufSize);

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

protected:
	FUNPlayerService	m_funPlayerService;

	TCHAR			m_szFullTitle[LRC_TITLE_MAX_LEN];
	DWORD			m_dwPos;

	TCHAR			m_szSrcMedia[MAX_PATH];

};
