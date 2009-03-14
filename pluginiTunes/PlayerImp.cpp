#include "../MLPluginLib/MlPlugin.h"
#include "PlayerImp.h"
#include <PROCESS.H>
#include <shlobj.h>

CPlayerImp		g_Player;

//
// Thread for MiniLyrics
//
HANDLE			g_hThread = NULL;
DWORD			g_dwThreadID = 0;

DWORD WINAPI ThreadML(LPVOID lpParam)
{
	// now we can use other g_MLService functions.
	if (!InitMiniLyricsPlugin(g_hInstance, &g_Player, ML_SUPPORT_EMBEDDED | ML_EMBEDDED_WND_IMMOVABLE | ML_EMBEDDED_SUPPORT_CLASSIC_SKINS))
		return 0;

	// now we can use other g_MLService functions.
	char	szMsg[256];

	wsprintf(szMsg, "Start iTunes Player plugin for minilyrics. HINSTANCE: %#X", g_hInstance);
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


//
// 去掉字符串两端的空格
void TrimStr(LPWSTR szString)
{
	int		nLen;
	LPWSTR	szBeg;
	szBeg = szString;

	while (*szBeg == ' ')
		szBeg++;

	nLen = wcslen(szBeg);
	if (nLen > 0)
	{
		while (szBeg[nLen - 1] == ' ')
			nLen--;
		szBeg[nLen] = '\0';
	}

	if (szBeg != szString)
		memmove(szString, szBeg, sizeof(WCHAR) * (nLen + 1));
}

///////////////////////////////////////////////////////////////////////////////


class CPlayerInfoQueryWnd : public CPlayerInfoQueryBaseWnd
{
public:
	enum 
	{
		TIMER_QUERYITUNES		= TIMER_NEXT + 1,
		WM_SONG_CHANGED			= WM_USER + 1,
	};

	void OnCreate()
	{
		SetTimer(m_hWnd, TIMER_GETPOS, TIME_OUT_UPDATE_POS, NULL);
		SetTimer(m_hWnd, TIMER_QUERYITUNES, 500, NULL);
	}

	void OnDestroy()
	{
		KillTimer(m_hWnd, TIMER_GETPOS);
		KillTimer(m_hWnd, TIMER_QUERYITUNES);

		PostQuitMessage(0);
	}

	void OnTimer(int nTimerID)
	{
		if (nTimerID == TIMER_GETPOS)
		{
			if (g_Player.m_bPlayerState == PS_PLAYING)
			{
				// 更新时间
				g_Player.m_dwPos = GetTickCount() - g_Player.m_dwBegTickCount;
				g_MLService.OnPlayTimeChanged(g_Player.m_dwPos);
			}
		}
		else if (nTimerID == TIMER_QUERYITUNES)
		{
			g_hwndPlayer = FindWindow("iTunes", "iTunes");
			if (g_hwndPlayer)
				KillTimer(m_hWnd, TIMER_QUERYITUNES);
		}
	}

	void OnMediaChanged(ITTrackInfo	*pTrackInfo)
	{
		MLMediaInfo		mediaInfo;
		WCHAR			szMediaUrl[MAX_PATH], szArtist[MAX_PATH], szAlbum[MAX_PATH], szTitle[MAX_PATH];

		wcscpy_safe(szMediaUrl, CountOf(szMediaUrl), (LPCWSTR)pTrackInfo->fileName + 1);
		TrimStr(szMediaUrl);

		wcscpy_safe(szArtist, CountOf(szArtist), (LPCWSTR)pTrackInfo->artist + 1);
		TrimStr(szArtist);

		wcscpy_safe(szAlbum, CountOf(szAlbum), (LPCWSTR)pTrackInfo->album + 1);
		TrimStr(szAlbum);

		wcscpy_safe(szTitle, CountOf(szTitle), (LPCWSTR)pTrackInfo->name + 1);
		TrimStr(szTitle);

		{
			CIiTunesPtr		iITunes;
			HRESULT			nRes;

			if (iITunes.IsValid())
			{
				// Get playing position
				long			nPosSec;
				nRes = iITunes->get_PlayerPosition(&nPosSec);
				if (SUCCEEDED(nRes))
					g_Player.m_dwBegTickCount = GetTickCount() - nPosSec * 1000;

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

		mediaInfo.szMediaUrl = szMediaUrl;
		mediaInfo.szArtist = szArtist;
		mediaInfo.szAlbum = szAlbum;
		mediaInfo.szTitle = szTitle;
		mediaInfo.nMediaLength = pTrackInfo->totalTimeInMS;
		g_MLService.OnMediaInfoChanged(&mediaInfo);

		g_Player.m_bPlayerState = PS_PLAYING;
		g_MLService.OnPlayStateChanged(g_Player.m_bPlayerState);
	}

	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_SONG_CHANGED)
		{
			ITTrackInfo		*pTrackInfo = (ITTrackInfo *)wParam;
			assert(pTrackInfo);

			OnMediaChanged(pTrackInfo);

			delete pTrackInfo;
			return 1;
		}
		else
			return CPlayerInfoQueryBaseWnd::WndProc(uMsg, wParam, lParam);
	}

};

CPlayerInfoQueryWnd	g_wndInfoQuery;

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

	m_hWndiTunesVisual = NULL;
	m_rciTunesVisual.left = m_rciTunesVisual.top = m_rciTunesVisual.right = m_rciTunesVisual.bottom = -1;
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "iTunes");

	return PR_OK;
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
	return m_dwPos;
}

// control player
BOOL CPlayerImp::Play()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
	{
		HRESULT		hRes;
		hRes = iITunes->Play();
		if (FAILED(hRes))
		{
			g_MLService.Log("iITunes->Play() FAILED!");
		}
	}

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->Pause();

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->PlayPause();

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->Stop();

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->BackTrack();

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->NextTrack();

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
		iITunes->put_PlayerPosition(nMsPos / 1000);

	return TRUE;
}

BOOL CPlayerImp::Init()
{
	g_MLService.AddWndCloseto(NULL, "iTunes", "iTunes");

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

HWND CPlayerImp::GetAttachedParentWnd(RECT *prc)
{
	return m_hWndiTunesVisual;
}

void CPlayerImp::OnNotifySkinLoaded()
{
	if (m_hWndiTunesVisual)
	{
		if (GetParent(g_MLService.GetMainWindow()) == m_hWndiTunesVisual)
		{
			MoveWindow(g_MLService.GetMainWindow(), m_rciTunesVisual.left, m_rciTunesVisual.top, m_rciTunesVisual.right - m_rciTunesVisual.left, m_rciTunesVisual.bottom - m_rciTunesVisual.top, true);
		}
	}
}

void CPlayerImp::OnNotifyDetachedFromParentWnd()
{
}

int CPlayerImp::GetEmbeddedSkinName(TCHAR * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "Embedded into iTunes Visual Area");
	return PR_OK;
}

int CPlayerImp::GetSkinThemeColor(SKIN_CLR_NAME scn, COLORREF *clrValue)
{
	if (scn == SCN_FONT_HIGH)
		*clrValue = RGB(0, 0, 0);
	else if (scn == SCN_FONT_LOW)
	{
		*clrValue = RGB(0x84, 0x84, 0x84);
	}
	else if (scn == SCN_NORMAL_BG)
		*clrValue = RGB(0xD6, 0xDB, 0xBD);
	else if (scn == SCN_SEL_BG)
		*clrValue = RGB(0xE4, 0xE4, 0xE4);
	else
	{
		assert(0);
		return -1;
	}

	return PR_OK;
}

void CPlayerImp::OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan)
{
	g_wndInfoQuery.SetUpdatePosTimer(nPlayPosUpdateSpan);
}

void CPlayerImp::OniTunesChangeVisualWnd(HWND hWnd, const Rect *rc)
{
	m_hWndiTunesVisual = hWnd;

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

	if (g_wndInfoQuery.m_hWnd)
	{
		PostMessage(g_wndInfoQuery.m_hWnd, CPlayerInfoQueryWnd::WM_SONG_CHANGED, (WPARAM)new ITTrackInfo(*pTrackInfo), 0);
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
			g_MLService.SwitchToEmbeddedSkin(true);
			break;
			
		/*
			Sent when iTunes is no longer displayed.
		*/
		case kVisualPluginHideWindowMessage:
			g_Player.OniTunesChangeVisualWnd(nil, nil);
			g_MLService.SwitchToEmbeddedSkin(false);
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
			g_Player.m_bPlayerState = PS_STOPED;
			g_MLService.OnPlayStateChanged(g_Player.m_bPlayerState);
			break;

		/*
			Sent when the player changes the track position.
		*/
		case kVisualPluginSetPositionMessage:
			if (g_Player.m_bPlayerState == PS_PLAYING)
				g_Player.m_dwBegTickCount = GetTickCount() - messageInfo->u.setPositionMessage.positionTimeInMS;
			else
			{
				g_Player.m_dwPos = messageInfo->u.setPositionMessage.positionTimeInMS;
				g_MLService.OnPlayTimeChanged(g_Player.m_dwPos);
			}
			break;

		/*
			Sent when the player pauses.  iTunes does not currently use pause or unpause.
			A pause in iTunes is handled by stopping and remembering the position.
		*/
		case kVisualPluginPauseMessage:
			g_Player.m_bPlayerState = PS_PAUSED;
			g_MLService.OnPlayStateChanged(g_Player.m_bPlayerState);
			break;
			
		/*
			Sent when the player unpauses.  iTunes does not currently use pause or unpause.
			A pause in iTunes is handled by stopping and remembering the position.
		*/
		case kVisualPluginUnpauseMessage:
			g_Player.m_dwBegTickCount = GetTickCount() + g_Player.m_dwPos;
			g_Player.m_bPlayerState = PS_PLAYING;
			g_MLService.OnPlayStateChanged(g_Player.m_bPlayerState);
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

				if (!g_hwndPlayer)
					g_hwndPlayer = FindWindow("iTunes", "iTunes");

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
				}
				g_hThread = NULL;
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
