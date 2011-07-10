#include "../MLPluginLib/mlplugin.h"

#define WA_DLG_IMPLEMENT
#include "wa_dlg.h"
#include "wa_ipc.h"
#include "Gen.h"
#include "wa_wrap.h"

#include "PlayerImp.h"

#define SZ_REG_MINILYRICS		_T("Software\\ZikiPlayer")
#define SZ_MINILYRICS			_T("MiniLyrics")

// function prototypes
void config();
void quit();
int init();

WNDPROC	g_pOrgWinampProc = NULL;

winampGeneralPurposePlugin plugin = {GPPHDR_VER, SZ_MINILYRICS, init, config, quit, };

CPlayerImp			g_Player;

bool g_bKmplayer = false;
bool g_bWinamp = false;
bool g_bMediaMonkey = false;
const UINT g_msgMediaMonkeyEmbeder = RegisterWindowMessage(_T("MediaMonkeyEmbederToMiniLyrics"));

inline HWND GetPlayerWnd()
{
	return plugin.hwndParent;
}

WNDPROC SetWindowProc(HWND hWnd, WNDPROC wndProc)
{
	if (IsWindowUnicode(hWnd))
		return (WNDPROC)SetWindowLongW(hWnd, GWL_WNDPROC, (LONG)wndProc);
	else
		return (WNDPROC)SetWindowLongA(hWnd, GWL_WNDPROC, (LONG)wndProc);
}

UINT Get_WMCOMMAND_Id(UINT fallback)
{
	DWORD	dwVersion = 0;

	WAGetVersion(GetPlayerWnd(), dwVersion);

	if (dwVersion >= 0x5010)
	{
		return SendMessage(GetPlayerWnd(), WM_WA_IPC, 0, IPC_REGISTER_LOWORD_COMMAND);
	}

	return fallback;
}

PLAYER_STATE GetPlayerState()
{
	int		nPlayState;
	if (!WAIsPlaying(GetPlayerWnd(), nPlayState))
		return PS_STOPED;

	switch (nPlayState)
	{
	case 0: return PS_STOPED;
	case 1: return PS_PLAYING;
	case 3: return PS_PAUSED;
	default:
		break;
	}
	return PS_STOPED;
}


class CWinampMenu
{
public:
	CWinampMenu() {
		m_hMenuMain = (HMENU)SendMessage(GetPlayerWnd(), WM_WA_IPC, 0, IPC_GET_HMENU);
		if (ID_ML_MENU_ITEM == 0)
			ID_ML_MENU_ITEM = Get_WMCOMMAND_Id(27562);
	}

	void CheckMiniLyrics(bool bChecked) {
		CheckMenuItem(m_hMenuMain, ID_ML_MENU_ITEM, MF_BYCOMMAND | (bChecked ? MF_CHECKED : MF_UNCHECKED));
	}

	int GetMiniLyricsItemCount() {
		MENUITEMINFO	itemInfo;
		TCHAR			szString[256];
		int				count = 0;

		for (int i = 0; i < 256; i++)
		{
			memset(&itemInfo, 0, sizeof(itemInfo));

			itemInfo.cbSize = sizeof(itemInfo);
			itemInfo.fMask = MIIM_TYPE;
			itemInfo.dwTypeData = szString;
			itemInfo.cch = CountOf(szString);
			szString[0] = '\0';

			// get menu item by pos.
			if (!GetMenuItemInfo(m_hMenuMain, i, TRUE, &itemInfo))
				break;

			if (_tcsstr(szString, SZ_MINILYRICS) != NULL)
				count++;
		}

		return count;
	}

	void AddMiniLyrics() {
		int pos = 0, seperatorCount = 0;
		for (; seperatorCount < 3 && pos < 50; pos++)
		{
			int prior_item = GetMenuItemID(m_hMenuMain, pos);
			if (prior_item == 0)
				seperatorCount++;
		}

		// now add the menu item using prior_item as the item to insert after
		InsertMenu(m_hMenuMain, pos - 1, MF_BYPOSITION, ID_ML_MENU_ITEM, SZ_MINILYRICS);
		CheckMiniLyrics(g_Player.m_bWorking);

		// SendMessage(GetPlayerWnd(), WM_WA_IPC, (bAdd ? 1 : -1), IPC_ADJUST_OPTIONSMENUPOS);
	}

	void RemoveMiniLyrics() {
		DeleteMenu(m_hMenuMain, ID_ML_MENU_ITEM, MF_BYCOMMAND);
	}

	static UINT ID_ML_MENU_ITEM;

protected:
	HMENU		m_hMenuMain;

};

UINT CWinampMenu::ID_ML_MENU_ITEM = 0;

bool WAGetSourceMedia(wchar_t *wszUrl, int nBufSize)
{
	DWORD		dwIndex;

	EmptyStr(wszUrl);

	if (!WAGetListPos(GetPlayerWnd(), dwIndex))
		return false;

	LPWSTR		wszSong = NULL;
	if (WAGetPlayListFileW(GetPlayerWnd(), dwIndex, wszSong) && (DWORD)wszSong > 0xFFFF)
	{
		wcscpy_safe(wszUrl, nBufSize, wszSong);
		return true;
	}

	char		*szSong = NULL;
	if (WAGetPlayListFile(GetPlayerWnd(), dwIndex, szSong) && szSong)
	{
		MbcsToUCS2(szSong, -1, wszUrl, nBufSize, CP_ACP);
		return true;
	}

	return false;
}

// rule: 1) http://xxxx	
//       2) without the extention of song file.
bool IsShoutcastMedia(LPCWSTR szMedia)
{
	if (_wcsnicmp(szMedia, L"http://", 7) == 0)
		return true;
	else if (_wcsnicmp(szMedia, L"uvox://", 7) == 0)
		return true;

	return false;
}

BOOL AnalysisMediaTitle(LPCWSTR szFullTitleNew, wchar_t szArtist[]/* out */, wchar_t szTitle[]/* out */)
{
	LPCWSTR		szEndArtist;
	LPCWSTR		szBegTitle;

	// 作者和标题之间是以：' - ' or ' － ' or '-'为分隔
	szEndArtist = szBegTitle = wcsstr(szFullTitleNew, L" - ");
	if (szBegTitle)
		szBegTitle += 3;	// strlen(_T(" - "));
	else
	{
		szEndArtist = szBegTitle = wcschr(szFullTitleNew, L'-');
		if (szBegTitle)
			szBegTitle++;	// -
	}

	if (szEndArtist == NULL)
	{
		// 认为 szFullTitle 都是 Title, Artist 为空
		szBegTitle = szFullTitleNew;
		szArtist[0] = '\0';
	}
	else
	{
		wcsncpy_safe(szArtist, MAX_PATH, szFullTitleNew, int(szEndArtist - szFullTitleNew));
	}

	wcscpy_safe(szTitle, MAX_PATH, szBegTitle);

	return TRUE;
}

bool GetMediaInfoW(LPCWSTR szFile, wchar_t szArtist[], wchar_t szAlbum[], wchar_t szTitle[])
{
	extendedFileInfoStructW	info;
	int						nRet;

	info.filename = szFile;
	info.retlen = MAX_PATH - 1;

	info.metadata = L"artist";
	info.ret = szArtist;
	memset(szArtist, 0, sizeof(wchar_t) * MAX_PATH);
	nRet = SendMessage(GetPlayerWnd(), WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW);

	info.metadata = L"title";
	memset(szTitle, 0, sizeof(wchar_t) * MAX_PATH);
	info.ret = szTitle;
	nRet = SendMessage(GetPlayerWnd(), WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW);

	info.metadata = L"album";
	memset(szAlbum, 0, sizeof(wchar_t) * MAX_PATH);
	info.ret = szAlbum;
	nRet = SendMessage(GetPlayerWnd(), WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW);

	return !IsEmptyString(szArtist) || !IsEmptyString(szAlbum) || !IsEmptyString(szTitle);
}

void UCS2ToMBCS(const WCHAR *str, int nLen, char *strOut, int nOut, int nEncodingIdx)
{
	int			n;

	n = WideCharToMultiByte(nEncodingIdx, 0,
		str, nLen, strOut, nOut, NULL, NULL);

	if (n < nOut)
		strOut[n] = '\0';
}

bool GetMediaInfo(LPCWSTR szFile, wchar_t szArtist[], wchar_t szAlbum[], wchar_t szTitle[])
{
	extendedFileInfoStruct	info;
	int						nRet;

	char		szFileA[MAX_PATH], szArtistA[MAX_PATH], szAlbumA[MAX_PATH], szTitleA[MAX_PATH];

	UCS2ToMBCS(szFile, -1, szFileA, CountOf(szFileA), CP_ACP);

	info.filename = szFileA;
	info.retlen = MAX_PATH - 1;

	info.metadata = "artist";
	info.ret = szArtistA;
	memset(szArtistA, 0, sizeof(szArtistA));
	nRet = SendMessage(GetPlayerWnd(), WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFO);

	info.metadata = "title";
	memset(szTitleA, 0, sizeof(szTitleA));
	info.ret = szTitleA;
	nRet = SendMessage(GetPlayerWnd(), WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFO);

	info.metadata = "album";
	memset(szAlbumA, 0, sizeof(szAlbumA));
	info.ret = szAlbumA;
	nRet = SendMessage(GetPlayerWnd(), WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFO);

	if (IsEmptyString(szArtistA) && IsEmptyString(szAlbumA) && IsEmptyString(szTitleA))
		return false;

	MbcsToUCS2(szArtistA, -1, szArtist, MAX_PATH);
	MbcsToUCS2(szAlbumA, -1, szAlbum, MAX_PATH);
	MbcsToUCS2(szTitleA, -1, szTitle, MAX_PATH);

	return true;
}

void SplitAlbumInfoOfKMPlayer(LPWSTR szTitleAlbum, wchar_t szAlbum[], int nLenAlbum)
{
	int		nLenStr = wcslen(szTitleAlbum);
	LPWSTR	szAlbumBeg = wcsrchr(szTitleAlbum, '(');
	if (szAlbumBeg && szTitleAlbum[nLenStr - 1] == ')')
	{
		szAlbumBeg[0] = '\0';
		szTitleAlbum[nLenStr - 1] = '\0';
		wcscpy_safe(szAlbum, nLenAlbum, szAlbumBeg + 1);
	}
}

void OnMediaChanged(LPWSTR szFile, LPCWSTR szMediaTitle)
{
	wchar_t		szArtist[MAX_PATH] = L"", szAlbum[MAX_PATH] = L"", szTitle[MAX_PATH] = L"";

	if (!GetMediaInfoW(szFile, szArtist, szAlbum, szTitle))
		GetMediaInfo(szFile, szArtist, szAlbum, szTitle);

	if (IsEmptyString(szArtist) && IsEmptyString(szTitle))
	{
		// Get artist and title from szMediaTitle
		AnalysisMediaTitle(szMediaTitle, szArtist, szTitle);
		if (g_bKmplayer)
		{
			// For KMPlayer, szTitle = title(album)
			SplitAlbumInfoOfKMPlayer(szTitle, szAlbum, CountOf(szAlbum));
		}
	}

	DWORD	dwLength = 0;
	if (WAGetCurSongLength(GetPlayerWnd(), dwLength))
		dwLength *= 1000;

	for (int i = 0; i < 10 && int(dwLength) <= 0; i++)
	{
		Sleep(50);
		if (WAGetCurSongLength(GetPlayerWnd(), dwLength))
			dwLength *= 1000;
	}

	g_MLService.OnMediaInfoChanged(szFile, szArtist, szAlbum, szTitle, dwLength, g_Player.HasVideo());
}

void RemovePlayingStatInfoInMediaTitle(LPWSTR szMediaTitle)
{
	// remove [buff %xx] in media title
	int	n = 0;
	if (szMediaTitle[0] == '[')
	{
		n = 1;
		while (szMediaTitle[n] != '\0' && szMediaTitle[n] !=']')
			n++;
		if (szMediaTitle[n] == ']')
		{
			n++;
			while (szMediaTitle[n] == ' ')
				n++;
			int	nMoveSize = wcslen(szMediaTitle + n) * sizeof(wchar_t);
			memmove(szMediaTitle, szMediaTitle + n, nMoveSize);
			szMediaTitle[nMoveSize] = '\0';
		}
	}
}

HWND FindMiniLyricsParentWnd(HWND hWndParent, HWND hWndMiniLyrics)
{
	HWND hWndNext = NULL;

	while (1)
	{
		hWndNext = FindWindowEx(hWndParent, hWndNext, NULL, NULL);
		if (!hWndNext)
			break;
		if (hWndNext == hWndMiniLyrics)
			return hWndParent;
		else
		{
			HWND hWnd = FindMiniLyricsParentWnd(hWndNext, hWndMiniLyrics);
			if (hWnd)
				return hWnd;
		}
	}

	return NULL;
}

bool IsMLEmbeddedInMediaMonkey()
{
	HWND hWndMiniLyrics = g_MLService.GetMainWindow();
	assert(hWndMiniLyrics);

	static HWND hWndMiniLyricsParent = NULL;
	if (hWndMiniLyricsParent
		&& FindMiniLyricsParentWnd(hWndMiniLyricsParent, hWndMiniLyrics))
		return true;

	HWND	hWndMediaMonkey = NULL;
	while (1)
	{
		hWndMediaMonkey = FindWindowEx(NULL, hWndMediaMonkey, _T("TFMainWindow"), NULL);
		if (!hWndMediaMonkey)
			break;
		hWndMiniLyricsParent = FindMiniLyricsParentWnd(hWndMediaMonkey, hWndMiniLyrics);
		if (hWndMiniLyricsParent)
			return true;
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK HookWinampWnd(
							   HWND hWnd,      // handle to window
							   UINT uMsg,      // message identifier
							   WPARAM wParam,  // first message parameter
							   LPARAM lParam   // second message parameter
							   )
{
	if (!g_Player.m_bWorking)
	{
		if (uMsg == WM_COMMAND && LOWORD(wParam) == CWinampMenu::ID_ML_MENU_ITEM)
		{
			// Open MiniLyrics
			RunMiniLyrics(g_hInstance, plugin.hwndParent, &g_Player);
			return 0;
		}
		return CallWindowProc(g_pOrgWinampProc, hWnd, uMsg, wParam, lParam);
	}

	switch(uMsg) 
	{
	case WM_DESTROY:
		{
			// g_MLService.Quit will set g_pOrgWinampProc to NULL, so save g_pOrgProc.
			WNDPROC pOrigProc = g_pOrgWinampProc;
			g_MLService.OnDisconnected();
			return CallWindowProc(pOrigProc, hWnd, uMsg, wParam, lParam);
		}
		break;
	case WM_COMMAND:
		{
			if (LOWORD(wParam) == 0x9D63)
			{
				int		nRet;
				nRet = CallWindowProc(g_pOrgWinampProc, hWnd, uMsg, wParam, lParam); 

				g_Player.SkinOfPlayerChanged();

				return nRet;
			}
			else if (LOWORD(wParam) == CWinampMenu::ID_ML_MENU_ITEM)
			{
				// Open/close MiniLyrics
				g_MLService.OnDisconnected();
			}
		}
		break;
	case WM_ACTIVATEAPP:
		{
			LRESULT		result = CallWindowProc(g_pOrgWinampProc, hWnd, uMsg, wParam, lParam);
			g_MLService.OnNotifyPlayerActivate(wParam);
			return result;
		}
		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED || wParam == SIZE_RESTORED)
		{
			LRESULT		result = CallWindowProc(g_pOrgWinampProc, hWnd, uMsg, wParam, lParam);
			g_MLService.OnNotifyPlayerMinRestore(wParam);
			return result;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			HWND		hWndChain[5];
			hWndChain[0] = FindWindow("Winamp v1.x", NULL);
			hWndChain[1] = FindWindow("Winamp EQ", NULL);
			hWndChain[2] = FindWindow("Winamp PE", NULL);
			hWndChain[3] = FindWindow("Winamp Gen", "Winamp Library");
			hWndChain[4] = FindWindow("Winamp MB", NULL);
			g_MLService.OnNotifyBeforeTrackMove(hWndChain, CountOf(hWndChain));
		}
		break;
	case WM_MOVE:
		g_MLService.OnNotifyTrackMove((int)(short) LOWORD(lParam), (int)(short) HIWORD(lParam));
		break;
	default:
		break;
	};

	// Call Winamp Window Proc
	return CallWindowProc(g_pOrgWinampProc, hWnd, uMsg, wParam, lParam); 
}

int init()
{
	// Hook Main Window
	if (g_pOrgWinampProc == NULL)
		g_pOrgWinampProc = SetWindowProc(GetPlayerWnd(), HookWinampWnd);

	CWinampMenu	menu;
	menu.AddMiniLyrics();

	RunMiniLyrics(plugin.hDllInstance, plugin.hwndParent, &g_Player);

	return 0;
}

void config()
{
	g_MLService.Config(GetPlayerWnd());
}

void quit()
{
	if (g_pOrgWinampProc && IsWindow(GetPlayerWnd()) && GetWindowLong(GetPlayerWnd(), GWL_WNDPROC) == (LONG)HookWinampWnd)
	{
		SetWindowProc(GetPlayerWnd(), g_pOrgWinampProc);
		g_pOrgWinampProc = NULL;
	}

	CWinampMenu	menu;
	menu.RemoveMiniLyrics();

	if (g_Player.m_hWndPluginParent)
	{
		DestroyWindow(g_Player.m_hWndPluginParent);
		g_Player.m_hWndPluginParent = NULL;
	}

	DisconnectMiniLyrics(&g_Player);
}

extern "C" {

__declspec( dllexport ) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin()
{
	return &plugin;
}

}

void GetPlayerName(WCHAR *lpszBuffer, int nBufSize)
{
	TCHAR	szFile[MAX_PATH];
	if (!GetModuleFileName(NULL, szFile, CountOf(szFile)))
	{
		wcscpy_safe(lpszBuffer, nBufSize, L"Winamp5");
		return;
	}

	LPCTSTR szExeName = UrlGetName(szFile);
	if (strcmp(szExeName, "MediaMonkey.exe") == 0)
	{
		g_bMediaMonkey = true;
		wcscpy_safe(lpszBuffer, nBufSize, L"MediaMonkey");
	}
	else if (strcmp(szExeName, "AIMP2.exe") == 0)
		wcscpy_safe(lpszBuffer, nBufSize, L"AIMP2");
	else if (strcmp(szExeName, "KMPlayer.exe") == 0)
	{
		g_bKmplayer = true;
		wcscpy_safe(lpszBuffer, nBufSize, L"KMPlayer");
	}
	else
	{
		g_bWinamp = true;
		if (g_Player.IsWinamp5())
			wcscpy_safe(lpszBuffer, nBufSize, L"Winamp5");
		else
			wcscpy_safe(lpszBuffer, nBufSize, L"Winamp2");
	}
}

//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	m_hWndPluginParent = NULL;
	m_dwShoutcastTitleChangedPos = 0;
	m_state = PS_STOPED;

	m_bWorking = false;
}

CPlayerImp::~CPlayerImp()
{
}

void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.startupFlags |= ML_SUPPORT_MINIMIZE_RESTORE_WITH_PLAYER;
	playerInfo.hWndPlayer = plugin.hwndParent;

	GetPlayerName(playerInfo.playerName, CountOf(playerInfo.playerName));

	wcscpy_safe(playerInfo.embeddedSkinName, CountOf(playerInfo.embeddedSkinName), playerInfo.playerName);
	wcscat_safe(playerInfo.embeddedSkinName, CountOf(playerInfo.embeddedSkinName), L"'s Skin");

	if (IsWinamp5())
		playerInfo.startupFlags |= ML_SUPPORT_EMBEDDED | ML_EMBEDDED_WND_IMMOVABLE | ML_SUPPORT_CLASSIC_SKINS;
	else
		playerInfo.startupFlags |= ML_SUPPORT_EMBEDDED | ML_SUPPORT_CLASSIC_SKINS;
}

void CPlayerImp::OnConnected()
{
	DWORD dwPlayerVer;
	WAGetVersion(GetPlayerWnd(), dwPlayerVer);

	m_bWorking = true;

	m_state = (PLAYER_STATE)GetPlayerState();

	if (dwPlayerVer >= 0x3000)
	{
		g_MLService.AddWndCloseto(NULL, L"BaseWindow_RootWnd", L"Playlist Editor");
		g_MLService.AddWndCloseto(NULL, L"BaseWindow_RootWnd", L"Player Window");
		g_MLService.AddWndCloseto(NULL, L"BaseWindow_RootWnd", L"Winamp Library");
		g_MLService.AddWndCloseto(NULL, L"BaseWindow_RootWnd", L"Visualizations");
		g_MLService.AddWndCloseto(NULL, L"BaseWindow_RootWnd", L"Media Library");
		g_MLService.AddWndCloseto(NULL, L"BaseWindow_RootWnd", L"");
	}
	g_MLService.AddWndCloseto(NULL, L"Winamp Gen", L"Winamp Library");
	g_MLService.AddWndCloseto(NULL, L"Winamp v1.x", L"");
	g_MLService.AddWndCloseto(NULL, L"Winamp EQ", L"");
	g_MLService.AddWndCloseto(NULL, L"Winamp PE", L"");
	g_MLService.AddWndCloseto(NULL, L"Winamp MB", L"");

	CWinampMenu	menu;
	menu.CheckMiniLyrics(true);
	SetTimer(TIMER_VERIFY_ML_IN_WINAMP_MENU, DURATION_VERIFY_MLIN_WINAMP_MENU);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	SetTimer(TIMER_CHECK_MEDIA, TIME_OUT_UPDATE_MEDIA_INFO);
	if (g_bMediaMonkey)
		SetTimer(TIMER_MEDIA_MONKEY_EMBEDER, 5000);

	// Update Media info
	CheckMediaUpdate(true);

	// Update player state
	g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerImp::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_CHECK_MEDIA);
	if (g_bMediaMonkey)
		KillTimer(TIMER_MEDIA_MONKEY_EMBEDER);

	m_bWorking = false;

	if (IsWindow(m_hWndPluginParent))
	{
		SaveAttachedParentWndSizePos();
		DestroyWindow(m_hWndPluginParent);
		m_hWndPluginParent = NULL;
	}

	CWinampMenu	menu;
	menu.AddMiniLyrics();
	menu.CheckMiniLyrics(false);

	SetTimer(TIMER_VERIFY_ML_IN_WINAMP_MENU, DURATION_VERIFY_MLIN_WINAMP_MENU);
}

void CPlayerImp::Play()
{
	WAPlay(GetPlayerWnd());
}

void CPlayerImp::Pause()
{
	WAPlayPause(GetPlayerWnd());
}

void CPlayerImp::PlayPause()
{
	if (GetPlayerState() == PS_PLAYING)
		Pause();
	else
		Play();
}

void CPlayerImp::Stop()
{
	WAStop(GetPlayerWnd());
}

void CPlayerImp::Prev()
{
	WAPrev(GetPlayerWnd());
}

void CPlayerImp::Next()
{
	WANext(GetPlayerWnd());
}

void CPlayerImp::SeekTo(int nMsPos)
{
	int		nResult;

	WAJumpToTime(GetPlayerWnd(), nMsPos, nResult);
}

void CPlayerImp::SwitchToEmbeddedSkin()
{
	PlayerSkinInfo	skinInfo;

	PrepareCurSkinInfo(skinInfo);
	g_MLService.SwitchToEmbeddedSkin(skinInfo);

	if (m_hWndPluginParent == NULL)
		return;

	// here we set the window's title
	SetWindowText(m_hWndPluginParent, SZ_MINILYRICS);

	// shows the owner window and this dialog
	ShowWindow(m_hWndPluginParent, SW_SHOWNORMAL);

	SetTimer(TIMER_VERIFY_ML_IN_WINAMP_MENU, DURATION_VERIFY_MLIN_WINAMP_MENU);
}

void CPlayerImp::EndEmbeddedSkin()
{
	CWinampMenu	menu;
	menu.AddMiniLyrics();

	SaveAttachedParentWndSizePos();
	DestroyWindow(m_hWndPluginParent);
	m_hWndPluginParent = NULL;

}

//////////////////////////////////////////////////////////////////////////

void CPlayerImp::SkinOfPlayerChanged()
{
	PlayerSkinInfo	skinInfo;

	PrepareCurSkinInfo(skinInfo);
	skinInfo.bEmbeddedSkin = false;
	g_MLService.OnNotifyPlayerSkinChanged(skinInfo);
}

void CPlayerImp::PrepareCurSkinInfo(PlayerSkinInfo &skinInfo)
{
	memset(&skinInfo, 0, sizeof(skinInfo));
	skinInfo.bEmbeddedSkin = true;
	skinInfo.bClrThemeValid = true;
	skinInfo.hWndToAttach = GetAttachedParentWnd();
	WAGetSkin(GetPlayerWnd(), skinInfo.skinPath);

	WADlg_init(plugin.hwndParent);

	skinInfo.clrBackground = WADlg_getColor(WADLG_ITEMBG);
	skinInfo.clrFontHighlight = WADlg_getColor(WADLG_ITEMFG);
	skinInfo.clrFontLowlight = AverageColor(skinInfo.clrBackground, skinInfo.clrFontHighlight);
	skinInfo.clrCurLineBg = WADlg_getColor(WADLG_SELCOLOR);

	WADlg_close();
}

HWND CPlayerImp::GetAttachedParentWnd()
{
	if (m_hWndPluginParent)
		return m_hWndPluginParent;

	if (IsWinamp5())
	{
		// this allocates the memory pointer so the structure can be accessed
		// by winamp (otherwise it will crash)
		if (!m_embeddedWndState)
		{
			m_embeddedWndState = (embedWindowState*)GlobalAlloc(GPTR, sizeof(embedWindowState));
			memset(m_embeddedWndState, 0, sizeof(embedWindowState));
		}

		ReadAttachedParentWndSizePos(m_embeddedWndState->r);

		// parent needs to be shown
		m_hWndPluginParent = (HWND)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)m_embeddedWndState, IPC_GET_EMBEDIF);
	}

	return m_hWndPluginParent;
}

void CPlayerImp::SaveAttachedParentWndSizePos()
{
	if (!m_hWndPluginParent || !IsWindow(m_hWndPluginParent))
		return;

	RECT	rc;
	if (!GetWindowRect(m_hWndPluginParent, &rc))
		return;

	RegWriteProfileInt(HKEY_LOCAL_MACHINE, SZ_REG_MINILYRICS, _T("winamp5_rc_left"), rc.left);
	RegWriteProfileInt(HKEY_LOCAL_MACHINE, SZ_REG_MINILYRICS, _T("winamp5_rc_top"), rc.top);
	RegWriteProfileInt(HKEY_LOCAL_MACHINE, SZ_REG_MINILYRICS, _T("winamp5_rc_right"), rc.right);
	RegWriteProfileInt(HKEY_LOCAL_MACHINE, SZ_REG_MINILYRICS, _T("winamp5_rc_bottom"), rc.bottom);
}

void CPlayerImp::ReadAttachedParentWndSizePos(RECT &rc)
{
	rc.left = RegGetProfileInt(HKEY_LOCAL_MACHINE, SZ_REG_MINILYRICS, _T("winamp5_rc_left"), 0);
	rc.top = RegGetProfileInt(HKEY_LOCAL_MACHINE, SZ_REG_MINILYRICS, _T("winamp5_rc_top"), 0);
	rc.right = RegGetProfileInt(HKEY_LOCAL_MACHINE, SZ_REG_MINILYRICS, _T("winamp5_rc_right"), 0);
	rc.bottom = RegGetProfileInt(HKEY_LOCAL_MACHINE, SZ_REG_MINILYRICS, _T("winamp5_rc_bottom"), 0);
}

DWORD CPlayerImp::GetPlayPos()
{
	DWORD		dwCurPos;

	WAGetCurSongPos(GetPlayerWnd(), dwCurPos);

	return dwCurPos - m_dwShoutcastTitleChangedPos;
}

BOOL CPlayerImp::IsWinamp5()
{
	DWORD dwPlayerVer;
	WAGetVersion(GetPlayerWnd(), dwPlayerVer);
	return dwPlayerVer >= 0x3000;
}

int CPlayerImp::HasVideo()
{
	if (IsWinamp5())
	{
		if (SendMessage(GetPlayerWnd(), WM_WA_IPC, 0, IPC_IS_PLAYING_VIDEO) > 1)
			return true;
		else
			return false;
	}
	else
		return R_UNDETERMINED;
}

void CPlayerImp::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		// update current play time.
		DWORD			dwCurPos;
		static DWORD	dwPosLast = 0;
		if (WAGetCurSongPos(GetPlayerWnd(), dwCurPos))
		{
			dwCurPos -= m_dwShoutcastTitleChangedPos;
			if (dwCurPos < dwPosLast)
			{
				// 如果上次的时间比现在的时间还大，则，可能切换了歌曲。
				dwPosLast = dwCurPos;
				CheckMediaUpdate();
			}
			dwPosLast = dwCurPos;
			g_MLService.OnPlayPosChanged(dwCurPos);
		}
	}
	else if (nTimerID == TIMER_CHECK_MEDIA)
		CheckMediaUpdate();
	else if (nTimerID == TIMER_MEDIA_MONKEY_EMBEDER)
	{
		if (IsMLEmbeddedInMediaMonkey())
		{
			// Has Embedded parent
			PostMessage(g_MLService.GetMainWindow(), g_msgMediaMonkeyEmbeder, true, 0);
			// KillTimer(m_hWnd, TIMER_MEDIA_MONKEY_EMBEDER);
		}
	}
	else if (nTimerID == TIMER_VERIFY_ML_IN_WINAMP_MENU)
	{
		// For winamp "Modern" skin, there might have 2 MiniLyrics menu item.
		CWinampMenu	menu;
		int count = menu.GetMiniLyricsItemCount();
		if (count > 1)
			menu.RemoveMiniLyrics();
		else if (count == 0)
			menu.AddMiniLyrics();

		static int n = 0;
		n++;
		if (n % 4 == 0)
			KillTimer(nTimerID);
	}
}

void CPlayerImp::CheckMediaUpdate(bool bForceChanged)
{
	wchar_t			szFile[MAX_PATH] = L"";
	wchar_t			szMediaTitle[MAX_PATH] = L"";
	static wchar_t	szFileLatest[MAX_PATH] = L"";
	static wchar_t	szMediaTitleLatest[MAX_PATH] = L"";
	PLAYER_STATE	stateNew;
	bool			bShoutcast;

	memset(szFile, 0, sizeof(szFile));
	WAGetSourceMedia(szFile, CountOf(szFile));

	memset(szMediaTitle, 0, sizeof(szMediaTitle));
	WAGetPlayingSongTitle(GetPlayerWnd(), szMediaTitle, CountOf(szMediaTitle));

	stateNew = (PLAYER_STATE)GetPlayerState();
	if (m_state != stateNew)
	{
		m_state = stateNew;
		g_MLService.OnPlayStateChanged(stateNew);
	}

	if (IsShoutcastMedia(szFile))
	{
		bShoutcast = true;
		// remove [buff %xx] in media title
		RemovePlayingStatInfoInMediaTitle(szMediaTitle);
	}
	else
		bShoutcast = false;

	if (!bForceChanged && lstrcmpW(szFile, szFileLatest) == 0 && lstrcmpW(szMediaTitle, szMediaTitleLatest) == 0)
		return;

	if (bShoutcast)
	{
		// It is a stream audio
		if (lstrcmpW(szFile, szFileLatest) == 0)
		{
			// Stream audio title changed, recount down the playing time.
			WAGetCurSongPos(GetPlayerWnd(), m_dwShoutcastTitleChangedPos);
			m_dwShoutcastTitleChangedPos += 2000;
		}
		else
			m_dwShoutcastTitleChangedPos = 0;
	}

	lstrcpyW(szFileLatest, szFile);
	lstrcpyW(szMediaTitleLatest, szMediaTitle);

	if (IsEmptyString(szFile) && IsEmptyString(szMediaTitle))
	{
		g_MLService.OnMediaInfoChanged(L"", NULL, NULL, NULL, 0);
		return;
	}

	OnMediaChanged(szFile, szMediaTitle);
}
