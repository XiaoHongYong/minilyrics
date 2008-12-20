#include "../MLPluginLib/MLPlugin.h"
#include <tchar.h>
#include "PlayerImp.h"
#include <process.H>
#include "xmpdsp.h"
#include <string>
using namespace std;

CPlayerImp		g_Player;
int				g_UpdateTimeSpan = 50;
char			_szMediaPathName[512];
DWORD			g_DdeInstID = 0;

#define MXS(c,x,s)			(((c) - (x)) <<  (s))
#define M80S(c,s)			MXS(c,0x80,s)
#define UTF8_1_to_UCS2(in)	((WCHAR) (in)[0])
#define UTF8_2_to_UCS2(in)	((WCHAR) (MXS((in)[0],0xC0, 6) | M80S((in)[1], 0)))
#define UTF8_3_to_UCS2(in)	((WCHAR) (MXS((in)[0],0xE0,12) | M80S((in)[1], 6) | M80S((in)[2], 0) ))

int UTF8ToUCS2(unsigned char* text, int length, WCHAR *ucs2, int ubuflen)
{
	unsigned char* p = text;
	unsigned char* last = text + length;
	int		ul;
	for(p = text, ul = 0; (p < last) && (ul < ubuflen) ; )
	{
		if( (*p) < 0x80)
		{	
			ucs2[ul++] = UTF8_1_to_UCS2(p);
			p += 1;
		}
		else if( (*p) < 0xc0 )
		{	
			assert( (*p) );	// Invalid UTF8 First Byte
			ucs2[ul++] = '?';
			p += 1;
		}
		else if( (*p) < 0xe0)
		{
			ucs2[ul++] = UTF8_2_to_UCS2(p);
			p += 2;
		}
		else if( (*p) < 0xf0)
		{
			ucs2[ul++] = UTF8_3_to_UCS2(p);
			p += 3;
		}
		else if( (*p) < 0xf8)
		{
			ucs2[ul++] = '?';
			p += 4;
		}
		else if( (*p) < 0xfc)
		{
			ucs2[ul++] = '?';
			p += 5;
		}
		else if( (*p) < 0xfe)
		{
			ucs2[ul++] = '?';
			p += 6;
		}
		else
		{
			assert( (*p) );	// Invalid UTF8 First Byte
			ucs2[ul++] = '?';
			p += 1;
		}
	}
	return ul;
}

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
		MessageBox(win, "MiniLyrics for XMPlay, 2006 Crintsoft.", "MiniLyrics for XMPlay", MB_OK);
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
	szBeg1 = stristr(info1, name);
	if (!szBeg1)
		return false;
	szBeg1 += strlen(name);

	// if id3v2 exist, use it first?
	szBeg = stristr(info1, name);
	if (szBeg)
		szBeg += strlen(name);
	else
		szBeg = szBeg1;

	szEnd = strchr(szBeg, '\n');
	if (szEnd)
	{
		int		len = int(szEnd - szBeg);
		strncpy(buf, szBeg, len);
		buf[len] = '\0';
	}
	else
		strcpy(buf, szBeg);

	return true;
}

BOOL XMP_ParseTrackTags(const char *info1, char *artist, char *title)
{
	BOOL	status = XMP_GetTagField(info1, "artist\t", artist);
	if (!status)
		status = XMP_GetTagField(info1, "author\t", artist);

	if( !status )
		return FALSE;

	status = XMP_GetTagField(info1, "title\t", title );
	if (!status)
		return FALSE;
// 
// 	status = XMP_GetTagField(info1, "album\t", album );
// 	if (!status)
// 		status = XMP_GetTagField(info1, "wm/albumtitle\t", album );

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

	XMP_ParseTrackTags((const char *)data, artist, title);
 
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

// format string pathname ---> filename
// "c:\windows\command.com" ----> "command"
void FileGetTitle(LPTSTR szBuff, int nBuffLen, LPCTSTR szFile)
{
	LPCTSTR		szBeg, szEnd;
	int			nLen;

	szEnd = strrchr(szFile, '.');

	szBeg = strrchr(szFile, '\\');
	if (szBeg == NULL)
	{
		szBeg = strrchr(szFile, '/');
		if (!szBeg)
			szBeg = szFile;
		else
			szBeg++;
	}
	else
		szBeg++;

	if (szEnd == NULL || szBeg > szEnd)
		nLen = szFile - szBeg + lstrlen(szFile);
	else
		nLen = szEnd - szBeg;

	strncpy(szBuff, szBeg, nLen);
	szBuff[nLen] = '\0';
}

HANDLE		g_hThread = NULL;
DWORD		g_dwThreadID = 0;

DWORD WINAPI ThreadML(LPVOID lpParam)
{
	g_hwndPlayer = FindWindow("XMPLAY-MAIN", NULL);

	if (!InitMiniLyricsPlugin(g_hInstance, &g_Player))
		return 0;	// we must return 0!

	// now we can use other g_MLService functions.
	char	szMsg[256];

	wsprintf(szMsg, "Start XMPlay plugin for minilyrics. HINSTANCE: %#X", g_hInstance);
	g_MLService.Log(szMsg);

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
		strcpy_safe(_szMediaPathName, CountOf(_szMediaPathName), "");
		g_Player.OnSongChanged(L"");
		return;
	}

	if (strcmp(file, _szMediaPathName) != 0)
	{
		WCHAR		wFullTitle[MAX_PATH];
		char		fullTitle[MAX_PATH];
		char		artist[MAX_PATH];
		char		title[MAX_PATH];
		char		album[MAX_PATH];

		EmptyStr(artist);
		EmptyStr(title);
		EmptyStr(album);

		strcpy_safe(_szMediaPathName, CountOf(_szMediaPathName), file);

		if (XMP_GetTrackInfo(artist, title, album))
		{
			if (!IsEmptyString(artist))
			{
				strcpy(fullTitle, artist);
				strcat(fullTitle, " - ");
			}
			else
				EmptyStr(fullTitle);

			strcat(fullTitle, title);
		}
		else
			FileGetTitle(fullTitle, CountOf(fullTitle), _szMediaPathName);

		UTF8ToUCS2((BYTE *)artist, strlen(artist) + 1, g_Player.m_szArtist, CountOf(g_Player.m_szArtist));
		UTF8ToUCS2((BYTE *)album, strlen(album) + 1, g_Player.m_szAlbum, CountOf(g_Player.m_szAlbum));
		UTF8ToUCS2((BYTE *)title, strlen(title) + 1, g_Player.m_szTitle, CountOf(g_Player.m_szTitle));

		UTF8ToUCS2((BYTE *)fullTitle, strlen(fullTitle) + 1, wFullTitle, CountOf(wFullTitle));

		g_Player.OnSongChanged(wFullTitle);
	}
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
// create window to get playback pos of foobar timely.
LPCTSTR	szGetPosWndClass = _T("GetPosWnd");
LPCTSTR szGetPosWndName = _T("GetPosWnd");
HWND	g_hWndGetPos = NULL;
const int	TIMER_GETPOS	= 0;

LRESULT CALLBACK GetPosWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL GetPosWndRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	if (GetClassInfo(hInstance, szGetPosWndClass, &wc))
		return TRUE;

	// regiszter pane class
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)GetPosWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon		= NULL;
	wc.hCursor		= NULL;
	wc.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName	= NULL;
	wc.lpszClassName	= szGetPosWndClass;

	int r = RegisterClass(&wc);
	if (r == 0)
	{
		int res = GetLastError();
		if (res != ERROR_CLASS_ALREADY_EXISTS)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL GetPosWndCreate(HINSTANCE hInstance)
{
   hInstance; // Store instance handle in our global variable

   g_hWndGetPos = CreateWindow(szGetPosWndClass, szGetPosWndName, WS_OVERLAPPEDWINDOW,
      0, 0, 100, 100, NULL, NULL, hInstance, NULL);

   if (!g_hWndGetPos)
   {
      return FALSE;
   }

   ShowWindow(g_hWndGetPos, SW_MINIMIZE);
   ShowWindow(g_hWndGetPos, SW_HIDE);

   return TRUE;
}

LRESULT CALLBACK GetPosWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_CREATE:
		SetTimer(hWnd, TIMER_GETPOS, g_UpdateTimeSpan, NULL);
		break;
	case WM_TIMER:
		g_Player.OnPlayTimeChanged(g_Player.GetPlayPos());
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_GETPOS);
		g_hWndGetPos = NULL;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// End of GetPosWindow
////////////////////////////////////////////////////////////////////////////////////

void GetMediaTitle(LPSTR szBuff, int nBuffLen, LPCSTR szMediaSrc)
{
	assert(szMediaSrc);

	LPCSTR		szBeg1, szBeg2;

	szBeg1 = strrchr(szMediaSrc, '\\');
	if (szBeg1 == NULL)
		szBeg1 = szMediaSrc;
	else
		szBeg1++;

	szBeg2 = strrchr(szMediaSrc, '/');
	if (szBeg2 == NULL)
		szBeg2 = szMediaSrc;
	else
		szBeg2++;

	if (szBeg2 > szBeg1)
		szBeg1 = szBeg2;

	strcpy_safe(szBuff, nBuffLen, szBeg1);
}

#define WM_WA_IPC WM_USER 
#define SM_WINAMP_TIMEOUT 1000 
#define IPC_GETOUTPUTTIME 105
#define IPC_ISPLAYING 104

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	EmptyStr(m_szArtist);
	EmptyStr(m_szAlbum);
	EmptyStr(m_szTitle);
	EmptyStr(m_szFullTitle);
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

int CPlayerImp::GetCurSkinPath(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);

	return 1;
}

int CPlayerImp::GetProxy(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);
	
	return 1;
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
int CPlayerImp::GetFullTitleW(WCHAR * lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szFullTitle);

	return 1;
}

int CPlayerImp::GetArtistW(WCHAR *lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szArtist);
	return PR_OK;
}

int CPlayerImp::GetAlbumW(WCHAR *lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szAlbum);
	return PR_OK;
}

int CPlayerImp::GetTitleW(WCHAR *lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szTitle);
	return PR_OK;
}

// 返回歌曲的全文件名（如c:\\...., httP;//wwww......)
int CPlayerImp::GetSourceMediaW(WCHAR * lpszSourceMedia, int nBufSize)
{
	UTF8ToUCS2((BYTE *)_szMediaPathName, strlen(_szMediaPathName) + 1, lpszSourceMedia, nBufSize);

	return 1;
}

// 歌曲的长度（单位ms）
DWORD CPlayerImp::GetMediaLength()
{
	DWORD	dwLength = 0;

	SendMessageTimeout(g_hwndPlayer, WM_WA_IPC, 1, IPC_GETOUTPUTTIME,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwLength);

	return dwLength * 1000;
}

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	DWORD	dwCurPos = 0;

	SendMessageTimeout(g_hwndPlayer, WM_WA_IPC, 0, IPC_GETOUTPUTTIME,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, SM_WINAMP_TIMEOUT, &dwCurPos);

	return dwCurPos;
}

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
void CPlayerImp::OnSongChanged(LPCWSTR szFullTitle)
{
	wcscpy_safe(m_szFullTitle, CountOf(m_szFullTitle), szFullTitle);
	g_MLService.OnFullTitleChangedW(szFullTitle);
}

// 歌曲播放位置改变了
void CPlayerImp::OnPlayTimeChanged(DWORD dwTimeCur)
{
	g_MLService.OnPlayTimeChanged(dwTimeCur);
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

	//
	// Create Window to get playback pos timely.
	//
	GetPosWndRegisterClass(g_hInstance);
	GetPosWndCreate(g_hInstance);

	g_MLService.AddWndCloseto(NULL, "XMPLAY-MAIN", NULL);
	g_MLService.AddWndCloseto(NULL, "XMPLAY-PANEL", NULL);
	g_MLService.AddWndCloseto(NULL, "XMPLAY-INFO", NULL);

	if (!IsEmptyString(m_szFullTitle))
		g_MLService.OnFullTitleChangedW(m_szFullTitle);

	return TRUE;
}

// COMMENT:
//		关闭WINAMP 状态查询线程
void CPlayerImp::Quit()
{
	if (g_DdeInstID)
	{
		DdeUninitialize(g_DdeInstID);
		g_DdeInstID = 0;
	}

	if (g_hWndGetPos)
		DestroyWindow(g_hWndGetPos);
}

void CPlayerImp::SetTimeUpdateFreq(int nFreq)
{
	if (nFreq == 0)
		g_UpdateTimeSpan = 100;
	else if (nFreq == 1)
		g_UpdateTimeSpan = 50;
	else
		g_UpdateTimeSpan = 10;

	KillTimer(g_hWndGetPos, TIMER_GETPOS);
	SetTimer(g_hWndGetPos, TIMER_GETPOS, g_UpdateTimeSpan, NULL);
}
