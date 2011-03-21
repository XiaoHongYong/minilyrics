// AutoProcessEmbeddedLyrics.cpp: implementation of the CAutoProcessEmbeddedLyrics class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricsTool.h"
#include "AutoProcessEmbeddedLyrics.h"

CAutoProcessEmbeddedLyrics		g_autoProcessEmbeddedLyrics;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAutoProcessEmbeddedLyrics::CAutoProcessEmbeddedLyrics()
{

}

CAutoProcessEmbeddedLyrics::~CAutoProcessEmbeddedLyrics()
{

}

int CAutoProcessEmbeddedLyrics::SaveEmbeddedLyrics(LPCTSTR szSongFile, LPCTSTR szLyricsFile, CCharBuffer *pBufLyrics, UINT uEmbeddedLyrType, bool bAddToQueueOnFailed)
{
	ASSERT(szLyricsFile || pBufLyrics);

	CMutexAutolock		autolock(m_mutex);

	CCharBuffer		bufLyrFile;

	// Open lyrics file to bufLyrFile
	if (szLyricsFile && !IsEmptyString(szLyricsFile))
	{
		LRC_SOURCE_TYPE	lst = GetLrcSourceType(szLyricsFile);
		if (lst == LST_LYRICS3V2 || lst == LST_ID3V2_SYLT || lst == LST_ID3V2_LYRICS)
		{
			uEmbeddedLyrType &= ~lst; // Don't save itself.

			CMLData		lyrics;
			int nRet = lyrics.OpenLyrics(szSongFile, 0, szLyricsFile);
			if (nRet != ERR_OK)
				return nRet;

			tstring str;
			lyrics.ToString(str, FT_LYRICS_LRC, true);
			TStringToProperFileData(str.c_str(), bufLyrFile);
		}
		else if (lst == LST_ID3V2_USLT)
		{
			// For ID3v2 unsync lyrics, It can't be converted to other(sync) embedded lyrics.
			DBGLOG2(_T("ID3v2 unsync lyrics, It can't be converted to other(sync) embedded lyrics: %s, %s"), szSongFile, szLyricsFile);
			return ERR_OK;
		}
		else if (lst == LST_FILE)
		{
			if (!OpenFileToBuffer(szLyricsFile, bufLyrFile))
				return ERR_OPEN_FILE;
		}
	}

	if (bufLyrFile.empty() && (pBufLyrics == NULL || pBufLyrics->empty()))
		return ERR_NOLYRICS_OPENED;

	LIST_ITEMS::iterator	it, itEnd;
	int		nRet;
	itEnd = m_listJobs.end();
	for (it = m_listJobs.begin(); it != itEnd; ++it)
	{
		Item		&item = *it;
		if (_tcscmp(szSongFile, item.m_strSongFile.c_str()) == 0)
		{
			item.m_bRemove = false;
			if (bufLyrFile.size())
				item.m_strLyrics = bufLyrFile;
			else
				item.m_strLyrics = *pBufLyrics;
			item.m_uEmbeddedLyrType	= uEmbeddedLyrType;
			item.m_nTryFailed = 0;
			nRet = DealEmbeddedLyrics(item);
			if (nRet == ERR_OK)
				m_listJobs.erase(it);
			return ERR_OK;
		}
	}

	Item	newItem;
	newItem.m_strSongFile = szSongFile;
	newItem.m_bRemove = false;
	if (bufLyrFile.size())
		newItem.m_strLyrics = bufLyrFile;
	else
		newItem.m_strLyrics = *pBufLyrics;
	newItem.m_uEmbeddedLyrType = uEmbeddedLyrType;
	newItem.m_nTryFailed = 0;
	nRet = DealEmbeddedLyrics(newItem);
	if (nRet != ERR_OK && bAddToQueueOnFailed)
		m_listJobs.push_back(newItem);

	return nRet;
}

int CAutoProcessEmbeddedLyrics::RemoveEmbeddedLyrics(LPCTSTR szSongFile, vector<string> &vSyncLang, vector<string> &vUnsyncLang, UINT uEmbeddedLyrType, bool bAddToQueueOnFailed)
{
	CMutexAutolock		autolock(m_mutex);

	LIST_ITEMS::iterator	it, itEnd;
	int		nRet;
	itEnd = m_listJobs.end();
	for (it = m_listJobs.begin(); it != itEnd; ++it)
	{
		Item		&item = *it;
		if (_tcscmp(szSongFile, item.m_strSongFile.c_str()) == 0)
		{
			item.m_bRemove = true;
			item.m_vSyncLang = vSyncLang;
			item.m_vUnsyncLang = vUnsyncLang;
			item.m_uEmbeddedLyrType	= uEmbeddedLyrType;
			item.m_nTryFailed = 0;
			nRet = DealEmbeddedLyrics(item);
			if (nRet == ERR_OK)
				m_listJobs.erase(it);
			return ERR_OK;
		}
	}

	Item	newItem;
	newItem.m_strSongFile = szSongFile;
	newItem.m_bRemove = true;
	newItem.m_uEmbeddedLyrType = uEmbeddedLyrType;
	newItem.m_nTryFailed = 0;
	newItem.m_vSyncLang = vSyncLang;
	newItem.m_vUnsyncLang = vUnsyncLang;
	nRet = DealEmbeddedLyrics(newItem);
	if (nRet != ERR_OK && bAddToQueueOnFailed)
		m_listJobs.push_back(newItem);

	return nRet;
}

void CAutoProcessEmbeddedLyrics::RemoveJob(LPCTSTR szSongFile)
{
	CMutexAutolock		autolock(m_mutex);

	LIST_ITEMS::iterator	it, itEnd;
	itEnd = m_listJobs.end();
	for (it = m_listJobs.begin(); it != itEnd; ++it)
	{
		Item		&item = *it;
		if (_tcscmp(szSongFile, item.m_strSongFile.c_str()) == 0)
		{
			m_listJobs.erase(it);
			return;
		}
	}
}

bool CAutoProcessEmbeddedLyrics::OnQuit()
{
	OnSongChanged();

	if (m_listJobs.empty())
		return true;

	// Show lyrics tool window to let user save.
	RunLyricsTool(NULL);
	return false;
}

//
// Save embedded lyrics of previous song file.
//
void CAutoProcessEmbeddedLyrics::OnSongChanged()
{
	CMutexAutolock		autolock(m_mutex);

	LIST_ITEMS::iterator	it;
	tstring					strSongFileCur;

	strSongFileCur = g_Player.GetSrcMedia();

	for (it = m_listJobs.begin(); it != m_listJobs.end(); )
	{
		Item		&item = *it;

		const int MAX_RETRY_COUNT = 3;
		if (item.m_nTryFailed <= MAX_RETRY_COUNT
			&& _tcscmp(strSongFileCur.c_str(), item.m_strSongFile.c_str()) != 0)
		{
			if (DealEmbeddedLyrics(item) == ERR_OK)
			{
				// remove succeeded job
				it = m_listJobs.erase(it);
				continue;
			}
			else
				item.m_nTryFailed++;
		}
		++it;
	}
}

int CAutoProcessEmbeddedLyrics::DealEmbeddedLyrics(Item &item)
{
	ASSERT(FileIsExtSame(item.m_strSongFile.c_str(), _T(".mp3")));

	SetFileNoReadOnly(item.m_strSongFile.c_str());

	int				nRet;
	if (item.m_bRemove)
		nRet = DealRemoveEmbeddedLyrics(item);
	else
		nRet = DealSaveEmbeddedLyrics(item);

	if (nRet == ERR_OK)
		item.m_bDealOK = true;

	return nRet;
}

int CAutoProcessEmbeddedLyrics::DealSaveEmbeddedLyrics(Item &item)
{
	int				nRet;
	bool			Id3v2Uslt, Id3v2Sylt, bLyrics3v2, bId3v2LyricsUserDefined;

	Id3v2Sylt = IsFlagSet(item.m_uEmbeddedLyrType, LST_ID3V2_SYLT);
	Id3v2Uslt = IsFlagSet(item.m_uEmbeddedLyrType, LST_ID3V2_USLT);
	bLyrics3v2 = IsFlagSet(item.m_uEmbeddedLyrType, LST_LYRICS3V2);
	bId3v2LyricsUserDefined = IsFlagSet(item.m_uEmbeddedLyrType, LST_ID3V2_LYRICS);

	// save embedded lyrics
	if (Id3v2Sylt || Id3v2Uslt || bId3v2LyricsUserDefined)
	{
		// save lyrics as id3v2 sylt and uslt
		CMLData		lyricsData;

		nRet = lyricsData.OpenLyrics(item.m_strSongFile.c_str(), 0, (LPBYTE)item.m_strLyrics.c_str(), item.m_strLyrics.size());
		if (nRet == ERR_OK)
		{
			if (lyricsData.GetLyrContentType() != LCT_LRC)
				Id3v2Sylt = false;

			nRet = lyricsData.SaveLyricsInSongOfID3v2(
				Id3v2Uslt,
				Id3v2Sylt,
				bId3v2LyricsUserDefined);
			if (nRet != ERR_OK)
				return nRet;
		}
	}

	if (bLyrics3v2)
	{
		// save lyrics as id3v2 uslt
		CLyrics3v2		lyrics3v2;

		ASSERT(item.m_strLyrics.size());
		nRet = lyrics3v2.WriteLyrcsInfo(item.m_strSongFile.c_str(), 
			item.m_strLyrics.c_str(), item.m_strLyrics.size(), TRUE);
		if (nRet != ERR_OK)
			return nRet;
	}

	if (Id3v2Sylt || bLyrics3v2)
		g_LyricSearch.CancelAssociate(item.m_strSongFile.c_str());

	return ERR_OK;
}

int CAutoProcessEmbeddedLyrics::DealRemoveEmbeddedLyrics(Item &item)
{
	ASSERT(item.m_bRemove);
	int				nRet;
	bool			Id3v2Uslt, Id3v2Sylt, bLyrics3v2, bId3v2LyricsUserDefined;

	Id3v2Sylt = IsFlagSet(item.m_uEmbeddedLyrType, LST_ID3V2_SYLT);
	Id3v2Uslt = IsFlagSet(item.m_uEmbeddedLyrType, LST_ID3V2_USLT);
	bLyrics3v2 = IsFlagSet(item.m_uEmbeddedLyrType, LST_LYRICS3V2);
	bId3v2LyricsUserDefined = IsFlagSet(item.m_uEmbeddedLyrType, LST_ID3V2_LYRICS);

	if (bLyrics3v2)
	{
		CLyrics3v2	lyrics3v2;

		nRet = lyrics3v2.ClearLyricsInfo(item.m_strSongFile.c_str());
		if (nRet != ERR_OK && nRet != ERR_NOT_FIND_LRC3V2)
			return nRet;
	}

	// remove embedded lyrics
	if (!(Id3v2Sylt || Id3v2Uslt || bId3v2LyricsUserDefined))
		return ERR_OK;

	CID3v2IF		id3v2(ED_SYSDEF);
	int				i;
	nRet = id3v2.Open(item.m_strSongFile.c_str(), true, false);
	if (nRet != ERR_OK)
		return nRet;

	if (Id3v2Uslt)
	{
		if (item.m_vUnsyncLang.empty())
			nRet = id3v2.RemoveUnsynchLyrics(NULL);
		else
		{
			for (i = 0; i < (int)item.m_vUnsyncLang.size(); i++)
			{
				nRet = id3v2.RemoveUnsynchLyrics(item.m_vUnsyncLang[i].c_str());
				if (nRet != ERR_OK)
					return nRet;
			}
		}
	}

	if (Id3v2Sylt)
	{
		if (item.m_vSyncLang.empty())
			nRet = id3v2.RemoveSynchLyrics(NULL);
		else
		{
			for (i = 0; i < (int)item.m_vSyncLang.size(); i++)
			{
				nRet = id3v2.RemoveSynchLyrics(item.m_vSyncLang[i].c_str());
				if (nRet != ERR_OK)
					return nRet;
			}
		}
	}

	if (bId3v2LyricsUserDefined)
		nRet = id3v2.RemoveUserDefinedTextFrameByDesc(SZ_ID3V2_LYR_USER_DEFINED);

	nRet = id3v2.Save();
	if (nRet != ERR_OK)
		return nRet;
	id3v2.Close();

	return ERR_OK;
}
