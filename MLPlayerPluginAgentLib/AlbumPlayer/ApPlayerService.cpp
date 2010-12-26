#include "StdAfx.h"
#include "../../MLPluginLib/MLPlugin.h"
#include "ApPlayerService.h"

namespace AlbumPlayerAgent
{

// 3.1.1	Remote control
#define WM_REMOTE_CONTROL		(WM_USER + 10)
// wparam:
#define RCMD_PLAYPAUSE			1
#define RCMD_STOP				2
#define RCMD_NEXT				3
#define RCMD_PREV				4
#define RCMD_PLAY				5
#define RCMD_PAUSE				6
#define RCMD_SEEK				9		// lparam: Position in ms  

// 3.1.2	Subscribe for notifications
#define WM_SUBSCRIBE_FOR_NOTIFICATIONS	(WM_USER + 11)

// 3.2.1	Notify status
#define WM_AP_NOTIFY		(WM_USER + 1)
#define N_POS_CHANGE		6		// lparam		-> progress in milliseconds
#define N_PLAY_STATE		1
// lparam = 0	-> stopped
// lparam = 1	-> playing
// lparam = 2 	-> paused
#define N_SETTINGS_CHANGE	20		// lparam = 0      -> preferences changed      lparam = 1      -> new skin applied

#define SZ_AP_CREATE_PANEL		"<?xml version=\"1.0\" encoding=\"utf-8\"?><AlbumPlayerCommand id=\"CreatePluginPanel\"><Plugin name=\"MiniLyrics\"></Plugin></AlbumPlayerCommand>"
#define SZ_AP_CLOSE_PANEL		"<?xml version=\"1.0\" encoding=\"utf-8\"?><AlbumPlayerCommand id=\"ClosePluginPanel\"><Plugin name=\"MiniLyrics\"></Plugin></AlbumPlayerCommand>"
#define SZ_AP_GET_SONG_INFO		"<?xml version=\"1.0\" encoding=\"utf-8\"?><AlbumPlayerCommand id=\"GetNowPlayingInfo\" />"
#define SZ_AP_GET_SETTINGS		"<AlbumPlayerCommand id=\"GetPluginSettings\"><Plugin name=\"MiniLyrics\"></Plugin></AlbumPlayerCommand>"

CPlayerService		g_Player;
HWND				g_hwndPlayer = NULL;

HWND GetPlayerWindow()
{
	return ::FindWindow(_T("TfrmPlayer"), NULL);
}

LPCTSTR SXNodeGetChildTextValue(SXNode *pParent, LPCTSTR szChildNodeName)
{
	SXNode		*pNodeVal;
	pNodeVal = pParent->GetChild(szChildNodeName);
	if (pNodeVal)
		return pNodeVal->strContent.c_str();

	return NULL;
}

bool GetColorValueEx(SXNode *pNode, LPCTSTR szName, COLORREF &clr)
{
	LPCTSTR str = SXNodeGetChildTextValue(pNode, szName);
	if (!str)
		return false;

	if (str[0] == '0' && str[1] == '0' && _tcslen(str) == 8)
		str += 2;
	COLORREF c = GetColorValue(str, 0xFFFFFFFF);
	if (c == 0xFFFFFFFF)
		return false;

	clr = RGB(GetBValue(c), GetGValue(c), GetRValue(c));
	return true;
}

void GetColorThemeValue(SXNode *pNode, PlayerSkinInfo &skinInfo)
{
	skinInfo.bClrThemeValid = false;

	if (GetColorValueEx(pNode, _T("BackgroundColor"), skinInfo.clrBackground)
		&& GetColorValueEx(pNode, _T("FontColor"), skinInfo.clrFontHighlight)
		&& GetColorValueEx(pNode, _T("SelectedTextBackgroundColor"), skinInfo.clrCurLineBg))
	{
		skinInfo.clrFontLowlight = AverageColor(skinInfo.clrBackground, skinInfo.clrFontHighlight);
		skinInfo.bClrThemeValid = true;
	}
}

//////////////////////////////////////////////////////////////////////

CPlayerService::CPlayerService()
{
	m_state = PS_STOPED;
	memset(&m_skinInfo, 0, sizeof(m_skinInfo));
	m_nPosNotify = 0;
}

CPlayerService::~CPlayerService()
{
}

void CPlayerService::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = GetPlayerWindow();
	playerInfo.startupFlags |= ML_SUPPORT_EMBEDDED | ML_EMBEDDED_WND_IMMOVABLE;

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"AlbumPlayer");
}

void CPlayerService::OnConnected()
{
	g_hwndPlayer = GetPlayerWindow();
	g_MLService.AddWndCloseto(NULL, L"TfrmPlayer", NULL);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);

	if (g_hwndPlayer)
	{
		SendMessage(g_hwndPlayer, WM_SUBSCRIBE_FOR_NOTIFICATIONS, (WPARAM)m_hWnd, WM_AP_NOTIFY);
		// SendAPCopyDataMsg(SZ_AP_CREATE_PANEL);
		// SendAPCopyDataMsg(SZ_AP_CLOSE_PANEL);
	}

	MSG			msg;
	for (int i = 0; i < 10 && !m_skinInfo.hWndToAttach && GetMessage(&msg, NULL, 0, 0); i++) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// SwitchToEmbeddedSkin();

	// Update Media info
	SendAPCopyDataMsg(SZ_AP_GET_SONG_INFO);

	// Update player state
	// g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerService::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);

	SendAPCopyDataMsg(SZ_AP_CLOSE_PANEL);
}

// control player
void CPlayerService::Play()
{
	::PostMessage(g_hwndPlayer, WM_REMOTE_CONTROL, RCMD_PLAY, 0);
}

void CPlayerService::Pause()
{
	::PostMessage(g_hwndPlayer, WM_REMOTE_CONTROL, RCMD_PAUSE, 0);
}

void CPlayerService::PlayPause()
{
	::PostMessage(g_hwndPlayer, WM_REMOTE_CONTROL, RCMD_PLAYPAUSE, 0);
}

void CPlayerService::Stop()
{
	::PostMessage(g_hwndPlayer, WM_REMOTE_CONTROL, RCMD_STOP, 0);
}

void CPlayerService::Prev()
{
	::PostMessage(g_hwndPlayer, WM_REMOTE_CONTROL, RCMD_PREV, 0);
}

void CPlayerService::Next()
{
	::PostMessage(g_hwndPlayer, WM_REMOTE_CONTROL, RCMD_NEXT, 0);
}

void CPlayerService::SeekTo(int nMsPos)
{
	::PostMessage(g_hwndPlayer, WM_REMOTE_CONTROL, RCMD_SEEK, nMsPos);
}

void CPlayerService::SwitchToEmbeddedSkin()
{
	if (m_skinInfo.hWndToAttach != NULL)
	{
		SendAPCopyDataMsg(SZ_AP_CLOSE_PANEL);
		m_skinInfo.hWndToAttach = NULL;
	}

	SendAPCopyDataMsg(SZ_AP_CREATE_PANEL);
}

void CPlayerService::EndEmbeddedSkin()
{
	ActivateWindow(g_hwndPlayer);

	SendAPCopyDataMsg(SZ_AP_CLOSE_PANEL);
}

void CPlayerService::OnCopyDataCheckSongTitle(COPYDATASTRUCT *pCopyDataStruct)
{
	LPCSTR		str;
	CSimpleXML	xml;

	str = (LPCSTR)pCopyDataStruct->lpData;
	if (!str)
		return;

	if (!xml.ParseData((LPVOID)str, min(pCopyDataStruct->cbData, strlen(str))))
		return;

	/*	<AlbumPlayerReply id="GetCurrentPlaylist">
		  <Name>Playlist Name</Name>
		  <Track albumId="423" number="1">
			<Name></Name>
			<Artist></Artist>
			<FileName></FileName>
			<Length></Length>
		  </Track>
		</AlbumPlayerReply>

		<AlbumPlayerReply id="CreatePluginPanel">
		  <ChildWindowHandle>14731</ ChildWindowHandle>
		  <BackgroundColor>0045E345</ BackgroundColor>
		  <FontColor>00000000</ FontColor>
		  <SelectedTextBackgroundColor>00345677</ SelectedTextBackgroundColor>
		</AlbumPlayerReply> */
	SXNode		*pNode;
	LPCTSTR		szCmdID;

	if (_tcscmp(xml.m_pRoot->strName.c_str(), _T("AlbumPlayerReply")) != 0)
		return;

	szCmdID = xml.m_pRoot->GetPropertySafe(_T("id"));
	if (_tcscmp(szCmdID, _T("CreatePluginPanel")) == 0)
	{
		m_skinInfo.hWndToAttach = NULL;

		GetColorThemeValue(xml.m_pRoot, m_skinInfo);

		LPCTSTR str = SXNodeGetChildTextValue(xml.m_pRoot, _T("ChildWindowHandle"));
		if (str)
		{
			m_skinInfo.hWndToAttach = (HWND)_ttoi(str);
			m_skinInfo.bEmbeddedSkin = true;
			g_MLService.SwitchToEmbeddedSkin(m_skinInfo);
		}
		return;
	}

	if (_tcscmp(szCmdID, _T("GetNowPlayingInfo")) == 0)
	{
		pNode = xml.m_pRoot->GetChild(_T("Track"));
		if (!pNode)
			return;

		// WCHAR			szArtist[256], szTitle[256], szAlbum[256];
		// WCHAR			szSrcMedia[MAX_PATH];
		int				nMediaLength = 0;
		LPCTSTR			szArtist, szTitle, szAlbum, szSrcMedia;

#define READ_STR_FEILD(szFieldName, szValue)	\
		szValue = SXNodeGetChildTextValue(pNode, szFieldName);	\
		if (!szValue)											\
			szValue = _T("");

		READ_STR_FEILD(_T("FileName"), szSrcMedia);
		READ_STR_FEILD(_T("Artist"), szArtist);
		READ_STR_FEILD(_T("Album"), szAlbum);
		READ_STR_FEILD(_T("Name"), szTitle);

		LPCTSTR str = SXNodeGetChildTextValue(pNode, _T("Length"));
		if (str)
			nMediaLength = _ttoi(str);

		// m_dwTimeBeg = GetTickCount() - m_nPosNotify;
		// m_dwTimeBeg = GetTickCount() - m_nPos;
		g_MLService.OnMediaInfoChanged(szSrcMedia, szArtist, szAlbum, szTitle, nMediaLength);

		return;
	}

	/*
	<AlbumPlayerReply id="GetPluginSettings">
	  <BackgroundColor>FFFFFF</BackgroundColor>
	  <FontColor>000000</FontColor>
	  <SelectedTextBackgroundColor>888888</ SelectedTextBackgroundColor>
	</AlbumPlayerReply>
	*/
	if (_tcscmp(szCmdID, _T("GetPluginSettings")) == 0)
	{
		GetColorThemeValue(xml.m_pRoot, m_skinInfo);

		g_MLService.OnNotifyPlayerSkinChanged(m_skinInfo);
	}	
}

void CPlayerService::SendAPCopyDataMsg(LPCSTR szXML)
{
	COPYDATASTRUCT		data;
	data.cbData = strlen(szXML) + 1;
	data.lpData = (LPVOID)szXML;
	data.dwData = 0;
	SendMessage(g_hwndPlayer, WM_COPYDATA, (WPARAM)GetHandle(), (LPARAM)&data);
}

void CPlayerService::OnPlayTimeChanged(DWORD dwTimeCur)
{
	m_nPos = dwTimeCur;
	g_MLService.OnPlayPosChanged(dwTimeCur);
}

int _abs(int number)
{
	return (number >= 0 ? number : -number);
}

void CPlayerService::OnPlayTimeChangeCheck()
{
	DWORD		nPos, nPosQuery;
	DWORD		dwTimeNew;

	static		bool bPauseCheck = false;
	static		DWORD nPosOld = 0;

	dwTimeNew = GetTickCount();
	nPos = dwTimeNew - m_dwTimeBeg;
	nPosQuery = m_nPosNotify;

	if (bPauseCheck)
	{
		if (nPosQuery != nPosOld)
		{
			bPauseCheck = false;
			m_dwTimeBeg = dwTimeNew - nPosQuery;
			OnPlayTimeChanged(nPosQuery);
		}
		return;
	}

	if (_abs(nPos - nPosQuery) > 1000 || nPosQuery == 0)
	{
		// use Seek or pause? let's adjust it.
		m_dwTimeBeg = dwTimeNew - nPosQuery;
		OnPlayTimeChanged(nPosQuery);
		nPosOld = nPosQuery;
		bPauseCheck = true;
	}
	else
		OnPlayTimeChanged(nPos);
}

void CPlayerService::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		OnPlayTimeChangeCheck();
	}
}

LRESULT CPlayerService::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_AP_NOTIFY)
	{
		if (wParam == N_POS_CHANGE)
		{
			m_nPosNotify = lParam;
		}
		else if (wParam == N_PLAY_STATE)
		{
			if (lParam == 1)
				m_state = PS_PLAYING;
			else if (lParam == 2)
				m_state = PS_PAUSED;
			else
				m_state = PS_STOPED;
		}
		else if (wParam == N_SETTINGS_CHANGE)
		{
			SendAPCopyDataMsg(SZ_AP_GET_SETTINGS);
		}

		return 1;
	}
	else if (uMsg == WM_COPYDATA)
	{
		if (CPlayerServiceBase::OnCopyData(wParam, (COPYDATASTRUCT*)lParam))
			return 1;

		OnCopyDataCheckSongTitle((COPYDATASTRUCT*)lParam);
		return 1;
	}
	else
		return CPlayerServiceBase::WndProc(uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////

CPlayerPluginAgent::CPlayerPluginAgent() : IPlayerPluginAgent(&g_Player)
{
}

bool CPlayerPluginAgent::IsPlayerRunning()
{
	HWND hWndPlayer = GetPlayerWindow();
	return hWndPlayer != NULL;
}

}
