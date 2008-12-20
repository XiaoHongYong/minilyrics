#include "../MLPluginLib/MlPlugin.h"
#include "PlayerImp.h"
#include <PROCESS.H>
#include <shlobj.h>

CPlayerImp		g_Player;
int				g_UpdateTimeSpan = 50;

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


LRESULT CALLBACK HookMainWnd(
							   HWND hwnd,      // handle to window
							   UINT uMsg,      // message identifier
							   WPARAM wParam,  // first message parameter
							   LPARAM lParam   // second message parameter
							   );

///////////////////////////////////////////////////////////////////////////////
// create window to get playback positon timely.
LPCSTR		szGetPosWndClass = "GetPosWnd";
LPCSTR		szGetPosWndName = "GetPosWnd";
HWND		g_hWndGetPos = NULL;
const int	TIMER_GETPOS		= 1;
const int	TIMER_QUERYITUNES	= 3;

LRESULT CALLBACK GetPosWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL GetPosWndRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	if (GetClassInfo(hInstance, szGetPosWndClass, &wc))
		return TRUE;

	// register pane class
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
		SetTimer(hWnd, TIMER_QUERYITUNES, 500, NULL);
		break;
	case WM_TIMER:
		if (wParam == TIMER_GETPOS)
		{
			if (g_Player.m_bPlayerState == PS_PLAYING)
			{
				// 更新时间
				g_Player.OnPlayTimeChanged(GetTickCount() - g_Player.m_dwBegTickCount);
			}
		}
		else if (wParam == TIMER_QUERYITUNES)
		{
			g_hwndPlayer = FindWindow("iTunes", "iTunes");
			if (g_hwndPlayer)
				KillTimer(hWnd, TIMER_QUERYITUNES);

			// Hook Main Window
			/*if (g_pOrigProc == NULL && g_hwndPlayer)
				g_pOrigProc = (WNDPROC)SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)HookMainWnd);

			if (g_pOrigProc)
				KillTimer(hWnd, TIMER_QUERYITUNES);*/
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_GETPOS);
		KillTimer(hWnd, TIMER_QUERYITUNES);
		g_hWndGetPos = NULL;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// End of GetPosWindow
////////////////////////////////////////////////////////////////////////////////////

//
// iTunes data
//


#define GRAPHICS_DEVICE	HWND

#define	MAIN iTunesPluginMain
#define IMPEXP	__declspec(dllexport)

#define kSampleVisualPluginName		"MiniLyrics Visualizer"
#define	kSampleVisualPluginCreator	'dxhy'

#define	kSampleVisualPluginMajorVersion		1
#define	kSampleVisualPluginMinorVersion		0
#define	kSampleVisualPluginReleaseStage		0x80
#define	kSampleVisualPluginNonFinalRelease	0


/*

static HWND _hDebugWnd;

#define SENDDEBUGMESSAGE	19771212
#define	DEBUGCLASSNAME		"Debug Tracer Class"
#define	DEBUGWNDNAME		"Debug Tracer"

void LogTrace(LPVOID lpData, int nSize)
{
	COPYDATASTRUCT data;

	data.dwData = SENDDEBUGMESSAGE;
	data.lpData = (LPVOID)lpData;
	data.cbData = nSize;

	if (_hDebugWnd == NULL)
		_hDebugWnd = FindWindow(DEBUGCLASSNAME, DEBUGWNDNAME);

	if (_hDebugWnd)
		::SendMessage(_hDebugWnd, WM_COPYDATA, NULL, (LPARAM)&data);
}


void DBGLOG0(LPCTSTR szData)
{
	TCHAR		szBuffer[1024];
	SYSTEMTIME	LocalTime;
	int		nLen;
	GetLocalTime(&LocalTime);
	nLen = wsprintf(szBuffer, "|T%d-%d %d:%d:%d|M%s", LocalTime.wMonth, LocalTime.wDay,
			LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, szData);

	LogTrace((LPVOID)szBuffer, nLen);
}
*/


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
/*
LRESULT CALLBACK HookMainWnd(
							   HWND hwnd,      // handle to window
							   UINT uMsg,      // message identifier
							   WPARAM wParam,  // first message parameter
							   LPARAM lParam   // second message parameter
							   )
{
	switch(uMsg) 
	{
	case WM_ACTIVATEAPP:
		g_MLService.OnNotifyPlayerActivate(wParam);
		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED || wParam == SIZE_RESTORED)
			g_MLService.OnNotifyPlayerMinRestore(wParam);
		break;
	case WM_LBUTTONDOWN:
		{
			HWND		hWndChain[1];
			hWndChain[0] = FindWindow("iTunes", NULL);
			g_MLService.OnNotifyBeforeTrackMove(hWndChain, 1);
		}
		break;
	case WM_MOVE:
		g_MLService.OnNotifyTrackMove((int)(short) LOWORD(lParam), (int)(short) HIWORD(lParam));
		break;
	default:
		break;
	};

	// Call Winamp Window Proc
	return CallWindowProc(g_pOrigProc, hwnd, uMsg, wParam, lParam); 
}*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	EmptyStr(m_szFullTitle);
	EmptyStr(m_szSrcMedia);
	EmptyStr(m_szFileName);
	EmptyStr(m_szArtist);
	EmptyStr(m_szAlbum);
	EmptyStr(m_szTitle);
	m_nMediaLength = 0;
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

int CPlayerImp::GetCurSkinPath(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);

	return 0;
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
int CPlayerImp::GetFullTitleW(WCHAR * lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szFullTitle);

	return PR_OK;
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

// 返回歌曲的全文件名（如c:\\...., http://wwww......)
int CPlayerImp::GetSourceMediaW(WCHAR * lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szSrcMedia);

	return PR_OK;
}

// 歌曲的长度（单位ms）
DWORD CPlayerImp::GetMediaLength()
{
	return m_nMediaLength;
}

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	return m_dwPos;
}

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
void CPlayerImp::OnSongChanged(LPCWSTR szFullTitle)
{
	//
	// 重新取得当前的播放文件
	//
	CIiTunesPtr		iITunes;
	if (iITunes.IsValid())
	{
		HRESULT			hRes;
		IITTrack		*iTrack = NULL;
		hRes = iITunes->get_CurrentTrack(&iTrack);
		if (SUCCEEDED(hRes) && iTrack)
		{
			IITFileOrCDTrack	*iFileTrack = NULL;
			hRes = iTrack->QueryInterface(IID_IITFileOrCDTrack, (LPVOID *)&iFileTrack);
			if (SUCCEEDED(hRes) && iFileTrack)
			{
				BSTR	bstr = NULL;
				hRes = iFileTrack->get_Location(&bstr);
				if (SUCCEEDED(hRes) && bstr)
				{
					wcscpy_safe(m_szSrcMedia, CountOf(m_szSrcMedia), bstr);
				}
				iFileTrack->Release();
			}
			iTrack->Release();
		}
		if (FAILED(hRes))
			wcscpy_safe(m_szSrcMedia, CountOf(m_szSrcMedia), m_szFileName);
	}
	else
		wcscpy_safe(m_szSrcMedia, CountOf(m_szSrcMedia), m_szFileName);

	//
	// 通知 MiniLyrics
	//
	wcscpy_safe(m_szFullTitle, CountOf(m_szFullTitle), szFullTitle);
	g_MLService.OnFullTitleChangedW(szFullTitle);
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
	
	//
	// Create Window to get playback positon timely.
	//
	GetPosWndRegisterClass(g_hInstance);
	GetPosWndCreate(g_hInstance);

	return TRUE;
}

// COMMENT:
//		关闭WINAMP 状态查询线程
void CPlayerImp::Quit()
{
	// 恢复Player窗口的窗口过程
/*	if (g_pOrigProc)
	{
		SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)g_pOrigProc);
		g_pOrigProc = NULL;
	}*/

	if (g_hWndGetPos)
		DestroyWindow(g_hWndGetPos);

//	CIiTunesPtr::Release();
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

void CPlayerImp::OniTunesChangeVisualWnd(HWND hWnd, const Rect *rc)
{
	if (m_hWndiTunesVisual != hWnd)
	{
		m_hWndiTunesVisual = hWnd;
	}

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
static void RenderVisualPort(GRAPHICS_DEVICE destPort, const Rect *destRect)
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

	wcscpy_safe(g_Player.m_szArtist, CountOf(g_Player.m_szArtist), (LPCWSTR)pTrackInfo->artist + 1);
	TrimStr(g_Player.m_szArtist);

	wcscpy_safe(g_Player.m_szAlbum, CountOf(g_Player.m_szAlbum), (LPCWSTR)pTrackInfo->album + 1);
	TrimStr(g_Player.m_szAlbum);

	wcscpy_safe(g_Player.m_szTitle, CountOf(g_Player.m_szTitle), (LPCWSTR)pTrackInfo->name + 1);
	TrimStr(g_Player.m_szTitle);

	wcscpy_safe(g_Player.m_szFullTitle, CountOf(g_Player.m_szFullTitle), (LPCWSTR)pTrackInfo->artist + 1);
	TrimStr(g_Player.m_szFullTitle);

	if (!IsEmptyString(g_Player.m_szFullTitle))
	{
		wcscat_safe(g_Player.m_szFullTitle, CountOf(g_Player.m_szFullTitle), L" - ");
	}

	WCHAR		szName[256];
	WCHAR		szOldFile[MAX_PATH];

	wcscpy_safe(szName, 256, (LPCWSTR)pTrackInfo->name + 1);
	TrimStr(szName);
	wcscat_safe(g_Player.m_szFullTitle, CountOf(g_Player.m_szFullTitle), szName);

	wcscpy_safe(szOldFile, CountOf(szOldFile), g_Player.m_szFileName);

	wcscpy_safe(g_Player.m_szFileName, CountOf(g_Player.m_szFileName), (LPCWSTR)pTrackInfo->fileName + 1);
	TrimStr(g_Player.m_szFileName);

	g_Player.m_bPlayerState = PS_PLAYING;
	g_MLService.OnPlayStateChanged(g_Player.m_bPlayerState);
	if (IsEmptyString(g_Player.m_szFileName) || wcscmp(szOldFile, g_Player.m_szFileName) != 0)
	{
		g_Player.m_nMediaLength = pTrackInfo->totalTimeInMS;

		g_Player.m_dwBegTickCount = GetTickCount();

		g_Player.OnSongChanged(g_Player.m_szFullTitle);
	}
	else
	{
		// resume
		g_Player.m_dwBegTickCount = GetTickCount() - g_Player.m_dwPos;
	}

	{
		//
		// 重新取得该歌曲的播放时间
		//
		CIiTunesPtr		iITunes;

		if (iITunes.IsValid())
		{
			HRESULT			nRes;
			long			nPosSec;
			nRes = iITunes->get_PlayerPosition(&nPosSec);
			if (SUCCEEDED(nRes))
				g_Player.m_dwBegTickCount = GetTickCount() - nPosSec * 1000;
		}
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
				g_Player.OnPlayTimeChanged(g_Player.m_dwPos);
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
				// Hook Main Window
				//if (g_pOrigProc == NULL)
				//	g_pOrigProc = (WNDPROC)SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)HookMainWnd);

				// Create a thread to pump the messages...
				// ASSERT(g_hThread == NULL);
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
