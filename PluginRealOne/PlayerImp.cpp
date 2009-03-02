
#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "pntypes.h" 
#include "pncom.h"     /* IUnknown */
#include "rmaerror.h"  /* IRMAErrorMessages */
#include "rmaplugn.h"  /* IRMAPlugin */
#include "rmacore.h"
#include "rmapckts.h"

#include "../MLPluginLib/MLPlugin.h"
#include <PROCESS.H>
#include "ID3v1.h"

#include "PlayerImp.h"


extern IRMAPlayer		*g_pRMPlayer;

CPlayerImp		g_Player;


///////////////////////////////////////////////////////////////////////////////
// create window to get playback pos timely.
LPCTSTR	szGetPosWndClass = "GetPosWnd";
LPCTSTR szGetPosWndName = "GetPosWnd";
HWND	g_hWndGetPos = NULL;
const int	TIMER_GETPOS	= 1;
int		g_UpdateTimeSpan = 50;

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
	switch (message) 
	{
	case WM_CREATE:
		SetTimer(hWnd, TIMER_GETPOS, g_UpdateTimeSpan, NULL);
		break;
	case WM_TIMER:
		{
			// 更新时间
			g_Player.OnPlayTimeChanged(g_Player.GetPlayPos());
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
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

void GetPosWndSetTimeUpdateFreq(int nFreq)
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

// End of GetPosWindow
////////////////////////////////////////////////////////////////////////////////////


/*
extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	//	g_hInst = (HINSTANCE)hInst;
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		g_hInstance = (HINSTANCE)hInst;
	}

	return TRUE;
}*/

extern "C" BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to the DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpvReserved   // reserved
)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		g_hInstance = (HINSTANCE)hinstDLL;
	}
	return TRUE;
}

char * __cdecl strrep(char * str, char chSrc, char chDest)
{
        char *cp = (char *) str;
		while (*cp)
		{
			if (*cp == chSrc)
				*cp = chDest;
			cp++;
		}

		return str;
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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	EmptyStr(m_szFullTitle);
	EmptyStr(m_szSrcMedia);
	m_nMediaLength = 0;
	m_dwPos = 0;
	m_dwBegTickCount = 0;
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "RealPlayer");

	return 1;
}

// get player state and song info
DWORD CPlayerImp::GetPlayerState()
{
	return m_bPlayerState;
}

int CPlayerImp::GetCurSkinPath(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);

	return 1;
}

int CPlayerImp::GetProxy(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);

	return 0;
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
int CPlayerImp::GetFullTitle(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, m_szFullTitle);

	return 1;
}

// 可以暂时不实现
int CPlayerImp::GetTitle(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);

	return 1;
}

// 可以暂时不实现
int CPlayerImp::GetArtist(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);

	return 1;
}

// 可以暂时不实现
int CPlayerImp::GetAlbum(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);

	return 0;
}

// 返回歌曲的全文件名（如c:\\...., http://wwww......)
int CPlayerImp::GetSourceMedia(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);
	if (!g_pRMPlayer)
		return 0;

	IUnknown	*punknown;
	PN_RESULT	nRet;
	IRMAStreamSource	*pstreamSource;

	int		nCount = g_pRMPlayer->GetSourceCount();
	nRet = g_pRMPlayer->GetSource(0, punknown);
	if (!punknown)
		return 0;

	nRet = punknown->QueryInterface(IID_IRMAStreamSource, (LPVOID *)&pstreamSource);
	punknown->Release();

	if (!pstreamSource)
		return 0;

	m_szFullTitle[0] = '\0';

	strcpy_safe(m_szSrcMedia, MAX_PATH, pstreamSource->GetURL());
	if (strnicmp(m_szSrcMedia, "file://", 7) == 0)
	{
		strcpy(lpszBuffer, m_szSrcMedia + 7);
		strrep(lpszBuffer, '/', '\\');

		CID3v1	id3v1;
		char	szTitle[MAX_PATH];
		char	szArtist[MAX_PATH];

		m_szFullTitle[0] = '\0';
		if (id3v1.GetInfo(lpszBuffer, szTitle, szArtist))
		{
			m_szFullTitle[0] = '\0';
			if (szArtist[0] != '\0')
			{
				lstrcpy(m_szFullTitle, szArtist);
				lstrcat(m_szFullTitle, " - ");
			}
			if (szTitle[0] != '\0')
				lstrcat(m_szFullTitle, szTitle);
		}

		if (m_szFullTitle[0] == '\0')
			FileGetTitle(m_szFullTitle, MAX_PATH, lpszBuffer);
	}

	pstreamSource->Release();

/*	IRMAStream	*pstream;
	nRet = pstreamSource->GetStreamCount();
	nRet = pstreamSource->GetStream(0, punknown);
	if (!punknown)
		return 0;
	punknown->QueryInterface(IID_IRMAStream, (LPVOID *)&pstream);
	IRMAValues *pValue = pstream->GetHeader();
	char	*szName;
	IRMABuffer *pBuffer;
	nRet = pValue->GetFirstPropertyCString(szName, pBuffer);

	while (nRet == PNR_OK)
		nRet = pValue->GetNextPropertyCString(szName, pBuffer);
*/
	return 1;
}

// 歌曲的长度（单位ms）
DWORD CPlayerImp::GetMediaLength()
{
	return m_nMediaLength;
}

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	if (g_pRMPlayer)
		return g_pRMPlayer->GetCurrentPlayTime();
	else
		return 0;
}

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
void CPlayerImp::OnSongChanged(LPCTSTR szFullTitle)
{
	strcpy_safe(m_szFullTitle, CountOf(m_szFullTitle), szFullTitle);
	g_MLService.OnFullTitleChanged(szFullTitle);
}

// 歌曲播放位置改变了
void CPlayerImp::OnPlayTimeChanged(DWORD dwTimeCur)
{
	m_dwPos = dwTimeCur;
	g_MLService.OnPlayTimeChanged(dwTimeCur);
}

// control player
BOOL CPlayerImp::Play()
{
	if (g_pRMPlayer)
		g_pRMPlayer->Begin();
	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	if (g_pRMPlayer)
		g_pRMPlayer->Pause();
	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	if (g_pRMPlayer)
	{
		if (m_bPlayerState == PS_STOPED || m_bPlayerState == PS_PAUSED)
		{
			m_bPlayerState = PS_PLAYING;
			g_pRMPlayer->Begin();
		}
		else
		{
			m_bPlayerState = PS_PAUSED;
			g_pRMPlayer->Pause();
		}
	}
	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	if (g_pRMPlayer)
		g_pRMPlayer->Stop();

	return TRUE;
}

BOOL CPlayerImp::Prev()
{

	return TRUE;
}

BOOL CPlayerImp::Next()
{

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	if (g_pRMPlayer)
		g_pRMPlayer->Seek(nMsPos);

	return 0;
}

BOOL CPlayerImp::Init()
{
	g_hwndPlayer = FindWindow("GeminiWindowClass", NULL);

	g_MLService.AddWndCloseto(NULL, "GeminiWindowClass", NULL);

	//
	// Create Window to get playback pos timely.
	//
	GetPosWndRegisterClass(g_hInstance);
	GetPosWndCreate(g_hInstance);

	return TRUE;
}

// COMMENT:
//		关闭WINAMP 状态查询线程
void CPlayerImp::Quit()
{
	if (g_hWndGetPos)
		DestroyWindow(g_hWndGetPos);
}

int CPlayerImp::HasVideo()
{
	return -1;
}

void CPlayerImp::SetTimeUpdateFreq(int nFreq)
{
	GetPosWndSetTimeUpdateFreq(nFreq);
}
