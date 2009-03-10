#include "MlPlugin.h"
#include <shlobj.h>

HINSTANCE		g_hInstance = NULL;
HMODULE			g_hModMiniLyrics = NULL;
CMLService		g_MLService;
HWND			g_hwndPlayer = NULL;
PlayerServiceEntry g_PlayerEntry;
bool			g_bIsUcs2Prefer = false;

IPlayer			*g_pPlayer = NULL;

int MBCSToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut, int nCodePage)
{
	int		n;

	n = MultiByteToWideChar(nCodePage, 0,
		str, nLen, strOut, nOut);
	if (n == 0 && nLen > 0 && GetLastError() == ERROR_INVALID_PARAMETER)
	{
		n = MultiByteToWideChar(CP_ACP, 0,
			str, nLen, strOut, nOut);
	}

	strOut[n] = '\0';

	return n;
}

int UTF8ToUCS2(const char* text, int length, WCHAR *ucs2, int ubuflen)
{
	return MBCSToUCS2(text, length, ucs2, ubuflen, CP_UTF8);
}

// PURPOSE   : Get full task save file name by task name.
//
// PARAMETER: 
//      szPath is the path that returned: format is like : "c:\\antivirus\\"
BOOL GetProgramPath(char szPath[], HINSTANCE hInstance/* = NULL*/)
{
	__try
	{
		int nLen;

		nLen = GetModuleFileNameA(hInstance, szPath, MAX_PATH);
		nLen --;
		while (nLen >= 0 && szPath[nLen] != '\\')
			nLen --;
		if (nLen < 0)
			return FALSE;

		szPath[nLen + 1] = '\0';
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL WritePrivateProfileInt(
							LPCSTR lpAppName,  // section name
							LPCSTR lpKeyName,  // key name
							int nValue,			// int to add
							LPCSTR lpFileName  // initialization file
							)
{
	char	szBuffer[16];

	wsprintfA(szBuffer, "%d", nValue);
	return WritePrivateProfileStringA(lpAppName, lpKeyName, szBuffer, lpFileName);
}

int RegGetProfileInt(HKEY hRoot, LPCTSTR szSubKey, LPCTSTR szKeyName, int nDefault)
{
	HKEY	hKey;
	int		nValue = nDefault;

	if (RegOpenKeyEx(hRoot, szSubKey,
		0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD		nSizeRet = sizeof(nDefault);
		DWORD		dwType = REG_DWORD;
		if (RegQueryValueEx(hKey, szKeyName, NULL, &dwType, (BYTE *)&nValue, &nSizeRet) != ERROR_SUCCESS)
		{
			// LOG1(LOG_LVL_ERROR, _T("RegQueryValueEx: %s, FAILED!"), szKeyName);
		}
		RegCloseKey(hKey);
	}

	return nValue;
}

BOOL RegWriteProfileInt(HKEY hRoot, LPCTSTR szSubKey, LPCTSTR szKeyName, int nValue)
{
	HKEY	hKey;
	DWORD	dwDispositon;
	BOOL	bRet = FALSE;

	if (RegCreateKeyEx(hRoot, szSubKey,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &dwDispositon) == ERROR_SUCCESS)
	{
		DWORD		dwType = REG_DWORD;
		if (RegSetValueEx(hKey, szKeyName, 0, dwType, (BYTE *)&nValue, sizeof(nValue)) == ERROR_SUCCESS)
		{
			bRet = TRUE;
		}
		RegCloseKey(hKey);
	}

	return bRet;
}

#define SZ_REG_MINILYRICS		"SOFTWARE\\MiniLyrics"

char *strcpy_safe(char *strDestination, int nLenMax, const char *strSource)
{
	char	*cp = strDestination;
	char	*cpEnd = strDestination + nLenMax - 1;

	for (; cp < cpEnd && *strSource; cp++, strSource++)
	{
		*cp = *strSource;
	}

	*cp = '\0';

	return(strDestination);
}

// 安全的字符串拷贝，如果超出长度范围，则进行截断
char *strcat_safe(char * dst, int Size, const char * src)
{
	char * cp = dst;
	char * endcp = dst + Size - 1;

	while( *cp )
		cp++;                   /* find end of dst */

	while((cp < endcp) && (*cp++ = *src++)) ;       /* Copy src to end of dst */

	*cp = '\0';

	return( dst );                  /* return dst */
}


wchar_t *wcscpy_safe(wchar_t *strDestination, int nLenMax, const wchar_t *strSource)
{
    wchar_t	*cp = strDestination;
	wchar_t	*cpEnd = strDestination + nLenMax - 1;

	for (; cp < cpEnd && *strSource; cp++, strSource++)
	{
		*cp = *strSource;
	}

	*cp = '\0';

    return(strDestination);
}

WCHAR * __cdecl wcscat_safe (
        WCHAR * dst,
		int	Size,
        const WCHAR * src
        )
{
        WCHAR * cp = dst;
		WCHAR * endcp = dst + Size - 1;

        while( *cp )
                cp++;                   /* find end of dst */

        while((cp < endcp) && (*cp++ = *src++)) ;       /* Copy src to end of dst */

		*cp = '\0';

        return( dst );                  /* return dst */

}

string UrlGetTitle(LPCSTR szFile)
{
	LPCSTR		szBeg, szBeg2, szEnd;
	int			nLen;
	string		strFileTitle;

	szEnd = strrchr(szFile, '.');

	szBeg = strrchr(szFile, '/');
	szBeg2 = strrchr(szFile, '\\');
	if (szBeg2 > szBeg)
		szBeg = szBeg2;

	if (szBeg == NULL)
		szBeg = szFile;
	else
		szBeg++;

	if (szEnd == NULL || szBeg > szEnd)
		nLen = szFile - szBeg + strlen(szFile);
	else
		nLen = szEnd - szBeg;

	strFileTitle.append(szBeg, nLen);

	return strFileTitle;
}

wstring UrlGetTitle(LPCWSTR szFile)
{
	LPCWSTR		szBeg, szBeg2, szEnd;
	int			nLen;
	wstring		strFileTitle;

	szEnd = wcsrchr(szFile, '.');

	szBeg = wcsrchr(szFile, '/');
	szBeg2 = wcsrchr(szFile, '\\');
	if (szBeg2 > szBeg)
		szBeg = szBeg2;

	if (szBeg == NULL)
		szBeg = szFile;
	else
		szBeg++;

	if (szEnd == NULL || szBeg > szEnd)
		nLen = szFile - szBeg + wcslen(szFile);
	else
		nLen = szEnd - szBeg;

	strFileTitle.append(szBeg, nLen);

	return strFileTitle;
}

void FormatFullTitle(LPCSTR szArtist, LPCSTR szTitle, LPCSTR szUri, string &strFullTitle)
{
	strFullTitle.resize(0);
	if (!IsEmptyString(szArtist))
	{
		strFullTitle = szArtist;
		strFullTitle += " - ";
	}

	strFullTitle += szTitle;
	if (strFullTitle.empty())
		strFullTitle = UrlGetTitle(szUri);
}

void FormatFullTitle(LPCWSTR szArtist, LPCWSTR szTitle, LPCWSTR szUri, wstring &strFullTitle)
{
	strFullTitle.resize(0);
	if (!IsEmptyString(szArtist))
	{
		strFullTitle = szArtist;
		strFullTitle += L" - ";
	}

	strFullTitle += szTitle;
	if (strFullTitle.empty())
		strFullTitle = UrlGetTitle(szUri);
}

bool IsFileExist(LPCSTR szFileName)
{
	return (GetFileAttributesA(szFileName) != -1);
}

void DirStringFormat(LPSTR	szDir)
{
	int		nLen;

	nLen = strlen(szDir);
	if (nLen > 0)
	{
		if (szDir[nLen - 1] != '\\')
		{
			szDir[nLen] = '\\';
			szDir[nLen + 1] = '\0';
		}
	}
}


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
	case cmdGetMediaLength:	// param1, long * length, ms
		return g_pPlayer->GetMediaLength();
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

	//
	// The messages bellow are Deprecated, use MLMediaInfoChanged instead.
	//

	// UNICODE supported messages
	case cmdSetUcs2:
		if (Param1)
			g_bIsUcs2Prefer = true;
		return g_pPlayer->IsUcs2Support();
	case cmdGetSourceMediaW:
		return g_pPlayer->GetSourceMediaW((WCHAR *)Param1, Param2);
	case cmdGetArtistW:
		return g_pPlayer->GetArtistW((WCHAR *)Param1, Param2);
	case cmdGetTitleW:
		return g_pPlayer->GetTitleW((WCHAR *)Param1, Param2);
	case cmdGetAlbumW:
		return g_pPlayer->GetAlbumW((WCHAR *)Param1, Param2);
	case cmdGetSourceMedia:	// param1, const char *buffer, param2, buffer length
		return g_pPlayer->GetSourceMedia((char *)Param1, Param2);
	case cmdGetArtist:
		return g_pPlayer->GetArtist((char *)Param1, Param2);
	case cmdGetTitle:
		return g_pPlayer->GetTitle((char *)Param1, Param2);
	case cmdGetAlbum:
		return g_pPlayer->GetAlbum((char *)Param1, Param2);
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

void CMLService::OnMediaInfoChanged(LPCSTR szMediaUrl, LPCSTR szArtist, LPCSTR szAlbum, LPCSTR szTitle)
{
	MLMediaInfo		mediaInfo;
	wchar_t			wszMediaUrl[MAX_PATH], wszArtist[MAX_PATH], wszAlbum[MAX_PATH], wszTitle[MAX_PATH];

	EmptyStr(wszMediaUrl);
	EmptyStr(wszArtist);
	EmptyStr(wszAlbum);
	EmptyStr(wszTitle);

	mediaInfo.szMediaUrl = wszMediaUrl;
	mediaInfo.szArtist = wszArtist;
	mediaInfo.szAlbum = wszAlbum;
	mediaInfo.szTitle = wszTitle;

	MBCSToUCS2(szMediaUrl, -1, wszMediaUrl, CountOf(wszMediaUrl), CP_ACP);
	MBCSToUCS2(szArtist, -1, wszArtist, CountOf(wszArtist), CP_ACP);
	MBCSToUCS2(szAlbum, -1, wszAlbum, CountOf(wszAlbum), CP_ACP);
	MBCSToUCS2(szTitle, -1, wszTitle, CountOf(wszTitle), CP_ACP);

	OnMediaInfoChanged(&mediaInfo);
}
