#pragma once

#include "../MLPluginLib/IPlayer.h"
#include "iTunesCOMInterface.h"
#include "iTunesVisualAPI.h"

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

	virtual HWND GetAttachedParentWnd(RECT *prc);
	virtual void OnNotifySkinLoaded();
	virtual void OnNotifyDetachedFromParentWnd();
	virtual int GetEmbeddedSkinName(TCHAR * lpszBuffer, int nBufSize);

	virtual int GetSkinThemeColor(SKIN_CLR_NAME scn, COLORREF *clrValue);

	virtual void OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan);

public:
	void OniTunesChangeVisualWnd(HWND hWnd, const Rect *rc);

	HWND			m_hWndiTunesVisual;
	Rect			m_rciTunesVisual;

public:
	DWORD			m_dwPos;
	bool			m_bWorking;

	PLAYER_STATE	m_bPlayerState;
	DWORD			m_dwBegTickCount;

};
