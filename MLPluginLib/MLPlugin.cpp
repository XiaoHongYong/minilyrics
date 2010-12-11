#include "StdAfx.h"
#include "MlPlugin.h"
#include <shlobj.h>

HINSTANCE		g_hInstance = NULL;
CMLService		g_MLService;
IPlayer			*g_pPlayer = NULL;
CPlayerServiceWndBase *g_pPlayerServiceWnd = NULL;


BOOL Execute(LPCTSTR szCmd)
{
	STARTUPINFO			startInfo;
	PROCESS_INFORMATION	procInfo;

	memset(&startInfo, 0, sizeof(startInfo));
	memset(&procInfo, 0, sizeof(procInfo));

	if (!CreateProcess(NULL, (LPTSTR)szCmd, NULL, NULL, false, 0, NULL, NULL, &startInfo, &procInfo))
		return FALSE;

	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);

	return TRUE;
}

BOOL RunMiniLyrics(HINSTANCE hInst, HWND hWndPlayer, IPlayer *pPlayer, CPlayerServiceWndBase *pPlayerServiceWnd)
{
	ASSERT(hInst);
	ASSERT(pPlayer);
	ASSERT(pPlayerServiceWnd);

	g_pPlayer = pPlayer;
	g_hInstance = hInst;
	g_pPlayerServiceWnd = pPlayerServiceWnd;

	pPlayerServiceWnd->Create(hInst);

	// Launch "MiniLyrics.exe /Wnd 0xWNDHANDLE /Player "Winamp"" to start MiniLyrics

	char	szPluginIni[MAX_PATH];
	GetProgramPath(szPluginIni, g_hInstance);
	strcat_safe(szPluginIni, CountOf(szPluginIni), "MLPlugin.ini");

	char	szWorkingFolder[MAX_PATH];
	GetPrivateProfileStringA("MiniLyrics", "WorkingFolder", "", szWorkingFolder, MAX_PATH, szPluginIni);
	if (szWorkingFolder[0] == '\0')
		GetProgramPath(szWorkingFolder, g_hInstance);
	DirStringFormat(szWorkingFolder);

	BOOL	bNoPrompt;
	bNoPrompt = GetPrivateProfileIntA("MiniLyrics", "NoPrompt", FALSE, szPluginIni);

	char	szMsg[256];
	char	szMiniLyricsExe[MAX_PATH];
	strcpy_safe(szMiniLyricsExe, CountOf(szMiniLyricsExe), szWorkingFolder);
	strcat_safe(szMiniLyricsExe, CountOf(szMiniLyricsExe), "MiniLyrics.exe");
	if (!IsFileExist(szMiniLyricsExe))
	{
		// Is MiniLyrics in the same drive of Player?
		szMiniLyricsExe[0] = szPluginIni[0];
		if (!IsFileExist(szMiniLyricsExe))
		{
			// Is MiniLyrics in Plugin directory?
			if (!IsFileExist("MiniLyrics.exe"))
			{
				strcpy_safe(szMsg, CountOf(szMsg), "Can't find MiniLyrics.exe, Please reinstall MiniLyrics.");
				goto INIT_FAILED;
			}
		}
	}

	char	szCmdLine[MAX_PATH];
	sprintf_s(szCmdLine, CountOf(szCmdLine), "%s /Wnd 0x%X", szMiniLyricsExe, pPlayerServiceWnd->GetHandle());
	Execute(szCmdLine);

	return TRUE;

INIT_FAILED:
	if (!bNoPrompt)
	{
		if (WritePrivateProfileInt("MiniLyrics", "NoPrompt", TRUE, szPluginIni) &&
			GetPrivateProfileIntA("MiniLyrics", "NoPrompt", FALSE, szPluginIni))
		{
			// Make sure NoPrompt write successfully, then prompt this message.
			MessageBoxA(hWndPlayer , szMsg, "MiniLyrics", MB_OK);
		}
	}
	return FALSE;
}

void DisconnectMiniLyrics()
{
	g_pPlayerServiceWnd->Destroy();

	if (g_MLService.IsValid())
		g_MLService.OnDisconnected();
}
