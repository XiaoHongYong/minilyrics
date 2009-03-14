#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"
#include "QCDModGeneral.h"
#include "QCDCtrlMsgs.h"
#include <process.h>

CPlayerImp		g_Player;
HANDLE			g_hThread = NULL;
DWORD			g_dwThreadId = 0;
bool			g_bCurMediaVideo = false;	// Is Video file?

inline void QCDPlayerCommand(DWORD dwCmd)
{
	SendMessage(g_hwndPlayer, WM_COMMAND, dwCmd, 0);
}

QCDModInitGen	*QCDCallbacks;


//-----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		g_hInstance = hInst;
	}
	return TRUE;
}


//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	//
	// TODO : Cleanup
	//
	g_MLService.Quit();
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	//
	// TODO : Show your "configuration" dialog.
	//
	g_MLService.Config(g_hwndPlayer);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	//
	// TODO : Show your "about" dialog.
	//
	g_MLService.About(g_hwndPlayer);
}
//-------------------------------------------------------------------------

//-----------------------------------------------------------------------------
DWORD WINAPI ThreadMiniLyrics(
						LPVOID lpParameter
						)
{
	if (!InitMiniLyricsPlugin(g_hInstance, &g_Player))
		return 0;

	// now we can use other g_MLService functions.
	char	szMsg[256];

	wsprintf(szMsg, "Start QCD Player plugin for minilyrics. HINSTANCE: %#X", g_hInstance);
	g_MLService.Log(szMsg);

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

PLUGIN_API BOOL GENERALDLL_ENTRY_POINT(QCDModInitGen *ModInit, QCDModInfo *ModInfo)
{
	ModInit->version = PLUGIN_API_VERSION;
	ModInfo->moduleString = "MiniLyrics for QCD Player plugin";

	ModInit->toModule.ShutDown			= ShutDown;
	ModInit->toModule.About				= About;
	ModInit->toModule.Configure			= Configure;

	QCDCallbacks = ModInit;

	g_hwndPlayer = (HWND)QCDCallbacks->Service(opGetParentWnd, 0, 0, 0);


	g_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMiniLyrics, NULL, 0, &g_dwThreadId);
	CloseHandle(g_hThread);
	g_hThread = NULL;

	// return TRUE for successful initialization
	return TRUE;
}

WNDPROC		pOrigProc = NULL;

LRESULT CALLBACK HookQCDPWnd(
							 HWND hwnd,      // handle to window
							 UINT uMsg,      // message identifier
							 WPARAM wParam,  // first message parameter
							 LPARAM lParam   // second message parameter
							 )
{
	switch(uMsg) 
	{
	case WM_LBUTTONDOWN:
		{
			HWND		hWndChain[2];
			hWndChain[0] = FindWindow("PlayerCanvas", NULL);
			hWndChain[1] = FindWindow("ExtPlayerCanvas", NULL);
			g_MLService.OnNotifyBeforeTrackMove(hWndChain, 2);
		}
		break;
	case WM_MOVE:
		g_MLService.OnNotifyTrackMove((int)(short) LOWORD(lParam), (int)(short) HIWORD(lParam));
		break;
	case WM_PN_PLAYBACKPROGRESS:
/*		TCHAR	szMsg[256];
		wsprintf(szMsg, "Position Changed: %d, %d, %d, %d, %d", wParam, lParam,
			QCDCallbacks->Service(opGetTime, NULL, 0, 0), QCDCallbacks->Service(opGetTime, NULL, 1, 0), QCDCallbacks->Service(opGetTime, NULL, 2, 0));
		g_MLService.Log(szMsg);
		// DBGLOG2("Position Changed: %d, %d", wParam, lParam);*/
		g_Player.OnPlayTimeChanged(g_bCurMediaVideo ? lParam : lParam * 10);
		break;
	case WM_PN_PLAYLISTCHANGED:
		// 保证当播放列表变空时清空 MiniLyrics 的歌词显示内容：
		if(QCDCallbacks->Service(opGetNumTracks, NULL, 0, 0) == 0)
			g_MLService.OnMediaInfoChanged(NULL);
		break;
	case WM_PN_INFOCHANGED:
		// DBGLOG0("WM_PN_INFOCHANGED");
	case WM_PN_TRACKCHANGED:
		// DBGLOG0("WM_PN_TRACKCHANGED");
	case WM_PN_PLAYSTARTED:
		{
			char	szArtist[MAX_PATH] = "", szAlbum[MAX_PATH] = "", szTitle[MAX_PATH] = "";
			char	szSrcMedia[MAX_PATH] = "";
			int		nRet;

			// QCDCallbacks->Service(opGetOutputTime, NULL, -1, 0);
			nRet = QCDCallbacks->Service(opGetMediaType, NULL, -1, 0);
			if (nRet == DIGITAL_VIDEOFILE_MEDIA)
				g_bCurMediaVideo = true;
			else
				g_bCurMediaVideo = false;

			nRet = QCDCallbacks->Service(opGetArtistName, szArtist, MAX_PATH, -1);

			nRet = QCDCallbacks->Service(opGetDiscName, szAlbum, MAX_PATH, -1);

			nRet = QCDCallbacks->Service(opGetTrackName, szTitle, MAX_PATH, -1);

			EmptyStr(szSrcMedia);
			nRet = QCDCallbacks->Service(opGetTrackFile, szSrcMedia, CountOf(szSrcMedia), -1);

			DWORD	dwLength = QCDCallbacks->Service(opGetTrackLength, NULL, -1, 1);

			g_MLService.OnMediaInfoChanged(szSrcMedia, szArtist, szAlbum, szTitle, dwLength);
		}
		break;
/*	case WM_PN_PLAYSTOPPED:
		// DBGLOG0("WM_PN_PLAYSTOPPED");
		break;
	case WM_PN_PLAYPAUSED:
		// DBGLOG0("WM_PN_PLAYPAUSED");
		break;
	case WM_PN_PLAYDONE:
		// DBGLOG0("WM_PN_PLAYDONE");
		break;
	case WM_PN_MEDIAEJECTED:
		// DBGLOG0("WM_PN_MEDIAEJECTED");
		break;
	case WM_PN_MEDIAINSERTED:
		// DBGLOG0("WM_PN_MEDIAINSERTED");
		break;*/
	default:
		break;
	}

	// Call qcd player Window Proc
	return CallWindowProc(pOrigProc, hwnd, uMsg, wParam, lParam); 
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	m_dwPos = 0;
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "Quintessential Player");

	return 1;
}

// get player state and song info
DWORD CPlayerImp::GetPlayerState()
{
	return QCDCallbacks->Service(opGetPlayerState, NULL, 0, 0);
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

DWORD CPlayerImp::GetPlayPos()
{
	return m_dwPos;
}

int CPlayerImp::HasVideo()
{
	return g_bCurMediaVideo;
}

void CPlayerImp::OnPlayTimeChanged(DWORD dwTimeCur)
{
	m_dwPos = dwTimeCur;
	g_MLService.OnPlayTimeChanged(dwTimeCur);
}

// control player
BOOL CPlayerImp::Play()
{
	QCDPlayerCommand(QCD_COMMAND_PLAY);

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	QCDPlayerCommand(QCD_COMMAND_PAUSE);

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	QCDPlayerCommand(QCD_COMMAND_PAUSE);

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	QCDPlayerCommand(QCD_COMMAND_STOP);

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	QCDPlayerCommand(QCD_COMMAND_TRKBWD);

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	QCDPlayerCommand(QCD_COMMAND_TRKFWD);

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	return QCDCallbacks->Service(opSetSeekPosition, NULL, nMsPos, 1);
}

BOOL CPlayerImp::Init()
{
	// DBGLOG0("CPlayerImp::Init()");

	g_MLService.AddWndCloseto(NULL, "ExtPlayerCanvas", NULL);
	g_MLService.AddWndCloseto(NULL, "PlayerCanvas", NULL);

	// Hook Main Window
 	if (pOrigProc == NULL)
 		pOrigProc = (WNDPROC)SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)HookQCDPWnd);

	return TRUE;
}

void CPlayerImp::Quit()
{
	if (pOrigProc)
	{
		SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)pOrigProc);
		pOrigProc = NULL;
	}

	// 发送消息，释放迷你歌词窗口的相关资源（如：删除托盘栏上的图标）
	if (g_MLService.GetMainWindow())
	{
		ShowWindow(g_MLService.GetMainWindow(), SW_HIDE);
		SendMessage(g_MLService.GetMainWindow(), WM_DESTROY, 0, 0);
	}
}
