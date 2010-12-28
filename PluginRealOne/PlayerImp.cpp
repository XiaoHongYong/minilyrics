#include "pntypes.h" 
#include "pncom.h"     /* IUnknown */
#include "rmaerror.h"  /* IRMAErrorMessages */
#include "rmaplugn.h"  /* IRMAPlugin */
#include "rmacore.h"
#include "rmapckts.h"

#include "../MLPluginLib/MLPlugin.h"

#include "PlayerImp.h"

extern IRMAPlayer		*g_pRMPlayer;

CPlayerImp		g_Player;

HWND GetPlayerWindow()
{
	return FindWindow("GeminiWindowClass", NULL);
}

void InitRealOneML()
{
	RunMiniLyrics(g_hInstance, GetPlayerWindow(), &g_Player);
}

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

CPlayerImp::CPlayerImp()
{
	m_dwLength = 0;
}

CPlayerImp::~CPlayerImp()
{
}

void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = GetPlayerWindow();
	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"RealPlayer");
}

void CPlayerImp::OnConnected()
{
	g_MLService.AddWndCloseto(NULL, L"GeminiWindowClass", NULL);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);

	// Update player state
	g_MLService.OnPlayStateChanged(m_playerState);

	OnMediaChanged();
}

void CPlayerImp::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
}

void CPlayerImp::Play()
{
	if (g_pRMPlayer)
		g_pRMPlayer->Begin();
}

void CPlayerImp::Pause()
{
	if (g_pRMPlayer)
		g_pRMPlayer->Pause();
}

void CPlayerImp::PlayPause()
{
	if (g_pRMPlayer)
	{
		if (m_playerState == PS_STOPED || m_playerState == PS_PAUSED)
		{
			m_playerState = PS_PLAYING;
			g_pRMPlayer->Begin();
		}
		else
		{
			m_playerState = PS_PAUSED;
			g_pRMPlayer->Pause();
		}
	}
}

void CPlayerImp::Stop()
{
	if (g_pRMPlayer)
		g_pRMPlayer->Stop();
}

void CPlayerImp::Prev()
{
}

void CPlayerImp::Next()
{
}

void CPlayerImp::SeekTo(int nMsPos)
{
	if (g_pRMPlayer)
		g_pRMPlayer->Seek(nMsPos);
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

void CPlayerImp::OnTimer(int nTimerID)
{
	if (g_pRMPlayer)
		g_MLService.OnPlayPosChanged(g_pRMPlayer->GetCurrentPlayTime());
}
