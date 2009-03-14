#include "../MLPluginLib/mlplugin.h"
#include "PlayerImp.h"
#include "../MLPluginLib/PlayerInfoQueryBaseWnd.h"

#include "bsp.h"
#include "bspplg.h"

CPlayerImp			g_Player;

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
		if (nTimerID == TIMER_GETPOS)
		{
			g_MLService.OnPlayTimeChanged(g_Player.GetPlayPos());
		}
	}

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPlayerInfoQueryWnd	g_wndInfoQuery;
PpluginInfo			g_pInfo;

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

void GetBSPPlayingMediaUrl(char szMediaUrl[], int nLen)
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

	strcpy_safe(szMediaUrl, nLen, buf);
}

int __stdcall CallbackProc(const DWORD cmdID,void * param1, void * param2)
{
	switch (cmdID) {
		case EV_UNLOAD:
			g_MLService.Quit();
			break;
		case EV_BEFORE_FILE_OPEN:
			break;
		case EV_FILE_OPEN:
			break;
		case EV_CONFIG:
			g_MLService.Config((HWND)param1);
			break;
		case EV_ABOUT:
			g_MLService.About((HWND)param1);
			break;
		case EV_LOAD_SUB:
			break;
		case EV_PLAY:
			{
				char		szMediaUrl[MAX_PATH];
				DWORD		dwLength;

				GetBSPPlayingMediaUrl(szMediaUrl, MAX_PATH);

				if (!SDMSG(WM_BSP_CMD, BSP_GetMovLen, 0, dwLength))
					dwLength = 0;

				g_MLService.OnMediaInfoChanged(szMediaUrl, NULL, NULL, NULL, dwLength);
			}
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

		if (!InitMiniLyricsPlugin(g_hInstance, &g_Player))
			return TRUE;	// we must return 0!

		// now we can use other g_MLService functions.
		char	szMsg[256];

		wsprintf(szMsg, "Start BSPlayer Player plugin for MiniLyrics. HINSTANCE: %#X", g_hInstance);
		g_MLService.Log(szMsg);

		return TRUE;
	}

#ifdef __cplusplus
}
#endif


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
	strcpy_safe(lpszBuffer, nBufSize, "BSPlayer");

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

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
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

	g_wndInfoQuery.Create(g_hInstance);

	return TRUE;
}

// COMMENT:
void CPlayerImp::Quit()
{
	g_wndInfoQuery.Destroy();
}

int CPlayerImp::HasVideo()
{
	return -1;
}

void CPlayerImp::OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan)
{
	g_wndInfoQuery.SetUpdatePosTimer(nPlayPosUpdateSpan);
}
