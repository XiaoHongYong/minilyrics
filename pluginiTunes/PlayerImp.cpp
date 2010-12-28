#include "../MLPluginLib/MlPlugin.h"
#include "PlayerImp.h"
#include <PROCESS.H>
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

		if (FindWindow("iTunesCustomModalDialog", NULL))
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

#define kSampleVisualPluginName		"MiniLyrics Visualizer"
#define	kSampleVisualPluginCreator	'dxhy'

#define	kSampleVisualPluginMajorVersion		1
#define	kSampleVisualPluginMinorVersion		0
#define	kSampleVisualPluginReleaseStage		0x80
#define	kSampleVisualPluginNonFinalRelease	0


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
	m_rciTunesVisual.left = m_rciTunesVisual.top = m_rciTunesVisual.right = m_rciTunesVisual.bottom = -1;
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
	skinInfo.bClrThemeValid = true;
	skinInfo.hWndToAttach = m_hWndiTunesVisual;
	skinInfo.clrFontHighlight = RGB(0, 0, 0);
	skinInfo.clrFontLowlight = RGB(0x84, 0x84, 0x84);
	skinInfo.clrBackground = RGB(0xD6, 0xDB, 0xBD);
	skinInfo.clrCurLineBg = RGB(0xE4, 0xE4, 0xE4);

	g_MLService.SwitchToEmbeddedSkin(skinInfo);

	if (m_hWndiTunesVisual && GetParent(g_MLService.GetMainWindow()) == m_hWndiTunesVisual)
	{
		MoveWindow(g_MLService.GetMainWindow(), m_rciTunesVisual.left, m_rciTunesVisual.top, m_rciTunesVisual.right - m_rciTunesVisual.left, m_rciTunesVisual.bottom - m_rciTunesVisual.top, true);
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
		g_hWndPlayer = FindWindow("iTunes", "iTunes");
		if (g_hWndPlayer)
		{
			if (FindWindowEx(g_hWndPlayer, NULL, NULL, NULL) == NULL)
			{
				// No Child window, it is NOT iTunes' Window, try another one.
				g_hWndPlayer = FindWindowEx(NULL, g_hWndPlayer, "iTunes", "iTunes");
			}
			if (g_hWndPlayer)
			{
				KillTimer(TIMER_QUERYITUNES);

				// Hook iTunes Window
				if (g_pOrgiTunesProc == NULL)
					g_pOrgiTunesProc = (WNDPROC)SetWindowLong(g_hWndPlayer, GWL_WNDPROC, (LONG)HookiTunesWnd);
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
		g_MLService.OnNotifyPlayerActivate(wParam);
	}
	else if (uMsg == WM_U_MIN_MAX)
	{
		g_MLService.OnNotifyPlayerMinRestore(wParam);
	}
	else
		return CPlayerServiceBase::WndProc(uMsg, wParam, lParam);

	return 1;
}

void CPlayerImp::OniTunesChangeVisualWnd(HWND hWnd, const Rect *rc)
{
	m_hWndiTunesVisual = hWnd;

	SwitchToEmbeddedSkin();

	if (hWnd)
	{
		if (rc->left != m_rciTunesVisual.left || rc->top != m_rciTunesVisual.top ||
			rc->right != m_rciTunesVisual.right || rc->bottom != m_rciTunesVisual.bottom)
		{
			m_rciTunesVisual = *rc;
			if (GetParent(g_MLService.GetMainWindow()) == m_hWndiTunesVisual)
			{
				MoveWindow(g_MLService.GetMainWindow(), rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, true);
			}
		}
	}
	else
	{
	}
}


//////////////////////////////////////////////////////////////////////////

enum
{
	kSettingsDialogResID	= 1000,
	
	kSettingsDialogOKButton	= 1,
	kSettingsDialogCancelButton,
	
	kSettingsDialogCheckBox1,
	kSettingsDialogCheckBox2,
	kSettingsDialogCheckBox3
};


// ClearMemory
//
static void ClearMemory (LogicalAddress dest, SInt32 length)
{
	register unsigned char	*ptr;

	ptr = (unsigned char *) dest;
	
	if( length > 16 )
	{
		register unsigned long	*longPtr;
		
		while( ((unsigned long) ptr & 3) != 0 )
		{
			*ptr++ = 0;
			--length;
		}
		
		longPtr = (unsigned long *) ptr;
		
		while( length >= 4 )
		{
			*longPtr++ 	= 0;
			length		-= 4;
		}
		
		ptr = (unsigned char *) longPtr;
	}
	
	while( --length >= 0 )
	{
		*ptr++ = 0;
	}
}

/*
	RenderVisualPort
*/
static void RenderVisualPort(HWND destPort, const Rect *destRect)
{
	if (destPort == nil)
		return;

	HBRUSH	hBrush;
	HDC		hdc;
	RECT	srcRect;
	
	srcRect.left = destRect->left;
	srcRect.top = destRect->top;
	srcRect.right = destRect->right;
	srcRect.bottom = destRect->bottom;

	hdc = GetDC(destPort);
	hBrush = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(hdc, &srcRect, hBrush);
	DeleteObject(hBrush);
	ReleaseDC(destPort, hdc);
}


void OnTrackChanged(ITTrackInfo *pTrackInfo)
{
	if (!pTrackInfo)
		return;

	if (g_Player.m_hWnd)
	{
		PostMessage(g_Player.m_hWnd, CPlayerImp::WM_SONG_CHANGED, (WPARAM)new ITTrackInfo(*pTrackInfo), 0);
	}
}

/*
	VisualPluginHandler
*/
static OSStatus VisualPluginHandler(OSType message, VisualPluginMessageInfo *messageInfo, void *refCon)
{
	OSStatus			status;
	
	status = noErr;

	switch (message)
	{
		/*
			Sent when the visual plugin is registered.  The plugin should do minimal
			memory allocations here.  The resource fork of the plugin is still available.
		*/		
		case kVisualPluginInitMessage:
		{
			break;
		}

		/*
			Sent when the visual plugin is unloaded
		*/		
		case kVisualPluginCleanupMessage:
			break;

		/*
			Sent when the visual plugin is enabled.  iTunes currently enables all
			loaded visual plugins.  The plugin should not do anything here.
		*/
		case kVisualPluginEnableMessage:
		case kVisualPluginDisableMessage:
			break;

		/*
			Sent if the plugin requests idle messages.  Do this by setting the kVisualWantsIdleMessages
			option in the PlayerRegisterVisualPluginMessage.options field.
		*/
		case kVisualPluginIdleMessage:
			break;

		/*
			Sent when iTunes is going to show the visual plugin in a port.  At
			this point, the plugin should allocate any large buffers it needs.
		*/
		case kVisualPluginShowWindowMessage:
			g_Player.OniTunesChangeVisualWnd(messageInfo->u.showWindowMessage.window,
										&messageInfo->u.showWindowMessage.drawRect);
			RenderVisualPort(messageInfo->u.showWindowMessage.window, &messageInfo->u.showWindowMessage.drawRect);
			break;
			
		/*
			Sent when iTunes is no longer displayed.
		*/
		case kVisualPluginHideWindowMessage:
			g_Player.OniTunesChangeVisualWnd(nil, nil);
			break;
		
		/*
			Sent when iTunes needs to change the port or rectangle of the currently
			displayed visual.
		*/
		case kVisualPluginSetWindowMessage:
			g_Player.OniTunesChangeVisualWnd(messageInfo->u.setWindowMessage.window,
										&messageInfo->u.setWindowMessage.drawRect);
			RenderVisualPort(messageInfo->u.setWindowMessage.window, &messageInfo->u.setWindowMessage.drawRect);
			break;

		/*
			Sent for the visual plugin to render a frame.
		*/
		case kVisualPluginRenderMessage:
			break;
			
		/*
			Sent in response to an update event.  The visual plugin should update
			into its remembered port.  This will only be sent if the plugin has been
			previously given a ShowWindow message.
		*/	
		case kVisualPluginUpdateMessage:
 			RenderVisualPort(g_Player.m_hWndiTunesVisual, &g_Player.m_rciTunesVisual);
			break;
		
		/*
			Sent when the player starts.
		*/
		case kVisualPluginPlayMessage:
			OnTrackChanged(messageInfo->u.playMessage.trackInfoUnicode);
			break;

		/*
			Sent when the player changes the current track information.  This
			is used when the information about a track changes, or when the CD
			moves onto the next track.  The visual plugin should update any displayed
			information about the currently playing song.
		*/

		case kVisualPluginChangeTrackMessage:
			OnTrackChanged(messageInfo->u.changeTrackMessage.trackInfoUnicode);
			break;

		/*
			Sent when the player stops.
		*/
		case kVisualPluginStopMessage:
			g_Player.m_playerState = PS_STOPED;
			g_MLService.OnPlayStateChanged(g_Player.m_playerState);
			break;

		/*
			Sent when the player changes the track position.
		*/
		case kVisualPluginSetPositionMessage:
			if (g_Player.m_playerState == PS_PLAYING)
				g_Player.m_dwBegTickCount = GetTickCount() - messageInfo->u.setPositionMessage.positionTimeInMS;
			else
			{
				g_Player.m_dwPos = messageInfo->u.setPositionMessage.positionTimeInMS;
				g_MLService.OnPlayPosChanged(g_Player.m_dwPos);
			}
			break;

		/*
			Sent when the player pauses.  iTunes does not currently use pause or unpause.
			A pause in iTunes is handled by stopping and remembering the position.
		*/
		case kVisualPluginPauseMessage:
			g_Player.m_playerState = PS_PAUSED;
			g_MLService.OnPlayStateChanged(g_Player.m_playerState);
			break;
			
		/*
			Sent when the player unpauses.  iTunes does not currently use pause or unpause.
			A pause in iTunes is handled by stopping and remembering the position.
		*/
		case kVisualPluginUnpauseMessage:
			g_Player.m_dwBegTickCount = GetTickCount() + g_Player.m_dwPos;
			g_Player.m_playerState = PS_PLAYING;
			g_MLService.OnPlayStateChanged(g_Player.m_playerState);
			break;
		
		/*
			Sent to the plugin in response to a MacOS event.  The plugin should return noErr
			for any event it handles completely, or an error (unimpErr) if iTunes should handle it.
		*/
		case kVisualPluginEventMessage:
			status = unimpErr;
			break;

		default:
			status = unimpErr;
			break;
	}

	return status;	
}


/*
	RegisterVisualPlugin
*/
static OSStatus RegisterVisualPlugin (PluginMessageInfo *messageInfo)
{
	OSStatus			status;
	PlayerMessageInfo	playerMessageInfo;

	ClearMemory(&playerMessageInfo.u.registerVisualPluginMessage,sizeof(playerMessageInfo.u.registerVisualPluginMessage));
	
	// copy in name length byte first
	playerMessageInfo.u.registerVisualPluginMessage.name[0] = lstrlen(kSampleVisualPluginName);
	// now copy in actual name
	memcpy(&playerMessageInfo.u.registerVisualPluginMessage.name[1], kSampleVisualPluginName, lstrlen(kSampleVisualPluginName));

	SetNumVersion(&playerMessageInfo.u.registerVisualPluginMessage.pluginVersion, kSampleVisualPluginMajorVersion, kSampleVisualPluginMinorVersion, kSampleVisualPluginReleaseStage, kSampleVisualPluginNonFinalRelease);

	playerMessageInfo.u.registerVisualPluginMessage.options					= 0;//kVisualWantsIdleMessages | kVisualWantsConfigure;
	playerMessageInfo.u.registerVisualPluginMessage.handler					= VisualPluginHandler;
	playerMessageInfo.u.registerVisualPluginMessage.registerRefCon			= 0;
	playerMessageInfo.u.registerVisualPluginMessage.creator					= kSampleVisualPluginCreator;
	
	playerMessageInfo.u.registerVisualPluginMessage.timeBetweenDataInMS		= 0xFFFFFFFF; // 16 milliseconds = 1 Tick, 0xFFFFFFFF = Often as possible.
	playerMessageInfo.u.registerVisualPluginMessage.numWaveformChannels		= 2;
	playerMessageInfo.u.registerVisualPluginMessage.numSpectrumChannels		= 2;
	
	playerMessageInfo.u.registerVisualPluginMessage.minWidth				= 64;
	playerMessageInfo.u.registerVisualPluginMessage.minHeight				= 64;
	playerMessageInfo.u.registerVisualPluginMessage.maxWidth				= 32767;
	playerMessageInfo.u.registerVisualPluginMessage.maxHeight				= 32767;
	playerMessageInfo.u.registerVisualPluginMessage.minFullScreenBitDepth	= 0;
	playerMessageInfo.u.registerVisualPluginMessage.maxFullScreenBitDepth	= 0;
	playerMessageInfo.u.registerVisualPluginMessage.windowAlignmentInBytes	= 0;
	
	status = PlayerRegisterVisualPlugin(messageInfo->u.initMessage.appCookie, messageInfo->u.initMessage.appProc,&playerMessageInfo);

	return status;	
}


#ifdef __cplusplus
extern "C" {
#endif

/*
	MAIN
*/
IMPEXP OSStatus MAIN (OSType message, PluginMessageInfo *messageInfo, void *refCon)
{
	OSStatus		status;
	
	(void) refCon;
	
	switch (message)
	{
		case kPluginInitMessage:
			{
				// Is old MiniLyrics plug-in running?
				char	szModuleName[MAX_PATH];
				char	sziTunesPluginPath[MAX_PATH];

				GetModuleFileName(g_hInstance, szModuleName, MAX_PATH);
				SHGetSpecialFolderPath(NULL, sziTunesPluginPath, CSIDL_APPDATA, FALSE);
				DirStringFormat(sziTunesPluginPath);
				lstrcat(sziTunesPluginPath , "Apple Computer\\iTunes\\iTunes Plug-ins\\iTunesMinilyrics.dll");
				if (IsFileExist(sziTunesPluginPath) && lstrcmp(szModuleName, sziTunesPluginPath) != 0)
				{
					if (!DeleteFile(sziTunesPluginPath))
						break;
				}

				status = RegisterVisualPlugin(messageInfo);

				if (!g_hWndPlayer)
					g_hWndPlayer = FindWindow("iTunes", "iTunes");

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
