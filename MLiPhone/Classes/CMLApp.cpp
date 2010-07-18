#import "../../base/mlstdafx.h"
#import "../../base/base.h"
#import "../../Utils/Utils.h"
#import "../../OurMFCLib/OurMFClib.h"
#import "../../gfx/gfx.h"
#import "../../MLLib/MLLib.h"
#import "CMLApp.h"
#import "LyricsShowGraphBase.h"

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
	LPTSTR  vArgv[2] = { 0 };
	tstring strAppDirDir = GetAppDataDir() + _T("MiniLyrics.app");
	vArgv[0] = (LPTSTR)strAppDirDir.c_str();

	InitBaseFrameWork(1, vArgv, NULL, _T("MiniLyrics.ini"), _T("MiniLyrics"));
	
	g_Profile.DoCache();
	
	SetDefaultSettings();

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

#define MAKE_SURE_STR_NOT_NULL(str)		if (str == NULL) str = _T("")

void CMLApp::OnMediaChanged(LPCTSTR szMediaKey, LPCTSTR szArtist, LPCTSTR szAlbum, LPCTSTR szTitle, LPCTSTR szEmbeddedLyr, int nMediaLength)
{
	MAKE_SURE_STR_NOT_NULL(szMediaKey);
	MAKE_SURE_STR_NOT_NULL(szArtist);
	MAKE_SURE_STR_NOT_NULL(szAlbum);
	MAKE_SURE_STR_NOT_NULL(szTitle);
	MAKE_SURE_STR_NOT_NULL(szEmbeddedLyr);

	m_strCurMediaKey = szMediaKey;
	m_strArtist = szArtist;
	m_strAlbum = szAlbum;
	m_strTitle = szTitle;
	m_nMediaLength = nMediaLength;

	m_lyrData.Close();

/*	tstring strTestLyr = GetResourceFile(_T("lyrics.lrc"));
	int nRet = m_lyrData.OpenLyrics(szMediaKey, nMediaLength, strTestLyr.c_str());

	PostNotifyLyricsChanged();
	
	return;*/

	if (m_strCurMediaKey.empty() && m_strArtist.empty() && m_strTitle.empty())
	{
		PostNotifyLyricsChanged();
		return;
	}

	if (m_localLyrSearch.IsAssociatedWithNoneLyrics(m_strCurMediaKey.c_str()))
	{
		PostNotifyLyricsChanged();
		return;
	}

	TCHAR	szLyricFile[MAX_PATH];
	if (m_localLyrSearch.GetBestMatchLyrics(szMediaKey, szArtist, szTitle, szLyricFile))
	{
		// Open lyrics and return.
		int nRet = m_lyrData.OpenLyrics(szMediaKey, m_nMediaLength, szLyricFile);
		if (nRet == ERR_OK)
		{
			PostNotifyLyricsChanged();
			return;
		}
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

	PostNotifyLyricsChanged();
}

void CMLApp::OnLyricsSearchEnd(int nResult, bool &bPopupSearchWnd)
{
	if (nResult == ERR_OK)
	{
		bPopupSearchWnd = false;
	
		// Lyrics are opened already?
		if (m_localLyrSearch.IsAssociatedLyrics(m_strCurMediaKey.c_str()))
			return;

		// Got lyrics search result, so search it.
		SearchInCacheResult(true);
	}
	else
	{
		tstring		str;
		str = TOLOCAL(_T("Failed to search lyrics with error:"));
		str += _T(" ");
		str += Error2Str(nResult);
		PostNotifyMessage(str.c_str());
	}
}

bool CMLApp::SearchCacheForCurrentMedia(V_LRCSEARCHRESULT &vLrcSearchResult)
{
	return m_onlineLyrSearch.SearchCache(m_strCurMediaKey.c_str(), m_strArtist.c_str(), m_strTitle.c_str(), &vLrcSearchResult);
}

void CMLApp::DownloadLyrics(LPCTSTR szMediaKey, LPCTSTR szUrl, LPCTSTR szFileName)
{
	tstring		str;
	str = TOLOCAL(_T("Downloading lyrics:"));
	str += _T(" ");
	str += szFileName;
	PostNotifyMessage(str.c_str());
	
	m_lyrDownloadMgr.DownloadLyrics(szMediaKey, szUrl, szFileName);
}

void CMLApp::OnLyricsDownloaded(LPCTSTR szMediaKey, LPCTSTR szFile)
{
	// Associate lyrics
	m_localLyrSearch.AssociateLyrics(szMediaKey, szFile);

	// Open Lyrics
	if (_tcscmp(szMediaKey, m_strCurMediaKey.c_str()) == 0)
	{
		m_lyrData.OpenLyrics(szMediaKey, m_nMediaLength, szFile);
		PostNotifyLyricsChanged();
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

			// Download it
			DownloadLyrics(m_strCurMediaKey.c_str(), result.strUrl.c_str(), result.strSaveFileName.c_str());
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

void CMLApp::SetDefaultSettings()
{
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("Font"), _T(", 18, bold, 0, 0, "));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("FgColor"), _T("#FFFFFF"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("FgLowColor"), _T("#929292"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("BgColor"), _T("#404040"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("FocusLineBgColor"), _T("#8A94AA"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("TagColor"), _T("#FF8242"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("HilightBorderColor"), _T("#000000"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("LowlightBorderColor"), _T("#000000"));
	
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("HilightOBM"), OBM_COLOR);
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("HilightGradient1"), _T("#FFFF80"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("HilightGradient2"), _T("#FF2D2D"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("HilightGradient3"), _T("#AE0000"));
	
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("FgColor"), _T("#FF0000"));
	
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("LowlightOBM"), OBM_COLOR);
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("LowlightGradient1"), _T("#0000A0"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("LowlightGradient2"), _T("#0080FF"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("LowlightGradient3"), _T("#0000FF"));
	
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("FgLowColor"), _T("#0000FF"));
}
