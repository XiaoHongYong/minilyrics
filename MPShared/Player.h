#pragma once

#include "../MPlayerLib/IMPlayer.h"
#include "../MPlayerLib/MPTools.h"
#include "CurMediaAlbumArt.h"

#ifndef _MLPLAYER
#include "../MLPluginLib/MiniLyricsPlugin.h"
#endif

#define LRC_TITLE_MAX_LEN		128

enum PLAYER_TYPE
{
	P_NOTSET			= 0xFF,
	P_UNKNOWN			= 0,
	P_WINAMP2			= 1,
	P_WMP				= 3,
	P_FOOOBAR2000		= 4,
	P_QCDPLAYER			= 5,
	P_WINAMP5			= 6,
	P_ITUNES			= 7,
	P_ZIKIPLAYER		= 8,
};

class CMPlayerEventHandler;
class CMPluginManager;

class CPlayer
{
public:
	CPlayer();
	virtual ~CPlayer();

public:
	void OnInit(bool bMiniLyrics);
	void OnQuit();

	//////////////////////////////////////////////////////////////////////////
	//
	// APIs for MiniLyrics
	//
	void OnInitOfMiniLyrics();
	void OnQuitOfMiniLyrics();

#ifdef _WIN32
	static void QueryInfo(HWND hWndPluginService);

	bool IsConnected() { return m_playerInfo.hWndPluginService != NULL; }
#endif

#ifndef _MPLAYER
	void Connect(PlayerInfo &playerInfo);
	void Disconnect();

	void SendMainWndHandle();

	int SwitchToEmbeddedSkin();
	void EndEmbeddedSkin();

	bool IsSupportEmbedded() { return IsFlagSet(m_playerInfo.startupFlags, ML_SUPPORT_EMBEDDED); }
	bool IsEmbeddedImmovable() { return IsFlagSet(m_playerInfo.startupFlags, ML_EMBEDDED_WND_IMMOVABLE); }

	LPCTSTR GetPlayerName() const;

	// Get player skin path
	void GetCurSkinPath(TCHAR * lpszBuffer, int nBufSize) const;

	HWND GetMainWnd() const;

	const PlayerSkinInfo &GetPlayerSkinInfo() const
		{ return m_skinInfo; }

	BOOL IsFeatureSupported(MLStarupFlags feature)
		{ return IsFlagSet(m_playerInfo.startupFlags, feature); }

	// Get player skin path
	LPCTSTR GetEmbeddedSkinName() const
	{
#ifdef _WIN32
		return m_playerInfo.embeddedSkinName;
#else
		return "";
#endif
	}
#endif // #ifndef _MPLAYER

	void SendMessage(MessageToPlayer message, LPVOID param = NULL, size_t sizeParam = 0)
	{
#ifdef _WIN32
		ASSERT(m_playerInfo.hWndPluginService);

		COPYDATASTRUCT	cds;
		cds.cbData = sizeParam;
		cds.dwData = message;
		cds.lpData = param;
		::SendMessage(m_playerInfo.hWndPluginService, WM_COPYDATA, ML_MSG_VERSION, (LPARAM)&cds);
#endif
	}

	PLAYER_TYPE GetPlayerType() const { return m_PlayerType; }

	//////////////////////////////////////////////////////////////////////////
	//
	// APIs for ZikiPlayer
	//
	void OnInitOfZikiPlayer();
	void OnQuitOfZikiPlayer();

	// control player
	void Play();
	void Pause();
	void PlayPause();
	void Stop();
	void Prev();
	void Next();
	void SeekTo(DWORD nMsPos);

	// Get player skin path
	void GetCurSkinPath(TCHAR * lpszBuffer, int nBufSize);

	PLAYER_STATE GetPlayerState();

	DWORD GetMediaLength();

	DWORD GetPlayPos();
	bool HasVideo();

	void SetLoop(MP_LOOP_MODE loopMode);
	void SetToNextLoopMode();
	void SetShuffle(bool bValue);

	MP_LOOP_MODE GetLoop();
	bool IsShuffle();

	long GetVolume();
	void SetVolume(long nVol);

	bool IsMute();
	void SetMute(bool bValue);

	IMPlayer *GetIMPlayer() { return m_spPlayer; }

	MLRESULT GetMediaLibrary(IMediaLibrary **ppMediaLib)
		{ return m_spPlayer->GetMediaLibrary(ppMediaLib); }

	void ClearPlaylist();
	void NewCurrentPlaylist();

	tstring GetCurrentPlaylistFile() { return m_strCurrentPlaylist; }

	void AddDirToPlaylist(LPCTSTR szDir, bool bIncSubDir = true);
	void AddToPlaylist(LPCTSTR szMedia);

	bool LoadPlaylist(LPCTSTR szFile, bool bClear = true);
	bool IsMediaInPlaylist(LPCTSTR szMedia);

	void AddFileToMediaLib(LPCTSTR szMedia);
	void AddDirToMediaLib(LPCTSTR szDir, bool bIncSubDir);

	bool IsCurrentMediaFileExist();

	bool IsMediaOpened();

	long GetCurrentMediaIndex();

	void PlayMedia(long nPlaylistIndex);

	void GetFileOpenDlgExtention(tstring &strExtentions) const;
	void GetSupportedExtentions(vector<tstring> &vExtentions) const;
	bool IsExtSupported(LPCTSTR szExt);
	bool IsExtAudioFile(LPCTSTR szExt);
	static bool IsExtPlaylistFile(LPCTSTR szExt);

	void SetPlaylistModified(bool bModified) { m_bPlModified = bModified; }

	static void FormatMediaTitle(IMedia *pMedia, tstring &strTitle);

	void FilterLowRatingMedia(IPlaylist *playlist);

	LPCTSTR GetArtist() { return m_szArtist; }
	LPCTSTR GetTitle() { return m_szTitle; }
	LPCTSTR GetAlbum() { return m_szAlbum; }
	LPCTSTR GetFullTitle() { return m_szFullTitle; }
	LPCTSTR GetSrcMedia() { return m_szSrcMedia; }
	void GetSrcMedia(TCHAR * lpszSourceMedia, int nBufSize)
		{ _tcscpy_safe(lpszSourceMedia, nBufSize, m_szSrcMedia); }

	MLRESULT GetCurrentMedia(IMedia **ppMedia);
	MLRESULT GetCurrentPlaylist(IPlaylist **ppPlaylist);
	MLRESULT NewPlaylist(IPlaylist **ppPlaylist);
	MLRESULT SetCurrentPlaylist(IPlaylist *pPlaylist);
	MLRESULT SetCurrentMediaInPlaylist(long nIndex);
	MLRESULT NewMedia(IMedia **ppMedia, LPCXSTR szUrl);

	int UnregisterVis(IVis *pVis);
	int RegisterVis(IVis *pVis);

	void GetCurMediaAttribute(MediaAttribute mediaAttr, tstring &strValue);

	void SaveCurrentPlaylist();
	void SaveCurrentPlaylistAs(LPCTSTR szFile);

	int GetMediaOfAlbumArt(IMedia *pMedia);

	void SetLyrDrawUpdateFast(bool bFast);

	//
	// Use UI Seekbar time as playing time
	//
	void UISeekbarOnSeek(int nPos);
	void SetUseSeekTimeAsPlayingTime(bool bUseSeekTimeAsPlayingTime) { m_bUseSeekTimeAsPlayingTime = bUseSeekTimeAsPlayingTime; }
	bool IsUseSeekTimeAsPlayingTime() const { return m_bUseSeekTimeAsPlayingTime; }

#ifndef _MPLAYER
	void OnMediaInfoChangedOfMiniLyrics(MLMediaInfo *pMediaInfo);
#endif // #ifndef _MPLAYER
	void OnMediaChangedOfZikiPlayer();

	tstring GetTitleFilterFile();
	bool SetFieldValue(TCHAR szField[], int lenField, LPCTSTR szValue);

protected:
	TCHAR					m_szSrcMedia[MAX_PATH];
	TCHAR					m_szArtist[LRC_TITLE_MAX_LEN], m_szTitle[LRC_TITLE_MAX_LEN];
	TCHAR					m_szAlbum[LRC_TITLE_MAX_LEN], m_szFullTitle[LRC_TITLE_MAX_LEN * 2];
	DWORD					m_nMediaLength;
	int						m_hasVideo;
	DWORD					m_dwPlayingPos;

	bool					m_bUseSeekTimeAsPlayingTime;

	VecString				m_vTitleFilters;

protected:
	// For MiniLyrics
	friend class CMLPlayerMgr;
	friend class CMPSkinMainWnd;
	PLAYER_TYPE				m_PlayerType;
	PlayerInfo				m_playerInfo;
	PlayerSkinInfo			m_skinInfo;
	PLAYER_STATE			m_playerState;

protected:
	// For ZikiPlayer
	CMPAutoPtr<CMPluginManager>	m_pPluginMgr;
	tstring					m_strCurrentPlaylist;
	bool					m_bPlModified;
	int						m_nRatingFilterMin;

	IMPlayer				*m_spPlayer;
	CMPlayerEventHandler	*m_pEventHandler;

};

extern CPlayer			g_Player;
