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

bool CMediaMonkey::GetCurrentMedia(LPWSTR szFile)
{
	if (!MakeSureInterfaceOK())
		return false;

	CMPAutoPtr<ISDBPlayer> player;

	HRESULT hr = m_app->get_Player(&player);
	if (FAILED(hr))
		return false;

	CMPAutoPtr<ISDBSongData> curSong;
	hr = player->get_CurrentSong(&curSong);
	if (FAILED(hr))
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
	if (FAILED(hr))
		return false;

	CMPAutoPtr<ISDBSongData> curSong;
	hr = player->get_CurrentSong(&curSong);
	if (FAILED(hr))
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
		if (FAILED(hr))
		{
			g_MLService.Log("CoCreateInstance of MediaMonkey FAILED!");
			return false;
		}
		g_MLService.Log("after MakeSureInterfaceOK!");
	}

	return true;
}

void CMediaMonkey::Close()
{
	m_app = NULL;
}
