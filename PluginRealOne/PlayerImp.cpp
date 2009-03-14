#include "pntypes.h" 
#include "pncom.h"     /* IUnknown */
#include "rmaerror.h"  /* IRMAErrorMessages */
#include "rmaplugn.h"  /* IRMAPlugin */
#include "rmacore.h"
#include "rmapckts.h"

#include "../MLPluginLib/MLPlugin.h"
#include "PlayerInfoQueryWnd.h"

#include "PlayerImp.h"

extern IRMAPlayer		*g_pRMPlayer;

CPlayerImp		g_Player;


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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	m_dwLength = 0;
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

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	if (g_pRMPlayer)
		return g_pRMPlayer->GetCurrentPlayTime();
	else
		return 0;
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

	g_wndInfoQuery.Create(g_hInstance);

	return TRUE;
}

void CPlayerImp::Quit()
{
	g_wndInfoQuery.Destroy();
}

int CPlayerImp::HasVideo()
{
	return -1;
}

void CPlayerImp::OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan)
{
	g_wndInfoQuery.SetUpdatePosTimer(nPlayPosUpdateSpan);
}

void CPlayerImp::OnMediaChanged()
{
	if (!g_pRMPlayer)
		return;

	IUnknown			*punknown;
	PN_RESULT			nRet;
	IRMAStreamSource	*pstreamSource;

	int		nCount = g_pRMPlayer->GetSourceCount();
	nRet = g_pRMPlayer->GetSource(0, punknown);
	if (!punknown)
		return;

	nRet = punknown->QueryInterface(IID_IRMAStreamSource, (LPVOID *)&pstreamSource);
	punknown->Release();

	if (!pstreamSource)
		return;

	char		szUrl[MAX_PATH];

	strcpy_safe(szUrl, MAX_PATH, pstreamSource->GetURL());
	if (strnicmp(szUrl, "file://", 7) == 0)
	{
		LPSTR		szFile = szUrl + 7;
		strrep(szFile, '/', '\\');

		g_MLService.OnMediaInfoChanged(szFile, NULL, NULL, NULL, m_dwLength);
	}
	else
		g_MLService.OnMediaInfoChanged(szUrl, NULL, NULL, NULL, m_dwLength);

	pstreamSource->Release();
}
