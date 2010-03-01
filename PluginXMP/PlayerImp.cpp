#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"
#include "xmpdsp.h"
#include <string>
using namespace std;

#define WM_WA_IPC WM_USER 
#define SM_WINAMP_TIMEOUT 1000 
#define IPC_GETOUTPUTTIME 105
#define IPC_ISPLAYING 104

CPlayerImp		g_Player;
DWORD			g_DdeInstID = 0;


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

// 安全的字符串拷贝，如果超出长度范围，则进行截断
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

static XMPDSP dsp={
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
	g_hwndPlayer = FindWindow("XMPLAY-MAIN", NULL);

	if (InitMiniLyricsPlugin(g_hInstance, &g_Player, ML_SUPPORT_EMBEDDED | ML_EMBEDDED_WND_IMMOVABLE | ML_EMBEDDED_SUPPORT_CLASSIC_SKINS))
	{
		// now we can use other g_MLService functions.
		char	szMsg[256];

		wsprintf(szMsg, "Start XMPlay plugin for MiniLyrics. HINSTANCE: %#X", g_hInstance);
		g_MLService.Log(szMsg);
	}

	// if Init failed, Don't return, because "Restart MiniLyrics" function need the message loop below.

	MSG			msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	g_MLService.Quit();

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

// new track has been opened (or closed if file=NULL)
static void WINAPI DSP_NewTrack(void *inst, const char *file)
{
	if (!file)
	{
		g_MLService.OnMediaInfoChanged(NULL);
		return;
	}

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

	MLMediaInfo		mediaInfo;
	mediaInfo.szMediaUrl = szMediaUrl;
	mediaInfo.szArtist = szArtist;
	mediaInfo.szAlbum = szAlbum;
	mediaInfo.szTitle = szTitle;

	DWORD	dwLength = 0;

	SendMessageTimeout(g_hwndPlayer, WM_WA_IPC, 1, IPC_GETOUTPUTTIME,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwLength);

	mediaInfo.nMediaLength = dwLength * 1000;

	g_MLService.OnMediaInfoChanged(&mediaInfo);
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


///////////////////////////////////////////////////////////////////////////////



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
		g_MLService.OnPlayTimeChanged(g_Player.GetPlayPos());
	}

};

CPlayerInfoQueryWnd	g_wndInfoQuery;

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
	strcpy_safe(lpszBuffer, nBufSize, "XMPlay");

	return 1;
}

// get player state and song info
DWORD CPlayerImp::GetPlayerState()
{
	DWORD	dwRet = 0;

	SendMessageTimeout(g_hwndPlayer, WM_WA_IPC, 0, IPC_ISPLAYING,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwRet);

	if (dwRet == 1)
		return PS_PLAYING;
	else if (dwRet == 3)
		return PS_PAUSED;
	else
		return PS_STOPED;
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	DWORD	dwCurPos = 0;

	SendMessageTimeout(g_hwndPlayer, WM_WA_IPC, 0, IPC_GETOUTPUTTIME,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwCurPos);

	return dwCurPos;
}

// control player
BOOL CPlayerImp::Play()
{
	if (GetPlayerState() != PS_PLAYING)
		XMP_ExeCmd(XC_PLAYPAUSE);

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	if (GetPlayerState() == PS_PAUSED)
		XMP_ExeCmd(XC_PLAYPAUSE);

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	XMP_ExeCmd(XC_PLAYPAUSE);

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	if (GetPlayerState() != PS_STOPED)
		XMP_ExeCmd(XC_STOP);

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	XMP_ExeCmd(XC_PREV);

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	XMP_ExeCmd(XC_NEXT);

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
// 	DWORD	nResult;
// 
// 	return SendMessageTimeout(g_hwndPlayer, WM_WA_IPC, nMsPos, IPC_JUMPTOTIME,
// 		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, (LPDWORD)&nResult);
	return FALSE;
}

BOOL CPlayerImp::Init()
{
	UINT res = DdeInitialize(&g_DdeInstID, DdeCallback, APPCMD_CLIENTONLY |
		MF_ERRORS | MF_LINKS | MF_CONV/* |
		MF_ERRORS | MF_HSZ_INFO | MF_POSTMSGS | MF_SENDMSGS | MF_LINKS |
		MF_CONV*/, 0);
	if (res != DMLERR_NO_ERROR)
	{
		g_MLService.Log("XMP_InitDde FAILED!");
		return false;
	}

	g_MLService.AddWndCloseto(NULL, "XMPLAY-MAIN", NULL);
	g_MLService.AddWndCloseto(NULL, "XMPLAY-PANEL", NULL);
	g_MLService.AddWndCloseto(NULL, "XMPLAY-INFO", NULL);

	g_wndInfoQuery.Create(g_hInstance);

	return TRUE;
}

void CPlayerImp::Quit()
{
	if (g_DdeInstID)
	{
		DdeUninitialize(g_DdeInstID);
		g_DdeInstID = 0;
	}

	g_wndInfoQuery.Destroy();
}

void CPlayerImp::OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan)
{
	g_wndInfoQuery.SetUpdatePosTimer(nPlayPosUpdateSpan);
}
