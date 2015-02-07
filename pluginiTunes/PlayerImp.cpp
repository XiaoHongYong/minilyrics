#include "../MLPluginLib/MlPlugin.h"
#include "PlayerImp.h"
#include "iTunesPlugIn.h"
#include <shlobj.h>

CPlayerImp		g_Player;
HWND			g_hWndPlayer = NULL;

//
// Thread for MiniLyrics
//
HANDLE			g_hThread = NULL;
DWORD			g_dwThreadID = 0;
WNDPROC			g_pOrgiTunesProc = NULL;

LRESULT CALLBACK HookiTunesWnd(
							   HWND hwnd,      // handle to window
							   UINT uMsg,      // message identifier
							   WPARAM wParam,  // first message parameter
							   LPARAM lParam   // second message parameter
							   );

DWORD WINAPI ThreadML(LPVOID lpParam)
{
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


class CIiTunesPtr
{
public:
	CIiTunesPtr()
	{
		m_iITunes = NULL;

		HRESULT		hRes;

		if (FindWindow(_T("iTunesCustomModalDialog"), NULL))
			return;

		hRes = ::CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&m_iITunes);
		if (FAILED(hRes))
		{
			g_MLService.Log("CoCreateInstance of CLSID_iTunesApp FAILED!");
		}
	}
	~CIiTunesPtr()
	{
		if (m_iITunes)
			m_iITunes->Release();
	}

	IiTunes *operator->()
	{
		return m_iITunes;
	}

	bool IsValid()
	{
		return m_iITunes != NULL;
	}

protected:
	IiTunes			*m_iITunes;

};

///////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK HookiTunesWnd(
							   HWND hwnd,      // handle to window
							   UINT uMsg,      // message identifier
							   WPARAM wParam,  // first message parameter
							   LPARAM lParam   // second message parameter
							   )
{
	if (!g_Player.m_bWorking)
		return CallWindowProc(g_pOrgiTunesProc, hwnd, uMsg, wParam, lParam);

	switch(uMsg) 
	{
	case WM_DESTROY:
		{
			// g_MLService.Quit will set g_pOrgiTunesProc to NULL, so save g_pOrgProc.
			WNDPROC pOrigProc = g_pOrgiTunesProc;
			g_MLService.OnDisconnected();
			return CallWindowProc(pOrigProc, hwnd, uMsg, wParam, lParam); 
		}
		break;
	case WM_ACTIVATEAPP:
		::PostMessage(g_Player.GetHandle(), CPlayerImp::WM_U_ACTIVATEAPP, wParam, lParam);
		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED || wParam == SIZE_RESTORED)
			::PostMessage(g_Player.GetHandle(), CPlayerImp::WM_U_MIN_MAX, wParam, lParam);
		break;
	default:
		break;
	};

	return CallWindowProc(g_pOrgiTunesProc, hwnd, uMsg, wParam, lParam); 
}

//////////////////////////////////////////////////////////////////////////

//
// iTunes data
//

#define	MAIN iTunesPluginMain
#define IMPEXP	__declspec(dllexport)

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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	m_dwPos = 0;
	m_dwBegTickCount = 0;
	m_bWorking = false;

	m_hWndiTunesVisual = NULL;
}

CPlayerImp::~CPlayerImp()
{
}


void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = (WND_HANDLE)g_hWndPlayer;
	playerInfo.startupFlags |= ML_SUPPORT_EMBEDDED | ML_EMBEDDED_WND_IMMOVABLE;
	wcscpy_safe(playerInfo.embeddedSkinName, CountOf(playerInfo.embeddedSkinName), L"Embedded into iTunes Visual Area");

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"iTunes");
}

void CPlayerImp::OnConnected()
{
	m_bWorking = true;

	g_MLService.AddWndCloseto(NULL, L"iTunes", L"iTunes");

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	if (g_hWndPlayer == NULL)
		SetTimer(TIMER_QUERYITUNES, 500);

	// SwitchToEmbeddedSkin();

	// Update Media info
	// OnMediaChanged();

	// Update player state
	// g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerImp::OnDisconnect()
{
	m_bWorking = false;

	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_QUERYITUNES);
}

void CPlayerImp::Play()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->Play();
}

void CPlayerImp::Pause()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->Pause();
}

void CPlayerImp::PlayPause()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->PlayPause();
}

void CPlayerImp::Stop()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->Stop();
}

void CPlayerImp::Prev()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->BackTrack();
}

void CPlayerImp::Next()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->NextTrack();
}

void CPlayerImp::SeekTo(int nMsPos)
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->put_PlayerPosition(nMsPos / 1000);
}

void CPlayerImp::SwitchToEmbeddedSkin()
{
	PlayerSkinInfo	skinInfo;

	memset(&skinInfo, 0, sizeof(skinInfo));
	skinInfo.bEmbeddedSkin = m_hWndiTunesVisual != NULL;
	skinInfo.bClrThemeValid = false;
	skinInfo.hWndToAttach = (WND_HANDLE)m_hWndiTunesVisual;
	skinInfo.clrFontHighlight = RGB(0, 0, 0);
	skinInfo.clrFontLowlight = RGB(0x84, 0x84, 0x84);
	skinInfo.clrBackground = RGB(0xD6, 0xDB, 0xBD);
	skinInfo.clrCurLineBg = RGB(0xE4, 0xE4, 0xE4);

	g_MLService.SwitchToEmbeddedSkin(skinInfo);

	if (m_hWndiTunesVisual && GetParent(g_MLService.GetMainWindow()) == m_hWndiTunesVisual)
	{
		RECT rc;
		GetClientRect(m_hWndiTunesVisual, &rc);
		MoveWindow(g_MLService.GetMainWindow(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);
	}
}

void CPlayerImp::EndEmbeddedSkin()
{
}

void CPlayerImp::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		if (m_playerState == PS_PLAYING)
		{
			m_dwPos = GetTickCount() - m_dwBegTickCount;
			g_MLService.OnPlayPosChanged(m_dwPos);
		}
	}
	else if (nTimerID == TIMER_QUERYITUNES)
	{
		g_hWndPlayer = FindWindow(_T("iTunes"), _T("iTunes"));
		if (g_hWndPlayer)
		{
			if (FindWindowEx(g_hWndPlayer, NULL, NULL, NULL) == NULL)
			{
				// No Child window, it is NOT iTunes' Window, try another one.
				g_hWndPlayer = FindWindowEx(NULL, g_hWndPlayer, _T("iTunes"), _T("iTunes"));
			}
			if (g_hWndPlayer)
			{
				KillTimer(TIMER_QUERYITUNES);

				// Hook iTunes Window
				if (g_pOrgiTunesProc == NULL)
					g_pOrgiTunesProc = (WNDPROC)SetWindowLongPtr(g_hWndPlayer, GWLP_WNDPROC, (LONG_PTR)HookiTunesWnd);
			}
		}
	}
}

void CPlayerImp::OnMediaChanged(ITTrackInfo	*pTrackInfo)
{
	WCHAR			szMediaUrl[MAX_PATH], szArtist[MAX_PATH], szAlbum[MAX_PATH], szTitle[MAX_PATH];

	wcsncpy_safe(szMediaUrl, CountOf(szMediaUrl), (LPCWSTR)pTrackInfo->fileName + 1, pTrackInfo->fileName[0]);

	wcsncpy_safe(szArtist, CountOf(szArtist), (LPCWSTR)pTrackInfo->artist + 1, pTrackInfo->artist[0]);

	wcsncpy_safe(szAlbum, CountOf(szAlbum), (LPCWSTR)pTrackInfo->album + 1, pTrackInfo->album[0]);

	wcsncpy_safe(szTitle, CountOf(szTitle), (LPCWSTR)pTrackInfo->name + 1, pTrackInfo->name[0]);

	{
		CIiTunesPtr		iITunes;
		HRESULT			nRes;

		if (iITunes.IsValid())
		{
			// Get playing position
			long			nPosSec;
			nRes = iITunes->get_PlayerPosition(&nPosSec);
			if (SUCCEEDED(nRes))
				m_dwBegTickCount = GetTickCount() - nPosSec * 1000;

			// Get full path name of media.
			IITTrack		*iTrack = NULL;
			nRes = iITunes->get_CurrentTrack(&iTrack);
			if (SUCCEEDED(nRes) && iTrack)
			{
				IITFileOrCDTrack	*iFileTrack = NULL;
				nRes = iTrack->QueryInterface(IID_IITFileOrCDTrack, (LPVOID *)&iFileTrack);
				if (SUCCEEDED(nRes) && iFileTrack)
				{
					BSTR	bstr = NULL;
					nRes = iFileTrack->get_Location(&bstr);
					if (SUCCEEDED(nRes) && bstr)
					{
						wcscpy_safe(szMediaUrl, CountOf(szMediaUrl), bstr);
					}
					iFileTrack->Release();
				}
				iTrack->Release();
			}
		}
	}

	g_MLService.OnMediaInfoChanged(szMediaUrl, szArtist, szAlbum, szTitle, pTrackInfo->totalTimeInMS);

	m_playerState = PS_PLAYING;
	g_MLService.OnPlayStateChanged(m_playerState);
}

LRESULT CPlayerImp::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SONG_CHANGED)
	{
		ITTrackInfo		*pTrackInfo = (ITTrackInfo *)wParam;
		assert(pTrackInfo);

		OnMediaChanged(pTrackInfo);

		delete pTrackInfo;
	}
	else if (uMsg == WM_U_ACTIVATEAPP)
	{
		g_MLService.OnNotifyPlayerActivate((BOOL)wParam);
	}
	else if (uMsg == WM_U_MIN_MAX)
	{
		g_MLService.OnNotifyPlayerMinRestore((BOOL)wParam);
	}
	else
		return CPlayerServiceBase::WndProc(uMsg, wParam, lParam);

	return 1;
}

void CPlayerImp::OniTunesChangeVisualWnd(HWND hWnd)
{
	m_hWndiTunesVisual = hWnd;

	SwitchToEmbeddedSkin();

	if (hWnd)
	{
		if (GetParent(g_MLService.GetMainWindow()) == m_hWndiTunesVisual)
		{
			RECT rc;
			::GetClientRect(hWnd, &rc);
			MoveWindow(g_MLService.GetMainWindow(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);
		}
	}
}

void CPlayerImp::OniTunesMoveVisualWnd()
{
	if (m_hWndiTunesVisual)
	{
		if (GetParent(g_MLService.GetMainWindow()) == m_hWndiTunesVisual)
		{
			RECT rc;
			::GetClientRect(m_hWndiTunesVisual, &rc);
			MoveWindow(g_MLService.GetMainWindow(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);
		}
	}
}


#ifdef __cplusplus
extern "C" {
#endif

/*
	MAIN
*/
extern "C" __declspec(dllexport) OSStatus iTunesPluginMain(OSType message, PluginMessageInfo * messageInfo, void * refCon)
{
	OSStatus		status;
	
	(void) refCon;
	
	switch (message)
	{
		case kPluginInitMessage:
			{
				// MessageBox(NULL, _T("iTunes MiniLyircs"), _T("iTunes MiniLyircs"), MB_OK);
				// Is old MiniLyrics plug-in running?
				WCHAR	szModuleName[MAX_PATH];
				WCHAR	sziTunesPluginPath[MAX_PATH];

				GetModuleFileName(g_hInstance, szModuleName, MAX_PATH);
				SHGetSpecialFolderPath(NULL, sziTunesPluginPath, CSIDL_APPDATA, FALSE);
				DirStringFormat(sziTunesPluginPath);
				lstrcat(sziTunesPluginPath , _T("Apple Computer\\iTunes\\iTunes Plug-ins\\iTunesMinilyrics.dll"));
				if (IsFileExist(sziTunesPluginPath) && lstrcmp(szModuleName, sziTunesPluginPath) != 0)
				{
					if (!DeleteFile(sziTunesPluginPath))
						break;
				}

				status = RegisterVisualPlugin(messageInfo);

				if (!g_hWndPlayer)
					g_hWndPlayer = FindWindow(_T("iTunes"), _T("iTunes"));

				// Create a thread to pump the messages...
				g_hThread = CreateThread(NULL, 0, ThreadML, NULL, 0, &g_dwThreadID);
			}
			break;
		case kPluginCleanupMessage:
			{
				if (g_dwThreadID)
				{
					PostThreadMessage(g_dwThreadID, WM_QUIT, 0, 0);
					WaitForSingleObject(g_hThread, INFINITE);
					CloseHandle(g_hThread);
					g_hThread = NULL;
					g_dwThreadID = 0;
				}
				status = noErr;
			}
			break;
		default:
			status = unimpErr;
			break;
	}

	return status;
}

#ifdef __cplusplus
}
#endif
