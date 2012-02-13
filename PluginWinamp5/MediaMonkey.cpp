#include "../base/base.h"
#include "../base/common.h"
#include "../MLPluginLib/MLPlugin.h"
// #import "C:\\Program Files\\MediaMonkey\\MediaMonkey.exe"
#undef GetFreeSpace
#undef EOF
#include "MediaMonkey.tlh"

#include "MediaMonkey.h"

using namespace SongsDB;

CMPAutoPtr<ISDBApplication>	m_app;

CMediaMonkey::CMediaMonkey(void)
{
}

CMediaMonkey::~CMediaMonkey(void)
{
}

void FindEmbeddedMMWindow();

bool CMediaMonkey::GetCurrentMedia(LPWSTR szFile)
{
	EmptyStr(szFile);

	FindEmbeddedMMWindow();

	if (!MakeSureInterfaceOK())
		return false;

	CMPAutoPtr<ISDBPlayer> player;

	HRESULT hr = m_app->get_Player(&player);
	if (FAILED(hr) || player == NULL)
		return false;

	CMPAutoPtr<ISDBSongData> curSong;
	hr = player->get_CurrentSong(&curSong);
	if (FAILED(hr) || curSong == NULL)
		return false;

	bstr_s path;
	hr = curSong->get_Path(&path);
	if (FAILED(hr))
		return false;

	if (path.c_str())
		wcscpy_safe(szFile, MAX_PATH, path.c_str());

	return true;
}

bool CMediaMonkey::GetCurrentMediaInfo(wchar_t szArtist[], wchar_t szAlbum[], wchar_t szTitle[])
{
	if (!MakeSureInterfaceOK())
		return false;

	CMPAutoPtr<ISDBPlayer> player;

	HRESULT hr = m_app->get_Player(&player);
	if (FAILED(hr) || player == NULL)
		return false;

	CMPAutoPtr<ISDBSongData> curSong;
	hr = player->get_CurrentSong(&curSong);
	if (FAILED(hr) || curSong == NULL)
		return false;

	bstr_s artist, title, album;
	hr = curSong->get_ArtistName(&artist);
	if (FAILED(hr))
		return false;

	hr = curSong->get_Title(&title);
	if (FAILED(hr))
		return false;

	hr = curSong->get_AlbumName(&album);
	if (FAILED(hr))
		return false;

	if (artist.c_str())
		wcscpy_safe(szArtist, MAX_PATH, artist.c_str());
	if (title.c_str())
		wcscpy_safe(szTitle, MAX_PATH, title.c_str());
	if (album.c_str())
		wcscpy_safe(szAlbum, MAX_PATH, album.c_str());

	return true;
}

/*
HWND CMediaMonkey::CreateEmbeddedWnd()
{
	if (!MakeSureInterfaceOK())
		return false;

	CMPAutoPtr<ISDBUI> ui;
	HRESULT hr = m_app->get_UI(&ui);
	if (FAILED(hr))
		return false;

	CMPAutoPtr<ISDBUIDockablePanel> panel;
	hr = ui->get_NewDockablePanel(&panel);
	if (FAILED(hr))
		return false;

	panel->put_Caption(L"MiniLyrics");
	panel->put_DockedTo(3);
	CMPAutoPtr<ISDBUICommon> uicommon;
	hr = panel->get_Common(&uicommon);
	if (FAILED(hr))
		return false;

	

	uicommon->put_Width(300);
	uicommon->put_Height(125);
	

}*/


bool CMediaMonkey::MakeSureInterfaceOK()
{
	if (m_app == NULL)
	{
		g_MLService.Log("Before MakeSureInterfaceOK!");
		HRESULT hr = ::CoCreateInstance(__uuidof(SDBApplication), NULL, CLSCTX_LOCAL_SERVER, __uuidof(ISDBApplication), (PVOID *)&m_app);
		if (FAILED(hr) || m_app == NULL)
		{
			g_MLService.Log("CoCreateInstance of MediaMonkey FAILED!");
			return false;
		}
		g_MLService.Log("after MakeSureInterfaceOK!");
	}

	return true;
}

#define EMBEDDED_MM_WND_NAME		_T("Embedded MiniLyrics")

HWND g_hWndEWofMM = NULL;
const UINT g_msgMediaMonkeyEmbeder = RegisterWindowMessage(_T("MediaMonkeyEmbederToMiniLyrics"));

bool FindChildrenWnd(HWND hWndParent)
{
	HWND	hWndChild = NULL;

	while (true)
	{
		hWndChild = FindWindowEx(hWndParent, hWndChild, NULL, NULL);
		if (!hWndChild)
			break;

		TCHAR		szName[256];
		::GetWindowText(hWndChild, szName, 256);
		if (_tcscmp(szName, EMBEDDED_MM_WND_NAME) == 0)
		{
			g_hWndEWofMM = hWndChild;
			return true;
		}

		if (FindChildrenWnd(hWndChild))
			return true;
	}

	return false;
}

BOOL CALLBACK EnumWindowProc(HWND hwnd, LPARAM lParam)
{
	DWORD dwCurProcessId = (DWORD)lParam;

	DWORD dwProcessID;
	GetWindowThreadProcessId(hwnd, &dwProcessID);
	if (dwProcessID != dwCurProcessId)
		return TRUE;

	if (FindChildrenWnd(hwnd))
		return FALSE;

	return TRUE;
}

void FindEmbeddedMMWindow()
{
	static int nRetries = 0;
	if (g_hWndEWofMM != NULL || nRetries > 20)
		return;
	nRetries++;

	EnumWindows(EnumWindowProc, (LPARAM)GetCurrentProcessId());

	if (g_hWndEWofMM)
	{
		// Find its first child window
		HWND hWnd = FindWindowEx(g_hWndEWofMM, NULL, NULL, NULL);
		if (hWnd != NULL)
			g_hWndEWofMM = hWnd;
		else if (nRetries < 18)
			g_hWndEWofMM = NULL;
	}
}

void CMediaMonkey::OnTimerCheckEmbeddedMiniLyricsWindow()
{
	FindEmbeddedMMWindow();

	if (!IsWindow(g_hWndEWofMM))
		return;

	// Move main window?
	HWND hWndML = g_MLService.GetMainWindow();
	if (::GetParent(hWndML) != g_hWndEWofMM)
	{
		DWORD	dwStyle;
		dwStyle = GetWindowLong(hWndML, GWL_STYLE);
		SetWindowLong(hWndML, GWL_STYLE, dwStyle | WS_CHILD);
		::SetParent(hWndML, g_hWndEWofMM);

		PostMessage(g_MLService.GetMainWindow(), g_msgMediaMonkeyEmbeder, true, 0);
	}

	RECT rcParent, rcML;
	if (GetWindowRect(g_hWndEWofMM, &rcParent)
		&& GetWindowRect(hWndML, &rcML))
	{
		if (rcParent.left == rcML.left && rcParent.top == rcML.top
			&& rcParent.right == rcML.right && rcParent.bottom == rcParent.bottom)
			return;

		MoveWindow(hWndML, 0, 0, rcParent.right - rcParent.left, rcParent.bottom - rcParent.top, TRUE);
	}
}

void CMediaMonkey::Close()
{
	m_app = NULL;
}
