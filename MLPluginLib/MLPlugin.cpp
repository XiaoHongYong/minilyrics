#include "StdAfx.h"
#include "MlPlugin.h"
#include <shlobj.h>

HINSTANCE		g_hInstance = NULL;
HMODULE			g_hModMiniLyrics = NULL;
CMLService		g_MLService;
HWND			g_hwndPlayer = NULL;
PlayerServiceEntry g_PlayerEntry;

IPlayer			*g_pPlayer = NULL;

bool IsWin2kOrLater()
{
	OSVERSIONINFO		version;

	memset(&version, 0, sizeof(version));
	version.dwOSVersionInfoSize = sizeof(version);

	GetVersionEx(&version);

	// Major version: 5, windows 2000
	if (version.dwMajorVersion >= 5)
		return true;
	else
		return false;
}

BOOL InitMiniLyricsPlugin(HINSTANCE hInst, IPlayer *pPlayer, DWORD mlStartupFlags)
{
	char	szMiniLyricsDll[MAX_PATH];
	char	szPluginIni[MAX_PATH];
	BOOL	bNoPrompt;
	char	szMsg[256];
	char	szWorkingFolder[MAX_PATH];


	FUNMiniLyricsInitEntryPoint	funMLInitEntryPoint;

	g_hInstance = hInst;
	g_pPlayer = pPlayer;

	g_PlayerEntry.uVersion = ML_PLAYER_IF_VERSION;
	g_PlayerEntry.uSize = sizeof(g_PlayerEntry);
	g_PlayerEntry.funPlayerService = PlayerImpService;

	GetProgramPath(szPluginIni, g_hInstance);
	strcat_safe(szPluginIni, CountOf(szPluginIni), "MLPlugin.ini");

	bNoPrompt = GetPrivateProfileIntA("MiniLyrics", "NoPrompt", FALSE, szPluginIni);

	GetPrivateProfileStringA("MiniLyrics", "WorkingFolder", "", szWorkingFolder, MAX_PATH, szPluginIni);
	if (szWorkingFolder[0] == '\0')
		GetProgramPath(szWorkingFolder, g_hInstance);
	DirStringFormat(szWorkingFolder);

	//
	if (!IsWin2kOrLater())
	{
		strcpy_safe(szMiniLyricsDll, CountOf(szMiniLyricsDll), szWorkingFolder);
		strcat_safe(szMiniLyricsDll, CountOf(szMiniLyricsDll), "UnicoWs.dll");
		if (!IsFileExist(szMiniLyricsDll))
		{
			strcpy_safe(szMsg, CountOf(szMsg), "Can't find UnicoWs.dll, Please reinstall MiniLyrics!");
			goto INIT_FAILED;
		}

		g_hModMiniLyrics = LoadLibraryA(szMiniLyricsDll);
		if (!g_hModMiniLyrics)
		{
			wsprintfA(szMsg, "Load UnicoWs.dll dynamic library: %s FAILED!", szMiniLyricsDll);
			goto INIT_FAILED;
		}
	}

	strcpy_safe(szMiniLyricsDll, CountOf(szMiniLyricsDll), szWorkingFolder);
	strcat_safe(szMiniLyricsDll, CountOf(szMiniLyricsDll), "MiniLyrics.dll");
	if (!IsFileExist(szMiniLyricsDll))
	{
		// Is MiniLyrics in the same drive of Player?
		szMiniLyricsDll[0] = szPluginIni[0];
		if (!IsFileExist(szMiniLyricsDll))
		{
			// Is MiniLyrics in Plugin directory?
			if (!IsFileExist("MiniLyrics.dll"))
			{
				strcpy_safe(szMsg, CountOf(szMsg), "Can't find MiniLyrics.dll, Please reinstall MiniLyrics!");
				goto INIT_FAILED;
			}
		}
	}

	g_hModMiniLyrics = LoadLibraryA(szMiniLyricsDll);
	if (!g_hModMiniLyrics)
	{
		wsprintfA(szMsg, "Load MiniLyrics dynamic library: %s FAILED!", szMiniLyricsDll);
		goto INIT_FAILED;
	}

	funMLInitEntryPoint = (FUNMiniLyricsInitEntryPoint)GetProcAddress(g_hModMiniLyrics, "MiniLyricsInitEntryPoint");
	assert(funMLInitEntryPoint);
	if (funMLInitEntryPoint)
	{
		MLServiceEntry	*pMLService;

		pMLService = funMLInitEntryPoint(&g_PlayerEntry);
		if (!pMLService)
			return FALSE;

		if (!g_MLService.SetEntryPoint(pMLService))
			return FALSE;
	}
	else
	{
		strcpy_safe(szMsg, CountOf(szMsg), "Get Minilyrics.dll entry point FAILED!");
		goto INIT_FAILED;
	}

	// Initialize minilyrics's all module
	if (!g_MLService.Init(g_hModMiniLyrics, mlStartupFlags))
		return FALSE;

	return TRUE;

INIT_FAILED:
	if (!bNoPrompt)
	{
		if (WritePrivateProfileInt("MiniLyrics", "NoPrompt", TRUE, szPluginIni) &&
			GetPrivateProfileIntA("MiniLyrics", "NoPrompt", FALSE, szPluginIni))
		{
			// Make sure NoPrompt write sucessfully, then prompt this message.
			MessageBoxA(g_hwndPlayer, szMsg, "MiniLyrics", MB_OK);
		}
	}
	return FALSE;
}

long PlayerImpService(int nCommand, MLPARAM Param1, MLPARAM Param2)
{
	if (!g_pPlayer)
		return -1;
	switch(nCommand)
	{
	case cmdPlay:
		g_pPlayer->Play();
		break;
	case cmdPause:
		g_pPlayer->Pause();
		break;
	case cmdPlayPause:
		g_pPlayer->PlayPause();
		break;
	case cmdStop:
		g_pPlayer->Stop();
		break;
	case cmdPrev:
		g_pPlayer->Prev();
		break;
	case cmdNext:
		g_pPlayer->Next();
		break;
	case cmdSeek:	// param1, seek pos, ms
		g_pPlayer->SeekTo(Param1);
		break;
	case cmdGetPos:	// param1, long * pos, ms
		return g_pPlayer->GetPlayPos();
	case cmdGetPlayerState:	// param1, long * player state,1-stop, 2-play, 3-pause
		return g_pPlayer->GetPlayerState();
	case cmdGetPlayerName:	// param1, const char *buffer, param2, buffer length
		return g_pPlayer->GetPlayerName((char *)Param1, Param2);
	case cmdInit:			// init the player interface
		g_pPlayer->Init();
		break;
	case cmdQuit:			// quit the player interface
		g_pPlayer->Quit();
		break;
	case cmdIsFeatureSupported:			// quit the player interface
		return g_pPlayer->IsFeatureSupported(PlayerFeature(Param1));
	// for winamp2
	case cmdGetCurSkinPath:	// param1, const char *buffer, param2, buffer length
		return g_pPlayer->GetCurSkinPath((char *)Param1, Param2);
	case cmdGetMainWnd:
		return (long)g_pPlayer->GetMainWnd();
	case cmdHasVideo:
		return (long)g_pPlayer->HasVideo();
	case cmdGetPlayerPlugInstance:
		return (long)g_hInstance;
	case cmdGetAttachedParentWnd:
		return (long)g_pPlayer->GetAttachedParentWnd((RECT*)Param1);
	case notifySkinLoaded:
		g_pPlayer->OnNotifySkinLoaded();
		return 1;
	case notifyDetachedFromParentWnd:
		g_pPlayer->OnNotifyDetachedFromParentWnd();
		return 1;
	case cmdGetEmbeddedSkinName:
		return g_pPlayer->GetEmbeddedSkinName((char *)Param1, Param2);
	case cmdGetSkinThemeColor:
		return g_pPlayer->GetSkinThemeColor((SKIN_CLR_NAME)Param1, (COLORREF *)Param2);
	case cmdSetTimeUpdateFreq:
		// set play time postion's update frequency
		// 0: slow, 1 : normal: 2: fast
		{
			int			nPlayPosUpdateSpan = 50;
			if (Param1 == 0)
				nPlayPosUpdateSpan = 100;
			else if (Param1 == 1)
				nPlayPosUpdateSpan = 50;
			else
				nPlayPosUpdateSpan = 30;
			g_pPlayer->OnPlayPosUpdateSpanChanged(nPlayPosUpdateSpan);
		}
		break;
	default:
		// unknown command
		{
			char	szMsg[256];
			wsprintfA(szMsg, "Unprocessed player control message: %d", nCommand);
			g_MLService.Log(szMsg);
			return -1;
		}
		break;
	}

	return 1;
}

void CMLService::OnMediaInfoChanged(LPCSTR szMediaUrl, LPCSTR szArtist, LPCSTR szAlbum, LPCSTR szTitle, unsigned long nMediaLength)
{
	MLMediaInfo		mediaInfo;
	wchar_t			wszMediaUrl[512], wszArtist[512], wszAlbum[512], wszTitle[512];

	EmptyStr(wszMediaUrl);
	EmptyStr(wszArtist);
	EmptyStr(wszAlbum);
	EmptyStr(wszTitle);

	mediaInfo.szMediaUrl = wszMediaUrl;
	mediaInfo.szArtist = wszArtist;
	mediaInfo.szAlbum = wszAlbum;
	mediaInfo.szTitle = wszTitle;
	mediaInfo.nMediaLength = nMediaLength;

	MBCSToUCS2(szMediaUrl, -1, wszMediaUrl, CountOf(wszMediaUrl), CP_ACP);
	if (szArtist)
		MBCSToUCS2(szArtist, -1, wszArtist, CountOf(wszArtist), CP_ACP);
	if (szAlbum)
		MBCSToUCS2(szAlbum, -1, wszAlbum, CountOf(wszAlbum), CP_ACP);
	if (szTitle)
		MBCSToUCS2(szTitle, -1, wszTitle, CountOf(wszTitle), CP_ACP);

	OnMediaInfoChanged(&mediaInfo);
}
