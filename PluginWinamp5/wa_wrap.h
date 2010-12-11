#ifndef _WA_WRAP_H_	
#define _WA_WRAP_H_



/**************************************************************************/

/*
** Finally there are some WM_COMMAND messages that you can use to send 
** Winamp misc commands.
** 
** To send these, use:
**
** SendMessage(hwnd_winamp, WM_COMMAND,command_name,0);
*/

#define WINAMP_OPTIONS_EQ               40036 // toggles the EQ window
#define WINAMP_OPTIONS_PLEDIT           40040 // toggles the playlist window
#define WINAMP_VOLUMEUP                 40058 // turns the volume up a little
#define WINAMP_VOLUMEDOWN               40059 // turns the volume down a little
#define WINAMP_FFWD5S                   40060 // fast forwards 5 seconds
#define WINAMP_REW5S                    40061 // rewinds 5 seconds

// the following are the five main control buttons, with optionally shift 
// or control pressed
// (for the exact functions of each, just try it out)
#define WINAMP_BUTTON1                  40044
#define WINAMP_BUTTON2                  40045
#define WINAMP_BUTTON3                  40046
#define WINAMP_BUTTON4                  40047
#define WINAMP_BUTTON5                  40048
#define WINAMP_BUTTON1_SHIFT            40144
#define WINAMP_BUTTON2_SHIFT            40145
#define WINAMP_BUTTON3_SHIFT            40146
#define WINAMP_BUTTON4_SHIFT            40147
#define WINAMP_BUTTON5_SHIFT            40148
#define WINAMP_BUTTON1_CTRL             40154
#define WINAMP_BUTTON2_CTRL             40155
#define WINAMP_BUTTON3_CTRL             40156
#define WINAMP_BUTTON4_CTRL             40157
#define WINAMP_BUTTON5_CTRL             40158

#define WINAMP_FILE_PLAY                40029 // pops up the load file(s) box
#define WINAMP_OPTIONS_PREFS            40012 // pops up the preferences
#define WINAMP_OPTIONS_AOT              40019 // toggles always on top
#define WINAMP_HELP_ABOUT               40041 // pops up the about box :)


#define WM_WA_IPC WM_USER
#define SM_WINAMP_TIMEOUT 1000

// #define IPC_GETPLAYLISTTITLE 212
BOOL WAGetPlayListTitle(HWND hwndWinamp, int index, LPSTR &szTitle)
{
	return SendMessageTimeout(hwndWinamp, WM_WA_IPC, index, IPC_GETPLAYLISTTITLE,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, (LPDWORD)&szTitle);
}

BOOL WAGetPlayListTitleW(HWND hwndWinamp, int index, LPWSTR &szTitle)
{
	return SendMessageTimeout(hwndWinamp, WM_WA_IPC, index, IPC_GETPLAYLISTTITLEW,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, (LPDWORD)&szTitle);
}

/*
** int version = SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GETVERSION);
**
** Version will be 0x20yx for winamp 2.yx. versions previous to Winamp 2.0
** typically (but not always) use 0x1zyx for 1.zx versions. Weird, I know.
**
** The basic format for sending messages to Winamp is:
** int result=SendMessage(hwnd_winamp,WM_WA_IPC,command_data,command);
** (for the version check, command_data is 0).
*/
BOOL WAGetVersion(HWND hwndWinamp, DWORD &dwVersion)
{
	return SendMessageTimeout(hwndWinamp, WM_WA_IPC, 0, IPC_GETVERSION,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwVersion);
}


BOOL WAPrev(HWND hwndWinamp)
{
	DWORD	dwResult;
	return SendMessageTimeout(hwndWinamp, WM_COMMAND, WINAMP_BUTTON1, 0,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwResult);
}

BOOL WAPlayPause(HWND hwndWinamp)
{
	DWORD	dwResult;
	return SendMessageTimeout(hwndWinamp, WM_COMMAND, WINAMP_BUTTON3, 0,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwResult);
}

BOOL WAStop(HWND hwndWinamp)
{
	DWORD	dwResult;
	return SendMessageTimeout(hwndWinamp, WM_COMMAND, WINAMP_BUTTON4, 0,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwResult);
}

BOOL WANext(HWND hwndWinamp)
{
	DWORD	dwResult;
	return SendMessageTimeout(hwndWinamp, WM_COMMAND, WINAMP_BUTTON5, 0,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwResult);
}

BOOL WAPlay(HWND hwndWinamp)
{
	DWORD	dwResult;
	return SendMessageTimeout(hwndWinamp, WM_COMMAND, WINAMP_BUTTON2, 0,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwResult);
}

/*
** int res = SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_ISPLAYING);
**
** IPC_ISPLAYING returns the status of playback.
** If it returns 1, it is playing. if it returns 3, it is paused, 
** if it returns 0, it is not playing.
*/
BOOL WAIsPlaying(HWND hwndWinamp, int &nPlayState)
{
	return SendMessageTimeout(hwndWinamp, WM_WA_IPC, 0, IPC_ISPLAYING,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, (LPDWORD)&nPlayState);
}


BOOL WAGetCurSongPos(HWND hwndWinamp, DWORD &dwCurPos)
{
	return SendMessageTimeout(hwndWinamp, WM_WA_IPC, 0, IPC_GETOUTPUTTIME,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwCurPos);
}

BOOL WAGetCurSongLength(HWND hwndWinamp, DWORD &dwLength)
{
	return SendMessageTimeout(hwndWinamp, WM_WA_IPC, 1, IPC_GETOUTPUTTIME,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwLength);
}

BOOL WAJumpToTime(HWND hwndWinamp, DWORD msPos, int &nResult)
{
	return SendMessageTimeout(hwndWinamp, WM_WA_IPC, msPos, IPC_JUMPTOTIME,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, (LPDWORD)&nResult);
}

// #define IPC_GETLISTLENGTH 124
BOOL WAGetListLength(HWND hwndWinamp, DWORD &dwLength)
{
	return SendMessageTimeout(hwndWinamp, WM_WA_IPC, 0, IPC_GETLISTLENGTH,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwLength);
}

// #define IPC_GETSKIN 201
BOOL WAGetSkin(HWND hwndWinamp, LPWSTR skinname_buffer)
{
	DWORD	dwResult;
	char	szSkin[MAX_PATH] = "";
	BOOL bRet = SendMessageTimeout(hwndWinamp, WM_WA_IPC, (WPARAM)szSkin, IPC_GETSKIN,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwResult);
	MBCSToUCS2(szSkin, strlen(szSkin), skinname_buffer, MAX_PATH);
	return bRet;
}

// #define IPC_GETPLAYLISTFILE 211
BOOL WAGetPlayListFile(HWND hwndWinamp, int index, LPSTR &szFile)
{
	return SendMessageTimeout(hwndWinamp, WM_WA_IPC, index, IPC_GETPLAYLISTFILE,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, (LPDWORD)&szFile);
}

// #define IPC_GETLISTPOS 125
BOOL WAGetListPos(HWND hwndWinamp, DWORD &dwPos)
{
	return SendMessageTimeout(hwndWinamp, WM_WA_IPC, 0, IPC_GETLISTPOS,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwPos);
}

BOOL WAGetPlayingSongTitle(HWND hwndWinamp, wchar_t szTitleOut[], int nBuffLen)
{
	DWORD		dwPos = 0, dwLength = 0;

	szTitleOut[0] = '\0';

	//
	// 通过从列表中取得歌曲的方式
	if (WAGetListPos(hwndWinamp, dwPos) && WAGetListLength(hwndWinamp, dwLength))
	{
		if (dwPos < dwLength)
		{
			LPWSTR		szTitleW = NULL;
			if (WAGetPlayListTitleW(hwndWinamp, dwPos, szTitleW) && (DWORD)szTitleW > 0xFFFF)
			{
				wcscpy_safe(szTitleOut, nBuffLen, szTitleW);
				return TRUE;
			}

			LPSTR		szTitle = NULL;
			if (WAGetPlayListTitle(hwndWinamp, dwPos, szTitle) && szTitle)
			{
				MBCSToUCS2(szTitle, -1, szTitleOut, nBuffLen, CP_ACP);
				return TRUE;
			}
		}
	}

	return FALSE;
}

#endif
