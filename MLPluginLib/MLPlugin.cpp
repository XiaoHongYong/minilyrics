#include "StdAfx.h"
#include "MlPlugin.h"
#include <shlobj.h>

HINSTANCE		g_hInstance = NULL;
CMLService		g_MLService;


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

// Launch "MiniLyrics.exe /Wnd 0xWNDHANDLE /Player "Winamp"" to start MiniLyrics
BOOL RunMiniLyrics(HINSTANCE hInst, HWND hWndPlayer, CPlayerServiceBase *pPlayerService, bool bManual)
{
	ASSERT(hInst);
	ASSERT(pPlayerService);

	g_hInstance = hInst;

	pPlayerService->Create(hInst);

	// Launch "MiniLyrics.exe /Wnd 0xWNDHANDLE /Player "Winamp"" to start MiniLyrics

	TCHAR	szPluginIni[MAX_PATH];
	GetProgramPath(szPluginIni, g_hInstance);
	strcat_safe(szPluginIni, CountOf(szPluginIni), _T("MLPlugin.ini"));

	TCHAR	szWorkingFolder[MAX_PATH];
	GetPrivateProfileString(_T("MiniLyrics"), _T("WorkingFolder"), _T(""), szWorkingFolder, MAX_PATH, szPluginIni);
	if (szWorkingFolder[0] == '\0')
		GetProgramPath(szWorkingFolder, g_hInstance);
	DirStringFormat(szWorkingFolder);

	BOOL	bNoPrompt;
	bNoPrompt = GetPrivateProfileInt(_T("MiniLyrics"), _T("NoPrompt"), FALSE, szPluginIni);

	TCHAR	szMsg[256];
	TCHAR	szMiniLyricsExe[MAX_PATH];
	strcpy_safe(szMiniLyricsExe, CountOf(szMiniLyricsExe), szWorkingFolder);
	strcat_safe(szMiniLyricsExe, CountOf(szMiniLyricsExe), _T("MiniLyrics.exe"));
	if (!IsFileExist(szMiniLyricsExe))
	{
		// Is MiniLyrics in the same drive of Player?
		szMiniLyricsExe[0] = szPluginIni[0];
		if (!IsFileExist(szMiniLyricsExe))
		{
			// Is MiniLyrics in Plugin directory?
			GetProgramPath(szMiniLyricsExe, g_hInstance);
			strcat_safe(szMiniLyricsExe, CountOf(szMiniLyricsExe), _T("MiniLyrics.exe"));
			if (!IsFileExist(szMiniLyricsExe))
			{
				// Is MiniLyrics in Plugin parent directory?
				GetProgramPath(szMiniLyricsExe, g_hInstance);
				strcat_safe(szMiniLyricsExe, CountOf(szMiniLyricsExe), _T("..\\MiniLyrics.exe"));
				if (!IsFileExist(szMiniLyricsExe))
				{
					strcpy_safe(szMsg, CountOf(szMsg), _T("Can't find MiniLyrics.exe, Please reinstall MiniLyrics."));
					goto INIT_FAILED;
				}
			}
		}
	}

	TCHAR	szCmdLine[MAX_PATH];
	_stprintf_s(szCmdLine, CountOf(szCmdLine), _T("%s /Wnd 0x%X"), szMiniLyricsExe, pPlayerService->GetHandle());
	if (bManual)
		strcat_safe(szCmdLine, CountOf(szCmdLine), _T(" /Manual"));
	Execute(szCmdLine);

	return TRUE;

INIT_FAILED:
	if (!bNoPrompt)
	{
		if (WritePrivateProfileInt(_T("MiniLyrics"), _T("NoPrompt"), TRUE, szPluginIni) &&
			GetPrivateProfileInt(_T("MiniLyrics"), _T("NoPrompt"), FALSE, szPluginIni))
		{
			// Make sure NoPrompt write successfully, then prompt this message.
			MessageBox(hWndPlayer , szMsg, _T("MiniLyrics"), MB_OK);
		}
	}
	return FALSE;
}

void DisconnectMiniLyrics(CPlayerServiceBase *pPlayerService)
{
	pPlayerService->Destroy();

	if (g_MLService.IsValid())
		g_MLService.OnDisconnected();
}
