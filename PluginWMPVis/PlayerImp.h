#pragma once

#include "../MLPluginLib/IPlayer.h"
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


	virtual HWND GetAttachedParentWnd(RECT *prc);
	virtual void OnNotifySkinLoaded();
	virtual void OnNotifyDetachedFromParentWnd();
	virtual int GetEmbeddedSkinName(TCHAR * lpszBuffer, int nBufSize);
public:
	HWND			m_hWndVis;

public:
	// for wmplayer
	void OnMediaChanged();

	void SetWMPCore(IWMPCore *pCore);
	void ReleaseWMPCore();

	int GetProperty(const WCHAR *szProperty, WCHAR *szValue, int nLen);

protected:
	CComPtr<IWMPCore>		m_pCore;
	CComPtr<IWMPControls>	m_pControl;

};
