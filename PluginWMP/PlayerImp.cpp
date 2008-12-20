#include "stdafx.h"
#include "wmplyrics.h"
#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"
#include <PROCESS.H>

CPlayerImp		g_Player;

WNDPROC			g_pOrigProc = NULL;


///////////////////////////////////////////////////////////////////////////////
// create window to get playback pos timely.
LPCTSTR	szGetPosWndClass = "GetPosWnd";
LPCTSTR szGetPosWndName = "GetPosWnd";
HWND	g_hWndGetPos = NULL;
const int	TIMER_GETPOS	= 1;
int			g_UpdateTimeSpan = 50;

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

void GetMediaTitle(LPWSTR szBuff, int nBuffLen, LPCWSTR szMediaSrc)
{
	assert(szMediaSrc);

	LPCWSTR		szBeg1, szBeg2;

	szBeg1 = wcsrchr(szMediaSrc, '\\');
	if (szBeg1 == NULL)
		szBeg1 = szMediaSrc;
	else
		szBeg1++;

	szBeg2 = wcsrchr(szMediaSrc, '/');
	if (szBeg2 == NULL)
		szBeg2 = szMediaSrc;
	else
		szBeg2++;

	if (szBeg2 > szBeg1)
		szBeg1 = szBeg2;

	wcscpy_safe(szBuff, nBuffLen, szBeg1);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	EmptyStr(m_szFullTitle);
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "Windows Media Player");

	return 1;
}

// get player state and song info
DWORD CPlayerImp::GetPlayerState()
{
	if (!m_pCore)
		return PS_STOPED;

	WMPPlayState	state;
	if (SUCCEEDED(m_pCore->get_playState(&state)))
	{
		if (state == wmppsPlaying)
			return PS_PLAYING;
		else if (state == wmppsPaused)
			return PS_PAUSED;
		else
			return PS_STOPED;
	}

	return PS_STOPED;
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

int CPlayerImp::GetArtistW(WCHAR * lpszBuffer, int nBufSize)
{
	return GetProperty(L"Artist", lpszBuffer, nBufSize);
}

int CPlayerImp::GetAlbumW(WCHAR * lpszBuffer, int nBufSize)
{
	return GetProperty(L"Album", lpszBuffer, nBufSize);
}

int CPlayerImp::GetTitleW(WCHAR * lpszBuffer, int nBufSize)
{
	return GetProperty(L"Title", lpszBuffer, nBufSize);
}

// 返回歌曲的全文件名（如c:\\...., httP://wwww......)
int CPlayerImp::GetSourceMediaW(WCHAR * lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szSrcMedia);

	return 1;
}

// 歌曲的长度（单位ms）
DWORD CPlayerImp::GetMediaLength()
{
	return m_dwSongLength;
}

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	double	dbTime = 0;

	assert(m_pControl);
	if (!m_pControl)
		return 0;

	HRESULT hr = m_pControl->get_currentPosition(&dbTime);
// 	if (!SUCCEEDED(hr))
// 	{
// 		g_MLService.Log("WMP_get_currentPosition FAILED!");
// 	}

	return (DWORD)(dbTime * 1000);
}

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
void CPlayerImp::OnSongChanged(LPCWSTR szFullTitle, LPCWSTR szSrcMedia)
{
	if (wcscmp(szFullTitle, m_szFullTitle) == 0 
		&& wcscmp(szSrcMedia, m_szSrcMedia) == 0)
		return;

	wcscpy_safe(m_szSrcMedia, CountOf(m_szSrcMedia), szSrcMedia);
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
	if (m_pControl)
		m_pControl->play();

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	if (m_pControl)
		m_pControl->pause();

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	if (m_pControl && m_pCore)
	{
		WMPPlayState	state;
		if (SUCCEEDED(m_pCore->get_playState(&state)))
		{
			if (state == wmppsPlaying)
				m_pControl->pause();
			else
				m_pControl->play();
		}
	}

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	if (m_pControl)
		m_pControl->stop();

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	if (m_pControl)
		m_pControl->previous();

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	if (m_pControl)
		m_pControl->next();

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	if (m_pControl)
		m_pControl->put_currentPosition(((double)nMsPos) / 1000);

	return TRUE;
}

BOOL CPlayerImp::Init()
{
	HRESULT		hr;

	assert(m_pCore);

	if (!m_pCore)
		return FALSE;

	hr = m_pCore->get_controls(&m_pControl);

	if (!SUCCEEDED(hr))
	{
		g_MLService.Log("Get WMP control interface FAILED!");
		return FALSE;
	}

	g_MLService.AddWndCloseto(NULL, "WMPlayerApp", NULL);
	g_MLService.AddWndCloseto(NULL, "WMP Skin Host", NULL);

	//
	// Create Window to get playback pos timely.
	//
	GetPosWndRegisterClass(g_hInstance);
	GetPosWndCreate(g_hInstance);

	OnMediaChanged();

	return TRUE;
}

// COMMENT:
//		关闭WINAMP 状态查询线程
void CPlayerImp::Quit()
{
	if (g_hWndGetPos)
		DestroyWindow(g_hWndGetPos);

	m_pControl.Release();
}

void CPlayerImp::OnMediaChanged()
{
	CComPtr<IWMPMedia>	pMedia;
	WCHAR		szFullTitle[LRC_TITLE_MAX_LEN];
	WCHAR		szSrcMedia[LRC_TITLE_MAX_LEN];

	if (!m_pControl)
		return;

	m_dwSongLength = 0;

	EmptyStr(szFullTitle);
	EmptyStr(szSrcMedia);

	HRESULT hr = m_pControl->get_currentItem(&pMedia);
	if (SUCCEEDED(hr) && pMedia)
	{
		double	dbDuration = 0;
		BSTR	pbstrSourceUrl;
		BSTR	pbstrItemName, pbstrAr = NULL, pbstrTi = NULL;

		pMedia->get_duration(&dbDuration);
		m_dwSongLength = (DWORD)(dbDuration * 1000);

		hr = pMedia->get_sourceURL(&pbstrSourceUrl);
		if (SUCCEEDED(hr))
		{
			wcscpy_safe(szSrcMedia, CountOf(szSrcMedia), pbstrSourceUrl);
		}

		pbstrItemName = SysAllocString(L"Artist");
		hr = pMedia->getItemInfo(pbstrItemName, &pbstrAr);
		SysFreeString(pbstrItemName);
		if (SUCCEEDED(hr))
		{
			wcscpy_safe(szFullTitle, CountOf(szFullTitle), pbstrAr);
			if (!IsEmptyString(szFullTitle))
				wcscat_safe(szFullTitle, CountOf(szFullTitle), L" - ");
		}

		pbstrItemName = SysAllocString(L"Title");
		hr = pMedia->getItemInfo(pbstrItemName, &pbstrTi);
		SysFreeString(pbstrItemName);
		if (SUCCEEDED(hr))
		{
			wcscat_safe(szFullTitle, CountOf(szFullTitle), pbstrTi);
		}

		if (!SUCCEEDED(hr) || IsEmptyString(szFullTitle))
		{
			EmptyStr(szFullTitle);
			// if artist is null, try get ay author
/*			pbstrItemName = SysAllocString(L"Author");
			hr = pMedia->getItemInfo(pbstrItemName, &pbstrAr);
			SysFreeString(pbstrItemName);
			if (SUCCEEDED(hr))
			{
				wcscpy_safe(szFullTitle, CountOf(szFullTitle), pbstrAr);
				if (!IsEmptyString(szFullTitle))
					wcscat_safe(szFullTitle, CountOf(szFullTitle), L" - ");
			}
*/
			BSTR	pbstrName = NULL;
			hr = pMedia->get_name(&pbstrName);
			if (SUCCEEDED(hr))
			{
				wcscat_safe(szFullTitle, CountOf(szFullTitle), pbstrName);
			}
			else
			{
				GetMediaTitle(szFullTitle, CountOf(szFullTitle), szSrcMedia);
			}
		}
	}

	OnSongChanged(szFullTitle, szSrcMedia);
}

void CPlayerImp::SetWMPCore(IWMPCore *pCore)
{
	ReleaseWMPCore();

	m_pCore = pCore;
}

void CPlayerImp::ReleaseWMPCore()
{
	m_pCore.Release();
}

void CPlayerImp::SetTimeUpdateFreq(int nFreq)
{
	GetPosWndSetTimeUpdateFreq(nFreq);
}

int CPlayerImp::GetProperty(const WCHAR *szProperty, WCHAR *szValue, int nLen)
{
	CComPtr<IWMPMedia>	pMedia;

	if (!m_pControl)
		return PR_FAILED;

	HRESULT hr = m_pControl->get_currentItem(&pMedia);
	if (SUCCEEDED(hr) && pMedia)
	{
		BSTR	pbstrItemName, pbstrValue = NULL;

		pbstrItemName = SysAllocString(szProperty);
		hr = pMedia->getItemInfo(pbstrItemName, &pbstrValue);
		SysFreeString(pbstrItemName);
		if (SUCCEEDED(hr) && pbstrValue)
		{
			wcscpy_safe(szValue, nLen, pbstrValue);
		}
	}

	return PR_OK;
}
