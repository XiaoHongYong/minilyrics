#include "../../MLLib/StdAfx.h"
//#include "../../MLLib/MLLib.h"
#include "../../MLLib/LyricsSearch.h"
#include "LyricsLocalSearch.h"

CLyricsLocalSearch::CLyricsLocalSearch(void)
{
}

CLyricsLocalSearch::~CLyricsLocalSearch(void)
{
}

void CLyricsLocalSearch::SearchAllMatchLyrics(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT &vLyrics)
{
	CLyricsSearchParameter searchParam(szSongFile, szArtist, szTitle);
	SearchLyrics(searchParam, vLyrics);
}

// Search lyrics by artist and title.
void CLyricsLocalSearch::SearchLyrics(CLyricsSearchParameter &searchParam, V_LRCSEARCHRESULT &vLyrics)
{
	SearchEmbeddedLyrics(searchParam.szSongFile, vLyrics, searchParam.bOnlySearchBestMatch);
	if (searchParam.bOnlySearchBestMatch)
	{
		V_LRCSEARCHRESULT::iterator	it = vLyrics.GetTheBestMatchLyrics();
		if (it != vLyrics.end())
		{
			LrcSearchResult		&result = *it;
			if (result.nMatchValue >= MATCH_VALUE_EMBEDDED_LRC)
			{
				searchParam.strBestMatchLyrics = result.strUrl.c_str();
				searchParam.nMatchValueOfBest = (int)result.nMatchValue;
				return;
			}
		}
	}

	tstring	strDir;

	// Search lyrics in the same folder of song file.
	{
		if (!IsEmptyString(searchParam.szSongFile))
		{
			strDir = FileGetPath(searchParam.szSongFile);
			SearchMatchLyricsInDir(strDir.c_str(), searchParam, vLyrics, false);
			if (searchParam.IsBestMatchLyricsFound())
				return;
		}
	}

	// Search in lyrics download folder
	{
		strDir = m_lyricsDirOfDownload;
		DirStringAddSlash(strDir);

		SearchMatchLyricsInDir(strDir.c_str(), searchParam, vLyrics, true);
		if (searchParam.IsBestMatchLyricsFound())
			return;
	}

	// Search in the folder that user specified.
	strDir = m_lyricsDirOfUser;
	DirStringAddSlash(strDir);

	SearchMatchLyricsInDir(strDir.c_str(), searchParam, vLyrics, true);
	if (searchParam.IsBestMatchLyricsFound())
		return;

	// Finally use the best match lyrics in vLyrics.
	V_LRCSEARCHRESULT::iterator	it = vLyrics.GetTheBestMatchLyrics();
	if (it != vLyrics.end())
	{
		LrcSearchResult		&result = *it;
		if (result.nMatchValue >= MATCH_VALUE_OK)
		{
			searchParam.strBestMatchLyrics = result.strUrl.c_str();
			searchParam.nMatchValueOfBest = (int)result.nMatchValue;
			return;
		}
	}
}

// Return the best match lyrics only
bool CLyricsLocalSearch::GetBestMatchLyrics(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, tstring &strLyrFile)
{
	V_LRCSEARCHRESULT vLyrics;
	CLyricsSearchParameter searchParam(szSongFile, szArtist, szTitle, true);
	SearchLyrics(searchParam, vLyrics);
	strLyrFile = searchParam.strBestMatchLyrics;
	return strLyrFile.size() > 0;
}
