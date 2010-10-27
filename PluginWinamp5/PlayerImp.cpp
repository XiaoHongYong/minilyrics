#include <tchar.h>
#include "../MLPluginLib/mlplugin.h"
#include "PlayerImp.h"
#include "../MLPluginLib/PlayerInfoQueryBaseWnd.h"

#define WA_DLG_IMPLEMENT
#include "wa_dlg.h"
#include "wa_ipc.h"
#include "Gen.h"

#include "wa_wrap.h"

bool g_bKmplayer = false;

bool g_bMediaMonkey = false;
const UINT g_msgMediaMonkeyEmbeder = RegisterWindowMessage(_T("MediaMonkeyEmbederToMiniLyrics"));

UINT Get_WMCOMMAND_Id(UINT fallback)
{
	DWORD	dwVersion = 0;

	WAGetVersion(g_hwndPlayer, dwVersion);

	if (dwVersion >= 0x5010)
	{
		return SendMessage(g_hwndPlayer, WM_WA_IPC, 0, IPC_REGISTER_LOWORD_COMMAND);
	}

	return fallback;

}

static UINT nIDMenuMLItem = 27562;

void CheckMLItemInWinampMenu(bool bChecked)
{
	HMENU main_menu = (HMENU)SendMessage(g_hwndPlayer, WM_WA_IPC, 0, IPC_GET_HMENU);
	if (!main_menu)
		return;

	CheckMenuItem(main_menu, nIDMenuMLItem, MF_BYCOMMAND | (bChecked ? MF_CHECKED : MF_UNCHECKED));
}

void AddRemoveMLItemInWinampMenu(bool bAdd)
{
	static bool bGetMenuID = false;
	if (!bGetMenuID)
	{
		bGetMenuID = true;
		nIDMenuMLItem = Get_WMCOMMAND_Id(nIDMenuMLItem);
	}

	HMENU main_menu = (HMENU)SendMessage(g_hwndPlayer, WM_WA_IPC, 0, IPC_GET_HMENU);
	if (!main_menu)
		return;

	if (bAdd)
	{
		int pos = 0, seperatorCount = 0;
		for (; seperatorCount < 3 && pos < 50; pos++)
		{
			int prior_item = GetMenuItemID(main_menu, pos);
			if (prior_item == 0)
				seperatorCount++;
		}

		// now add the menu item using prior_item as the item to instert after
		InsertMenu(main_menu, pos - 1, MF_BYPOSITION, nIDMenuMLItem, "MiniLyrics");
		CheckMLItemInWinampMenu(g_Player.m_bWorking);
	}
	else
	{
		DeleteMenu(main_menu, nIDMenuMLItem, MF_BYCOMMAND);
	}

	// SendMessage(g_hwndPlayer, WM_WA_IPC, (bAdd ? 1 : -1), IPC_ADJUST_OPTIONSMENUPOS);
}


// #define IPC_GETPLAYLISTFILE 211
BOOL WAGetPlayListFileW(HWND hwndWinamp, int index, LPWSTR &szFile)
{
	return SendMessageTimeout(hwndWinamp, WM_WA_IPC, index, IPC_GETPLAYLISTFILEW,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, (LPDWORD)&szFile);
}

bool WAGetSourceMedia(wchar_t *wszUrl, int nBufSize)
{
	DWORD		dwIndex;

	EmptyStr(wszUrl);

	if (!WAGetListPos(g_hwndPlayer, dwIndex))
		return false;

	LPWSTR		wszSong = NULL;
	if (WAGetPlayListFileW(g_hwndPlayer, dwIndex, wszSong) && (DWORD)wszSong > 0xFFFF)
	{
		wcscpy_safe(wszUrl, nBufSize, wszSong);
		return true;
	}

	char		*szSong = NULL;
	if (WAGetPlayListFile(g_hwndPlayer, dwIndex, szSong) && szSong)
	{
		MBCSToUCS2(szSong, -1, wszUrl, nBufSize, CP_ACP);
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

// 安全的字符串拷贝，如果超出长度范围，则进行截断
// 并且将结尾设置为以'\0'结束
int	wcsncpysz_safe(wchar_t *strDestination, int nLenMax, const wchar_t *strSource, int nToCopy)
{
	if (nToCopy >= nLenMax)
		nToCopy = nLenMax - 1;

	int		org = nToCopy;

	while (nToCopy && (*strDestination++ = *strSource++))    /* copy string */
		nToCopy--;

	*strDestination = '\0';

	return org - nToCopy;
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
		szEndArtist = szBegTitle = wcsstr(szFullTitleNew, L" － ");
		if (szBegTitle)
			szBegTitle += 4;	// strlen(_T(" － "));
		else
		{
			szEndArtist = szBegTitle = wcschr(szFullTitleNew, L'-');
			if (szBegTitle)
				szBegTitle++;	// -
		}
	}

	//
	// 处理作者……
	if (szEndArtist == NULL)
	{
		//
		// 认为 szFullTitle 都是 Title, Artist 为空
		szBegTitle = szFullTitleNew;

		szArtist[0] = '\0';
	}
	else
	{
		wcsncpysz_safe(szArtist, MAX_PATH, szFullTitleNew, int(szEndArtist - szFullTitleNew));
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
	nRet = SendMessage(g_hwndPlayer, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW);

	info.metadata = L"title";
	memset(szTitle, 0, sizeof(wchar_t) * MAX_PATH);
	info.ret = szTitle;
	nRet = SendMessage(g_hwndPlayer, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW);

	info.metadata = L"album";
	memset(szAlbum, 0, sizeof(wchar_t) * MAX_PATH);
	info.ret = szAlbum;
	nRet = SendMessage(g_hwndPlayer, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFOW);

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
	nRet = SendMessage(g_hwndPlayer, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFO);

	info.metadata = "title";
	memset(szTitleA, 0, sizeof(szTitleA));
	info.ret = szTitleA;
	nRet = SendMessage(g_hwndPlayer, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFO);

	info.metadata = "album";
	memset(szAlbumA, 0, sizeof(szAlbumA));
	info.ret = szAlbumA;
	nRet = SendMessage(g_hwndPlayer, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFO);

	if (IsEmptyString(szArtistA) && IsEmptyString(szAlbumA) && IsEmptyString(szTitleA))
		return false;

	MBCSToUCS2(szArtistA, -1, szArtist, MAX_PATH);
	MBCSToUCS2(szAlbumA, -1, szAlbum, MAX_PATH);
	MBCSToUCS2(szTitleA, -1, szTitle, MAX_PATH);

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
	if (WAGetCurSongLength(g_hwndPlayer, dwLength))
		dwLength *= 1000;

	for (int i = 0; i < 10 && int(dwLength) <= 0; i++)
	{
		Sleep(50);
		if (WAGetCurSongLength(g_hwndPlayer, dwLength))
			dwLength *= 1000;
	}

	MLMediaInfo		mediaInfo;

	mediaInfo.nMediaLength = dwLength;
	mediaInfo.szMediaUrl = szFile;
	mediaInfo.szArtist = szArtist;
	mediaInfo.szAlbum = szAlbum;
	mediaInfo.szTitle = szTitle;

	g_MLService.OnMediaInfoChanged(&mediaInfo);
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

void CheckMediaUpdate()
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
	WAGetPlayingSongTitle(g_hwndPlayer, szMediaTitle, CountOf(szMediaTitle));

	stateNew = (PLAYER_STATE)g_Player.GetPlayerState();
	if (g_Player.m_state != stateNew)
	{
		g_Player.m_state = stateNew;
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

	if (lstrcmpW(szFile, szFileLatest) == 0 && lstrcmpW(szMediaTitle, szMediaTitleLatest) == 0)
		return;

	if (bShoutcast)
	{
		// It is a stream audio
		if (lstrcmpW(szFile, szFileLatest) == 0)
		{
			// Stream audio title changed, recount down the playing time.
			WAGetCurSongPos(g_hwndPlayer, g_Player.m_dwShoutcastTitleChangedPos);
			g_Player.m_dwShoutcastTitleChangedPos += 2000;
		}
		else
			g_Player.m_dwShoutcastTitleChangedPos = 0;
	}

	lstrcpyW(szFileLatest, szFile);
	lstrcpyW(szMediaTitleLatest, szMediaTitle);

	if (IsEmptyString(szFile) && IsEmptyString(szMediaTitle))
	{
		g_MLService.OnMediaInfoChanged(NULL);
		return;
	}

	OnMediaChanged(szFile, szMediaTitle);
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

class CPlayerInfoQueryWnd : public CPlayerInfoQueryBaseWnd
{
public:
	enum
	{
		TIMER_MEDIA_MONKEY_EMBEDER = TIMER_NEXT + 1
	};
	void OnCreate()
	{
		SetTimer(m_hWnd, TIMER_GETPOS, TIME_OUT_UPDATE_POS, NULL);
		SetTimer(m_hWnd, TIMER_CHECK_MEDIA, TIME_OUT_UPDATE_MEDIA_INFO, NULL);
		if (g_bMediaMonkey)
			SetTimer(m_hWnd, TIMER_MEDIA_MONKEY_EMBEDER, 5000, NULL);
	}

	void OnDestroy()
	{
		KillTimer(m_hWnd, TIMER_GETPOS);
		KillTimer(m_hWnd, TIMER_CHECK_MEDIA);
		if (g_bMediaMonkey)
			KillTimer(m_hWnd, TIMER_MEDIA_MONKEY_EMBEDER);
	}

	void OnTimer(int nTimerID)
	{
		if (nTimerID == TIMER_GETPOS)
		{
			// update current play time.
			DWORD			dwCurPos;
			static DWORD	dwPosLast = 0;
			if (WAGetCurSongPos(g_hwndPlayer, dwCurPos))
			{
				dwCurPos -= g_Player.m_dwShoutcastTitleChangedPos;
				if (dwCurPos < dwPosLast)
				{
					// 如果上次的时间比现在的时间还大，则，可能切换了歌曲。
					dwPosLast = dwCurPos;
					CheckMediaUpdate();
				}
				dwPosLast = dwCurPos;
				g_MLService.OnPlayTimeChanged(dwCurPos);
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
	}

};

////////////////////////////////////////////////////////////////////////////////////

CPlayerImp			g_Player;
CPlayerInfoQueryWnd	g_wndInfoQuery;

// function prototypes
void config();
void quit();
int init();

#define app_name	"MiniLyrics"

WNDPROC g_pOrgWaPluginWndProc = 0;
WNDPROC g_pOrgMLWndPro = 0;
WNDPROC	g_pOrgWinampProc = NULL;


embedWindowState* wa_wnd = NULL;

winampGeneralPurposePlugin plugin = {GPPHDR_VER, "MiniLyrics for Winamp", init, config, quit, };

HBITMAP WACreateScrollBmp()
{
#define VSB_WIDTH			14
#define VSB_BTN_HEIGHT		14
#define VSB_THUMB_HEIGHT		28
#define VSB_TRACK_HEIGHT		28
#define VSB_START_Y					31
#define VSB_HEIGHT					(VSB_BTN_HEIGHT + VSB_THUMB_HEIGHT + VSB_TRACK_HEIGHT + VSB_BTN_HEIGHT)

	HDC		hdcbk;
	HDC		hdcSb;
	HBITMAP	hBmpSb;
	HBITMAP	hBmpOld1, hBmpOld2;
	HBRUSH	hbrTrack, hbrOld;

	HDC		hdc;

	hdc = GetDC(NULL);
	hdcbk = CreateCompatibleDC(hdc);
	hdcSb = CreateCompatibleDC(hdc);
	hBmpSb = CreateCompatibleBitmap(hdc, VSB_WIDTH * 3, VSB_HEIGHT);
	hBmpOld1 = (HBITMAP)SelectObject(hdcbk, wadlg_bitmap);
	hBmpOld2 = (HBITMAP)SelectObject(hdcSb, hBmpSb);
	hbrTrack = CreateSolidBrush(wadlg_colors[WADLG_SCROLLBAR_BGCOLOR]);
	hbrOld = (HBRUSH)SelectObject(hdcSb, hbrTrack);

	int		x = 0, y = 0;
	RECT	rc;

	//
	// draw normal scrollbar
	//

	// top button
	BitBlt(hdcSb, x, y, VSB_WIDTH, VSB_BTN_HEIGHT, hdcbk, 0, VSB_START_Y, SRCCOPY);
	y += VSB_BTN_HEIGHT;

	// thumb button
	BitBlt(hdcSb, x, y, VSB_WIDTH, VSB_THUMB_HEIGHT, hdcbk, VSB_WIDTH * 4, VSB_START_Y, SRCCOPY);
	y += VSB_THUMB_HEIGHT;

	// track
	rc.left = x, rc.top = y;
	rc.right = x + VSB_WIDTH;
	rc.bottom = y + VSB_TRACK_HEIGHT;
	FillRect(hdcSb, &rc, hbrTrack);
	y += VSB_TRACK_HEIGHT;

	// bottom button
	BitBlt(hdcSb, x, y, VSB_WIDTH, VSB_BTN_HEIGHT, hdcbk, VSB_WIDTH * 1, VSB_START_Y, SRCCOPY);

	//
	// draw down scrollbar
	//
	x += VSB_WIDTH;
	y = 0;

	// top button
	BitBlt(hdcSb, x, y, VSB_WIDTH, VSB_BTN_HEIGHT, hdcbk, VSB_WIDTH * 2, VSB_START_Y, SRCCOPY);
	y += VSB_BTN_HEIGHT;

	// thumb button
	BitBlt(hdcSb, x, y, VSB_WIDTH, VSB_THUMB_HEIGHT, hdcbk, VSB_WIDTH * 5, VSB_START_Y, SRCCOPY);
	y += VSB_THUMB_HEIGHT;

	// track
	rc.left = x, rc.top = y;
	rc.right = x + VSB_WIDTH;
	rc.bottom = y + VSB_TRACK_HEIGHT;
	FillRect(hdcSb, &rc, hbrTrack);
	y += VSB_TRACK_HEIGHT;

	// bottom button
	BitBlt(hdcSb, x, y, VSB_WIDTH, VSB_BTN_HEIGHT, hdcbk, VSB_WIDTH * 3, VSB_START_Y, SRCCOPY);

	//
	// draw focus scrollbar
	//
	x += VSB_WIDTH;
	y = 0;

	// top button
	BitBlt(hdcSb, x, y, VSB_WIDTH, VSB_BTN_HEIGHT, hdcbk, 0, VSB_START_Y, SRCCOPY);
	y += VSB_BTN_HEIGHT;

	// thumb button
	BitBlt(hdcSb, x, y, VSB_WIDTH, VSB_THUMB_HEIGHT, hdcbk, VSB_WIDTH * 4, VSB_START_Y, SRCCOPY);
	y += VSB_THUMB_HEIGHT;

	// track
	rc.left = x, rc.top = y;
	rc.right = x + VSB_WIDTH;
	rc.bottom = y + VSB_TRACK_HEIGHT;
	FillRect(hdcSb, &rc, hbrTrack);
	y += VSB_TRACK_HEIGHT;

	// bottom button
	BitBlt(hdcSb, x, y, VSB_WIDTH, VSB_BTN_HEIGHT, hdcbk, VSB_WIDTH * 1, VSB_START_Y, SRCCOPY);

	SelectObject(hdcbk, hBmpOld1);
	SelectObject(hdcSb, hBmpOld2);
	SelectObject(hdcSb, hbrOld);
	DeleteObject(hbrTrack);
	DeleteDC(hdcbk);
	DeleteDC(hdcSb);

	ReleaseDC(NULL, hdc);

	return hBmpSb;
}

LRESULT CALLBACK WndProcMLHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_DESTROY:
		// PostQuitMessage(0);
		{
			WADlg_close();
			if (GetWindowLong(hWnd, GWL_WNDPROC) == (LONG)WndProcMLHook)
			{
				SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_pOrgMLWndPro);
			}
			break;
		}
	default:
		break;
	}

	return CallWindowProc(g_pOrgMLWndPro, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK ProcWAPluginParentHook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	int ret;
	
	ret = CallWindowProc(g_pOrgWaPluginWndProc, hwnd, msg, wp, lp);

	// unsublcass once the WM_DESTROY message has been processed for safety
	if (msg == WM_DESTROY)
	{
		if (GetWindowLong(hwnd, GWL_WNDPROC) == (LONG)ProcWAPluginParentHook)
		{
			SetWindowLong(hwnd, GWL_WNDPROC, (LONG)g_pOrgWaPluginWndProc);
		}

		AddRemoveMLItemInWinampMenu(true);

		g_Player.m_hwndPluginParent = NULL;
	}

	return ret;
}

int init()
{
	g_hInstance = plugin.hDllInstance;

	g_hwndPlayer = plugin.hwndParent; // FindWindow("Winamp v1.x", NULL);

	WAGetVersion(g_hwndPlayer, g_Player.m_dwPlayerVer);

	AddRemoveMLItemInWinampMenu(true);

	if (!InitMiniLyricsPlugin(g_hInstance, &g_Player, g_Player.GetStartupFlags()))
		return 0;	// we must return 0!

	if (g_Player.IsWinamp5() && g_MLService.GetSkinMode() == CMLService::MLSKIN_PLAYER)
	{
			HWND	hWndMiniLyrics = 0;

			hWndMiniLyrics = g_MLService.GetMainWindow();

			ShowWindow(GetParent(hWndMiniLyrics), SW_SHOWNORMAL);

			g_pOrgMLWndPro = (WNDPROC)SetWindowLong(hWndMiniLyrics, GWL_WNDPROC, (LONG)WndProcMLHook);
	}

	// now we can use other g_MLService functions.
	char	szMsg[256];

	wsprintf(szMsg, "Start Winamp2 Player plugin for MiniLyrics. HINSTANCE: %#X", g_hInstance);
	g_MLService.Log(szMsg);

	return 0;
}


void config()
{
	g_MLService.Config(g_hwndPlayer);
}


void quit()
{
	AddRemoveMLItemInWinampMenu(false);

	// close all the windows and free all parts correctly
	g_MLService.Quit();

	if (g_Player.m_hwndPluginParent)
		DestroyWindow(g_Player.m_hwndPluginParent);

	if (wa_wnd)
	{
		GlobalFree(wa_wnd);
		wa_wnd = NULL;
	}
}


LRESULT CALLBACK HookWinampWnd(
							   HWND hwnd,      // handle to window
							   UINT uMsg,      // message identifier
							   WPARAM wParam,  // first message parameter
							   LPARAM lParam   // second message parameter
							   )
{
	if (!g_Player.m_bWorking)
	{
		if (uMsg == WM_COMMAND && LOWORD(wParam) == nIDMenuMLItem)
		{
			// Open MiniLyrics
			g_MLService.Init(g_hModMiniLyrics, g_Player.GetStartupFlags());
			return 0;
		}
		return CallWindowProc(g_pOrgWinampProc, hwnd, uMsg, wParam, lParam);
	}

	switch(uMsg) 
	{
	case WM_DESTROY:
		{
			// g_MLService.Quit will set g_pOrgWinampProc to NULL, so save g_pOrgProc.
			WNDPROC pOrigProc = g_pOrgWinampProc;
			g_MLService.Quit();
			return CallWindowProc(pOrigProc, hwnd, uMsg, wParam, lParam); 
		}
		break;
	case WM_COMMAND:
		{
			if (LOWORD(wParam) == 0x9D63)
			{
				int		nRet;
				nRet = CallWindowProc(g_pOrgWinampProc, hwnd, uMsg, wParam, lParam); 

				// DBGLOG0("Winamp's Skin Changed!");
 				WADlg_init(plugin.hwndParent);

 				// set scroll bar bitmap.
 				g_MLService.SetWa5Bmp(1, WACreateScrollBmp());
				g_MLService.OnNotifyPlayerSkinChanged();

				return nRet;
			}
// 			else if (LOWORD(wParam) == 0x9C53)
// 			{
// 				// 顶层窗口
// 				// g_wndLyricShow.OnCommand(IDC_ALWAYS_ON_TOP);
// 			}
			else if (LOWORD(wParam) == nIDMenuMLItem)
			{
				// Open/close MiniLyrics
				g_MLService.Quit();
			}
		}
		break;
	case WM_ACTIVATEAPP:
		{
			LRESULT		result = CallWindowProc(g_pOrgWinampProc, hwnd, uMsg, wParam, lParam);
			g_MLService.OnNotifyPlayerActivate(wParam);
			return result;
		}
		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED || wParam == SIZE_RESTORED)
		{
			LRESULT		result = CallWindowProc(g_pOrgWinampProc, hwnd, uMsg, wParam, lParam);
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
			g_MLService.OnNotifyBeforeTrackMove(hWndChain, 5);
		}
		break;
	case WM_MOVE:
		g_MLService.OnNotifyTrackMove((int)(short) LOWORD(lParam), (int)(short) HIWORD(lParam));
		break;
	default:
		break;
	};

	// Call Winamp Window Proc
	return CallWindowProc(g_pOrgWinampProc, hwnd, uMsg, wParam, lParam); 
}

#ifdef __cplusplus
extern "C" {
#endif

	__declspec( dllexport ) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin()
	{
		return &plugin;
	}

#ifdef __cplusplus
}
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	m_dwPlayerVer = 0;
	m_hwndPluginParent = NULL;

	m_bWorking = false;
}

CPlayerImp::~CPlayerImp()
{
}

HWND CPlayerImp::GetAttachedParentWnd(RECT *prc)
{
	if (m_hwndPluginParent)
		return m_hwndPluginParent;

	if (g_Player.IsWinamp5())
	{
		static char* des = app_name;
		plugin.description = des;
		
		// this allocates the memory pointer so the structure can be accessed
		// by winamp (otherwise it will crash)
		if (!wa_wnd)
			wa_wnd = (embedWindowState*)GlobalAlloc(GPTR, sizeof(embedWindowState));

		// enable this to set the window as not being resizeable
		//wa_wnd->flags = EMBED_FLAGS_NORESIZE;

		if (prc)
		{
			wa_wnd->r = *prc;
		}
		else
		{
			wa_wnd->r.left = 0;
			wa_wnd->r.top = 0;
			wa_wnd->r.right = wa_wnd->r.bottom = 0;
		}
		
		// parent needs to be shown
		g_Player.m_hwndPluginParent = (HWND)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)wa_wnd, IPC_GET_EMBEDIF);
		if(g_Player.m_hwndPluginParent)
		{
			// here we attach a dialog into the window
			// if need by this could been done with CreateWindow(), etc depending
			// on what you want to do
			// hWndPlugin = CreateDialog(plugin.hDllInstance, MAKEINTRESOURCE(IDD_DIALOG1), g_Player.m_hwndPluginParent, (DLGPROC)DlgProc);

			WADlg_init(plugin.hwndParent);

			// set scroll bar bitmap.
			g_MLService.SetWa5Bmp(1, WACreateScrollBmp());

			g_pOrgWaPluginWndProc = (WNDPROC)SetWindowLong(g_Player.m_hwndPluginParent, GWL_WNDPROC, (LONG)ProcWAPluginParentHook);
		}
	}

	AddRemoveMLItemInWinampMenu(false);

	return m_hwndPluginParent;
}

void CPlayerImp::OnNotifySkinLoaded()
{
	if (!g_Player.IsWinamp5() || m_hwndPluginParent == NULL)
		return;

	// here we set the window's title
	SetWindowText(m_hwndPluginParent, "MiniLyrics");

	// shows the owner window and this dialog
	ShowWindow(m_hwndPluginParent, SW_SHOWNORMAL);
}

void CPlayerImp::OnNotifyDetachedFromParentWnd()
{
	DestroyWindow(m_hwndPluginParent);
}

int CPlayerImp::GetEmbeddedSkinName(TCHAR * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "Winamp's Skin");
	return PR_OK;
}

int CPlayerImp::GetSkinThemeColor(SKIN_CLR_NAME scn, COLORREF *clrValue)
{
	if (scn == SCN_FONT_HIGH)
		*clrValue = WADlg_getColor(WADLG_ITEMFG);
	else if (scn == SCN_FONT_LOW)
	{
		COLORREF	clr1, clr2;
		clr1 = WADlg_getColor(WADLG_WNDBG);
		clr2 = WADlg_getColor(WADLG_ITEMFG);
		*clrValue = RGB((GetRValue(clr1) + GetRValue(clr2)) / 2, 
					(GetGValue(clr1) + GetGValue(clr2)) / 2, 
					(GetBValue(clr1) + GetBValue(clr2)) / 2);
	}
	else if (scn == SCN_NORMAL_BG)
		*clrValue = WADlg_getColor(WADLG_ITEMBG);
	else if (scn == SCN_SEL_BG)
		*clrValue = WADlg_getColor(WADLG_SELCOLOR);
	else
	{
		assert(0);
		return PR_UNIMPLEMENTED;
	}

	return PR_OK;
}

BOOL CPlayerImp::IsWinamp5()
{
	assert(m_dwPlayerVer > 0);
	return m_dwPlayerVer >= 0x3000;
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	TCHAR	szFile[MAX_PATH];
	if (!GetModuleFileName(NULL, szFile, CountOf(szFile)))
	{
		strcpy_safe(lpszBuffer, nBufSize, "Winamp5");
		return PR_OK;
	}

	LPCTSTR szExeName = UrlGetName(szFile);
	if (strcmp(szExeName, "winamp.exe") == 0)
	{
		if (IsWinamp5())
			strcpy_safe(lpszBuffer, nBufSize, "Winamp5");
		else
			strcpy_safe(lpszBuffer, nBufSize, "Winamp2");
	}
	else if (strcmp(szExeName, "MediaMonkey.exe") == 0)
	{
		g_bMediaMonkey = true;
		strcpy_safe(lpszBuffer, nBufSize, "MediaMonkey");
	}
	else if (strcmp(szExeName, "AIMP2.exe") == 0)
		strcpy_safe(lpszBuffer, nBufSize, "AIMP2");
	else if (strcmp(szExeName, "KMPlayer.exe") == 0)
	{
		g_bKmplayer = true;
		strcpy_safe(lpszBuffer, nBufSize, "KMPlayer");
	}
	else
	{
		char	szPlayerIni[MAX_PATH];

		GetProgramPath(szPlayerIni, g_hInstance);
		strcat_safe(szPlayerIni, CountOf(szPlayerIni), "MLPlugin.ini");

		GetPrivateProfileString("MiniLyrics", "PlayerName", "", lpszBuffer, nBufSize, szPlayerIni);
	}

	return PR_OK;
}

// get player state and song info
DWORD CPlayerImp::GetPlayerState()
{
	if (g_hwndPlayer == NULL)
		return PS_STOPED;

	int		nPlayState;

	if (!WAIsPlaying(g_hwndPlayer, nPlayState))
		return PS_STOPED;

	// -1, paused
	// 0, stopped
	// 1, playing
	switch (nPlayState)
	{
	case 0:
		return PS_STOPED;
	case 1:
		return PS_PLAYING;
	case 3:
		return PS_PAUSED;
	default:
		break;
	}
	return PS_STOPED;
}

int CPlayerImp::GetCurSkinPath(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);
	WAGetSkin(g_hwndPlayer, lpszBuffer);

	return PR_OK;
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	DWORD		dwCurPos;

	WAGetCurSongPos(g_hwndPlayer, dwCurPos);

	return dwCurPos - m_dwShoutcastTitleChangedPos;
}

// control player
BOOL CPlayerImp::Play()
{
	WAPlay(g_hwndPlayer);

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	WAPlayPause(g_hwndPlayer);

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	if (GetPlayerState() == PS_PLAYING)
		Pause();
	else
		Play();

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	WAStop(g_hwndPlayer);

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	WAPrev(g_hwndPlayer);

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	WANext(g_hwndPlayer);

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	int		nResult;

	return WAJumpToTime(g_hwndPlayer, nMsPos, nResult);
}

BOOL CPlayerImp::Init()
{
	if (!g_hwndPlayer)
		g_hwndPlayer = FindWindow("Winamp v1.x", NULL);

	if (m_dwPlayerVer >= 0x3000)
	{
		g_MLService.AddWndCloseto(NULL, "BaseWindow_RootWnd", "Playlist Editor");
		g_MLService.AddWndCloseto(NULL, "BaseWindow_RootWnd", "Player Window");
		g_MLService.AddWndCloseto(NULL, "BaseWindow_RootWnd", "Winamp Library");
		g_MLService.AddWndCloseto(NULL, "BaseWindow_RootWnd", "Visualizations");
		g_MLService.AddWndCloseto(NULL, "BaseWindow_RootWnd", "Media Library");
		g_MLService.AddWndCloseto(NULL, "BaseWindow_RootWnd", "");
	}
	g_MLService.AddWndCloseto(NULL, "Winamp Gen", "Winamp Library");
	g_MLService.AddWndCloseto(NULL, "Winamp v1.x", "");
	g_MLService.AddWndCloseto(NULL, "Winamp EQ", "");
	g_MLService.AddWndCloseto(NULL, "Winamp PE", "");
	g_MLService.AddWndCloseto(NULL, "Winamp MB", "");

	// Hook Main Window
	if (g_pOrgWinampProc == NULL)
		g_pOrgWinampProc = (WNDPROC)SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)HookWinampWnd);
	m_bWorking = true;

	m_state = (PLAYER_STATE)GetPlayerState();

	g_wndInfoQuery.Create(g_hInstance);

	CheckMLItemInWinampMenu(true);

	return TRUE;
}

void CPlayerImp::Quit()
{
	CheckMLItemInWinampMenu(false);

	// 恢复winamp窗口的窗口过程
	if (g_pOrgWinampProc && IsWindow(g_hwndPlayer) && GetWindowLong(g_hwndPlayer, GWL_WNDPROC) == (LONG)HookWinampWnd)
	{
		SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)g_pOrgWinampProc);
		g_pOrgWinampProc = NULL;
	}
	m_bWorking = false;

	g_wndInfoQuery.Destroy();

	if (IsWindow(m_hwndPluginParent))
	{
		DestroyWindow(m_hwndPluginParent);
		m_hwndPluginParent = NULL;
	}
}

int CPlayerImp::HasVideo()
{
	if (IsWinamp5())
	{
		if (SendMessage(g_hwndPlayer, WM_WA_IPC, 0, IPC_IS_PLAYING_VIDEO) > 1)
			return PR_OK;
		else
			return PR_UNIMPLEMENTED;
	}
	else
		return PR_UNIMPLEMENTED;
}

void CPlayerImp::OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan)
{
	g_wndInfoQuery.SetUpdatePosTimer(nPlayPosUpdateSpan);
}

DWORD CPlayerImp::GetStartupFlags()
{
	DWORD	dwStartupFlag = 0;
	if (IsWinamp5())
		dwStartupFlag = ML_SUPPORT_EMBEDDED | ML_EMBEDDED_WND_IMMOVABLE | ML_EMBEDDED_SUPPORT_CLASSIC_SKINS;
	else
		dwStartupFlag = ML_SUPPORT_EMBEDDED | ML_EMBEDDED_SUPPORT_CLASSIC_SKINS;

	return dwStartupFlag;
}
