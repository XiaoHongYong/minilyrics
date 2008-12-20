#include "../MLPluginLib/mlplugin.h"
#include "PlayerImp.h"

#include "bsp.h"
#include "bspplg.h"

#include <PROCESS.H>

CPlayerImp		g_Player;
int				g_UpdateTimeSpan = 50;
WNDPROC			procPlayer = 0;
PpluginInfo		g_pInfo;

#define DBGMSG(param)		MessageBox(NULL, param, "Message", MB_OK);

bool SDMSG(UINT msg, WPARAM wParam, LPARAM lParam, DWORD &dwResult)
{
	return SendMessageTimeout(g_pInfo->hwndParent, msg, wParam, lParam, SMTO_ABORTIFHUNG | SMTO_NORMAL, 1000, (DWORD *)&dwResult);
}


#ifdef __cplusplus
extern "C" {
#endif

BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		g_hInstance = (HINSTANCE)hModule;

	return TRUE;
}

#ifdef __cplusplus
}
#endif
// 
// int loadSubtitles(const char * subName)
// {
// 	int subID;
// 
// 	if (!g_pInfo->createSubs || !g_pInfo->addLine)
// 		return 0;
// 
// 	//You would parse subtitle(subName) file here
// 	//but for example we will just add a few lines
// 
// 	//Create new subtitles
// 	subID = g_pInfo->createSubs("MiniLyrics subtitles");
// 
// 	//Add new line that will start at 5 sec and last to 12 sec
// 	g_pInfo->addLine(subID,5000,12000,"This is subtitle from 5 to 12 sec");
// 
// 	//Add few more lines
// 	g_pInfo->addLine(subID,15000,17000,"<b>This is subtitle from 15 to 17 sec bold</b>");
// 	g_pInfo->addLine(subID,20000,24000,"This is subtitle from 20 to 24 sec\\nLine 2");
// 
// 	//g_pInfo->activateSubs(subID);
// 
// 	return 0; //All OK
// }

int __stdcall CallbackProc(const DWORD cmdID,void * param1, void * param2)
{
	switch (cmdID) {
		case EV_UNLOAD:
			// DBGMSG("EV_UNLOAD");
			g_MLService.Quit();
			break;
		case EV_BEFORE_FILE_OPEN:
			break;
		case EV_FILE_OPEN:
			// loadSubtitles("");
			break;
		case EV_CONFIG:
			g_MLService.Config((HWND)param1);
			break;
		case EV_ABOUT:
			g_MLService.About((HWND)param1);
			break;
		case EV_LOAD_SUB:
			break;
			// return loadSubtitles((const char *)param1);
		case EV_PLAY:
			g_Player.OnSongChanged();
			break;
/*		case EV_STOP:
			break;
		default:
			{
				char	szMsg[256];
				wsprintf(szMsg, "Unknown: %d", cmdID);
				DBGMSG(szMsg);
			}
			break;*/
	}

	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

	BOOL __stdcall pluginInit(PpluginInfo plgInfo)
	{
		strcpy(plgInfo->description, "MiniLyrics plug-in");
		plgInfo->plgCallback = CallbackProc;

		g_pInfo = plgInfo;
		// g_pInfo->subRegisterExt("*.*");

		if (!InitMiniLyricsPlugin(g_hInstance, &g_Player))
			return TRUE;	// we must return 0!

		// now we can use other g_MLService functions.
		char	szMsg[256];

		wsprintf(szMsg, "Start Winamp2 Player plugin for minilyrics. HINSTANCE: %#X", g_hInstance);
		g_MLService.Log(szMsg);

		return TRUE;
	}

#ifdef __cplusplus
}
#endif


// format string pathname ---> filename
// "c:\windows\command.com" ----> "command"
void FileGetTitle(LPTSTR szBuff, int nBuffLen, LPCTSTR szFile)
{
	LPCTSTR		szBeg, szEnd;
	int			nLen;

	szEnd = strrchr(szFile, '.');

	szBeg = strrchr(szFile, '\\');
	if (szBeg == NULL)
		szBeg = szFile;
	else
		szBeg++;

	if (szEnd == NULL || szBeg > szEnd)
		nLen = szFile - szBeg + strlen(szFile);
	else
		nLen = szEnd - szBeg;

	strncpy(szBuff, szBeg, nLen);
	szBuff[nLen] = '\0';
}

WNDPROC			g_pOrigProc = NULL;
HANDLE			g_hEventThrdQuery = NULL;			// 停止 查询 播放器 状态的线程 的事件
HANDLE			g_hThreadPlayPosQuery = NULL;			// 查询 播放器 状态的线程

void ThreadPlayPosQuery(void *);

LRESULT CALLBACK HookWndProcPlayer(
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
// 	case WM_DESTROY:
// 		{
// 			// g_MLService.Quit will set g_pOrigProc to NULL, so save g_pOrgProc.
// 			WNDPROC pOrigProc = g_pOrigProc;
// 			g_MLService.Quit();
// 			return CallWindowProc(pOrigProc, hwnd, uMsg, wParam, lParam); 
// 		}
// 		break;
	case WM_ACTIVATEAPP:
		g_MLService.OnNotifyPlayerActivate(wParam);
		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED || wParam == SIZE_RESTORED)
			g_MLService.OnNotifyPlayerMinRestore(wParam);
		break;
	case WM_LBUTTONDOWN:
		{
			HWND		hWndChain[5];
			hWndChain[0] = g_pInfo->hwndParent;
			hWndChain[1] = g_pInfo->hwndVideoWin;
			g_MLService.OnNotifyBeforeTrackMove(hWndChain, 2);
		}
		break;
	case WM_MOVE:
		g_MLService.OnNotifyTrackMove((int)(short) LOWORD(lParam), (int)(short) HIWORD(lParam));
		break;
	default:
		break;
	};

	// Call Player Window Proc
	return CallWindowProc(g_pOrigProc, hwnd, uMsg, wParam, lParam); 
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	EmptyStr(m_szFullTitle);

	m_bWorking = false;
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "bsplayer");

	return 1;
}

// get player state and song info
DWORD CPlayerImp::GetPlayerState()
{
	DWORD		dwValue;

	if (!SDMSG(WM_BSP_CMD, BSP_GetStatus, 0, dwValue))
		dwValue = 0;

	// Return player status
	// 0 - STOP
	// 1 - PAUSE
	// 2 - PLAY
	// 4 - No movie open
	switch (dwValue)
	{
	case 0: return PS_STOPED;
	case 1: return PS_PAUSED;
	case 2: return PS_PLAYING;
	default: break;
	}
	return PS_STOPED;
}

int CPlayerImp::GetCurSkinPath(char * lpszBuffer, int nBufSize)
{
	return -1;
}

int CPlayerImp::GetProxy(char * lpszBuffer, int nBufSize)
{
	return -1;
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
int CPlayerImp::GetFullTitle(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, m_szFullTitle);

	return 1;
}

// 返回歌曲的全文件名（如c:\\...., httP;//wwww......)
int CPlayerImp::GetSourceMedia(char * lpszBuffer, int nBufSize)
{
	COPYDATASTRUCT	cds;
	char			buf[MAX_PATH];
	void			*adr;
	DWORD			dwResult;
	
	EmptyStr(buf);
	adr = &buf;
	cds.dwData = BSP_GetFileName;
	cds.lpData = &adr;
	cds.cbData = sizeof(buf);

	SendMessageTimeout(g_pInfo->hwndParent, WM_COPYDATA, (WPARAM)g_MLService.GetMainWindow(), (LPARAM)&cds, SMTO_ABORTIFHUNG | SMTO_NORMAL, 1000, &dwResult);

	strcpy_safe(lpszBuffer, nBufSize, buf);

	return 1;
}

// 歌曲的长度（单位ms）
DWORD CPlayerImp::GetMediaLength()
{
	DWORD	dwLength;

	if (!SDMSG(WM_BSP_CMD, BSP_GetMovLen, 0, dwLength))
		dwLength = 0;

	return dwLength;
}

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	DWORD		dwCurPos;

	if (!SDMSG(WM_BSP_CMD, BSP_GetMovPos, 0, dwCurPos))
		dwCurPos = 0;

	if (dwCurPos < 0)
		dwCurPos = 0;

	return dwCurPos;
}

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
void CPlayerImp::OnSongChanged()
{
	TCHAR	szMediaSrc[MAX_PATH];

	GetSourceMedia(szMediaSrc, CountOf(szMediaSrc));
	if (strcmp(szMediaSrc, m_szMediaSrc) == 0)
		return;

	FileGetTitle(m_szFullTitle, CountOf(m_szFullTitle), szMediaSrc);

	g_MLService.OnFullTitleChanged(m_szFullTitle);
}

// 歌曲播放位置改变了
void CPlayerImp::OnPlayTimeChanged(DWORD dwTimeCur)
{
	g_MLService.OnPlayTimeChanged(dwTimeCur);
}

// control player
BOOL CPlayerImp::Play()
{
	DWORD		dwValue;

	if (!SDMSG(WM_BSP_CMD, BSP_Play, 0, dwValue))
		dwValue = 0;

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	DWORD		dwValue;

	if (!SDMSG(WM_BSP_CMD, BSP_Pause, 0, dwValue))
		dwValue = 0;

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	if (GetPlayerState() == PS_STOPED)
		Play();
	else
		Pause();

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	DWORD		dwValue;

	if (!SDMSG(WM_BSP_CMD, BSP_Stop, 0, dwValue))
		dwValue = 0;

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	DWORD		dwValue;

	if (!SDMSG(WM_BSP_CMD, BSP_Prev, 0, dwValue))
		dwValue = 0;

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	DWORD		dwValue;

	if (!SDMSG(WM_BSP_CMD, BSP_Next, 0, dwValue))
		dwValue = 0;

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	DWORD		dwValue;

	return SDMSG(WM_BSP_CMD, BSP_Seek, nMsPos, dwValue);
}

BOOL CPlayerImp::Init()
{
	g_hwndPlayer = g_pInfo->hwndParent;

	g_MLService.AddWndCloseto(NULL, "BSPVideoWindow", NULL);
	g_MLService.AddWndCloseto(g_hwndPlayer, NULL, NULL);

	// Hook Main Window
//	if (g_pOrigProc == NULL)
//		g_pOrigProc = (WNDPROC)SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)HookWndProcPlayer);
	m_bWorking = true;

	//
	// 创建线程查询播放器的状态
	g_hEventThrdQuery = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(g_hEventThrdQuery);

	g_hThreadPlayPosQuery = (HANDLE)_beginthread(ThreadPlayPosQuery, 0, NULL);
	if (g_hThreadPlayPosQuery == NULL)
	{
		return FALSE;
	}

	return TRUE;
}

// COMMENT:
void CPlayerImp::Quit()
{
	// 恢复winamp窗口的窗口过程
	if (g_pOrigProc && IsWindow(g_hwndPlayer) && GetWindowLong(g_hwndPlayer, GWL_WNDPROC) == (LONG)HookWndProcPlayer)
	{
		SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)g_pOrigProc);
		g_pOrigProc = NULL;
	}
	m_bWorking = false;

	// 关闭状态查询线程
	// 设置退出线程的事件
	// 等待，直到线程退出，如果超时，则强制终止该线程
	SetEvent(g_hEventThrdQuery);
	DWORD dwRet = WaitForSingleObject(g_hThreadPlayPosQuery, 1000);
	if (dwRet != WAIT_OBJECT_0 && dwRet != -1)
	{
		TerminateThread(g_hThreadPlayPosQuery, 0);
	}

	g_hThreadPlayPosQuery = NULL;
	CloseHandle(g_hEventThrdQuery);
	g_hEventThrdQuery = NULL;
}

int CPlayerImp::HasVideo()
{
	return -1;
}

void CPlayerImp::SetTimeUpdateFreq(int nFreq)
{
	if (nFreq == 0)
		g_UpdateTimeSpan = 100;
	else if (nFreq == 1)
		g_UpdateTimeSpan = 50;
	else
		g_UpdateTimeSpan = 10;
}

void ThreadPlayPosQuery(void *)
{
	DWORD	dwRet;
	DWORD	dwPosLast = 0;
	TCHAR   szSongLast[MAX_PATH] = "";
	TCHAR	szSong[MAX_PATH] = "";
	int		nCount = 5;

	while (1)
	{
		dwRet = WaitForSingleObject(g_hEventThrdQuery, g_UpdateTimeSpan);

		if (dwRet == WAIT_TIMEOUT)
		{
			// update current play time.
			g_Player.OnPlayTimeChanged(g_Player.GetPlayPos());
		}
		else if (dwRet == WAIT_OBJECT_0)
		{
			// Exit Thread
			break;
		}
		else
		{
			// Other return value
			break;
		}
	}
}
