// AutoProcessEmbeddedLyrics.cpp: implementation of the CAutoProcessEmbeddedLyrics class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
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

int CAutoProcessEmbeddedLyrics::SaveEmbeddedLyrics(LPCTSTR szSongFile, LPCTSTR szLyricsFile, LPCTSTR szLyrics, UINT uEmbeddedLyrType, bool bAddToQueueOnFailed)
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
			item.m_bRemove = false;
			item.m_strLyricsFile = szLyricsFile;
			item.m_strLyrics = szLyrics;
			item.m_uEmbeddedLyrType	= uEmbeddedLyrType;
			item.m_bTryFailed = false;
			nRet = DealEmbeddedLyrics(item);
			if (nRet == ERR_OK)
				m_listJobs.erase(it);
			return ERR_OK;
		}
	}

	Item	newItem;
	newItem.m_strSongFile = szSongFile;
	newItem.m_bRemove = false;
	newItem.m_strLyricsFile = szLyricsFile;
	newItem.m_strLyrics = szLyrics;
	newItem.m_uEmbeddedLyrType = uEmbeddedLyrType;
	newItem.m_bTryFailed = false;
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
			item.m_bTryFailed = false;
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
	newItem.m_bTryFailed = false;
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

void CAutoProcessEmbeddedLyrics::OnQuit()
{
	OnSongChanged();

/*#ifndef _WIN32_WCE
	if (!m_listJobs.empty())
	{
		// Show a dialog to let users choose.
		// g_mpApp.GetMainWnd()->MessageOut(_T("CAutoProcessEmbeddedLyrics quit!"));
		CDlgBatchProcessEmbeddedLyrics	dlg;

		dlg.DoModal(g_mpApp.GetMainWnd());
	}
#endif*/

	m_listJobs.clear();
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

	for (it = m_listJobs.begin(); it != m_listJobs.end(); ++it)
	{
		Item		&item = *it;
		if (!item.m_bTryFailed && _tcscmp(strSongFileCur.c_str(), item.m_strSongFile.c_str()) != 0)
		{
			if (DealEmbeddedLyrics(item) == ERR_OK)
			{
				// remove successfull job
				m_listJobs.erase(it);

				// From start
				if (m_listJobs.empty())
					return;
				it = m_listJobs.begin();
			}
			else
			{
				item.m_bTryFailed = true;
			}
		}
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

		nRet = ERR_FAILED;
		if (!item.m_strLyricsFile.empty())
		{
			nRet = lyricsData.OpenLyrics(item.m_strSongFile.c_str(), 0, 
				item.m_strLyricsFile.c_str(), false);
		}
		else
		{
			// save lyrics as temp lyrics file, then open it
			TCHAR		szFile[MAX_PATH];

			GetAppDataDir(szFile);
			_tcscat_safe(szFile, CountOf(szFile), _T("TempSaveEmbeddedLyrics.lrc"));
			if (SaveDataAsFile(szFile, item.m_strLyrics.c_str(), item.m_strLyrics.size() * sizeof(TCHAR)))
			{
				CHAR_ENCODING	nEncodingID;
#ifdef UNICODE
				nEncodingID = ED_UNICODE;
#else
				nEncodingID = ED_SYSDEF;
#endif
				nRet = lyricsData.OpenLyrics(item.m_strSongFile.c_str(), 0, 
					szFile, TRUE, nEncodingID);

				DeleteFile(szFile);
			}
		}
		if (nRet == ERR_OK)
		{
			if (lyricsData.GetLyrContentType() != LCT_LRC)
				Id3v2Sylt = false;

			nRet = lyricsData.SaveLyricsInSongOfID3v2(
				Id3v2Uslt,
				Id3v2Sylt,
				bId3v2LyricsUserDefined,
				lyricsData.Properties().GetEncodingIndex());
			if (nRet != ERR_OK)
				return nRet;
		}
	}

	if (bLyrics3v2)
	{
		// save lyrics as id3v2 uslt
		CLyrics3v2		lyrics3v2;
		stringex	str;

		if (!item.m_strLyricsFile.empty())
			OpenFileToBuffer(item.m_strLyricsFile.c_str(), str);
		else
		{
#ifdef UNICODE
			str += (TCHAR)0xFF;
			str += (TCHAR)0xFE;
#endif
			str.append((const char *)item.m_strLyrics.c_str(), item.m_strLyrics.size() * sizeof(TCHAR));
		}

		if (!str.empty())
		{
			nRet = lyrics3v2.WriteLyrcsInfo(item.m_strSongFile.c_str(), str.c_str(), str.size(), TRUE);
			if (nRet != ERR_OK)
				return nRet;
		}
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
