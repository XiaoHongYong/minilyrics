// LyricsSearch.h: interface for the CLyricsSearch class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../MLProtocol/MLUtils.h"

DECLAR_CPPUNIT_TEST_REG(LyricsSearch)

#ifndef _IPHONE
#define _ID3V2_SUPPORT
#endif

#ifdef _ID3V2_SUPPORT
#include "Lyrics3v2.h"
#include "../TagLib/TagLib.h"
#endif // #ifdef _ID3V2_SUPPORT

#define MATCH_VALUE_MIN				30
#define MATCH_VALUE_OK				60
#define MATCH_VALUE_BETTER			70
#define MATCH_VALUE_MAX				100
#define MATCH_VALUE_EMBEDDED_LRC	95

class CLyricsSearchParameter 
{
public:
	LPCTSTR szSongFile, szArtist, szTitle;

	tstringex strArtistFiltered, strTitleFiltered;
	tstring strSongTitle;
	tstring strSongDir;

	bool bOnlySearchBestMatch;
	int nMatchValueOfBest;
	tstring strBestMatchLyrics;

	// Used for cache memory alloc
	tstringex strLyrArFiltered, strLyrTiFiltered;

public:
	CLyricsSearchParameter(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, bool bOnlySearchBestMatch = false);

	int CalMatchValueByName(LPCTSTR szLyricsDir, LPCTSTR szLyricsFileName, MLFileType fileType);

	int CalMatchValueByTitle(LPCTSTR szLyrArtist, LPCTSTR szLyrTitle, MLFileType fileType);

	bool IsBestMatchLyricsFound();

};

struct LrcSearchResult
{
	tstring	strUrl;
	tstring	strSaveFileName;
	tstring	strArtist;
	tstring	strTitle;
	tstring	strAlbum;
	tstring	strUploader;
	float	nMatchValue;
	float	fRate;					// rate info of lyrics
	int		nRateCount;				// 
	int		nDownloads;				// total downloads of lyrics

	LrcSearchResult()
	{
		nMatchValue = 0;
		fRate = 0.0;
		nRateCount = 0;
		nDownloads = 0;
	}
};
bool operator<=(const LrcSearchResult &l1, const LrcSearchResult &l2);
bool operator<(const LrcSearchResult &l1, const LrcSearchResult &l2);

class V_LRCSEARCHRESULT : public list<LrcSearchResult>
{
public:
	BOOL AddResult(const LrcSearchResult &Result);
	iterator GetTheBestMatchLyrics(BOOL bCheckExist = FALSE);

	bool IsAllTxtLyrics() const;

	void Sort() { sort(); }

	void UpdateLyricsName();

	LrcSearchResult *SearchByUrl(LPCTSTR szUrl);

	LrcSearchResult *GetByOrder(int n);
	void DeleteByOrder(int n);

};
