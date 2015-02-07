#include "stdafx.h"
#include "wmpplug.h"
#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"

CPlayerImp		g_Player;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
}

CPlayerImp::~CPlayerImp()
{
}

void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = (WND_HANDLE)FindWindow(_T("WMPlayerApp"), NULL);
	playerInfo.startupFlags |= ML_SUPPORT_EMBEDDED | ML_EMBEDDED_WND_IMMOVABLE;

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Windows Media Player");
}

void CPlayerImp::OnConnected()
{
	assert(m_pCore);
	if (!m_pCore || !m_pControl)
		return;

	g_MLService.AddWndCloseto(NULL, L"WMPlayerApp", NULL);
	g_MLService.AddWndCloseto(NULL, L"WMP Skin Host", NULL);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);

	SwitchToEmbeddedSkin();

	// Update Media info
	OnMediaChanged();

	// Update player state
	g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerImp::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
}

void CPlayerImp::Play()
{
	if (m_pControl)
		m_pControl->play();
}

void CPlayerImp::Pause()
{
	if (m_pControl)
		m_pControl->pause();
}

void CPlayerImp::PlayPause()
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
}

void CPlayerImp::Stop()
{
	if (m_pControl)
		m_pControl->stop();
}

void CPlayerImp::Prev()
{
	if (m_pControl)
		m_pControl->previous();
}

void CPlayerImp::Next()
{
	if (m_pControl)
		m_pControl->next();
}

void CPlayerImp::SeekTo(int nMsPos)
{
	if (m_pControl)
		m_pControl->put_currentPosition(((double)nMsPos) / 1000);
}

void CPlayerImp::SwitchToEmbeddedSkin()
{
	PlayerSkinInfo	skinInfo;

	memset(&skinInfo, 0, sizeof(skinInfo));
	skinInfo.bEmbeddedSkin = true;
	skinInfo.bClrThemeValid = false;
	skinInfo.hWndToAttach = (WND_HANDLE)m_hWndVis;

	g_MLService.SwitchToEmbeddedSkin(skinInfo);
}

void CPlayerImp::EndEmbeddedSkin()
{
}

PLAYER_STATE CPlayerImp::GetPlayerState()
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

DWORD CPlayerImp::GetPlayPos()
{
	double	dbTime = 0;

	assert(m_pControl);
	if (!m_pControl)
		return 0;

	HRESULT hr = m_pControl->get_currentPosition(&dbTime);

	return (DWORD)(dbTime * 1000);
}

void CPlayerImp::OnMediaChanged()
{
	if (!m_pControl || !g_MLService.IsValid())
		return;

	WCHAR		szSrcMedia[MAX_PATH];
	WCHAR		szArtist[MAX_PATH];
	WCHAR		szAlbum[MAX_PATH];
	WCHAR		szTitle[MAX_PATH];
	DWORD		dwSongLength = 0;

	EmptyStr(szSrcMedia);
	EmptyStr(szArtist);
	EmptyStr(szAlbum);
	EmptyStr(szTitle);

	CComPtr<IWMPMedia>	pMedia;
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

	g_MLService.OnMediaInfoChanged(szSrcMedia, szArtist, szAlbum, szTitle, dwSongLength);
}

void CPlayerImp::SetWMPCore(IWMPCore *pCore)
{
	ReleaseWMPCore();

	if (!pCore)
		return;

	m_pCore = pCore;

	HRESULT hr = m_pCore->get_controls(&m_pControl);
	if (!SUCCEEDED(hr))
	{
		g_MLService.Log("Get WMP control interface FAILED!");
		return;
	}

	// Get crossfade time:
	// if (RegGetProfileInt(HKEY_CURRENT_USER, _T("Software\\Microsoft\\MediaPlayer\\Preferences\\EqualizerSettings"), _T("CrossFade"), 0))
	//	m_nCrossFadeTime = RegGetProfileInt(HKEY_CURRENT_USER, _T("Software\\Microsoft\\MediaPlayer\\Preferences\\EqualizerSettings"), _T("CrossFadeWindow"), 0);
}

void CPlayerImp::ReleaseWMPCore()
{
	m_pCore.Release();
}

void CPlayerImp::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		// update current play time.
		g_MLService.OnPlayPosChanged(g_Player.GetPlayPos());
	}
}

void WmpPluginInit(HINSTANCE hInstance)
{
	RunMiniLyrics(hInstance, FindWindow(_T("WMPlayerApp"), NULL),
		&g_Player);
}

void WmpPluginQuit()
{
	DisconnectMiniLyrics(&g_Player);
}
