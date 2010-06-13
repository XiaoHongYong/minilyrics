#include "../../base/mlstdafx.h"
#include "../../base/base.h"
#include "../../Utils/Utils.h"
#include "../../OurMFCLib/OurMFClib.h"
#include "../../gfx/gfx.h"
#include "../../MLLib/MLLib.h"
#include "helper.h"
#include "CMLApp.h"

CMLApp			*CMLApp::m_pAppInstance = NULL;

CMLApp::CMLApp(void)
{
}

CMLApp::~CMLApp(void)
{
}

CMLApp *CMLApp::GetInstance()
{
	if (!m_pAppInstance)
		m_pAppInstance = new CMLApp();

	return m_pAppInstance;
}

int CMLApp::Init()
{
	m_localLyrSearch.Init();
	m_onlineLyrSearch.Init();
	m_lyrDownloadMgr.Init();

	return ERR_OK;
}

void CMLApp::Quit()
{
	m_localLyrSearch.Quit();
	m_onlineLyrSearch.Quit();
	m_lyrDownloadMgr.Quit();
}

void CMLApp::OnMediaChanged(LPCTSTR szMediaKey, LPCTSTR szArtist, LPCTSTR szAlbum, LPCTSTR szTitle)
{
	m_strCurMediaKey = szMediaKey;
	m_strArtist = szArtist;
	m_strAlbum = szAlbum;
	m_strTitle = szTitle;

	m_lyrData.Close();

	if (m_strCurMediaKey.empty() && m_strArtist.empty() && m_strTitle.empty())
		return;

	if (m_localLyrSearch.IsAssociatedWithNoneLyrics(m_strCurMediaKey.c_str()))
		return;

	TCHAR	szLyricFile[MAX_PATH];
	if (m_localLyrSearch.GetBestMatchLyrics(szMediaKey, szArtist, szTitle, szLyricFile))
	{
		// Open lyrics and return.
		int nRet = m_lyrData.OpenLyrics(szMediaKey, m_nMediaLength, szLyricFile);
		if (nRet == ERR_OK)
			return;
	}

/*	if (g_LyricData.GetLyrContentType() == LCT_TXT &&
		!m_localLyrSearch.IsAssociatedLyrics(g_Player.GetSrcMedia(), g_Player.GetFullTitle()))
	{
		//
		// continue to search online .lrc, if .txt lyrics is opened
		//
		if (!g_LyricsDownloader.SearchInCacheResult(false))
			g_OnlineSearch.AutoSearch(g_Player.GetArtist(), g_Player.GetTitle());
	}*/

	// Add Media Info:
	V_Str		vMediaInfo;
	if (m_strArtist.size() > 0)
		vMediaInfo.push_back(tstring(TOLOCAL(_T("Artist:"))) + _T(" ") + m_strArtist);
	if (m_strTitle.size() > 0)
		vMediaInfo.push_back(tstring(TOLOCAL(_T("Title:"))) + _T(" ") + m_strTitle);
	if (m_strAlbum.size() > 0)
		vMediaInfo.push_back(tstring(TOLOCAL(_T("Album:"))) + _T(" ") + m_strAlbum);
	m_lyrData.AddTextInLyrics(vMediaInfo);

	if (m_strTitle.size() >= 3)
	{
		// title at least 3 char long
		if (!SearchInCacheResult(false))
		{
			// 使用在线检索，搜索歌词
			m_onlineLyrSearch.AutoSearch(szMediaKey, szArtist, szTitle);
		}
	}
}

void CMLApp::OnLyricsSearchEnd(int nResult)
{
	// Lyrics are opened already?
	if (m_localLyrSearch.IsAssociatedLyrics(m_strCurMediaKey.c_str()))
		return;

	// Got lyrics search result, so search it.
	SearchInCacheResult(true);
}

void CMLApp::OnLyricsDownloaded(LPCTSTR szMediaKey, LPCTSTR szFile)
{
	// Associate lyrics
	m_localLyrSearch.AssociateLyrics(szMediaKey, szFile);

	// Open Lyrics
	if (_tcscmp(szMediaKey, m_strCurMediaKey.c_str()) == 0)
	{
		m_lyrData.OpenLyrics(szMediaKey, m_nMediaLength, szFile);
	}
}

BOOL CMLApp::SearchInCacheResult(bool bShowInfoText)
{
	V_LRCSEARCHRESULT		vLrcSearchResult;

	if (!m_onlineLyrSearch.SearchCache(m_strCurMediaKey.c_str(), m_strArtist.c_str(), m_strTitle.c_str(), &vLrcSearchResult))
	{
		static	int nFailedCount = 0;
		nFailedCount++;
		if (bShowInfoText && nFailedCount > 2)
		{
			tstring		str;
			str = TOLOCAL(_T("Failed to get searching lyrics results."));
			str += _T(" ");
			str += TOLOCAL(_T("Please contact us to report this issue."));
			PostNotifyMessage(str.c_str());
		}
		return FALSE;
	}

	if (vLrcSearchResult.size() == 0)
	{
		if (bShowInfoText)
		{
			tstring		str;
			str = TOLOCAL(_T("Search returned no results."));
			PostNotifyMessage(str.c_str());
		}
		return FALSE;
	}

	float	nValueMax = 0;
	V_LRCSEARCHRESULT::iterator itBest;

	//
	// 分值超过 MATCH_VALUE_OK
	itBest = vLrcSearchResult.GetTheBestMatchLyrics(FALSE);
	if (itBest != vLrcSearchResult.end())
	{
		LrcSearchResult		&result = *itBest;
		nValueMax = result.nMatchValue;
		if (nValueMax >= MATCH_VALUE_OK)
		{
			// Do not download .txt lyrics
			if (FileIsExtSame(result.strUrl.c_str(), _T(".txt"))
				&& g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("OnlyDlSyncLyr"), false))
			{
				PostNotifyMessage(TOLOCAL(_T("None of the lyrics are .lrc extension lyrics, so they are not downloaded.")));
				return TRUE;
			}

			// If current lyrics is text lyrics, and the match lyrics is also text
			// lyrics, do NOT download it.
			if (m_lyrData.GetLyrContentType() != LCT_UNKNOWN
				&& result.nMatchValue < MATCH_VALUE_BETTER)
				return TRUE;

			// 就下载它了
			tstring		str;
			str = TOLOCAL(_T("Downloading lyrics:"));
			str += _T(" ");
			str += result.strSaveFileName.c_str();
			PostNotifyMessage(str.c_str());

			m_lyrDownloadMgr.DownloadLyrics(m_strCurMediaKey.c_str(), result.strUrl.c_str(), result.strSaveFileName.c_str());
			return TRUE;
		}
	}

	// If any lyrics is opened, do not continue to download.
	if (m_lyrData.HasLyricsOpened())
		return true;

	if (bShowInfoText)
	{
		PostNotifyMessage(
			TOLOCAL(_T("Found some similar lyrics, please click in 'Search Lyrics' button.")));
	}

	return TRUE;
}
