#include "stdafx.h"
#include "../MPlayerLib/MPlayer.h"
#include "MPlayerAppBase.h"
#include "Player.h"
#include "PlayListFile.h"
#include "MPluginManager.h"

#ifdef _WIN32
#include "MPMsg.h"
#include "win32/PlayerEventDispatcher.h"
#endif

#ifdef _MAC_OSX
#include "mac/PlayerEventDispatcher.h"
#endif

#ifndef _MPLAYER

#ifdef _MAC_OSX
#include "../MiniLyrics/mac/MLPlayerMgr.h"
#endif

#ifdef _WIN32
#include "../MiniLyrics/win32/MLPlayerMgr.h"
#endif

#endif // #ifndef _MPLAYER

CPlayer			g_Player;
CPlayerEventDispatcher		g_playerEventDispatcher;


void FormatFullTitle(LPCTSTR szArtist, LPCTSTR szTitle, LPCTSTR szUri, tstring &strFullTitle)
{
	strFullTitle = FormatMediaTitle(szArtist, szTitle);
	if (strFullTitle.empty())
		strFullTitle = UrlGetTitle(szUri);
}

void CopyPlaylist(IPlaylist *src, IPlaylist *dst)
{
	int			nCount = src->GetCount();
	for (int i = 0; i < nCount; i++)
	{
		CMPAutoPtr<IMedia>		media;
		if (src->GetItem(i, &media) == ERR_OK)
			dst->InsertItem(-1, media);
	}
}

///////////////////////////////////////////////////////////////////////////
//
class CMPlayerEventHandler : public IMPEvent
{
	OBJ_REFERENCE_DECL
public:
	CMPlayerEventHandler() { OBJ_REFERENCE_INIT }
	virtual ~CMPlayerEventHandler() { }

	virtual void OnStateChanged(PLAYER_STATE state)
	{
		CEventPlayerStatusChanged	*pEvent = new CEventPlayerStatusChanged();
		pEvent->eventType = ET_PLAYER_STATUS_CHANGED;
		pEvent->status = state;
		CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);

		g_playerEventDispatcher.StopLyrDrawUpdate();

		if (state == PS_PLAYING)
			g_playerEventDispatcher.StartLyrDrawUpdate();
	}

	virtual void OnSeek()
	{
		IEvent	*pEvent = new IEvent();
		pEvent->eventType = ET_PLAYER_SEEK;
		CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
	}

	virtual void OnCurrentMediaChanged()
	{
		g_Player.OnMediaChangedOfZikiPlayer();

		IEvent	*pEvent = new IEvent();
		pEvent->eventType = ET_PLAYER_CUR_MEDIA_CHANGED;
		CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
	}

	virtual void OnCurrentPlaylistChanged(IMPEvent::MP_PLAYLIST_CHANGE_ACTION action, long nIndex, long nIndexOld)
	{
		CEventPlaylistChanged	*pEvent = new CEventPlaylistChanged();
		pEvent->eventType = ET_PLAYER_CUR_PLAYLIST_CHANGED;
		pEvent->action = action;
		pEvent->nIndex = nIndex;
		pEvent->nIndexOld = nIndexOld;
		CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEventByNoUIThread(pEvent);
	}

	virtual void OnSettingChanged(MP_SETTING_TYPE settingType, long nValue)
	{
		CEventPlayerSettingChanged	*pEvent = new CEventPlayerSettingChanged();
		pEvent->eventType = ET_PLAYER_SETTING_CHANGED;
		pEvent->settingType = settingType;
		pEvent->nValue = nValue;
		CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
	}

	virtual void OnEQSettingChanged(const EQualizer *eq)
	{
		CEventPlayerEQChanged	*pEvent = new CEventPlayerEQChanged();
		pEvent->eventType = ET_PLAYER_EQ_SETTING_CHANGED;
		pEvent->eqlalizer = *eq;
		CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
	}

	virtual void OnPlayHaltError(MLRESULT nError)
	{
	}

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayer::CPlayer()
{
	EmptyStr(m_szFullTitle);
	EmptyStr(m_szArtist);
	EmptyStr(m_szTitle);
	EmptyStr(m_szSrcMedia);
	m_pEventHandler = NULL;
	m_nRatingFilterMin = 3;

	m_PlayerType = P_NOTSET;
	m_bUseSeekTimeAsPlayingTime = false;
	m_nMediaLength = 0;
	m_hasVideo = R_UNDETERMINED;
	m_playerState = PS_STOPED;
	m_dwPlayingPos = 0;

	memset(&m_playerInfo, 0, sizeof(m_playerInfo));
	memset(&m_skinInfo, 0, sizeof(m_skinInfo));
}

CPlayer::~CPlayer()
{
}

void CPlayer::OnInit(bool bMiniLyrics)
{
	// Open title filters
	tstringex strFilter;
	if (OpenFileToTString(GetTitleFilterFile().c_str(), strFilter))
	{
		StrSplit(strFilter.c_str(), '\n', m_vTitleFilters);
		for (size_t i = 0; i < m_vTitleFilters.size();)
		{
			if (m_vTitleFilters[i].empty() || StrBeginWith(m_vTitleFilters[i].c_str(), _T(";")))
				m_vTitleFilters.erase(m_vTitleFilters.begin() + i);
			else
				i++;
		}
		TrimStr(m_vTitleFilters, '\r');
		TrimStr(m_vTitleFilters, ' ');
	}

	if (bMiniLyrics)
		OnInitOfMiniLyrics();
	else
		OnInitOfZikiPlayer();
}

void CPlayer::OnQuit()
{
	if (m_spPlayer)
		OnQuitOfZikiPlayer();
	else
		OnQuitOfMiniLyrics();
}

//////////////////////////////////////////////////////////////////////////
//
// For MiniLyrics
//

#ifdef _MPLAYER
HWND GetMLSericeWindow() { return NULL; }
#else
HWND GetMLSericeWindow();
#endif

#ifndef _MPLAYER

void CPlayer::OnInitOfMiniLyrics()
{
	g_playerEventDispatcher.Init();
}

void CPlayer::OnQuitOfMiniLyrics()
{
	g_playerEventDispatcher.Quit();

	Disconnect();
}

void CPlayer::Connect(PlayerInfo &playerInfo)
{
	m_playerInfo = playerInfo;

	MLInfo		mlInfo;
#ifdef _WIN32
	mlInfo.hWndMLService = GetMLSericeWindow();
	ASSERT(mlInfo.hWndMLService);
#endif
	SendMessage(M2P_CONNECTED, &mlInfo, sizeof(mlInfo));

	LPCTSTR		playerName = GetPlayerName();
	if (_tcsicmp(playerName, _T("Winamp2")) == 0)
		m_PlayerType = P_WINAMP2;
	else if (_tcsicmp(playerName, _T("Winamp5")) == 0)
		m_PlayerType = P_WINAMP5;
	else if (_tcsicmp(playerName, _T("windows media player")) == 0)
		m_PlayerType = P_WMP;
	else if (_tcsicmp(playerName, _T("foobar2000")) == 0)
		m_PlayerType = P_FOOOBAR2000;
	else if (_tcsicmp(playerName, _T("Quintessenial Player")) == 0)
		m_PlayerType = P_QCDPLAYER;
	else if (_tcsicmp(playerName, _T("iTunes")) == 0)
		m_PlayerType = P_ITUNES;
	else
		m_PlayerType = P_UNKNOWN;

#ifdef _WIN32
	if (IsSupportEmbedded() && IsEmptyString(m_playerInfo.embeddedSkinName))
	{
		wcscpy_safe(m_playerInfo.embeddedSkinName, CountOf(m_playerInfo.embeddedSkinName),
			_TLM("Embedded Skin"));
	}
#endif
}

void CPlayer::Disconnect()
{
	SendMessage(M2P_DISCONNECT);
}

#ifdef _MINILYRICS_WIN32
void CPlayer::SendMainWndHandle()
{
	HWND	hWndML = CMPlayerAppBase::GetMainWnd()->GetHandle();
	ASSERT(hWndML);
	SendMessage(M2P_MAINWND_HANDLE, &hWndML, sizeof(hWndML));
}

int CPlayer::SwitchToEmbeddedSkin()
{
	if (!m_playerInfo.hWndPluginService || !IsSupportEmbedded())
		return ERR_FALSE;

	SendMessage(M2P_SWITCH_2_EMBEDDED_SKIN);
	return ERR_OK;
}

void CPlayer::EndEmbeddedSkin()
{
	SendMessage(M2P_END_EMBEDDED_SKIN);
}

void CPlayer::GetCurSkinPath(TCHAR *lpszBuffer, int nBufSize)
{
	if (m_spPlayer)
		EmptyStr(lpszBuffer);
	else
		_tcscpy_safe(lpszBuffer, nBufSize, m_skinInfo.skinPath);
}

HWND CPlayer::GetMainWnd() const
{
	if (m_spPlayer)
		return NULL;
	else
		return m_playerInfo.hWndPlayer;
}

void CPlayer::SendMessage(MessageToPlayer message, LPVOID param, size_t sizeParam)
{
	ASSERT(m_playerInfo.hWndPluginService);
	
	COPYDATASTRUCT	cds;
	cds.cbData = sizeParam;
	cds.dwData = message;
	cds.lpData = param;
	::SendMessage(m_playerInfo.hWndPluginService, WM_COPYDATA, ML_MSG_VERSION, (LPARAM)&cds);
}

#else // _MINILYRICS_WIN32

void CPlayer::SendMessage(MessageToPlayer message, LPVOID param, size_t sizeParam)
{
	g_playerMgr.SendMessageToPlayer(message, param, sizeParam);
}

#endif // _MINILYRICS_WIN32

LPCTSTR CPlayer::GetPlayerName() const
{
	if (m_spPlayer)
		return _T("Lyrics Downloader");
	else
		return m_playerInfo.playerName;
}

#else // #ifndef _MPLAYER

void CPlayer::OnInitOfMiniLyrics()
{
}

void CPlayer::OnQuitOfMiniLyrics()
{
}

#endif // #ifndef _MPLAYER

// control player
void CPlayer::Play()
{
	if (m_spPlayer)
		m_spPlayer->Play();
	else
		SendMessage(M2P_PLAY);
}

void CPlayer::Pause()
{
	if (m_spPlayer)
		m_spPlayer->Pause();
	else
		SendMessage(M2P_PAUSE);
}

void CPlayer::PlayPause()
{
	if (m_spPlayer)
	{
		if (m_spPlayer->GetState() == PS_PLAYING)
			m_spPlayer->Pause();
		else
			m_spPlayer->Play();
	}
	else
		SendMessage(M2P_PLAY_PAUSE);
}

void CPlayer::Stop()
{
	if (m_spPlayer)
		m_spPlayer->Stop();
	else
		SendMessage(M2P_STOP);
}

void CPlayer::Prev()
{
	if (m_spPlayer)
		m_spPlayer->Prev();
	else
		SendMessage(M2P_PREV);
}

void CPlayer::Next()
{
	if (m_spPlayer)
		m_spPlayer->Next();
	else
		SendMessage(M2P_NEXT);
}

void CPlayer::SeekTo(DWORD nMsPos)
{
	if (m_spPlayer)
		m_spPlayer->Seek(nMsPos);
	else
		SendMessage(M2P_SEEK, &nMsPos, sizeof(nMsPos));
}

// get player state and song info
PLAYER_STATE CPlayer::GetPlayerState()
{
	if (m_spPlayer)
		return m_spPlayer->GetState();
	else
		return m_playerState;
}

DWORD CPlayer::GetMediaLength()
{
	return m_nMediaLength;
}

DWORD CPlayer::GetPlayPos()
{
	if (m_spPlayer)
	{
		uint32		nPos;

		assert(m_spPlayer);
		if (!m_spPlayer)
			return 0;

		nPos = m_spPlayer->GetPos();

		return nPos;
	}
	else
		return m_dwPlayingPos;
}

bool CPlayer::HasVideo()
{
	if (m_spPlayer)
		return false;

	if (m_hasVideo != R_UNDETERMINED)
		return Tobool(m_hasVideo);

	if (IsEmptyString(m_szSrcMedia))
		return false;

	DWORD		dwFileSize;
	LPCTSTR		szExt;
	static LPCTSTR		SZ_AUDIO_FILE[] = { _T("mp3"), _T("wma"), _T("ogg"), _T("ape"), _T("mpc"), _T("wav"), };
	static LPCTSTR		SZ_VEDIO_FILE[] = { _T("rmvb"), _T("mpg"), _T("mpeg"), _T("avi"), _T("wmv"), _T("mkv"), _T("flv"), _T("asx"), };

	szExt = FileGetExt(m_szSrcMedia);
	if (szExt)
	{
		szExt++;

		int		i;
		for (i = 0; i < CountOf(SZ_AUDIO_FILE); i++)
		{
			if (_tcsicmp(szExt, SZ_AUDIO_FILE[i]) == 0)
				return false;
		}

		for (i = 0; i < CountOf(SZ_VEDIO_FILE); i++)
		{
			if (_tcsicmp(szExt, SZ_VEDIO_FILE[i]) == 0)
				return true;
		}
	}

	// If the file size is larger than 50M, consider it has video.
	dwFileSize = GetFileLength(m_szSrcMedia);
	if (dwFileSize != INVALID_FILE_SIZE && dwFileSize >= 1024 * 1024 * 50)
		return true;

	return false;
}

void CPlayer::SetLyrDrawUpdateFast(bool bFast)
{
	g_playerEventDispatcher.SetLyrDrawUpdateFast(bFast);
}

void CPlayer::SetToNextLoopMode()
{
	if (!m_spPlayer)
		return;

	MP_LOOP_MODE loopMode = m_spPlayer->GetLoop();

	if (loopMode == MP_LOOP_OFF)
		loopMode = MP_LOOP_ALL;
// 	else if (loopMode == MP_LOOP_ALL)
// 		loopMode = MP_LOOP_TRACK;
	else
		loopMode = MP_LOOP_OFF;

	SetLoop(loopMode);
}

void CPlayer::SetLoop(MP_LOOP_MODE loopMode)
{
	if (!m_spPlayer)
		return;

	m_spPlayer->SetLoop(loopMode);

	LPCTSTR		szValue;
	if (loopMode == MP_LOOP_ALL)
		szValue = _T("all");
	else if (loopMode == MP_LOOP_TRACK)
		szValue = _T("track");
	else
		szValue = _T("off");

	g_Profile.WriteProfileString(SZ_SECT_PLAYER, _T("repeat"), szValue);
}

void CPlayer::SetShuffle(bool bValue)
{
	if (!m_spPlayer)
		return;

	m_spPlayer->SetShuffle(bValue);

	g_Profile.WriteProfileString(SZ_SECT_PLAYER, _T("shuffle"), bValue ? SZ_TRUE : SZ_FALSE);
}

MP_LOOP_MODE CPlayer::GetLoop()
{
	if (!m_spPlayer)
		return MP_LOOP_OFF;

	return m_spPlayer->GetLoop();
}

bool CPlayer::IsShuffle()
{
	if (m_spPlayer)
		return m_spPlayer->GetShuffle();
	else
		return false;
}

long CPlayer::GetVolume()
{
	if (m_spPlayer)
		return m_spPlayer->GetVolume();
	else
		return g_playerEventDispatcher.GetMasterVolume();
}

void CPlayer::SetVolume(long nVol)
{
	if (m_spPlayer)
	{
		g_Profile.WriteProfileString(SZ_SECT_PLAYER, _T("mute"), SZ_FALSE);
		g_Profile.WriteProfileInt(SZ_SECT_PLAYER, _T("volume"), nVol);
		m_spPlayer->SetVolume(nVol);
	}
	else
		g_playerEventDispatcher.SetMasterVolume(nVol);
}

bool CPlayer::IsMute()
{
	if (m_spPlayer)
		return m_spPlayer->GetMute();
	else
		return false;
}

void CPlayer::SetMute(bool bValue)
{
	if (!m_spPlayer)
		return;

	m_spPlayer->SetMute(bValue);

	g_Profile.WriteProfileString(SZ_SECT_PLAYER, _T("mute"), bValue ? SZ_TRUE : SZ_FALSE);
}

void CPlayer::OnInitOfZikiPlayer()
{
	if (CMPlayer::GetInstance(&m_spPlayer) != ERR_OK)
		return;

	m_PlayerType = P_ZIKIPLAYER;

	ASSERT(m_pPluginMgr == NULL);

	m_pPluginMgr = new CMPluginManager;
	m_spPlayer->SetPluginManager(m_pPluginMgr);

	m_pEventHandler = new CMPlayerEventHandler;
	m_pEventHandler->AddRef();
	m_spPlayer->RegisterEvent(m_pEventHandler);

	LPCTSTR			szValue;
	MP_LOOP_MODE	loopMode;

	// get loop mode
	szValue = g_Profile.GetProfileString(SZ_SECT_PLAYER, _T("repeat"), _T("off"));
	if (_tcsicmp(szValue, _T("all")) == 0)
		loopMode = MP_LOOP_ALL;
	else if (_tcsicmp(szValue, _T("track")) == 0)
		loopMode = MP_LOOP_TRACK;
	else
		loopMode = MP_LOOP_OFF;
	m_spPlayer->SetLoop(loopMode);

	// shuffle
	m_spPlayer->SetShuffle(
		IsTRUE(g_Profile.GetProfileString(SZ_SECT_PLAYER, _T("shuffle"), SZ_FALSE)));

	// mute
	m_spPlayer->SetMute(
		IsTRUE(g_Profile.GetProfileString(SZ_SECT_PLAYER, _T("mute"), SZ_FALSE)));

	// volume
	m_spPlayer->SetVolume(g_Profile.GetProfileInt(SZ_SECT_PLAYER, _T("volume"), 30));

	// current playing music
	// m_spPlayer->SetCurrentMediaInPlaylist(
	//	g_Profile.GetProfileInt(SZ_SECT_PLAYER, _T("NowPlayingIdx"), 0));

	g_playerEventDispatcher.Init();

	if (g_Profile.GetProfileBool(_T("StartPlayAtStartup"), false))
		Play();
}

void CPlayer::OnQuitOfZikiPlayer()
{
	g_playerEventDispatcher.Quit();

	SaveCurrentPlaylist();

	// save latest playing song file
	g_Profile.WriteProfileInt(SZ_SECT_PLAYER, _T("NowPlayingIdx"), m_spPlayer->GetCurrentMediaInPlaylist());

	if (m_pEventHandler)
	{
		m_spPlayer->UnregisterEvent(m_pEventHandler);
		m_pEventHandler->Release();
	}

	ASSERT(m_spPlayer);
	m_spPlayer->Stop();
	CMPlayer::QuitInstance(&m_spPlayer);
}

void CPlayer::ClearPlaylist()
{
	if (!m_spPlayer)
		return;

	SetPlaylistModified(true);

	CMPAutoPtr<IPlaylist>	playlist;
	if (m_spPlayer->GetCurrentPlaylist(&playlist) == ERR_OK)
		playlist->Clear();
}

void CPlayer::NewCurrentPlaylist()
{
	ClearPlaylist();

	m_strCurrentPlaylist = GetAppDataDir();
	m_strCurrentPlaylist += _T("untitled playlist.m3u");

	if (!IsFileExist(m_strCurrentPlaylist.c_str()))
		return;

	CStrPrintf strFile;
	for (int i = 0; i < 100; i++)
	{
		strFile.printf(_T("%suntitled playlist %d.m3u"), GetAppDataDir(), i);

		if (!IsFileExist(strFile.c_str()))
		{
			m_strCurrentPlaylist = strFile.c_str();
			break;
		}
	}
}

void CPlayer::AddDirToPlaylist(LPCTSTR szDir, bool bIncSubDir)
{
	if (!m_spPlayer)
		return;

	CMLFileFind		find;
	tstring			strDir;
	tstring			strFile;

	if (!find.OpenDir(szDir))
		return;

	strDir = szDir;
	DirStringFormat(strDir);

	while (find.FindNext())
	{
		if (find.IsCurDir())
		{
			if (bIncSubDir && _tcscmp(_T("."), find.GetCurName()) != 0 &&
				_tcscmp(_T(".."), find.GetCurName()) != 0)
			{
				strFile = strDir + find.GetCurName();
				AddDirToPlaylist(strFile.c_str(), bIncSubDir);
			}
		}
		else
		{
			strFile = strDir + find.GetCurName();
			AddToPlaylist(strFile.c_str());
		}
	}
}

void CPlayer::AddToPlaylist(LPCTSTR szMedia)
{
	if (!m_spPlayer)
		return;

	IPlaylist		*playList;
	MLRESULT		nRet;
	LPCTSTR			szExt;

	szExt = FileGetExt(szMedia);
	if (IsEmptyString(szExt))
		return;

	if (IsExtPlaylistFile(szExt))
	{
		LoadPlaylist(szMedia, false);
		return;
	}

	if (!IsExtAudioFile(szExt))
		return;

	// do not add repeated item.
	if (IsMediaInPlaylist(szMedia))
		return;

	nRet = m_spPlayer->GetCurrentPlaylist(&playList);
	if (nRet == ERR_OK)
	{
		CMPAutoPtr<IMedia>	media;

		if (m_spPlayer->NewMedia(&media, szMedia) == ERR_OK)
			playList->InsertItem(-1, media);

 		playList->Release();
	}
}

bool CPlayer::IsMediaInPlaylist(LPCTSTR szMedia)
{
	if (!m_spPlayer)
		return true;

	CMPAutoPtr<IPlaylist>	playList;
	MLRESULT		nRet;
	bool			bRet = false;

	nRet = m_spPlayer->GetCurrentPlaylist(&playList);
	if (nRet == ERR_OK)
	{
		long			nCount;
		nCount = playList->GetCount();
		for (long i = 0; i < nCount; i++)
		{
			CMPAutoPtr<IMedia>	media;
			nRet = playList->GetItem(i, &media);
			if (nRet == ERR_OK)
			{
				CXStr	url;
				nRet = media->GetSourceUrl(&url);
				if (nRet == ERR_OK)
				{
					if (_tcscmp(szMedia, url.c_str()) == 0)
					{
						bRet = true;
						break;
					}
				}
			}
		}
	}

	return bRet;
}

bool CPlayer::LoadPlaylist(LPCTSTR szFile, bool bClear)
{
	if (!m_spPlayer)
		return false;

	MLRESULT				nRet;
	CMPAutoPtr<IPlaylist>	playlist, curPlaylist;

	if (m_spPlayer->NewPlaylist(&playlist) != ERR_OK)
		return false;

	if (bClear)
		m_strCurrentPlaylist = szFile;
	else
	{
		nRet = m_spPlayer->GetCurrentPlaylist(&curPlaylist);
		if (nRet == ERR_OK)
		{
			CopyPlaylist(curPlaylist, playlist);
		}
	}

	if (!::LoadPlaylist(m_spPlayer, playlist, szFile))
		return false;

	nRet = m_spPlayer->SetCurrentPlaylist(playlist);

	return nRet == ERR_OK;
}

long CPlayer::GetCurrentMediaIndex()
{
	if (!m_spPlayer)
		return 0;

	return m_spPlayer->GetCurrentMediaInPlaylist();
}

bool CPlayer::IsCurrentMediaFileExist()
{
	if (IsEmptyString(m_szSrcMedia) || !IsFileExist(m_szSrcMedia))
		return false;
	return true;
}

bool CPlayer::IsMediaOpened()
{
	if (m_spPlayer)
	{
		CMPAutoPtr<IMedia>	media;
		MLRESULT	nRet;

		nRet = m_spPlayer->GetCurrentMedia(&media);

		return nRet == ERR_OK;
	}
	else
	{
		if (IsEmptyString(m_szSrcMedia) && IsEmptyString(m_szFullTitle))
			return false;
		return true;
	}
}

void CPlayer::PlayMedia(long nPlaylistIndex)
{
	if (!m_spPlayer)
		return;

	m_spPlayer->Stop();
	m_spPlayer->SetCurrentMediaInPlaylist(nPlaylistIndex);
	m_spPlayer->Play();
}

void CPlayer::GetFileOpenDlgExtention(tstring &strExtentions) const
{
	// static TCHAR	szSupportedExtion[] = _T("All supported file\0*.m3u;*.pls;*.wpl;*.mp3;*.mp2;*.mpa;*.mp4;*.m4a;*.wav;*.ogg;*.wma\0Playlist file\0*.pls;*.wpl;*.m3u\0Mp3 file\0*.mp3;*.mp2;*.mpa\0WMA file\0*.wma\0MP4 file\0*.mp4;*.m4a\0Ogg file\0*.ogg\0Wave file\0*.wav\0All Files\0*.*\0\0");
	static TCHAR	szSupportedExtion[] = _T("All supported file\0*.m3u;*.pls;*.wpl;*.mp3;*.mp2;*.mpa;*.m4a;*.wav;*.ogg;*.wma\0Playlist file\0*.pls;*.wpl;*.m3u\0Mp3 file\0*.mp3;*.mp2;*.mpa\0WMA file\0*.wma\0MP4 file\0*.m4a\0Ogg file\0*.ogg\0Wave file\0*.wav\0All Files\0*.*\0\0");
	strExtentions.append(szSupportedExtion, CountOf(szSupportedExtion));
}

void CPlayer::GetSupportedExtentions(vector<tstring> &vExtentions) const
{
	if (!m_spPlayer)
		return;

	const ListDecodersInfo		&listDecoder = m_pPluginMgr->GetDecodersInfo();
	ListDecodersInfo::const_iterator	it, itEnd;

	itEnd = listDecoder.end();
	for (it = listDecoder.begin(); it != itEnd; ++it)
	{
		const DecoderInfo		&item = *it;
		vExtentions.insert(vExtentions.end(), item.vExt.begin(), item.vExt.end());
	}

	vExtentions.push_back(_T(".m3u"));
	vExtentions.push_back(_T(".pls"));
	vExtentions.push_back(_T(".wpl"));
}

bool CPlayer::IsExtSupported(LPCTSTR szExt)
{
	return IsExtAudioFile(szExt) || IsExtPlaylistFile(szExt);
}

bool CPlayer::IsExtAudioFile(LPCTSTR szExt)
{
	if (!m_spPlayer)
		return false;

	tstring strExtLowerCase = ToLower(szExt);

	const ListDecodersInfo		&listDecoder = m_pPluginMgr->GetDecodersInfo();
	ListDecodersInfo::const_iterator	it;

	for (it = listDecoder.begin(); it != listDecoder.end(); ++it)
	{
		const DecoderInfo		&item = *it;

		if (find(item.vExt.begin(), item.vExt.end(), strExtLowerCase) != item.vExt.end())
			return true;
	}

	return false;
}

bool CPlayer::IsExtPlaylistFile(LPCTSTR szExt)
{
	if (_tcsicmp(_T(".m3u"), szExt) == 0)
		return true;
	if (_tcsicmp(_T(".wpl"), szExt) == 0)
		return true;
	if (_tcsicmp(_T(".pls"), szExt) == 0)
		return true;

	return false;
}


void CPlayer::AddDirToMediaLib(LPCTSTR szDir, bool bIncSubDir)
{
	if (!m_spPlayer)
		return;

	CMLFileFind		find;
	tstring			strDir;
	tstring			strFile;

	if (!find.OpenDir(szDir))
		return;

	strDir = szDir;
	DirStringFormat(strDir);

	while (find.FindNext())
	{
		if (find.IsCurDir())
		{
			if (bIncSubDir && _tcscmp(_T("."), find.GetCurName()) != 0 &&
				_tcscmp(_T(".."), find.GetCurName()) != 0)
			{
				strFile = strDir + find.GetCurName();
				AddDirToMediaLib(strFile.c_str(), bIncSubDir);
			}
		}
		else
		{
			strFile = strDir + find.GetCurName();
			AddFileToMediaLib(strFile.c_str());
		}
	}
}

void CPlayer::AddFileToMediaLib(LPCTSTR szMedia)
{
	if (!m_spPlayer)
		return;

	MLRESULT		nRet;
	LPCTSTR			szExt;

	szExt = FileGetExt(szMedia);
	if (IsEmptyString(szExt))
		return;
// 
// 	if (IsExtPlaylistFile(szExt))
// 	{
// 		LoadPlaylist(szMedia, false);
// 		return;
// 	}

	if (!IsExtAudioFile(szExt))
		return;

	// do not add repeated item.
	CMPAutoPtr<IMediaLibrary>	pMediaLib;
	CMPAutoPtr<IMedia>			pMedia;
	CMPAutoPtr<IPlaylist>		playlist;

	if (m_spPlayer->GetMediaLibrary(&pMediaLib) != ERR_OK)
		return;

	if (pMediaLib->GetMediaByUrl(szMedia, &pMedia) == ERR_OK)
		return;

	nRet = pMediaLib->Add(szMedia, &pMedia);
	if (nRet == ERR_OK)
	{
		nRet = m_spPlayer->GetCurrentPlaylist(&playlist);
		if (nRet == ERR_OK)
			playlist->InsertItem(-1, pMedia);
	}
}

void CPlayer::FormatMediaTitle(IMedia *pMedia, tstring &strFullTitle)
{
	CXStr artist, title;
	pMedia->GetArtist(&artist);
	pMedia->GetTitle(&title);

	strFullTitle = ::FormatMediaTitle(artist.c_str(), title.c_str());
	if (strFullTitle.empty())
	{
		CXStr file;
		pMedia->GetSourceUrl(&file);
		strFullTitle = FileGetTitle(file.c_str());
	}
}

void CPlayer::FilterLowRatingMedia(IPlaylist *playlist)
{
	int		nRating;

	for (long i = playlist->GetCount() - 1; i >= 0; i--)
	{
		CMPAutoPtr<IMedia>		media;

		if (playlist->GetItem(i, &media) == ERR_OK)
		{
			media->GetAttribute(MA_RATING, &nRating);
			if (nRating < m_nRatingFilterMin)
			{
				playlist->RemoveItem(i);
			}
		}
	}
}

MLRESULT CPlayer::GetCurrentMedia(IMedia **ppMedia)
{
	if (m_spPlayer)
		return m_spPlayer->GetCurrentMedia(ppMedia);
	else
		return ERR_NOT_IMPLEMENTED;
}

MLRESULT CPlayer::GetCurrentPlaylist(IPlaylist **ppPlaylist)
{
	if (m_spPlayer)
		return m_spPlayer->GetCurrentPlaylist(ppPlaylist);
	else
		return ERR_NOT_IMPLEMENTED;
}

MLRESULT CPlayer::NewPlaylist(IPlaylist **ppPlaylist)
{
	if (m_spPlayer)
		return m_spPlayer->NewPlaylist(ppPlaylist);
	else
		return ERR_NOT_IMPLEMENTED;
}

MLRESULT CPlayer::SetCurrentPlaylist(IPlaylist *pPlaylist)
{
	if (m_spPlayer)
		return m_spPlayer->SetCurrentPlaylist(pPlaylist);
	else
		return ERR_NOT_IMPLEMENTED;
}

MLRESULT CPlayer::SetCurrentMediaInPlaylist(long nIndex)
{
	if (m_spPlayer)
		return m_spPlayer->SetCurrentMediaInPlaylist(nIndex);
	else
		return ERR_NOT_IMPLEMENTED;
}

MLRESULT CPlayer::NewMedia(IMedia **ppMedia, LPCXSTR szUrl)
{
	if (m_spPlayer)
		return m_spPlayer->NewMedia(ppMedia, szUrl);
	else
		return ERR_NOT_IMPLEMENTED;
}

int CPlayer::UnregisterVis(IVis *pVis)
{
	if (m_spPlayer)
		return m_spPlayer->UnregisterVis(pVis);
	else
		return ERR_NOT_IMPLEMENTED;
}

int CPlayer::RegisterVis(IVis *pVis)
{
	if (m_spPlayer)
		return m_spPlayer->RegisterVis(pVis);
	else
		return ERR_NOT_IMPLEMENTED;
}

void CPlayer::GetCurMediaAttribute(MediaAttribute mediaAttr, tstring &strValue)
{
	if (mediaAttr == MA_ARTIST)
		strValue = m_szArtist;
	else if (mediaAttr == MA_ALBUM)
		strValue = m_szAlbum;
	else if (mediaAttr == MA_TITLE)
		strValue = m_szTitle;
	else if (m_spPlayer)
	{
		strValue.resize(0);

		CMPAutoPtr<IMedia>	media;
		int nRet = m_spPlayer->GetCurrentMedia(&media);
		if (nRet == ERR_OK)
		{
			CXStr	str;
			nRet = media->GetAttribute(mediaAttr, &str);
			if (nRet == ERR_OK)
				strValue = str.c_str();
		}
	}
	else
		strValue.resize(0);
}

void CPlayer::SaveCurrentPlaylist()
{
	if (!m_bPlModified)
		return;

	CMPAutoPtr<IPlaylist>	playlist;

	if (GetCurrentPlaylist(&playlist) == ERR_OK)
	{
		if (SavePlaylistAsM3u(playlist, m_strCurrentPlaylist.c_str()))
		{
			SetPlaylistModified(false);
			// SaveRecentPlList(m_strCurrentPlaylist.c_str());
			g_Profile.WriteProfileString(_T("Latest Playlist"), m_strCurrentPlaylist.c_str());
		}
	}
}

void CPlayer::SaveCurrentPlaylistAs(LPCTSTR szFile)
{
	m_strCurrentPlaylist = szFile;
	SaveCurrentPlaylist();
}

void CPlayer::UISeekbarOnSeek(int nPos)
{
	g_LyricData.SetPlayElapsedTime(nPos);

	CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);
}

#ifndef _MPLAYER
void CPlayer::OnMediaInfoChangedOfMiniLyrics(MLMediaInfo *pMediaInfo)
{
	bool			bChanged = false;

#define		SAFE_COPY_FIELD(src, dst)	\
	if (SetFieldValue(dst, CountOf(dst), src)) bChanged = true;

#define		EMPTY_FIELD(dst)	\
	if (!IsEmptyString(dst))	\
	{							\
	bChanged = true;		\
	EmptyStr(dst);			\
	}

	if (pMediaInfo)
	{
		// Remove file:// prefix in source media url
		static LPCTSTR		SZ_FILE_PREFIX		= _T("file://");
		LPCTSTR szMediaUrl = pMediaInfo->MediaUrl();
		int			nLenPrefix = _tcslen(SZ_FILE_PREFIX);
		bool		bSongBird = false;
		if (szMediaUrl && _tcsnicmp(szMediaUrl, SZ_FILE_PREFIX, nLenPrefix) == 0)
		{
			szMediaUrl += nLenPrefix;

			// Songbird use file:///c:/xx...xxx.mp3 , remove the extra '/'
			if (szMediaUrl[0] == '/' && szMediaUrl[2] == ':')
			{
				szMediaUrl++;
				bSongBird = true;
			}
		}

		SAFE_COPY_FIELD(szMediaUrl, m_szSrcMedia);
		SAFE_COPY_FIELD(pMediaInfo->Artist(), m_szArtist);
		SAFE_COPY_FIELD(pMediaInfo->Album(), m_szAlbum);
		SAFE_COPY_FIELD(pMediaInfo->Title(), m_szTitle);
		m_nMediaLength = pMediaInfo->mediaLength;
		m_hasVideo = pMediaInfo->hasVideo;

		if (bSongBird)
		{
			// For songbird, it will convert all characters to URL format.
#ifdef _WIN32
			// Songbird will conver \ to /, so recover it.
			strrep(m_szSrcMedia, '/', '\\');
#endif
			tstring		strConverted;

#ifdef _WIN32
			UriUnescape(m_szSrcMedia, strConverted, ED_UTF8);
#else
			UriUnescape(m_szSrcMedia, strConverted);
#endif

			_tcscpy_safe(m_szSrcMedia, CountOf(m_szSrcMedia), strConverted.c_str());
		}
	}
	else
	{
		EMPTY_FIELD(m_szSrcMedia);
		EMPTY_FIELD(m_szArtist);
		EMPTY_FIELD(m_szAlbum);
		EMPTY_FIELD(m_szTitle);
		m_nMediaLength = 0;
		m_hasVideo = R_UNDETERMINED;
	}

	if (bChanged)
	{
		if (IsEmptyString(m_szArtist) && 
			(IsEmptyString(m_szTitle) || _tcscmp(FileGetTitle(m_szSrcMedia).c_str(), m_szTitle) == 0))
		{
			tstring		strAr, strTi;
			AnalysisLyricsFileNameEx(strAr, strTi, m_szSrcMedia);

			SetFieldValue(m_szArtist, CountOf(m_szArtist), strAr.c_str());
			SetFieldValue(m_szTitle, CountOf(m_szTitle), strTi.c_str());
		}

		tstring			strFullTitle;
		FormatFullTitle(m_szArtist, m_szTitle, m_szSrcMedia, strFullTitle);
		_tcscpy_safe(m_szFullTitle, CountOf(m_szFullTitle), strFullTitle.c_str());

		IEvent	*pEvent = new IEvent();
		pEvent->eventType = ET_PLAYER_CUR_MEDIA_CHANGED;
		CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
	}
}
#endif // #ifndef _MPLAYER

void CPlayer::OnMediaChangedOfZikiPlayer()
{
	if (!m_spPlayer)
		return;

	MLRESULT				nRet;
	CMPAutoPtr<IMedia>		media;

	EmptyStr(m_szSrcMedia);
	EmptyStr(m_szFullTitle);
	EmptyStr(m_szArtist);
	EmptyStr(m_szTitle);
	EmptyStr(m_szAlbum);
	m_nMediaLength = 0;

	nRet = m_spPlayer->GetCurrentMedia(&media);
	if (nRet == ERR_OK)
	{
		CXStr		str;

		nRet = media->GetSourceUrl(&str);
		if (nRet == ERR_OK)
			_tcscpy_safe(m_szSrcMedia, CountOf(m_szSrcMedia), str.c_str());

		m_nMediaLength = media->GetDuration();

		nRet = media->GetArtist(&str);
		if (nRet == ERR_OK)
			_tcscpy_safe(m_szArtist, CountOf(m_szArtist), str.c_str());

		nRet = media->GetTitle(&str);
		if (nRet == ERR_OK)
			_tcscpy_safe(m_szTitle, CountOf(m_szTitle), str.c_str());

		nRet = media->GetAlbum(&str);
		if (nRet == ERR_OK)
			_tcscpy_safe(m_szAlbum, CountOf(m_szAlbum), str.c_str());

		m_nMediaLength = media->GetDuration();

		tstring			strFullTitle;
		FormatFullTitle(m_szArtist, m_szTitle, m_szSrcMedia, strFullTitle);
		_tcscpy_safe(m_szFullTitle, CountOf(m_szFullTitle), strFullTitle.c_str());
	}
}

tstring CPlayer::GetTitleFilterFile()
{
	return DirStringJoint(GetAppDataDir(), _T("TitlFilters.txt"));
}

bool CPlayer::SetFieldValue(TCHAR szField[], int lenField, LPCTSTR szValue)
{
	if (szValue == NULL)
		return false;

	tstring str = szValue;
	if (szField != m_szSrcMedia)
	{
		for (size_t i = 0; i < m_vTitleFilters.size(); i++)
			strrep(str, m_vTitleFilters[i].c_str(), _T(""));
	}

	if (_tcscmp(szField, str.c_str()) != 0)
	{
		_tcscpy_safe(szField, lenField, str.c_str());
		return true;
	}

	return false;
}
