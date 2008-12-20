// Playlist.h: interface for the CPlaylist class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYLIST_H__33064DC2_9A3B_404E_9196_02D9F33C81E5__INCLUDED_)
#define AFX_PLAYLIST_H__33064DC2_9A3B_404E_9196_02D9F33C81E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"
#include "MPHelper.h"
#include "MPTime.h"

class CMPlayer;

class CMedia : public IMedia
{
	OBJ_REFERENCE_DECL
public:
	CMedia();
	virtual ~CMedia();

	// the ID in media library, 0 for not exist in media library
	virtual long GetID();

	virtual MLRESULT GetSourceUrl(IXStr *strUrl);
	virtual MLRESULT GetArtist(IXStr *strArtist);
	virtual MLRESULT GetTitle(IXStr *strTitle);
	virtual MLRESULT GetAlbum(IXStr *strAlbum);

	virtual long GetDuration();

	virtual MLRESULT SetSourceUrl(LPCXSTR strUrl);

	virtual bool IsInfoUpdatedToMediaLib();
	virtual MLRESULT SetInfoUpdatedToMediaLib(bool bUpdated);

	//
	// attribute methods
	//
	virtual MLRESULT GetAttribute(MediaAttribute mediaAttr, IXStr *strValue);
	virtual MLRESULT SetAttribute(MediaAttribute mediaAttr, LPCXSTR szValue);

	virtual MLRESULT GetAttribute(MediaAttribute mediaAttr, int *pnValue);
	virtual MLRESULT SetAttribute(MediaAttribute mediaAttr, int nValue);

protected:
	friend class CMediaLibrary;

	Mutex				m_mutexDataAccess;

	long				m_nID;

	tstring				m_strUrl;
	tstring				m_strArtist;
	tstring				m_strAlbum;
	tstring				m_strTitle;
	int16				m_nTrackNumb;
	int16				m_nYear;
	tstring				m_strGenre;
	tstring				m_strComment;
	long				m_nLength;
	long				m_nFileSize;
	CMPTime				m_nTimeAdded;
	CMPTime				m_nTimePlayed;

	uint16				m_nRating;	// 0 ~ 500
	bool				m_bIsUserRating;
	bool				m_bIsFileDeleted;
	long				m_nPlayed;
	long				m_nPlaySkipped;

	tstring				m_strLyricsFile;

	tstring				m_strFormat;
	long				m_nBitRate;
	bool				m_bIsVbr;
	BYTE				m_nChannels;
	BYTE				m_nBPS;
	bool				m_bInfoUpdated;		// if InfoUpdated, it will be update to database when playing finished.
	uint32				m_nSampleRate;
	tstring				m_strExtraInfo;

};

class CPlaylist : public IPlaylist  
{
	OBJ_REFERENCE_DECL
public:
	CPlaylist(CMPlayer *pPlayer);
	virtual ~CPlaylist();

	virtual uint32 GetCount();

	virtual MLRESULT GetItem(long nIndex, IMedia **ppMedia);

	virtual MLRESULT GetName(IXStr *str);

	virtual MLRESULT InsertItem(long nIndex, IMedia *pMedia);

	virtual MLRESULT MoveItem(long nIndexOld, long nIndexNew);

	virtual MLRESULT RemoveItem(long nIndex);

	virtual MLRESULT Clear();

public:
	MLRESULT GetItemIndex(IMedia *pMedia, long &nIndex);

protected:
	typedef vector<CMedia *>		V_MEDIA;

	Mutex				m_mutexDataAccess;
	V_MEDIA				m_vMedia;

	CMPlayer			*m_pPlayer;

};

#endif // !defined(AFX_PLAYLIST_H__33064DC2_9A3B_404E_9196_02D9F33C81E5__INCLUDED_)
