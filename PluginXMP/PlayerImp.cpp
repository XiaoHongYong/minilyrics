#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"
#include "xmpdsp.h"

#define WM_WA_IPC WM_USER 
#define SM_WINAMP_TIMEOUT 1000 
#define IPC_GETOUTPUTTIME 105
#define IPC_ISPLAYING 104

CPlayerImp		g_Player;
DWORD			g_DdeInstID = 0;
HWND			g_hWndPlayer = NULL;

// get player state and song info
PLAYER_STATE GetPlayerState()
{
	DWORD	dwRet = 0;

	SendMessageTimeout(g_hWndPlayer, WM_WA_IPC, 0, IPC_ISPLAYING,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwRet);

	if (dwRet == 1)
		return PS_PLAYING;
	else if (dwRet == 3)
		return PS_PAUSED;
	else
		return PS_STOPED;
}

DWORD GetPlayPos()
{
	DWORD	dwCurPos = 0;

	SendMessageTimeout(g_hWndPlayer, WM_WA_IPC, 0, IPC_GETOUTPUTTIME,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwCurPos);

	return dwCurPos;
}

///////////////////////////////////////////////////////////////////////////////

PLAYER_STATE	g_playerState = PS_STOPED;

//////////////////////////////////////////////////////////////////////////

char * __cdecl stristr (
        const char * str1,
        const char * str2
        )
{
        char *cp = (char *) str1;
        char *s1, *s2;
		int f,l;

        if ( !*str2 )
            return((char *)str1);

        while (*cp)
        {
                s1 = cp;
                s2 = (char *) str2;

                while ( *s1 && *s2)
				{
					if ( ((f = (unsigned char)(*(s1))) >= 'A') && (f <= 'Z') )
						f -= ('A' - 'a');

					if ( ((l = (unsigned char)(*(s2))) >= 'A') && (l <= 'Z') )
						l -= ('A' - 'a');
					if (f == l)
                        s1++, s2++;
					else
						break;
				}

                if (!*s2)
                        return(cp);

                cp++;
        }

        return(NULL);
}

int	strncpy_safe(char *strDestination, int nLenMax, const char *strSource, int nToCopy)
{
	if (nToCopy >= nLenMax)
		nToCopy = nLenMax - 1;

	int		org = nToCopy;

	while (nToCopy && (*strDestination++ = *strSource++))    /* copy string */
		nToCopy--;

	*strDestination = '\0';

	return org - nToCopy;
}

static void *WINAPI DSP_New();
static void WINAPI DSP_Free(void *inst);
static const char *WINAPI DSP_GetDescription(void *inst);
static void WINAPI DSP_Config(void *inst, HWND win);
static DWORD WINAPI DSP_GetConfig(void *inst, void *config);
static BOOL WINAPI DSP_SetConfig(void *inst, void *config, DWORD size);
static void WINAPI DSP_NewTrack(void *inst, const char *file);
static void WINAPI DSP_SetFormat(void *inst, const XMPFORMAT *form);
static void WINAPI DSP_Reset(void *inst);
static DWORD WINAPI DSP_Process(void *inst, float *srce, DWORD count);

static void WINAPI DSP_About(HWND win)
{
	if (g_MLService.IsValid())
		g_MLService.About(win);
	else
		MessageBox(win, "MiniLyrics for XMPlay, 2009 Crintsoft.", "MiniLyrics for XMPlay", MB_OK);
}

static XMPDSP dsp = {
	0, // doesn't support multiple instances or have a tail
	"MiniLyrics for XMPlay",
	DSP_About,
	DSP_New,
	DSP_Free,
	DSP_GetDescription,
	DSP_Config,
	DSP_GetConfig,
	DSP_SetConfig,
	DSP_NewTrack,
	DSP_SetFormat,
	DSP_Reset,
	DSP_Process
};

HDDEDATA CALLBACK DdeCallback(UINT uType,
                              UINT uFmt,
                              HCONV hconv,
                              HSZ hszTopic,
                              HSZ hszItem,
                              HDDEDATA hdata,
                              DWORD dwData1,
                              DWORD dwData2)
{
    HDDEDATA ret = 0;

	// g_MLService.Log("DdeCallback");
	// XMP_Log( "[DEBUG] DdeCallback (uType == %d, uFmt = %d)\n", uType, uFmt );
  /*  switch (uType)
    {
//
//      Allow DDE connections
//
        case XTYP_CONNECT:
            ret = (HDDEDATA)true;
            break;

//
//      Answer the DDE Requests. If the Topic is correct, obviously :P
//
        case XTYP_REQUEST:
//
//          We use the DdeCmpStringHandles instead of the strcmp
//          for two reasons: 1st we already have the strings in HSZ
//          format, 2nd this way it's case insensitive as the DDE
//          protocol is supposed to be.
//
            break;
//
//      No pokes this time...
//
        case XTYP_POKE:
//          Return FACK if poke message is processed
//          ret = DDE_FACK;

            ret = DDE_FNOTPROCESSED;
            break;
    }

    return ret;*/
    return DDE_FNOTPROCESSED;
}

const char * stristr_new_line(const char *text, const char *pattern)
{
	const char *p = text;
	while (1)
	{
		p = stristr(p, pattern);
		if (!p)
			return NULL;
		if (p == text || *(p - 1) == '\r' || *(p - 1) == '\n')
			return p;
		p += strlen(pattern);
	}
	return NULL;
}

/*
ID3v1:
Title	Sometimes (Radio Edit)
Artist	Britney Spears
Album	My Prerogative: Greatest Hits
Year	2005
Track	11

ID3v2:
Title	Sometimes (Radio Edit)
Album	My Prerogative: Greatest Hits
Track	11
Year	2005
Genre	(148)
Unique ID	http://www.cddb"
*/
bool XMP_GetTagField(const char *info1, const char *name, char *buf)
{
	LPCSTR		szBeg1, szBeg, szEnd;
	szBeg1 = stristr_new_line(info1, name);
	if (!szBeg1)
		return false;

	szBeg1 += strlen(name);

	// if id3v2 exist, use it first?
	szBeg = stristr_new_line(szBeg1, name);
	if (szBeg)
		szBeg += strlen(name);
	else
		szBeg = szBeg1;

	szEnd = strchr(szBeg, '\n');
	if (szEnd)
	{
		int		len = int(szEnd - szBeg);
		strncpy_safe(buf, MAX_PATH, szBeg, len);
		buf[len] = '\0';
	}
	else
		strcpy_safe(buf, MAX_PATH, szBeg);

	return true;
}

BOOL XMP_ParseTrackTags(const char *info1, char *artist, char *title, char *album)
{
	EmptyStr(artist);
	EmptyStr(title);
	EmptyStr(album);

	BOOL	status = XMP_GetTagField(info1, "title\t", title);
	if (!status)
		return FALSE;

	status = XMP_GetTagField(info1, "artist\t", artist);
	if (!status)
		status = XMP_GetTagField(info1, "author\t", artist);

	status = XMP_GetTagField(info1, "album\t", album );
	if (!status)
		status = XMP_GetTagField(info1, "wm/albumtitle\t", album);

	return true;
}

enum XMP_CMD
{
	XC_PLAYPAUSE	= 80,
	XC_STOP			= 81,
	XC_NEXT			= 128,
	XC_PREV			= 129,
};

bool XMP_ExeCmd(XMP_CMD cmd)
{
	HSZ		hszService = NULL;
	HSZ		hszTopic = NULL;
	HCONV	hconv = NULL;
	HDDEDATA hdata = NULL;
	LPBYTE	data = NULL;
	TCHAR	szCmd[256];
	int		nLen;

	if (!g_DdeInstID)
		return false;

	hszService = DdeCreateStringHandle(g_DdeInstID, "XMPlay", CP_WINANSI);
	if (!hszService)
	{
		g_MLService.Log("DdeCreateStringHandle FAILED!");
		goto R_FAILED;
	}

	hszTopic = DdeCreateStringHandle(g_DdeInstID, "XMPlay", CP_WINANSI);
	if (!hszTopic)
	{
		g_MLService.Log("DdeCreateStringHandle FAILED!");
		goto R_FAILED;
	}

	hconv = DdeConnect(g_DdeInstID, hszService, hszTopic, 0);
	if (!hconv)
	{
		g_MLService.Log("DdeConnect FAILED!");
		goto R_FAILED;
	}

	sprintf(szCmd, "key%d", cmd);
	nLen = strlen(szCmd) + 1;
	hdata = DdeClientTransaction((LPBYTE)szCmd, nLen, hconv, hszTopic, 0, XTYP_EXECUTE, 1000, 0);
	if (!hdata)
	{
		g_MLService.Log("DdeClientTransaction FAILED!");
		goto R_FAILED;
	}

	DdeFreeDataHandle(hdata);
	DdeFreeStringHandle(g_DdeInstID, hszService);
	DdeFreeStringHandle(g_DdeInstID, hszTopic);
	DdeDisconnect(hconv);
	return true;

R_FAILED:
	if (hdata)
		DdeFreeDataHandle(hdata);
	if (hszService)
		DdeFreeStringHandle(g_DdeInstID, hszService);
	if (hszTopic)
		DdeFreeStringHandle(g_DdeInstID, hszTopic);
	if (hconv)
		DdeDisconnect(hconv);

	return false;
}

bool XMP_GetTrackInfo(char *artist, char *title, char *album)
{
	HSZ		hszService = NULL;
	HSZ		hszTopic = NULL;
	HCONV	hconv = NULL;
	HDDEDATA hdata = NULL;
	LPBYTE	data = NULL;
	DWORD	size;
	DWORD	dwDdeInstID = 0;

	UINT res = DdeInitialize(&dwDdeInstID, DdeCallback, APPCMD_CLIENTONLY |
		MF_ERRORS | MF_LINKS | MF_CONV/* |
		MF_ERRORS | MF_HSZ_INFO | MF_POSTMSGS | MF_SENDMSGS | MF_LINKS |
		MF_CONV*/, 0);
	if (res != DMLERR_NO_ERROR)
		goto R_FAILED;

	hszService = DdeCreateStringHandle(dwDdeInstID, "XMPlay", CP_WINANSI);
	if (!hszService)
	{
		g_MLService.Log("DdeCreateStringHandle FAILED!");
		goto R_FAILED;
	}

	hszTopic = DdeCreateStringHandle(dwDdeInstID, "info1", CP_WINANSI);
	if (!hszTopic)
	{
		g_MLService.Log("DdeCreateStringHandle FAILED!");
		goto R_FAILED;
	}

	hconv = DdeConnect(dwDdeInstID, hszService, hszTopic, 0);
	if (!hconv)
	{
		g_MLService.Log("DdeConnect FAILED!");
		goto R_FAILED;
	}

	hdata = DdeClientTransaction(0, 0, hconv, hszTopic, CF_TEXT, XTYP_REQUEST, 1000, 0);
	if (!hdata)
	{
		g_MLService.Log("DdeClientTransaction FAILED!");
		goto R_FAILED;
	}

	data = DdeAccessData(hdata, &size);
	if (!data)
	{
		g_MLService.Log("DdeAccessData FAILED!");
		goto R_FAILED;
	}

	XMP_ParseTrackTags((const char *)data, artist, title, album);
 
	DdeUnaccessData(hdata);
	DdeFreeDataHandle(hdata);
	DdeFreeStringHandle(dwDdeInstID, hszService);
	DdeFreeStringHandle(dwDdeInstID, hszTopic);
	DdeDisconnect(hconv);
	DdeUninitialize(dwDdeInstID);
	return true;

R_FAILED:
	if (hdata)
		DdeUnaccessData(hdata);
	if (hdata)
		DdeFreeDataHandle(hdata);
	if (hszService)
		DdeFreeStringHandle(dwDdeInstID, hszService);
	if (hszTopic)
		DdeFreeStringHandle(dwDdeInstID, hszTopic);
	if (hconv)
		DdeDisconnect(hconv);

	if (dwDdeInstID)
		DdeUninitialize(dwDdeInstID);

	return false;
}

HANDLE		g_hThread = NULL;
DWORD		g_dwThreadID = 0;

DWORD WINAPI ThreadML(LPVOID lpParam)
{
	g_hWndPlayer = FindWindow("XMPLAY-MAIN", NULL);

	RunMiniLyrics(g_hInstance, g_hWndPlayer, &g_Player);

	MSG			msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	g_MLService.OnDisconnected();

	return 0;
}

// new DSP instance
static void *WINAPI DSP_New()
{
	// Create a thread to pump the messages...
	// ASSERT(g_hThread == NULL);
	g_hThread = CreateThread(NULL, 0, ThreadML, NULL, 0, &g_dwThreadID);

	return (void*)1; // no multi-instance, so just return anything not 0
}

// free DSP instance
static void WINAPI DSP_Free(void *inst)
{
	PostThreadMessage(g_dwThreadID, WM_QUIT, 0, 0);
	WaitForSingleObject(g_hThread, INFINITE);
	CloseHandle(g_hThread);
	g_hThread = NULL;
}

// get description for plugin list
static const char *WINAPI DSP_GetDescription(void *inst)
{
	return dsp.name;
}

// show config options
static void WINAPI DSP_Config(void *inst, HWND win)
{
	if (g_MLService.IsValid())
		g_MLService.Config(win);
	else
		MessageBox(win, "Please run MiniLyrics First.", "MiniLyrics for XMPlay", MB_OK);
}
// get DSP config
static DWORD WINAPI DSP_GetConfig(void *inst, void *config)
{
	return 0;
// 	memcpy(config,&flange,sizeof(flange));
// 	return sizeof(flange); // return size of config info
}
// set DSP config
static BOOL WINAPI DSP_SetConfig(void *inst, void *config, DWORD size)
{
// 	memcpy(&flange,config,sizeof(flange));
	return TRUE;
}

// reset DSP when seeking
static void WINAPI DSP_Reset(void *inst)
{
}

char		g_szCurMediaUrl[MAX_PATH] = "";

// new track has been opened (or closed if file=NULL)
static void WINAPI DSP_NewTrack(void *inst, const char *file)
{
	if (!file)
	{
		EmptyStr(g_szCurMediaUrl);
		if (g_MLService.IsValid())
			g_MLService.OnMediaInfoChanged(L"", NULL, NULL, NULL, 0);
		return;
	}
	strcpy_safe(g_szCurMediaUrl, CountOf(g_szCurMediaUrl), file);

	if (!g_MLService.IsValid())
		return;

	char		artist[MAX_PATH];
	char		title[MAX_PATH];
	char		album[MAX_PATH];

	EmptyStr(artist);
	EmptyStr(title);
	EmptyStr(album);

	XMP_GetTrackInfo(artist, title, album);

	WCHAR		szMediaUrl[MAX_PATH], szArtist[MAX_PATH], szAlbum[MAX_PATH], szTitle[MAX_PATH];

	UTF8ToUCS2(file, strlen(file) + 1, szMediaUrl, CountOf(szMediaUrl));
	UTF8ToUCS2(artist, strlen(artist) + 1, szArtist, CountOf(szArtist));
	UTF8ToUCS2(album, strlen(album) + 1, szAlbum, CountOf(szAlbum));
	UTF8ToUCS2(title, strlen(title) + 1, szTitle, CountOf(szTitle));

	DWORD	dwLength = 0;

	SendMessageTimeout(g_hWndPlayer, WM_WA_IPC, 1, IPC_GETOUTPUTTIME,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwLength);
	dwLength *= 1000;

	g_MLService.OnMediaInfoChanged(szMediaUrl, szArtist, szAlbum, szTitle, dwLength);
}

// set the sample format at start (or end if rate/chans=0) of playback
static void WINAPI DSP_SetFormat(void *inst, const XMPFORMAT *form)
{
}

// process the sample data
static DWORD WINAPI DSP_Process(void *inst, float *buffer, DWORD count)
{
	return count;
}

// the exported function that XMPlay calls to get the plugin interface
extern "C"
{
// 
// // shortcut to enable the DSP
// static void WINAPI ShortcutOn()
// {
// 	if (flange) { // make sure the DSP is setup first
// 		DSP_Reset(flange);
// 		flange->conf.on^=1;
// 		if (confwin) SendDlgItemMessage(confwin,10,BM_SETCHECK,flange->conf.on,0); // update config window if it's open
// 		xmpfmisc->ShowBubble(flange->conf.on?"Flanger = on":"Flanger = off",0); // show a bubble to the user
// 	}
// }
// 
// static const XMPSHORTCUT shortcut={0x10000,"DSP - Flanger on/off",ShortcutOn};

// get the plugin's XMPDSP interface
XMPDSP *WINAPI XMPDSP_GetInterface2(DWORD face, InterfaceProc faceproc)
{
// 	if (face!=XMPDSP_FACE) return NULL;
// 	xmpfmisc=(XMPFUNC_MISC*)faceproc(XMPFUNC_MISC_FACE); // import "misc" functions
// 	xmpfmisc->RegisterShortcut(&shortcut); // register a shortcut
//	return &xmpdsp;
	return &dsp;
}

BOOL WINAPI DllMain(HANDLE hDLL, DWORD reason, LPVOID reserved)
{
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			g_hInstance = (HINSTANCE)hDLL;
			DisableThreadLibraryCalls((HINSTANCE)hDLL);
			break;

		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

}

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
	playerInfo.hWndPlayer = g_hWndPlayer;
	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"XMPlay");

	wcscpy_safe(playerInfo.embeddedSkinName, CountOf(playerInfo.embeddedSkinName), playerInfo.playerName);
	wcscat_safe(playerInfo.embeddedSkinName, CountOf(playerInfo.embeddedSkinName), L"'s Skin");
}

void CPlayerImp::OnConnectedDone()
{
	UINT res = DdeInitialize(&g_DdeInstID, DdeCallback, APPCMD_CLIENTONLY |
		MF_ERRORS | MF_LINKS | MF_CONV/* |
		MF_ERRORS | MF_HSZ_INFO | MF_POSTMSGS | MF_SENDMSGS | MF_LINKS |
		MF_CONV*/, 0);
	if (res != DMLERR_NO_ERROR)
	{
		g_MLService.Log("XMP_InitDde FAILED!");
		return;
	}

	g_MLService.AddWndCloseto(NULL, L"XMPLAY-MAIN", NULL);
	g_MLService.AddWndCloseto(NULL, L"XMPLAY-PANEL", NULL);
	g_MLService.AddWndCloseto(NULL, L"XMPLAY-INFO", NULL);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);

	// Update Media info
	DSP_NewTrack(NULL, g_szCurMediaUrl);

	// Update player state
	g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerImp::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);

	if (g_DdeInstID)
	{
		DdeUninitialize(g_DdeInstID);
		g_DdeInstID = 0;
	}
}

// control player
void CPlayerImp::Play()
{
	if (GetPlayerState() != PS_PLAYING)
		XMP_ExeCmd(XC_PLAYPAUSE);
}

void CPlayerImp::Pause()
{
	if (GetPlayerState() == PS_PAUSED)
		XMP_ExeCmd(XC_PLAYPAUSE);
}

void CPlayerImp::PlayPause()
{
	XMP_ExeCmd(XC_PLAYPAUSE);
}

void CPlayerImp::Stop()
{
	if (GetPlayerState() != PS_STOPED)
		XMP_ExeCmd(XC_STOP);
}

void CPlayerImp::Prev()
{
	XMP_ExeCmd(XC_PREV);
}

void CPlayerImp::Next()
{
	XMP_ExeCmd(XC_NEXT);
}

void CPlayerImp::SeekTo(int nMsPos)
{
}

void CPlayerImp::OnTimer(int nTimerID)
{
	if (!g_MLService.IsValid())
		return;

	// Check player status
	static DWORD dwTimeLastCheck = GetTickCount();
	DWORD dwTimeCur = GetTickCount();
	static DWORD dwPosLastCheck = GetPlayPos();
	DWORD dwPosCur = GetPlayPos();
	if (dwTimeCur - dwTimeLastCheck >= 100)
	{
		// No changes, just return
		if (g_playerState != PS_PLAYING && dwTimeCur == dwPosLastCheck)
			return;

		if (g_playerState == PS_PLAYING && dwPosCur <= dwPosLastCheck
			|| g_playerState != PS_PLAYING && dwPosCur != dwPosLastCheck)
		{
			// Check player stat
			PLAYER_STATE newState = GetPlayerState();
			if (newState != g_playerState)
			{
				g_playerState = newState;
				g_MLService.OnPlayStateChanged(g_playerState);
				dwPosCur = GetPlayPos();
			}
		}
		dwTimeLastCheck = dwTimeCur;
		dwPosLastCheck = dwPosCur;
	}

	g_MLService.OnPlayPosChanged(dwPosCur);
}
