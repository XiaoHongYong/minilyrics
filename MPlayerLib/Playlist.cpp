// Playlist.cpp: implementation of the CPlaylist class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayer.h"
#include "Playlist.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMedia::CMedia()
{
	OBJ_REFERENCE_INIT

	m_nID = MEDIA_ID_INVALID;
	m_nTrackNumb = -1;
	m_nYear = -1;
 	m_nLength = MEDIA_LENGTH_INVALID;
// 	m_nFileSize = 0;
	m_nRating = 0;
	m_nPlayed = 0;
	m_nPlaySkipped = 0;
	m_nTimeAdded.GetCurrentTime();
	/*m_nBitRate = 0;
	m_bIsVbr = false;
	m_nChannels = 2;
	m_nBPS = 16;*/
	m_nBitRate = 0;
	m_nBPS = 16;
	m_nSampleRate = 0;
	m_bInfoUpdated = false;
	m_bIsUserRating = false;
}

CMedia::~CMedia()
{
}

OBJ_REFERENCE_IMP(CMedia)

long CMedia::GetID()
{
	return m_nID;
}

MLRESULT CMedia::GetSourceUrl(IXStr *strUrl)
{
	strUrl->copy(m_strUrl.c_str());
	return ERR_OK;
}

MLRESULT CMedia::GetArtist(IXStr *strArtist)
{
	strArtist->copy(m_strArtist.c_str());
	return ERR_OK;
}

MLRESULT CMedia::GetTitle(IXStr *strTitle)
{
	strTitle->copy(m_strTitle.c_str());
	return ERR_OK;
}

MLRESULT CMedia::GetAlbum(IXStr *strAlbum)
{
	strAlbum->copy(m_strAlbum.c_str());
	return ERR_OK;
}

long CMedia::GetDuration()
{
	return m_nLength;
}

MLRESULT CMedia::SetSourceUrl(LPCXSTR strUrl)
{
	m_strUrl = strUrl;
	return ERR_OK;
}

bool CMedia::IsInfoUpdatedToMediaLib()
{
	return m_bInfoUpdated;
}

MLRESULT CMedia::SetInfoUpdatedToMediaLib(bool bUpdated)
{
	m_bInfoUpdated = bUpdated;
	return ERR_OK;
}

void Long2XStr(long nValue, IXStr *str)
{
	str->reserve(32);
	_stprintf(str->data(), _T("%d"), nValue);
	str->resize(_tcslen(str->c_str()));
}

void MPTime2XStr(CMPTime &date, IXStr *str)
{
	MPTM		tm;

	if (date.GetMPTM(&tm))
	{
		str->reserve(200);
		_stprintf(str->data(), _T("%04d-%02d-%02d %02d:%02d:%02d"), 
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
		str->resize(_tcslen(str->c_str()));
	}
	else
		str->copy(_T("-"));
}

MLRESULT CMedia::GetAttribute(MediaAttribute mediaAttr, IXStr *strValue)
{
	strValue->clear();

	switch (mediaAttr)
	{
	case MA_ARTIST		: strValue->copy(m_strArtist.c_str()); break;
	case MA_ALBUM		: strValue->copy(m_strAlbum.c_str()); break;
	case MA_TITLE		: strValue->copy(m_strTitle.c_str()); break;
	case MA_TRACK_NUMB	: if (m_nTrackNumb != -1) Long2XStr(m_nTrackNumb, strValue); break;
	case MA_YEAR		: if (m_nYear != -1) Long2XStr(m_nYear, strValue); break;
	case MA_GENRE		: strValue->copy(m_strGenre.c_str()); break;
	case MA_COMMENT		: strValue->copy(m_strComment.c_str()); break;
	case MA_BITRATE		: Long2XStr(m_nBitRate, strValue); break;
	case MA_DURATION	: Long2XStr(m_nLength, strValue); break;
	case MA_FILESIZE	: Long2XStr(m_nFileSize, strValue); break;
	case MA_TIME_ADDED	: MPTime2XStr(m_nTimeAdded, strValue); break;
	case MA_TIME_PLAYED	: MPTime2XStr(m_nTimePlayed, strValue); break;
	case MA_RATING		: Long2XStr(m_nRating, strValue); break;
	case MA_IS_USER_RATING	: Long2XStr(m_bIsUserRating, strValue); break;
	case MA_TIMES_PLAYED: Long2XStr(m_nPlayed, strValue); break;
	case MA_TIMES_PLAY_SKIPPED: Long2XStr(m_nPlaySkipped, strValue); break;
	case MA_LYRICS_FILE	: strValue->copy(m_strLyricsFile.c_str()); break;

	// the following info will not stored in media library
	case MA_FORMAT		: strValue->copy(m_strFormat.c_str()); break;
	case MA_ISVBR		: Long2XStr(m_bIsVbr, strValue); break;
	case MA_BPS			: Long2XStr(m_nBPS, strValue); break;
	case MA_CHANNELS	: Long2XStr(m_nChannels, strValue); break;
	case MA_SAMPLE_RATE	: Long2XStr(m_nSampleRate, strValue); break;
	case MA_EXTRA_INFO	: strValue->copy(m_strExtraInfo.c_str()); break;
	default:
		return ERR_NOT_SUPPORT;
	}
	return ERR_OK;
}

MLRESULT CMedia::SetAttribute(MediaAttribute mediaAttr, LPCXSTR szValue)
{
	switch (mediaAttr)
	{
	case MA_ARTIST		: m_strArtist = szValue; break;
	case MA_ALBUM		: m_strAlbum = szValue; break;
	case MA_TITLE		: m_strTitle = szValue; break;
	case MA_TRACK_NUMB	: m_nTrackNumb = _ttoi(szValue); break;
	case MA_YEAR		: m_nYear = _ttoi(szValue); break;
	case MA_GENRE		: m_strGenre = szValue; break;
	case MA_COMMENT		: m_strComment = szValue; break;
	case MA_BITRATE		: m_nBitRate = _ttoi(szValue); break;
	case MA_DURATION	: m_nLength = _ttoi(szValue); break;
	case MA_FILESIZE	: m_nFileSize = _ttoi(szValue); break;
// 	case MA_TIME_ADDED	: m_nTimeAdded.dwTime = _ttoi(szValue); break;
// 	case MA_TIME_PLAYED	: m_nTimePlayed.dwTime = _ttoi(szValue); break;
	case MA_RATING		: m_nRating = _ttoi(szValue); break;
	case MA_IS_USER_RATING	: m_bIsUserRating = Tobool(_ttoi(szValue)); break;
	case MA_TIMES_PLAYED: m_nPlayed = _ttoi(szValue); break;
	case MA_TIMES_PLAY_SKIPPED: m_nPlaySkipped = _ttoi(szValue); break;
	case MA_LYRICS_FILE	: m_strLyricsFile = szValue; break;

	// the following info will not stored in media library
	case MA_FORMAT		: m_strFormat = szValue; break;
	case MA_ISVBR		: m_bIsVbr = Tobool(_ttoi(szValue)); break;
	case MA_BPS			: m_nBPS = _ttoi(szValue); break;
	case MA_CHANNELS	: m_nChannels = _ttoi(szValue); break;
	case MA_SAMPLE_RATE	: m_nSampleRate = _ttoi(szValue); break;
	case MA_EXTRA_INFO	: m_strExtraInfo = szValue; break;
	default:
		return ERR_NOT_SUPPORT;
	}
	return ERR_OK;
}

MLRESULT CMedia::GetAttribute(MediaAttribute mediaAttr, int *pnValue)
{
	switch (mediaAttr)
	{
	case MA_TRACK_NUMB	: *pnValue = m_nTrackNumb; break;
	case MA_YEAR		: *pnValue = m_nYear; break;
	case MA_BITRATE		: *pnValue = m_nBitRate; break;
	case MA_DURATION	: *pnValue = m_nLength; break;
	case MA_FILESIZE	: *pnValue = m_nFileSize; break;
	case MA_TIME_ADDED	: *pnValue = m_nTimeAdded.m_time; break;
	case MA_TIME_PLAYED	: *pnValue = m_nTimePlayed.m_time; break;
	case MA_RATING		: *pnValue = m_nRating; break;
	case MA_IS_USER_RATING	: *pnValue = m_bIsUserRating; break;
	case MA_TIMES_PLAYED: *pnValue = m_nPlayed; break;
	case MA_TIMES_PLAY_SKIPPED: *pnValue = m_nPlaySkipped; break;

	// the following info will not stored in media library
	case MA_ISVBR		: *pnValue = m_bIsVbr; break;
	case MA_BPS			: *pnValue = m_nBPS; break;
	case MA_CHANNELS	: *pnValue = m_nChannels; break;
	case MA_SAMPLE_RATE	: *pnValue = m_nSampleRate; break;
	default:
		return ERR_NOT_SUPPORT;
	}
	return ERR_OK;
}

MLRESULT CMedia::SetAttribute(MediaAttribute mediaAttr, int nValue)
{
	switch (mediaAttr)
	{
	case MA_TRACK_NUMB	: m_nTrackNumb = nValue; break;
	case MA_YEAR		: m_nYear = nValue; break;
	case MA_BITRATE		: m_nBitRate = nValue; break;
	case MA_DURATION	: m_nLength = nValue; break;
	case MA_FILESIZE	: m_nFileSize = nValue; break;
	case MA_TIME_ADDED	: m_nTimeAdded.m_time = nValue; break;
	case MA_TIME_PLAYED	: m_nTimePlayed.m_time = nValue; break;
	case MA_RATING		: m_nRating = nValue; break;
	case MA_IS_USER_RATING: m_bIsUserRating = Tobool(nValue); break;
	case MA_TIMES_PLAYED: m_nPlayed = nValue; break;
	case MA_TIMES_PLAY_SKIPPED: m_nPlaySkipped = nValue; break;

	// the following info will not stored in media library
	case MA_ISVBR		: m_bIsVbr = Tobool(nValue); break;
	case MA_BPS			: m_nBPS = nValue; break;
	case MA_CHANNELS	: m_nChannels = nValue; break;
	case MA_SAMPLE_RATE	: m_nSampleRate = nValue; break;
	default:
		return ERR_NOT_SUPPORT;
	}
	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
//
CPlaylist::CPlaylist(CMPlayer *pPlayer) : m_pPlayer(pPlayer)
{
	OBJ_REFERENCE_INIT
	if (m_pPlayer)
		m_pPlayer->AddRef();
}

CPlaylist::~CPlaylist()
{
	V_MEDIA::iterator	it, itEnd;

	itEnd = m_vMedia.end();
	for (it = m_vMedia.begin(); it != itEnd; ++it)
	{
		(*it)->Release();
	}
	m_vMedia.clear();

	if (m_pPlayer)
		m_pPlayer->Release();
}

OBJ_REFERENCE_IMP(CPlaylist)

uint32 CPlaylist::GetCount()
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	return m_vMedia.size();
}

MLRESULT CPlaylist::GetItem(long nIndex, IMedia **ppMedia)
{
	CMutexAutolock	autolock(m_mutexDataAccess);

//	ASSERT(nIndex >= 0 && nIndex < (int)m_vMedia.size());

	if (nIndex >= 0 && nIndex < (int)m_vMedia.size())
	{
		*ppMedia = m_vMedia[nIndex];
		(*ppMedia)->AddRef();
		return ERR_OK;
	}

	return ERR_NOT_FOUND;
}

MLRESULT CPlaylist::GetName(IXStr *str)
{
	return ERR_OK;
}

MLRESULT CPlaylist::InsertItem(long nIndex, IMedia *pMedia)
{
	m_mutexDataAccess.Acquire();

	pMedia->AddRef();

	if (nIndex >= 0 && nIndex < (int)m_vMedia.size())
	{
		m_vMedia.insert(m_vMedia.begin() + nIndex, (CMedia*)pMedia);
	}
	else
	{
		m_vMedia.push_back((CMedia*)pMedia);
		nIndex = m_vMedia.size() - 1;
	}

	m_mutexDataAccess.Release();

	m_pPlayer->NotifyPlaylistChanged(this, IMPEvent::PCA_INSERT, nIndex, 0);

	return ERR_OK;
}

MLRESULT CPlaylist::MoveItem(long nIndexOld, long nIndexNew)
{
	CMutexAutolock	autolock(m_mutexDataAccess);
	CMedia		*pTemp;

	ASSERT(nIndexOld >= 0 && nIndexOld < (int)m_vMedia.size());
	ASSERT(nIndexNew >= 0 && nIndexNew < (int)m_vMedia.size());

	if (nIndexOld >= 0 && nIndexOld < (int)m_vMedia.size() &&
		nIndexNew >= 0 && nIndexNew < (int)m_vMedia.size())
	{
		if (nIndexOld < nIndexNew)
		{
			pTemp = m_vMedia[nIndexOld];
			for (int i = nIndexOld; i < nIndexNew; i++)
			{
				m_vMedia[i] = m_vMedia[i + 1];
			}
			m_vMedia[nIndexNew] = pTemp;
		}
		else if (nIndexOld > nIndexNew)
		{
			pTemp = m_vMedia[nIndexOld];
			for (int i = nIndexOld; i > nIndexNew; i--)
			{
				m_vMedia[i] = m_vMedia[i - 1];
			}
			m_vMedia[nIndexNew] = pTemp;
		}

		autolock.Unlock();
		m_pPlayer->NotifyPlaylistChanged(this, IMPEvent::PCA_MOVE, nIndexNew, nIndexOld);

		return ERR_OK;
	}
	else
		return ERR_NOT_FOUND;
}

MLRESULT CPlaylist::RemoveItem(long nIndex)
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	ASSERT(nIndex >= 0 && nIndex < (int)m_vMedia.size());

	if (nIndex >= 0 && nIndex < (int)m_vMedia.size())
	{
		CMedia		*pTemp;
		pTemp = m_vMedia[nIndex];
		m_vMedia.erase(m_vMedia.begin() + nIndex);
		pTemp->Release();

		autolock.Unlock();
		m_pPlayer->NotifyPlaylistChanged(this, IMPEvent::PCA_REMOVE, nIndex, 0);
		return ERR_OK;
	}
	else
		return ERR_NOT_FOUND;
}

MLRESULT CPlaylist::Clear()
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	if (m_vMedia.size() == 0)
		return ERR_OK;

	for (uint32 i = 0; i < m_vMedia.size(); i++)
	{
		CMedia		*pTemp;
		pTemp = m_vMedia[i];
		pTemp->Release();
	}

	m_vMedia.clear();

	autolock.Unlock();
	m_pPlayer->NotifyPlaylistChanged(this, IMPEvent::PCA_CLEAR, 0, 0);

	return ERR_OK;
}

MLRESULT CPlaylist::GetItemIndex(IMedia *pMedia, long &nIndex)
{
	CMutexAutolock	autolock(m_mutexDataAccess);

	nIndex = -1;

	if (m_vMedia.size() == 0)
		return ERR_NOT_FOUND;

	for (uint32 i = 0; i < m_vMedia.size(); i++)
	{
		CMedia		*pTemp;
		pTemp = m_vMedia[i];
		if (pTemp == pMedia)
		{
			nIndex = i;
			return ERR_OK;
		}
	}

	return ERR_NOT_FOUND;
}
