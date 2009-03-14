#include "stdafx.h"
#include "wmplyrics.h"
#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"

CPlayerImp		g_Player;


class CPlayerInfoQueryWnd : public CPlayerInfoQueryBaseWnd
{
public:
	void OnCreate()
	{
		SetTimer(m_hWnd, TIMER_GETPOS, TIME_OUT_UPDATE_POS, NULL);
	}

	void OnDestroy()
	{
		KillTimer(m_hWnd, TIMER_GETPOS);
	}

	void OnTimer(int nTimerID)
	{
		g_MLService.OnPlayTimeChanged(g_Player.GetPlayPos());
	}

};

CPlayerInfoQueryWnd	g_wndInfoQuery;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "Windows Media Player");

	return 1;
}

// get player state and song info
DWORD CPlayerImp::GetPlayerState()
{
	if (!m_pCore)
		return PS_STOPED;

	WMPPlayState	state;
	if (SUCCEEDED(m_pCore->get_playState(&state)))
	{
		if (state == wmppsPlaying)
			return PS_PLAYING;
		else if (state == wmppsPaused)
			return PS_PAUSED;
		else
			return PS_STOPED;
	}

	return PS_STOPED;
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	double	dbTime = 0;

	assert(m_pControl);
	if (!m_pControl)
		return 0;

	HRESULT hr = m_pControl->get_currentPosition(&dbTime);

	return (DWORD)(dbTime * 1000);
}

// control player
BOOL CPlayerImp::Play()
{
	if (m_pControl)
		m_pControl->play();

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	if (m_pControl)
		m_pControl->pause();

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	if (m_pControl && m_pCore)
	{
		WMPPlayState	state;
		if (SUCCEEDED(m_pCore->get_playState(&state)))
		{
			if (state == wmppsPlaying)
				m_pControl->pause();
			else
				m_pControl->play();
		}
	}

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	if (m_pControl)
		m_pControl->stop();

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	if (m_pControl)
		m_pControl->previous();

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	if (m_pControl)
		m_pControl->next();

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	if (m_pControl)
		m_pControl->put_currentPosition(((double)nMsPos) / 1000);

	return TRUE;
}

BOOL CPlayerImp::Init()
{
	HRESULT		hr;

	assert(m_pCore);

	if (!m_pCore)
		return FALSE;

	hr = m_pCore->get_controls(&m_pControl);

	if (!SUCCEEDED(hr))
	{
		g_MLService.Log("Get WMP control interface FAILED!");
		return FALSE;
	}

	g_MLService.AddWndCloseto(NULL, "WMPlayerApp", NULL);
	g_MLService.AddWndCloseto(NULL, "WMP Skin Host", NULL);

	g_wndInfoQuery.Create(g_hInstance);

	OnMediaChanged();

	return TRUE;
}

void CPlayerImp::Quit()
{
	g_wndInfoQuery.Destroy();

	m_pControl.Release();
}

void CPlayerImp::OnMediaChanged()
{
	CComPtr<IWMPMedia>	pMedia;
	WCHAR		szSrcMedia[MAX_PATH];
	WCHAR		szArtist[MAX_PATH];
	WCHAR		szAlbum[MAX_PATH];
	WCHAR		szTitle[MAX_PATH];
	DWORD		dwSongLength = 0;

	if (!m_pControl)
		return;

	EmptyStr(szSrcMedia);
	EmptyStr(szArtist);
	EmptyStr(szAlbum);
	EmptyStr(szTitle);

	HRESULT hr = m_pControl->get_currentItem(&pMedia);
	if (SUCCEEDED(hr) && pMedia)
	{
		double	dbDuration = 0;
		BSTR	pbstrSourceUrl;
		BSTR	pbstrItemName, pbstrAr = NULL, pbstrTi = NULL, pbstrAl = NULL;

		pMedia->get_duration(&dbDuration);
		dwSongLength = (DWORD)(dbDuration * 1000);

		hr = pMedia->get_sourceURL(&pbstrSourceUrl);
		if (SUCCEEDED(hr))
		{
			wcscpy_safe(szSrcMedia, CountOf(szSrcMedia), pbstrSourceUrl);
		}

		pbstrItemName = SysAllocString(L"Artist");
		hr = pMedia->getItemInfo(pbstrItemName, &pbstrAr);
		SysFreeString(pbstrItemName);
		if (SUCCEEDED(hr))
		{
			wcscpy_safe(szArtist, CountOf(szArtist), pbstrAr);
			SysFreeString(pbstrAr);
		}

		pbstrItemName = SysAllocString(L"Album");
		hr = pMedia->getItemInfo(pbstrItemName, &pbstrAl);
		SysFreeString(pbstrItemName);
		if (SUCCEEDED(hr))
		{
			wcscpy_safe(szAlbum, CountOf(szAlbum), pbstrAl);
			SysFreeString(pbstrAl);
		}

		pbstrItemName = SysAllocString(L"Title");
		hr = pMedia->getItemInfo(pbstrItemName, &pbstrTi);
		SysFreeString(pbstrItemName);
		if (SUCCEEDED(hr))
		{
			wcscpy_safe(szTitle, CountOf(szTitle), pbstrTi);
			SysFreeString(pbstrTi);
		}
	}

	MLMediaInfo		mediaInfo;
	mediaInfo.szMediaUrl = szSrcMedia;
	mediaInfo.szArtist = szArtist;
	mediaInfo.szAlbum = szAlbum;
	mediaInfo.szTitle = szTitle;
	mediaInfo.nMediaLength = dwSongLength;

	g_MLService.OnMediaInfoChanged(&mediaInfo);
}

void CPlayerImp::SetWMPCore(IWMPCore *pCore)
{
	ReleaseWMPCore();

	m_pCore = pCore;
}

void CPlayerImp::ReleaseWMPCore()
{
	m_pCore.Release();
}

void CPlayerImp::OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan)
{
	g_wndInfoQuery.SetUpdatePosTimer(nPlayPosUpdateSpan);
}
