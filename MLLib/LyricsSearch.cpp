// LyricsSearch.cpp: implementation of the CLyricsSearch class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "LyricsSearch.h"
#include "MLData.h"
#include "HelperFun.h"


void GetID3v2LrcSourceLang(LPCTSTR szLrcSource, char szLang[], int len)
{
	int		nLen;

	nLen = _tcslen(SZ_SONG_ID3V2_SYLT);
	if (_tcsnicmp(szLrcSource, SZ_SONG_ID3V2_SYLT, nLen) == 0)
	{
		ConvertStr2(szLrcSource + nLen, -1, szLang, len);
		return;
	}

	nLen = _tcslen(SZ_SONG_ID3V2_USLT);
	if (_tcsnicmp(szLrcSource, SZ_SONG_ID3V2_USLT, nLen) == 0)
	{
		ConvertStr2(szLrcSource + nLen, -1, szLang, len);
		return;
	}

	EmptyStr(szLang);
}

void ListSupportedEmbeddedFormat(LPCTSTR szSongFile, V_LRCSEARCHRESULT &vLyricsFormat)
{
	LrcSearchResult				result;
	V_LRCSEARCHRESULT::iterator	it;
	bool						bAddSylt = true, bAddUslt = true;

	for (it = vLyricsFormat.begin(); it != vLyricsFormat.end(); it++)
	{
		LrcSearchResult			&result = *it;
		if (_tcsnicmp(result.strUrl.c_str(), SZ_SONG_ID3V2_SYLT, _tcslen(SZ_SONG_ID3V2_SYLT)) == 0)
			bAddSylt = false;
		if (_tcsnicmp(result.strUrl.c_str(), SZ_SONG_ID3V2_USLT, _tcslen(SZ_SONG_ID3V2_USLT)) == 0)
			bAddUslt = false;
	}

	if (bAddSylt)
	{
		result.strUrl = SZ_SONG_ID3V2_SYLT;
		result.strSaveFileName = SZ_SONG_ID3V2_SYLT_DESC;
		result.nMatchValue = -1;
		vLyricsFormat.AddResult(result);
	}

	result.strUrl = SZ_SONG_LYRICS3V2;
	result.strSaveFileName = SZ_SONG_LYRICS3V2_DESC;
	result.nMatchValue = -1;
	vLyricsFormat.AddResult(result);

	if (bAddUslt)
	{
		result.strUrl = SZ_SONG_ID3V2_USLT;
		result.strSaveFileName = SZ_SONG_ID3V2_USLT_DESC;
		result.nMatchValue = -1;
		vLyricsFormat.AddResult(result);
	}
}

bool ID3v2LyricsInfoGetSynchLyrics(ID3v2LyricsInfo &lyricsInfo, LPCTSTR szLrcSource, ID3v2SynchLyrics *&plyrics)
{
	tstringex			str;
	int					nLen;

	nLen = _tcslen(SZ_SONG_ID3V2_SYLT);
	if (_tcsnicmp(szLrcSource, SZ_SONG_ID3V2_SYLT, nLen) != 0)
		return false;

	for (ID3v2LyricsInfo::LIST_SYNCHLYRICS::iterator it = lyricsInfo.m_synchLyrics.begin();
		it != lyricsInfo.m_synchLyrics.end(); it++)
	{
		plyrics = *it;

		ConvertStr(plyrics->m_szLanguage, -1, str);

		if (_tcscmp(szLrcSource + nLen, str.c_str()) == 0)
		{
			return true;
		}
	}

	plyrics = NULL;
	return false;
}

bool ID3v2LyricsInfoGetUnsynchLyrics(ID3v2LyricsInfo &lyricsInfo, LPCTSTR szLrcSource, ID3v2UnsynchLyrics *&plyrics)
{
	tstringex			str;
	int					nLen;

	nLen = _tcslen(SZ_SONG_ID3V2_USLT);
	if (_tcsnicmp(szLrcSource, SZ_SONG_ID3V2_USLT, nLen) != 0)
		return false;

	for (ID3v2LyricsInfo::LIST_UNSYNLYRICS::iterator it = lyricsInfo.m_unsynchLyrics.begin();
		it != lyricsInfo.m_unsynchLyrics.end(); it++)
	{
		plyrics = *it;

		ConvertStr(plyrics->m_szLanguage, -1, str);

		if (_tcscmp(szLrcSource + nLen, str.c_str()) == 0)
		{
			return true;
		}
	}

	plyrics = NULL;
	return false;
}

void ID3v2LyricsInfoToResult(ID3v2LyricsInfo &lyricsInfo, V_LRCSEARCHRESULT &vLyrics, bool bOnlyListAvailable)
{
	tstringex			str;
	LrcSearchResult		result;

	for (ID3v2LyricsInfo::LIST_SYNCHLYRICS::iterator it = lyricsInfo.m_synchLyrics.begin();
		it != lyricsInfo.m_synchLyrics.end(); it++)
	{
		ID3v2SynchLyrics	*plyrics = *it;

		if (bOnlyListAvailable)
		{
			if (plyrics->m_vAnsiSynLyrics.size() < 1 && plyrics->m_vUcs2SynLyrics.size() < 1)
				continue;
		}

		ConvertStr(plyrics->m_szLanguage, -1, str);

		result.strUrl = SZ_SONG_ID3V2_SYLT;
		result.strUrl += str;

		result.strSaveFileName = TOLOCAL(SZ_SONG_ID3V2_SYLT_DESC);
		result.strSaveFileName += str;
		result.nMatchValue = MATCH_VALUE_EMBEDDED_LRC;

		vLyrics.push_back(result);
	}

	{
		for (ID3v2LyricsInfo::LIST_UNSYNLYRICS::iterator it = lyricsInfo.m_unsynchLyrics.begin();
			it != lyricsInfo.m_unsynchLyrics.end(); it++)
		{
			ID3v2UnsynchLyrics	*plyrics = *it;

			if (bOnlyListAvailable)
			{
				if (plyrics->m_strAnsiLyrics.size() < MIN_EMBEDDED_LYR_SIZE && plyrics->m_strUcs2Lyrics.size() < MIN_EMBEDDED_LYR_SIZE)
					continue;
			}

			ConvertStr(plyrics->m_szLanguage, -1, str);

			result.strUrl = SZ_SONG_ID3V2_USLT;
			result.strUrl += str;

			result.strSaveFileName = TOLOCAL(SZ_SONG_ID3V2_USLT_DESC);
			result.strSaveFileName += str;
			result.nMatchValue = MATCH_VALUE_OK + 2;

			vLyrics.push_back(result);
		}
	}

	if ((lyricsInfo.m_lyricsUserDefined.m_strValueAnsi.size() || lyricsInfo.m_lyricsUserDefined.m_strValueUCS2.size())
		&& (lyricsInfo.m_lyricsUserDefined.m_strDescAnsi.size() || lyricsInfo.m_lyricsUserDefined.m_strDescUCS2.size()))
	{
		result.strUrl = SZ_SONG_ID3V2_LYRICS;
		result.strSaveFileName = TOLOCAL(SZ_SONG_ID3V2_LYRICS_DESC);
		result.nMatchValue = MATCH_VALUE_EMBEDDED_LRC - 1;
		vLyrics.push_back(result);
	}
}


LPCTSTR			SZ_LRC_FILE_LEFT_COMMENT = _T("（");
LPCTSTR			SZ_LRC_FILE_RIGHT_COMMENT = _T("）");

// 判断一个字符串是否为数字(如果为空，也返回TRUE)
BOOL IsIgnoreNumber(LPCTSTR szString)
{
	LPCTSTR	szNew = IgnoreChars(szString, TRUE);

	if (*szNew == '\0')
		return FALSE;
	else
		return TRUE;
}


// COMMENT:
//		忽略以下的字符
//			"(注释)"
//			"（注释）"
//			"[注释]"
//			英文中非数字和字母
//			中文符号：0xA3A1(！) ~ 0xA3FE(￣)
//			。: A1, A3  〓: A1, FE
//			ⅰ: A2, A1	Ⅻ: A2, FC
//			！: A3, A1	￣: A3, FE
// 用例:
//		(comment)
//		（cmt)
//		（cmt）
//		(
//		（
// 用例 '.' ';' '^' '{'
LPCTSTR IgnoreChars(LPCTSTR szString, BOOL bIgnoreNumb)
{
	LPCTSTR	szNew, szCur;

	szNew = szString;

	while (1)
	{
		szCur = szNew;
		// "(注释)" 
		if (szCur[0] == '(')
		{
			szNew = _tcschr(szCur + 1, ')');
			if (szNew == NULL)
				szNew = szCur + 1;
			else
				szNew++;
		}
		else if (szCur[0] == '[')
		{
			// "[注释]"
			szNew = _tcschr(szCur + 1, ']');
			if (szNew == NULL)
				szNew = szCur + 1;
			else
				szNew++;
		}
		else if (szCur[0] == SZ_LRC_FILE_LEFT_COMMENT[0])
		{
			// "（注释）"
			if (szCur[1] == SZ_LRC_FILE_LEFT_COMMENT[1])
			{
				szNew = _tcsstr(szCur + 2, SZ_LRC_FILE_RIGHT_COMMENT);
				if (szNew == NULL)
					szNew = szCur + 2;
				else
					szNew += 2;
			}
		}

		if (szNew[0] == '\0')
			break;

		// 过滤是ASCII的字符
		// 除 0 ~ 9   A ~ Z   a ~ z 外，都要过滤
		// 用例 '.' ';' '^' '{'
		if (__isascii(szNew[0]))
		{
			if (bIgnoreNumb)
			{
				if (szNew[0] < 'A'
				 || (szNew[0] > 'Z' && szNew[0] < 'a') || szNew[0] > 'z')
					 szNew ++;
				else
					break;
			}
			else
			{
				if (szNew[0] < '0' || (szNew[0] > '9' && szNew[0] < 'A')
				 || (szNew[0] > 'Z' && szNew[0] < 'a') || szNew[0] > 'z')
					szNew ++;
				else
					break;
			}
		}
		else
		{
			//			中文符号：0xA3A1(！) ~ 0xA3FE(￣)
			if (BYTE(szNew[0]) >= 0xA1 && BYTE(szNew[0]) <= 0xA3)
			{
				if (BYTE(szNew[1]) >= 0xA1 && BYTE(szNew[1]) <= 0xFE)
					szNew += 2;
				else
					break;
			}
			else
				break;
		}
	}

	return szNew;
}

// COMMENT:
//		将 szString 中的一些字符过滤掉，过滤的规则和 IgnoreChars 函数一致。
//
void FilterChars(LPTSTR szBuffer, int nBuffMax, LPCTSTR szString, BOOL bFilterNumb)
{
	assert(nBuffMax != 0);
	assert(szBuffer);
	assert(szString);

	int		i = 0;
	LPCTSTR	szBeg;

	szBeg = szString;
	while (szBeg[0] != '\0')
	{
		szBeg = IgnoreChars(szBeg, bFilterNumb);

		if (szBeg[0] == '\0')
			break;
		if (i >= nBuffMax)
			break;
		if (__isascii(szBeg[0]))
		{
			szBuffer[i] = szBeg[0];
			i++;
			szBeg++;
		}
		else
		{
			szBuffer[i] = szBeg[0];
			szBuffer[i + 1] = szBeg[1];
			i += 2;
			szBeg += 2;
		}
	}
	if (i >= nBuffMax)
		i = nBuffMax - 1;
	szBuffer[i] = '\0';
}

int StrIgnoreCompare(LPCTSTR szStr1, LPCTSTR szStr2)
{
	LPCTSTR	szDest = szStr1, szTemplate = szStr2;

	StrIgnoreCompare(szDest, szTemplate, FALSE);

	return _tcsicmp(szDest, szTemplate);
}

// COMMENT:
//		此段代码会重复使用。
//		比较目标和模板是否匹配，如果匹配则移动指针
// EXP:
//		szDest:			Artist(comment)
//		szTemplate:		Artist
// INPUT/OUTPUT:
//		szDest		-	被比较对象，于szTemplate进行匹配，匹配多少则，移动
//						多少指针。
//		szTemplate	-	比较得模板
//		bIsNumTemplate	-	该模板是否为全数字
void StrIgnoreCompare(LPCTSTR &szDest, LPCTSTR &szTemplate, BOOL bIgnoreNumb)
{
	while (1)
	{
		LPCTSTR		szBeforeEat = szDest;
		LPCTSTR		szBeforeEat2 = szTemplate;
		while (1)
		{
			szDest = IgnoreChars(szDest, bIgnoreNumb);
			szTemplate = IgnoreChars(szTemplate, bIgnoreNumb);

			if (*szDest == *szTemplate && *szDest != '\0')
			{
				szDest++;
				szTemplate++;
			}
			else
				break;
		}

		if (*szDest == '\0' || *szTemplate == '\0')
			break;

		if (('a' <= (*szDest) && *szDest <= 'z') || ('A' <= (*szDest) && *szDest <= 'Z'))
		{
			// 忽略大小写进行比较
			if ((*szDest | ('z' - 'Z')) != (*szTemplate | ('z' - 'Z')))
				break;

			szDest++;
			szTemplate++;
		}
		else
			break;

		if (szBeforeEat == szDest && szBeforeEat2 == szTemplate)
		{
			// 没有吃掉字符
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool operator<=(const LrcSearchResult &l1, const LrcSearchResult &l2)
{
	if (l1.nMatchValue > l2.nMatchValue)
		return true;
	else if (l1.nMatchValue == l2.nMatchValue)
	{
		if (_tcsicmp(l1.strArtist.c_str(), l2.strArtist.c_str()) >= 0)
			return true;
		else
			return false;
	}
	else
		return false;
}

bool operator<(const LrcSearchResult &l1, const LrcSearchResult &l2)
{
	return l1.nMatchValue > l2.nMatchValue;
}

// 歌词文件匹配类型的合歌词文件格式共同决定他的“匹配值”
const int	_aMatchValue[LYRIC_MATCH_MAX_TYPE + 1][3] = 
{
	//			  srt,				TXT,	LRC
	/* NONE */	 {0,				0,		0				},
	/* PART_DEC*/{35,				10,		33				},
	/* PART */	 {44,				35,		40				},
	/* ALL_DEC */{MATCH_VALUE_BETTER,	MATCH_VALUE_OK,		MATCH_VALUE_BETTER	},
	/* ALL */	 {MATCH_VALUE_BETTER+5, MATCH_VALUE_OK+1,	MATCH_VALUE_BETTER + 5	}
};                           //60

// COMMENT:
//		根据 nMatchType 和 nLyricFileType 计算所得得分
int GetLyricsMatchValue(int nMatchType, MLFileType nLyricFileType)
{
	assert(nMatchType >= 0 && nMatchType <= LYRIC_MATCH_MAX_TYPE);
	if (!(nMatchType >= 0 && nMatchType <= LYRIC_MATCH_MAX_TYPE))
		return 0;

	int		n;

	if (nLyricFileType == FT_SUBTITLE_SRT)
		n = 0;
	else if (nLyricFileType == FT_LYRICS_TXT)
		n = 1;
	else if (nLyricFileType == FT_LYRICS_LRC)
		n = 2;
	else
		return 1;

	return _aMatchValue[nMatchType][n];
}

// COMMENT:
//		取得仅仅标题匹配的歌词的匹配度
LRC_MATCH_TYPE GetLyricsMatchTypeTitleOnly(LPCTSTR szLyricsFileName, LPCTSTR szTitle, BOOL bTitleIgnoreNumb)
{
	LPCTSTR		szTitleFile;
	LPCTSTR		szBegTitle, szBegTemp;

	//
	// 试试直接匹配，可以解决：
	// szLyricsFileName：标题 － 作者
	// szTitle：标题 作者
	//
	szBegTitle = szLyricsFileName;
	szBegTemp = szTitle;

	StrIgnoreCompare(szBegTitle, szBegTemp, bTitleIgnoreNumb);
	if (*szBegTemp == '\0' && *szBegTitle == '\0')
	{
		szBegTitle = szLyricsFileName;
		szBegTemp = szTitle;

		StrIgnoreCompare(szBegTitle, szBegTemp, FALSE);
		if (*szBegTemp == '\0' && *szBegTitle == '\0')
			return LYRIC_MATCH_PART;
		else
			return LYRIC_MATCH_PART_DEC;
	}

	//
	// 找到歌词的标题，检查是否匹配，匹配方式：
	// szLyricsFileName：作者 － 标题
	// szTitle：标题
	//

	// 查找标题
	szTitleFile = _tcsstr(szLyricsFileName, _T(" - "));
	if (szTitleFile)
		szTitleFile += 3;	// _tcslen(_T(" - "));
	else
	{
		szTitleFile = _tcsstr(szLyricsFileName, _T(" － "));
		if (szTitleFile)
			szTitleFile += 4;	// _tcslen(_T(" － "));
		else
		{
			szTitleFile = _tcschr(szLyricsFileName, '-');
			if (szTitleFile)
				szTitleFile++;	// -
		}
	}
	if (szTitleFile == NULL)
		szTitleFile = szLyricsFileName;

	szBegTitle = szTitleFile;
	szBegTemp = szTitle;

	StrIgnoreCompare(szBegTitle, szBegTemp, bTitleIgnoreNumb);
	if (*szBegTemp == '\0' && *szBegTitle == '\0')
	{
		szBegTitle = szTitleFile;
		szBegTemp = szTitle;

		StrIgnoreCompare(szBegTitle, szBegTemp, FALSE);
		if (*szBegTemp == '\0' && *szBegTitle == '\0')
			return LYRIC_MATCH_PART;
		else
			return LYRIC_MATCH_PART_DEC;
	}
	else if (*szBegTitle == '\0')
	{
		//
		// 接着上面的情况，匹配下面的样式：
		// szLyricsFileName：作者 － 标题
		// szTitle：标题 作者
		//               ^~~^
		//
		// 目前已经匹配了标题，再匹配作者就行了
		// szTemp 接着匹配使用
		TCHAR	szArtistFile[256];
		LPCTSTR	szBegArtist;

		_tcsncpysz_safe(szArtistFile, 256, szLyricsFileName, int(szTitleFile - szLyricsFileName));

		szBegArtist = szArtistFile;
		StrIgnoreCompare(szBegArtist, szBegTemp, bTitleIgnoreNumb);
		if (*szBegTemp == '\0' && *szBegArtist == '\0')
		{
			szBegTitle = szTitleFile;
			szBegTemp = szTitle;

			StrIgnoreCompare(szBegTitle, szBegTemp, FALSE);
			if (*szBegTemp == '\0' && *szBegTitle == '\0')
				return LYRIC_MATCH_ALL;
			else
				return LYRIC_MATCH_ALL_DEC;
		}
	}

	return LYRIC_MATCH_NONE;
}

// COMMENT:
//		取得文件名与歌曲的"匹配类型"
//		szFileName 的一般格式为 作者 － 歌曲名
// INPUT:
//		szTitle, szArtist	－	经过标准化的字符串，字符串中只包含：(均为小写)字母、数字。
LRC_MATCH_TYPE GetLyricsMatchType(LPCTSTR szLyricsFileName, LPCTSTR szTitle, BOOL bTitleIgnoreNumb, LPCTSTR szArtist, BOOL bArtIgnoreNumb)
{
	assert(szLyricsFileName);
	assert(szTitle);
	assert(szArtist);
	assert(szTitle[0] != '\0');

	if (szArtist[0] == '\0')
	{
		// 对Artist 为空的情况进行处理
		return GetLyricsMatchTypeTitleOnly(szLyricsFileName, szTitle, bTitleIgnoreNumb);
	}

	//
	// 将szLyricsFileName 按照“作者 － 歌曲名”格式进行比较
	//

	LPCTSTR		szBegFile, szBegTemp;

	szBegFile = szLyricsFileName;
	szBegTemp = szArtist;

	StrIgnoreCompare(szBegFile, szBegTemp, bArtIgnoreNumb);

	// 没有匹配到作者名
	if (*szBegTemp != '\0')
		return GetLyricsMatchTypeTitleOnly(szLyricsFileName, szTitle, bTitleIgnoreNumb);

	// 再匹配歌曲标题, 继续使用szBegFile
	LPCTSTR		szFileTitle = szBegFile;
	szBegTemp = szTitle;

	StrIgnoreCompare(szBegFile, szBegTemp, bTitleIgnoreNumb);

	// 完全匹配
	if (*szBegTemp == '\0' && *szBegFile == '\0')
	{
		szBegTemp = szTitle;
		StrIgnoreCompare(szFileTitle, szBegTemp, FALSE);
		if (*szBegTemp == '\0' && *szBegFile == '\0')
			return LYRIC_MATCH_ALL;
		else
			return LYRIC_MATCH_ALL_DEC;
	}

	return LYRIC_MATCH_NONE;
}

// COMMENT:
//		取得文件名与歌曲的"匹配类型"
//		szFileName 的一般格式为 作者 － 歌曲名
// INPUT:
//		szTitle, szArtist	－	经过标准化的字符串，字符串中只包含：(均为小写)字母、数字。
LRC_MATCH_TYPE GetLyricsMatchType(LPCTSTR szItemTitle, LPCTSTR szItemArtist, LPCTSTR szTitle, BOOL bTitleIgnoreNumb, LPCTSTR szArtist, BOOL bArtIgnoreNumb)
{
	assert(szItemTitle);
	assert(szTitle);
	assert(szArtist);
	assert(szTitle[0] != '\0');

	if (szArtist[0] == '\0')
	{
		// szTitle 可能的格式 title - artist
		// szTitle 可能的格式 artist - title
		LPCTSTR		szBeg1, szBeg2;

		szBeg1 = szItemTitle;
		szBeg2 = szTitle;

		StrIgnoreCompare(szBeg1, szBeg2, bTitleIgnoreNumb);

		if (*szBeg1 == '\0')
		{
			// szTitle 可能的格式 title - artist
			// szItemTitle 匹配完了

			//
			// szTitle: title
			if (*szBeg2 == '\0')
				return LYRIC_MATCH_PART;

			//
			// szTitle: title - artist
			szBeg1 = szItemArtist;
			StrIgnoreCompare(szBeg1, szBeg2, bTitleIgnoreNumb);
			if (*szBeg1 == '\0' && *szBeg2 == '\0')
			{
				return LYRIC_MATCH_ALL_DEC;
			}
			else
				return LYRIC_MATCH_PART_DEC;
		}
		else
		{
			// szTitle 可能的格式 artist - title
			szBeg1 = szItemArtist;
			szBeg2 = szTitle;

			StrIgnoreCompare(szBeg1, szBeg2, bTitleIgnoreNumb);
			if (*szBeg1 == '\0')
			{
				szBeg1 = szItemTitle;
				StrIgnoreCompare(szBeg1, szBeg2, bTitleIgnoreNumb);
				if (*szBeg1 == '\0' && *szBeg2 == '\0')
					return LYRIC_MATCH_ALL_DEC;
			}
		}

		return LYRIC_MATCH_NONE;
	}

	//
	// 将szLyricsFileName 按照“作者 － 歌曲名”格式进行比较
	//
	LPCTSTR		szBeg1, szBeg2;

	szBeg1 = szItemTitle;
	szBeg2 = szTitle;

	StrIgnoreCompare(szBeg1, szBeg2, bTitleIgnoreNumb);

	if (*szBeg1 == '\0' && *szBeg2 == '\0')
	{
		szBeg1 = szItemArtist;
		szBeg2 = szArtist;
		StrIgnoreCompare(szBeg1, szBeg2, bTitleIgnoreNumb);
		if (*szBeg1 == '\0' && *szBeg2 == '\0')
			return LYRIC_MATCH_ALL;
		else
			return LYRIC_MATCH_PART;
	}

	//
	// 将szLyricsFileName 按照“歌曲名 － 作者”格式进行比较
	//
	szBeg1 = szItemArtist;
	szBeg2 = szTitle;

	StrIgnoreCompare(szBeg1, szBeg2, bTitleIgnoreNumb);

	if (*szBeg1 == '\0' && *szBeg2 == '\0')
	{
		szBeg1 = szItemTitle;
		szBeg2 = szArtist;
		StrIgnoreCompare(szBeg1, szBeg2, bTitleIgnoreNumb);
		if (*szBeg1 == '\0' && *szBeg2 == '\0')
			return LYRIC_MATCH_ALL;
	}

	return LYRIC_MATCH_NONE;
}

int CalLyricsMatchValue(MLFileType fileType, LPCTSTR szTitleTarg, LPCTSTR szArtistTarg, LPCTSTR szTitle, BOOL bTitleIgnoreNumb, LPCTSTR szArtist, BOOL bArtIgnoreNumb)
{
	LRC_MATCH_TYPE	nMatchType;

	nMatchType = GetLyricsMatchType(szTitleTarg, szArtistTarg, szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb);

	return GetLyricsMatchValue(nMatchType, fileType);
}

// COMMENT:
//		根据 歌词名、文件类型 计算所得得分
int CalLyricsMatchValue(LPCTSTR szFile, LPCTSTR szTitle, BOOL bTitleIgnoreNumb, LPCTSTR szArtist, BOOL bArtIgnoreNumb)
{
	LRC_MATCH_TYPE	nMatchType;
	MLFileType		nFileType;

	nFileType = MLGetFileType(szFile);

	if (bTitleIgnoreNumb)
	{
		TCHAR		szFileTitle[512];
		FileGetTitle(szFileTitle, CountOf(szFileTitle), szFile);
		nMatchType = GetLyricsMatchType(szFileTitle, szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb);
	}
	else
	{
		TCHAR			szTitleTarg[MAX_PATH], szArtTarg[MAX_PATH];

		AnalysisLyricsFileNameEx(szArtTarg, CountOf(szArtTarg), szTitleTarg, CountOf(szTitleTarg), szFile);
		nMatchType = GetLyricsMatchType(szTitleTarg, szArtTarg, szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb);
	}

	return GetLyricsMatchValue(nMatchType, nFileType);
}

//////////////////////////////////////////////////////////////////////
// V_LRCSEARCHRESULT
//////////////////////////////////////////////////////////////////////
BOOL V_LRCSEARCHRESULT::AddResult(const LrcSearchResult &Result)
{
	for (iterator it = begin(); it != end(); it++)
	{
		if (_tcsicmp((*it).strUrl.c_str(), Result.strUrl.c_str()) == 0)
		{
			return TRUE;
		}
	}

	push_back(Result);
	return TRUE;
}

bool V_LRCSEARCHRESULT::IsAllTxtLyrics() const
{
	for (const_iterator it = begin(); it != end(); ++it)
	{
		const LrcSearchResult		&result = *it;
		if (!FileIsExtSame(result.strUrl.c_str(), _T(".txt")))
		{
			return false;
		}
	}
	return true;
}

V_LRCSEARCHRESULT::iterator V_LRCSEARCHRESULT::GetTheBestMatchLyrics(BOOL bCheckExist)
{
	float		nValueMax = 0;
	iterator	itMax = begin();
	for (iterator it = begin(); it != end(); it++)
	{
		LrcSearchResult		&result = *it;
		if (bCheckExist && GetLrcSourceType(result.strUrl.c_str()) == LST_FILE)
		{
			if (!IsFileExist(result.strUrl.c_str()))
				continue;
		}

		if (result.nMatchValue == MATCH_VALUE_MAX)
			return it;
		else if (result.nMatchValue > nValueMax)
		{
			nValueMax = result.nMatchValue;
			itMax = it;
		}
	}

	return itMax;
}

void V_LRCSEARCHRESULT::UpdateLyricsName()
{
	for (iterator it = begin(); it != end(); it++)
	{
		LrcSearchResult		&result = *it;

		if (result.strSaveFileName.empty())
		{
			result.strSaveFileName = FileGetName(result.strUrl.c_str());
		}
	}
}

LrcSearchResult *V_LRCSEARCHRESULT::SearchByUrl(LPCTSTR szUrl)
{
	for (iterator it = begin(); it != end(); it++)
	{
		LrcSearchResult		&result = *it;

		if (_tcsicmp(result.strUrl.c_str(), szUrl) == 0)
		{
			return &(*it);
		}
	}
	return NULL;
}

LrcSearchResult *V_LRCSEARCHRESULT::GetByOrder(int n)
{
	for (iterator it = begin(); it != end(); it++, n--)
	{
		if (n == 0)
		{
			return &(*it);
		}
	}
	return NULL;
}

