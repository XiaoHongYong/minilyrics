// LyricsSearch.h: interface for the CLyricsSearch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LYRICSSEARCH_H__B9364267_AFE5_41D9_BDCA_49CAE54D4ECC__INCLUDED_)
#define AFX_LYRICSSEARCH_H__B9364267_AFE5_41D9_BDCA_49CAE54D4ECC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../MLProtocol/MLUtils.h"

#include "Lyrics3v2.h"
#include "../ID3Lib/ID3v2.h"

// 歌词文件匹配类型的定义
enum LRC_MATCH_TYPE
{
	LYRIC_MATCH_NONE		= 0,
	LYRIC_MATCH_PART_DEC	= 1,
	LYRIC_MATCH_PART		= 2,
	LYRIC_MATCH_ALL_DEC		= 3,
	LYRIC_MATCH_ALL			= 4,
	LYRIC_MATCH_MAX_TYPE	= LYRIC_MATCH_ALL,
};

#define MIN_EMBEDDED_LYR_SIZE	10		// Minum file size of embedded lyrics

#define MAX_FILE_NAME		MAX_PATH

//
#define MATCH_VALUE_MAX			(100)
#define MATCH_VALUE_EMBEDDED_LRC	95
#define MATCH_VALUE_WITH_LRC	90
#define MATCH_VALUE_MIN			10
#define MATCH_VALUE_BETTER		(65)
#define MATCH_VALUE_OK			(60)
#define MATCH_VALUE_MIN_DL		(33)

void FilterChars(LPTSTR szBuffer, int nBuffMax, LPCTSTR szString, BOOL bFilterNumb);

void FilterSongFileExt(LPTSTR szFile);

int CalLyricsMatchValue(MLFileType fileType, LPCTSTR szTitleTarg, LPCTSTR szArtistTarg, LPCTSTR szTitle, BOOL bTitleIgnoreNumb, LPCTSTR szArtist, BOOL bArtIgnoreNumb);
int CalLyricsMatchValue(LPCTSTR szFile, LPCTSTR szTitle, BOOL bTitleIgnoreNumb, LPCTSTR szArtist, BOOL bArtIgnoreNumb);


struct LrcSearchResult
{
//	TCHAR	szLyricsName[128];			// 歌词名
//	TCHAR	szURL[256];
//	TCHAR	szSaveFileName[MAX_PATH];
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
//		EmptyStr(szLyricsName);
//		EmptyStr(szURL);
//		EmptyStr(szSaveFileName);
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

#define SZ_SONG_ID3V2_SYLT		_T("song://id3v2/sylt/")
#define SZ_SONG_ID3V2_USLT		_T("song://id3v2/uslt/")
#define SZ_SONG_LYRICS3V2		_T("song://lyrics3v2")
#define SZ_SONG_ID3V2_LYRICS	_T("song://id3v2/lyrics(nonstandard)")

#define SZ_SONG_ID3V2_SYLT_DESC	_T("ID3v2 synchronized Lyrics (Support Windows Media player)")
#define SZ_SONG_ID3V2_USLT_DESC	_T("ID3v2 unsynchronized Lyrics (Support iTunes, WMP)")
#define SZ_SONG_LYRICS3V2_DESC	_T("Lyrics3v2 lyrics (Few players support this format)")
#define SZ_SONG_ID3V2_LYRICS_DESC	_T("Embedded ID3v2 lyrics (Nonstandard)")

void GetID3v2LrcSourceLang(LPCTSTR szLrcSource, char szLang[], int len);

void ListSupportedEmbeddedFormat(LPCTSTR szSongFile, V_LRCSEARCHRESULT &vLyricsFormat);

bool ID3v2LyricsInfoGetSynchLyrics(ID3v2LyricsInfo &lyricsInfo, LPCTSTR szLrcSource, ID3v2SynchLyrics *&plyrics);
bool ID3v2LyricsInfoGetUnsynchLyrics(ID3v2LyricsInfo &lyricsInfo, LPCTSTR szLrcSource, ID3v2UnsynchLyrics *&plyrics);

void ID3v2LyricsInfoToResult(ID3v2LyricsInfo &lyricsInfo, V_LRCSEARCHRESULT &vLyrics, bool bOnlyListAvailable = false);

void StrIgnoreCompare(LPCTSTR &szDest, LPCTSTR &szTemplate, BOOL bIgnoreNumb);
int StrIgnoreCompare(LPCTSTR szStr1, LPCTSTR szStr2);

LPCTSTR IgnoreChars(LPCTSTR szString, BOOL bIgnoreNumb);

BOOL IsIgnoreNumber(LPCTSTR szString);


#endif // !defined(AFX_LYRICSSEARCH_H__B9364267_AFE5_41D9_BDCA_49CAE54D4ECC__INCLUDED_)
