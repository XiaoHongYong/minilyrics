#pragma once

#include "../MLPluginLib/MLPlugin.h"

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

	// control player
	BOOL Play();
	BOOL Pause();
	BOOL PlayPause();
	BOOL Stop();
	BOOL Prev();
	BOOL Next();
	BOOL SeekTo(int nMsPos);

	void OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan);

	void OnMusicChangeCheck();
	void OnPlayTimeChangeCheck();

public:
	IMJAutomationPtr	m_pMJ;

	WCHAR				m_szSrcMedia[MAX_PATH];
	DWORD				m_nPlaybackPosBeg;
	DWORD				m_dwTimeBeg;

};
