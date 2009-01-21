#include <tchar.h>
#include "../MLPluginLib/mlplugin.h"
#include "PlayerImp.h"

#define WA_DLG_IMPLEMENT
#include "wa_dlg.h"
#include "Gen.h"

#include "frontend.h"
#include <PROCESS.H>

bool WAGetSourceMedia(char * lpszBuffer, int nBufSize)
{
	int			nLen;
	DWORD		dwIndex;
	char		*szSong = NULL;

	EmptyStr(lpszBuffer);

	if (!WAGetListPos(g_hwndPlayer, dwIndex))
		return false;

	if (!WAGetPlayListFile(g_hwndPlayer, dwIndex, szSong))
		return false;

	if (szSong == NULL)
		return false;

	nLen = strlen(szSong);

	strcpy_safe(lpszBuffer, nBufSize, szSong);

	return true;
}

// rule: 1) http://xxxx	
//       2) without the extention of song file.
bool IsShoutcastMedia(LPCTSTR szMedia)
{
	if (_tcsnicmp(szMedia, _T("http://"), 7) == 0)
	{
		LPCTSTR p = _tcsrchr(szMedia + 7, '.');
		if (p)
		{
			if (_tcslen(p) <= 5)
				return false;
		}
		return true;
	}
	else if (_tcsnicmp(szMedia, _T("uvox://"), 7) == 0)
		return true;

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// create window to get playback pos timely.
LPCTSTR	szGetPosWndClass = _T("GetPosWnd");
LPCTSTR szGetPosWndName = _T("GetPosWnd");
HWND	g_hWndGetPos = NULL;
const int	TIMER_GETPOS	= 1;
const int	TIMER_CHECK_MEDIA	= 2;
int			g_UpdateTimeSpan = 50;

LRESULT CALLBACK GetPosWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL GetPosWndRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	if (GetClassInfo(hInstance, szGetPosWndClass, &wc))
		return TRUE;

	// regiszter pane class
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)GetPosWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon		= NULL;
	wc.hCursor		= NULL;
	wc.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName	= NULL;
	wc.lpszClassName	= szGetPosWndClass;

	int r = RegisterClass(&wc);
	if (r == 0)
	{
		int res = GetLastError();
		if (res != ERROR_CLASS_ALREADY_EXISTS)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL GetPosWndCreate(HINSTANCE hInstance)
{
   g_hWndGetPos = CreateWindow(szGetPosWndClass, szGetPosWndName, WS_OVERLAPPEDWINDOW,
      0, 0, 100, 100, NULL, NULL, hInstance, NULL);

   if (!g_hWndGetPos)
   {
      return FALSE;
   }

   ShowWindow(g_hWndGetPos, SW_MINIMIZE);
   ShowWindow(g_hWndGetPos, SW_HIDE);

   return TRUE;
}

void GetMediaTitle(LPTSTR szBuff, int nBuffLen, LPCTSTR szMediaSrc)
{
	assert(szMediaSrc);

	LPCTSTR		szBeg1, szBeg2;

	szBeg1 = strchr(szMediaSrc, '\\');
	if (szBeg1 == NULL)
		szBeg1 = szMediaSrc;
	else
		szBeg1++;

	szBeg2 = strchr(szMediaSrc, '/');
	if (szBeg2 == NULL)
		szBeg2 = szMediaSrc;
	else
		szBeg2++;

	if (szBeg2 > szBeg1)
		szBeg1 = szBeg2;

	lstrcpy(szBuff, szBeg1);
}


void CheckMediaUpdate()
{
	char			szFile[MAX_PATH] = _T("");
	char			szTitle[MAX_PATH] = _T("");
	static char		szFileLatest[MAX_PATH] = _T("");
	static char		szTitleLatest[MAX_PATH] = _T("");
	PLAYER_STATE	stateNew;

	memset(szFile, 0, sizeof(szFile));
	WAGetSourceMedia(szFile, CountOf(szFile));
	memset(szTitle, 0, sizeof(szTitle));
	WAGetPlayingSongTitle(g_hwndPlayer, szTitle);

	stateNew = (PLAYER_STATE)g_Player.GetPlayerState();
	if (g_Player.m_state != stateNew)
	{
		g_Player.m_state = stateNew;
		g_MLService.OnPlayStateChanged(stateNew);
	}

	if (lstrcmp(szFile, szFileLatest) != 0 || lstrcmp(szTitle, szTitleLatest) != 0)
	{
		lstrcpy(szFileLatest, szFile);
		lstrcpy(szTitleLatest, szTitle);

		g_Player.m_dwShoutcastTitleChangedPos = 0;

		// file same, check whether it is an stream audio?
		if (IsShoutcastMedia(szFile))
			g_Player.m_bShoutcast = true;
		else
			g_Player.m_bShoutcast = false;

		if (IsEmptyString(szFile) && IsEmptyString(szTitle))
		{
			g_Player.OnSongChanged("");
			return;
		}

		extendedFileInfoStruct	info;
		char			szArtist[MAX_PATH], szTitle[MAX_PATH], szFullTitle[MAX_PATH];
		int				nRet;

		info.filename = szFile;
		info.retlen = MAX_PATH;

		info.metadata = "artist";
		info.ret = szArtist;
		memset(szArtist, 0, sizeof(szArtist));
		nRet = SendMessage(g_hwndPlayer, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFO);

		info.metadata = "title";
		memset(szTitle, 0, sizeof(szTitle));
		info.ret = szTitle;
		nRet = SendMessage(g_hwndPlayer, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFO);

		if (!IsEmptyString(szArtist))
		{
			strcpy(szFullTitle, szArtist);
			strcat(szFullTitle, " - ");
		}
		else
			EmptyStr(szFullTitle);

		strcat(szFullTitle, szTitle);
		if (IsEmptyString(szFullTitle))
		{
			WAGetPlayingSongTitle(g_hwndPlayer, szFullTitle);
			if (IsEmptyString(szFullTitle))
				GetMediaTitle(szFullTitle, CountOf(szFullTitle), szFile);
		}

		g_Player.OnSongChanged(szFullTitle);
	}
	else
	{
		// file same, check whether it is an stream audio?
		if (g_Player.m_bShoutcast)
		{
			char			szFullTitle[MAX_PATH];

			// check the title.
			WAGetPlayingSongTitle(g_hwndPlayer, szFullTitle);

			// remove [buff %xx]
			int	n = 0;
			if (szFullTitle[0] == '[')
			{
				n = 1;
				while (szFullTitle[n] != '\0' && szFullTitle[n] !=']')
					n++;
				if (szFullTitle[n] == ']')
				{
					n++;
					while (szFullTitle[n] == ' ')
						n++;
					int	nMoveSize = strlen(szFullTitle + n);
					memmove(szFullTitle, szFullTitle + n, nMoveSize);
					szFullTitle[nMoveSize] = '\0';
				}
			}

			if (lstrcmp(szFullTitle, g_Player.m_szFullTitle) != 0)
			{
				
				// Notify song changed...
				g_Player.OnSongChanged(szFullTitle);
				WAGetCurSongPos(g_hwndPlayer, g_Player.m_dwShoutcastTitleChangedPos);
				// if (g_Player.m_dwShoutcastTitleChangedPos >= 1000)
					g_Player.m_dwShoutcastTitleChangedPos += 2000;
			}
		}
	}
}

LRESULT CALLBACK GetPosWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_CREATE:
		SetTimer(hWnd, TIMER_GETPOS, g_UpdateTimeSpan, NULL);
		SetTimer(hWnd, TIMER_CHECK_MEDIA, 500, NULL);
		break;
	case WM_TIMER:
		{
			if (wParam == TIMER_CHECK_MEDIA)
			{
				CheckMediaUpdate();
			}
			else if (wParam == TIMER_GETPOS)
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
					g_Player.OnPlayTimeChanged(dwCurPos);
				}
			}
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_GETPOS);
		KillTimer(hWnd, TIMER_CHECK_MEDIA);
		g_hWndGetPos = NULL;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

void GetPosWndSetTimeUpdateFreq(int nFreq)
{
	if (nFreq == 0)
		g_UpdateTimeSpan = 100;
	else if (nFreq == 1)
		g_UpdateTimeSpan = 50;
	else
		g_UpdateTimeSpan = 30;

	KillTimer(g_hWndGetPos, TIMER_GETPOS);
	SetTimer(g_hWndGetPos, TIMER_GETPOS, g_UpdateTimeSpan, NULL);
}

// End of GetPosWindow
////////////////////////////////////////////////////////////////////////////////////

//
// 有两种状态：	1、使用winamp的skin
//				2、使用自己的skin
//

CPlayerImp		g_Player;


// function prototypes
void config();
void quit();
int init();

#define app_name	"MiniLyrics"

WNDPROC procWinampOld = 0;
WNDPROC procMiniLyrics = 0;

HWND	hWndPlugin = 0;

embedWindowState* wa_wnd = NULL;

winampGeneralPurposePlugin plugin = {GPPHDR_VER, "MiniLyrics for Winamp", init, config, quit, };
//
//extern "C"
//{
//BOOL WINAPI DLLMAIN(HINSTANCE hModule, DWORD ul_reason_for_call, 
//							   LPVOID lpReserved){
//	return 1;
//}
//}

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
//	int wmId, wmEvent;
//	PAINTSTRUCT ps;
//	HDC hdc;
//	TCHAR szHello[] = "Hello";
//
	switch (message) 
	{
//	case WM_CREATE:
//		// call the internal window initialisation - makes sure that
//		// the internal controls will be winamp styled
//		WADlg_init(plugin.hwndParent);
//
//		// here we set the window's title
//		SetWindowText(GetParent(hWnd), "MiniLyrics");
//		
//		// shows the owner window and this dialog
//		//ShowWindow(hwnd, SW_SHOWNORMAL);
//		ShowWindow(GetParent(hWnd), SW_SHOWNORMAL);
//		break;
//	case WM_COMMAND:
//		wmId    = LOWORD(wParam); 
//		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
//			switch (wmId)
//			{
////				case IDM_ABOUT:
////				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
////				   break;
//				case IDM_EXIT:
//				   DestroyWindow(hWnd);
//				   break;
//				default:
//				   return DefWindowProc(hWnd, message, wParam, lParam);
//			}
//			DestroyWindow(GetParent(hwnd));
//		break;
//	case WM_PAINT:
//		hdc = BeginPaint(hWnd, &ps);
//		// TODO: Add any drawing code here...
//
//		{
//			HDC		hdcbk;
//			HBITMAP	hBmpOld;
//			HBITMAP	hBmpSb;
//			hdcbk = CreateCompatibleDC(hdc);
//			hBmpSb = WACreateScrollBmp();
//			hBmpOld = (HBITMAP)SelectObject(hdcbk, hBmpSb);
//			BitBlt(hdc, 0, 100, VSB_WIDTH * 3, VSB_HEIGHT, hdcbk, 0, 0, SRCCOPY);
//		//	BitBlt(hdc, 0, 0, 300, 300, hdcbk, 0, 0, SRCCOPY);
//
//			SelectObject(hdcbk, hBmpOld);
//			DeleteObject(hBmpSb);
//			DeleteDC(hdcbk);
//		}
//
//		RECT rt;
//		GetClientRect(hWnd, &rt);
//		DrawText(hdc, szHello, strlen(szHello), &rt, DT_CENTER);
//
//		EndPaint(hWnd, &ps);
//		break;
		// DestroyWindow(GetParent(hWnd));
		// g_Player.m_hwndPluginParent = NULL;
		// break;
	case WM_DESTROY:
		// PostQuitMessage(0);
		{
			WADlg_close();
			if (GetWindowLong(hWnd, GWL_WNDPROC) == (LONG)WndProcMLHook)
			{
				SetWindowLong(hWnd, GWL_WNDPROC, (LONG)procMiniLyrics);
			}
//			LRESULT hr = CallWindowProc(procMiniLyrics, hWnd, message, wParam, lParam);
//			return hr;
			break;
		}
// 	case WM_DISPLAYCHANGE:
// 		WADlg_init(plugin.hwndParent);
// 		{
// //			// this is a list of controls to winampise
// //			int tabs[] = {IDC_EDIT|DCW_SUNKENBORDER, IDC_STATIC4|DCW_SUNKENBORDER};
// //			WADlg_DrawChildWindowBorders(hWnd, tabs, sizeof(tabs)/sizeof(tabs[0]));
// 		}
// 
// 		// set scroll bar bitmap.
// 		g_MLService.SetWa5Bmp(1, WACreateScrollBmp());
// 		g_MLService.OnNotifyPlayerSkinChanged();
// 
// 		// if winamp changes skin then it sends a dummy WM_DISPLAYCHANGE
// 		// message. on receive do a quick hide and reshow
// 		// (simple hack to ensure the window is shown correctly with
// 		//  the newly loaded skin)
// 		if(!lParam && !wParam){
// 			ShowWindow(hWnd, SW_HIDE);
// 			ShowWindow(hWnd, SW_SHOW);
// 		}
// 		break;
	default:
		break;
		// return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return CallWindowProc(procMiniLyrics, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK ProcWAPluginParentHook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	int ret;
	
	// in here we can process any messages as needed
	
	ret = CallWindowProc(procWinampOld, hwnd, msg, wp, lp);
	
	// unsublcass once the WM_DESTROY message has been processed for safety
	if (msg == WM_DESTROY)
	{
		if (GetWindowLong(hwnd, GWL_WNDPROC) == (LONG)ProcWAPluginParentHook)
		{
			SetWindowLong(hwnd, GWL_WNDPROC, (LONG)procWinampOld);
		}

		g_Player.m_hwndPluginParent = NULL;
	}

	return ret;
}

int init()
{
	g_hInstance = plugin.hDllInstance;

	g_hwndPlayer = plugin.hwndParent; // FindWindow("Winamp v1.x", NULL);

	{
		//
		// Compatable for MediaMonkey.
		//
		char	szPlayerIni[MAX_PATH];

		GetProgramPath(szPlayerIni, g_hInstance);
		strcat(szPlayerIni, "MLPlugin.ini");

		EmptyStr(g_Player.m_szPlayerName);
		GetPrivateProfileString("MiniLyrics", "PlayerName", "", g_Player.m_szPlayerName, CountOf(g_Player.m_szPlayerName), szPlayerIni);
	}

	DWORD		dwStartupFlag;

	WAGetVersion(g_hwndPlayer, g_Player.m_dwPlayerVer);

	if (g_Player.IsWinamp5())
		dwStartupFlag = ML_SUPPORT_EMBEDDED | ML_EMBEDDED_WND_IMMOVABLE | ML_EMBEDDED_SUPPORT_CLASSIC_SKINS;
	else
		dwStartupFlag = ML_SUPPORT_EMBEDDED | ML_EMBEDDED_SUPPORT_CLASSIC_SKINS;

	if (!InitMiniLyricsPlugin(g_hInstance, &g_Player, dwStartupFlag))
		return 0;	// we must return 0!

	if (g_Player.IsWinamp5() && g_MLService.GetSkinMode() == CMLService::MLSKIN_PLAYER)
	{
		hWndPlugin = g_MLService.GetMainWindow();

		ShowWindow(GetParent(hWndPlugin), SW_SHOWNORMAL);

		procMiniLyrics = (WNDPROC)SetWindowLong(hWndPlugin, GWL_WNDPROC, (LONG)WndProcMLHook);
	}

	// now we can use other g_MLService functions.
	char	szMsg[256];

	wsprintf(szMsg, "Start Winamp2 Player plugin for minilyrics. HINSTANCE: %#X", g_hInstance);
	g_MLService.Log(szMsg);

	// don't load any further if the winamp window could not be created
	// of the sublcass fails
	return 0;
}


void config()
{
	g_MLService.Config(g_hwndPlayer);
}


void quit()
{
	// close all the windows and free all parts correctly
	g_MLService.Quit();

//	DestroyWindow(hWndPlugin);
	if (g_Player.m_hwndPluginParent)
		DestroyWindow(g_Player.m_hwndPluginParent);

	if (wa_wnd)
	{
		GlobalFree(wa_wnd);
	}
}


WNDPROC			g_pOrigProc = NULL;

LRESULT CALLBACK HookWinampWnd(
							   HWND hwnd,      // handle to window
							   UINT uMsg,      // message identifier
							   WPARAM wParam,  // first message parameter
							   LPARAM lParam   // second message parameter
							   )
{
	if (!g_Player.m_bWorking)
		return CallWindowProc(g_pOrigProc, hwnd, uMsg, wParam, lParam);

	switch(uMsg) 
	{
	case WM_DESTROY:
		{
			// g_MLService.Quit will set g_pOrigProc to NULL, so save g_pOrgProc.
			WNDPROC pOrigProc = g_pOrigProc;
			g_MLService.Quit();
			return CallWindowProc(pOrigProc, hwnd, uMsg, wParam, lParam); 
		}
		break;
// 	case WM_DISPLAYCHANGE:
// 		{
// 			// 使用Spy++可以看到winamp2的skin被改变之后，会收到此消息和WM_STYLECHANGED
// 			int		nRet;
// 
// 			nRet = CallWindowProc(g_pOrigProc, hwnd, uMsg, wParam, lParam);
// 			// DBGLOG0("Winamp's Skin Changed!");
// 
// 			g_MLService.OnNotifyPlayerSkinChanged();
// 
// 			return nRet;
// 		}
// 		break;
	case WM_COMMAND:
		{
			if (LOWORD(wParam) == 0x9D63)
			{
				int		nRet;
				nRet = CallWindowProc(g_pOrigProc, hwnd, uMsg, wParam, lParam); 

				// DBGLOG0("Winamp's Skin Changed!");
 				WADlg_init(plugin.hwndParent);

 				// set scroll bar bitmap.
 				g_MLService.SetWa5Bmp(1, WACreateScrollBmp());
				g_MLService.OnNotifyPlayerSkinChanged();

				return nRet;
			}
			else if (LOWORD(wParam) == 0x9C53)
			{
				// 顶层窗口
				// g_wndLyricShow.OnCommand(IDC_ALWAYS_ON_TOP);
			}
		}
		break;
	case WM_ACTIVATEAPP:
		{
			LRESULT		result = CallWindowProc(g_pOrigProc, hwnd, uMsg, wParam, lParam);
			g_MLService.OnNotifyPlayerActivate(wParam);
			return result;
		}
		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED || wParam == SIZE_RESTORED)
		{
			LRESULT		result = CallWindowProc(g_pOrigProc, hwnd, uMsg, wParam, lParam);
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
	return CallWindowProc(g_pOrigProc, hwnd, uMsg, wParam, lParam); 
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
	EmptyStr(m_szFullTitle);
	m_dwPlayerVer = 0;
	m_hwndPluginParent = NULL;
	EmptyStr(m_szPlayerName);

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
//			HBITMAP		hBmp;
//			hBmp = WACreateScrollBmp();
//			HDC		hdcbk;
//			HDC		hdc;
//			HBITMAP	hBmpOld;
//			hdc = GetDC(NULL);
//			hdcbk = CreateCompatibleDC(hdc);
//			hBmpOld = (HBITMAP)SelectObject(hdcbk, hBmp);
//			BitBlt(hdc, 0, 0, 200, 200, hdcbk, 0, 0, SRCCOPY);
//			SelectObject(hdcbk, hBmpOld);
//			DeleteDC(hdcbk);
//			ReleaseDC(NULL, hdc);


			// set scroll bar bitmap.
			g_MLService.SetWa5Bmp(1, WACreateScrollBmp());

			procWinampOld = (WNDPROC)SetWindowLong(g_Player.m_hwndPluginParent, GWL_WNDPROC, (LONG)ProcWAPluginParentHook);
		}
	}

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

int CPlayerImp::GetProperty(const char *szProperty, TCHAR *szValue, int nLen)
{
	if (m_dwPlayerVer < 0x2900)
		return PR_UNIMPLEMENTED;

	extendedFileInfoStruct	info;
	char			szFile[MAX_PATH];
	int				nRet;

	WAGetSourceMedia(szFile, CountOf(szFile));
	info.filename = szFile;
	info.retlen = MAX_PATH;

	info.metadata = (char*)szProperty;
	info.ret = szValue;
	memset(szValue, 0, sizeof(TCHAR) * nLen);
	nRet = SendMessage(g_hwndPlayer, WM_WA_IPC, (WPARAM)&info, IPC_GET_EXTENDED_FILE_INFO);

	return PR_OK;
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	if (m_szPlayerName[0] != '\0')
	{
		strcpy_safe(lpszBuffer, nBufSize, m_szPlayerName);
	}
	else
	{
		if (IsWinamp5())
			strcpy_safe(lpszBuffer, nBufSize, "Winamp5");
		else
			strcpy_safe(lpszBuffer, nBufSize, "Winamp2");
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

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
int CPlayerImp::GetFullTitle(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, m_szFullTitle);

	return PR_OK;
}

int CPlayerImp::GetTitle(char * lpszBuffer, int nBufSize)
{
	return GetProperty("title", lpszBuffer, nBufSize);
}

int CPlayerImp::GetArtist(char * lpszBuffer, int nBufSize)
{
	return GetProperty("artist", lpszBuffer, nBufSize);
}

int CPlayerImp::GetAlbum(char * lpszBuffer, int nBufSize)
{
	return GetProperty("album", lpszBuffer, nBufSize);
}

// 返回歌曲的全文件名（如c:\\...., httP;//wwww......)
int CPlayerImp::GetSourceMedia(char * lpszBuffer, int nBufSize)
{
	return WAGetSourceMedia(lpszBuffer, nBufSize);

// 	if (m_bShoutcast)
// 	{
// 		strcat_safe(lpszBuffer, nBufSize, "/shoutcast/");
// 		strcat_safe(lpszBuffer, nBufSize, m_szFullTitle);
// 	}
}

// 歌曲的长度（单位ms）
DWORD CPlayerImp::GetMediaLength()
{
	DWORD	dwLength;

	if (WAGetCurSongLength(g_hwndPlayer, dwLength))
		return dwLength * 1000;
	else
		return 0;
}

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	DWORD		dwCurPos;

	WAGetCurSongPos(g_hwndPlayer, dwCurPos);

	return dwCurPos - m_dwShoutcastTitleChangedPos;
}

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
void CPlayerImp::OnSongChanged(LPCTSTR szFullTitle)
{
	strcpy_safe(m_szFullTitle, CountOf(m_szFullTitle), szFullTitle);
	g_MLService.OnFullTitleChanged(szFullTitle);
}

// 歌曲播放位置改变了
void CPlayerImp::OnPlayTimeChanged(DWORD dwTimeCur)
{
	m_dwPos = dwTimeCur;
	g_MLService.OnPlayTimeChanged(dwTimeCur);
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
	if (g_pOrigProc == NULL)
		g_pOrigProc = (WNDPROC)SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)HookWinampWnd);
	m_bWorking = true;

	m_state = (PLAYER_STATE)GetPlayerState();

	//
	// Create Window to get playback pos timely.
	//
	GetPosWndRegisterClass(g_hInstance);
	GetPosWndCreate(g_hInstance);

	return TRUE;
}

// COMMENT:
//		关闭WINAMP 状态查询线程
void CPlayerImp::Quit()
{
	// 恢复winamp窗口的窗口过程
	if (g_pOrigProc && IsWindow(g_hwndPlayer) && GetWindowLong(g_hwndPlayer, GWL_WNDPROC) == (LONG)HookWinampWnd)
	{
		SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)g_pOrigProc);
		g_pOrigProc = NULL;
	}
	m_bWorking = false;

	if (g_hWndGetPos)
		DestroyWindow(g_hWndGetPos);

	if (IsWindow(g_Player.m_hwndPluginParent))
	{
		DestroyWindow(g_Player.m_hwndPluginParent);
		g_Player.m_hwndPluginParent = NULL;
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

void CPlayerImp::SetTimeUpdateFreq(int nFreq)
{
	GetPosWndSetTimeUpdateFreq(nFreq);
}
