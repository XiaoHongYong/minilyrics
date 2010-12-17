#include "../MLPluginLib/mlplugin.h"
#include "PlayerImp.h"

#include "bsp.h"
#include "bspplg.h"

CPlayerImp			g_Player;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

PpluginInfo			g_pInfo;

#define DBGMSG(param)		MessageBox(NULL, param, "Message", MB_OK);

BOOL SDMSG(UINT msg, WPARAM wParam, LPARAM lParam, DWORD &dwResult)
{
	return SendMessageTimeout(g_pInfo->hwndParent, msg, wParam, lParam, SMTO_ABORTIFHUNG | SMTO_NORMAL, 1000, (DWORD *)&dwResult);
}

BOOL SDMSG(UINT msg, WPARAM wParam)
{
	DWORD dwResult;
	return SendMessageTimeout(g_pInfo->hwndParent, msg, wParam, 0, SMTO_ABORTIFHUNG | SMTO_NORMAL, 1000, (DWORD *)&dwResult);
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
			g_MLService.OnDisconnected();
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
				g_Player.OnMediaChanged();
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
		strcpy_safe(plgInfo->description, CountOf(plgInfo->description), "MiniLyrics plug-in");
		plgInfo->plgCallback = CallbackProc;

		g_pInfo = plgInfo;

		RunMiniLyrics(g_hInstance, plgInfo->hwndParent, &g_Player);

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

void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = g_pInfo->hwndParent;

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"BSPlayer");
}

void CPlayerImp::OnConnect()
{
	g_MLService.AddWndCloseto(NULL, L"BSPVideoWindow", NULL);
	g_MLService.AddWndCloseto(g_pInfo->hwndParent, NULL, NULL);
}

void CPlayerImp::OnConnectedDone()
{
	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);

	// Update Media info
	// OnMediaChanged();

	// Update player state
	g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerImp::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
}

// control player
void CPlayerImp::Play()
{
	SDMSG(WM_BSP_CMD, BSP_Play);
}

void CPlayerImp::Pause()
{
	SDMSG(WM_BSP_CMD, BSP_Pause);
}

void CPlayerImp::PlayPause()
{
	if (GetPlayerState() == PS_STOPED)
		Play();
	else
		Pause();
}

void CPlayerImp::Stop()
{
	SDMSG(WM_BSP_CMD, BSP_Stop);
}

void CPlayerImp::Prev()
{
	SDMSG(WM_BSP_CMD, BSP_Prev);
}

void CPlayerImp::Next()
{
	SDMSG(WM_BSP_CMD, BSP_Next);
}

void CPlayerImp::SeekTo(int nMsPos)
{
	DWORD		dwValue;
	SDMSG(WM_BSP_CMD, BSP_Seek, nMsPos, dwValue);
}

// get player state and song info
PLAYER_STATE CPlayerImp::GetPlayerState()
{
	DWORD		dwValue;

	if (!SDMSG(WM_BSP_CMD, BSP_GetStatus, 0, dwValue))
		return PS_STOPED;

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

DWORD CPlayerImp::GetPlayPos()
{
	DWORD		dwCurPos;

	if (!SDMSG(WM_BSP_CMD, BSP_GetMovPos, 0, dwCurPos))
		dwCurPos = 0;

	if (dwCurPos < 0)
		dwCurPos = 0;

	return dwCurPos;
}

void CPlayerImp::OnMediaChanged()
{
	char		szMediaUrl[MAX_PATH];
	DWORD		dwLength;

	GetBSPPlayingMediaUrl(szMediaUrl, MAX_PATH);

	if (!SDMSG(WM_BSP_CMD, BSP_GetMovLen, 0, dwLength))
		dwLength = 0;

	g_MLService.OnMediaInfoChanged(szMediaUrl, NULL, NULL, NULL, dwLength);
}

void CPlayerImp::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		g_MLService.OnPlayPosChanged(GetPlayPos());
	}
}
