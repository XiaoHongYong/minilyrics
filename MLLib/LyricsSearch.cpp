// LyricsSearch.cpp: implementation of the CLyricsSearch class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "LyricsSearch.h"
#include "MLData.h"
#include "HelperFun.h"
#include "../MLServer/LyricsKeywordFilter.h"

//////////////////////////////////////////////////////////////////////////
// CLyricsSearchParameter
CLyricsSearchParameter::CLyricsSearchParameter(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, bool bOnlySearchBestMatch)
{
	this->szSongFile = szSongFile;
	this->szArtist = szArtist;
	this->szTitle = szTitle;
	this->bOnlySearchBestMatch = bOnlySearchBestMatch;
	this->nMatchValueOfBest = 0;

	CLyricsKeywordFilter::Filter(szArtist, strArtistFiltered);
	CLyricsKeywordFilter::Filter(szTitle, strTitleFiltered);

	if (!IsEmptyString(szSongFile))
	{
		strSongTitle = FileGetTitle(szSongFile);
		strSongDir = FileGetPath(szSongFile);
	}
}

//						   TXT, 			    LRC                     SRT
const int __vArTiSame[] = { MATCH_VALUE_OK,      MATCH_VALUE_OK + 20,	MATCH_VALUE_OK + 10  };
const int   __vTiSame[] = { MATCH_VALUE_OK - 30, MATCH_VALUE_OK - 10, 	MATCH_VALUE_OK - 20 };

inline int FileTypeToMatchValueCol(MLFileType fileType)
{
	int col = 0;
	if (fileType == FT_LYRICS_LRC)
		col = 1;
	else if (fileType == FT_SUBTITLE_SRT)
		col = 2;

	return col;
}

int CLyricsSearchParameter::CalMatchValueByName(LPCTSTR szLyricsDir, LPCTSTR szLyricsFileName, MLFileType fileType)
{
	ASSERT(StrEndWith(szLyricsDir, _T(SZ_DIR_SLASH)));

	bool isFileTitleSame = false;
	bool isDirSame = false;
	bool isArtistSame = false;

	tstring strLyrArtist, strLyrTitle;
	AnalysisLyricsFileNameEx(strLyrArtist, strLyrTitle, szLyricsFileName);

	// Calculate match by artist and title.
	int matchValue = CalMatchValueByTitle(strLyrArtist.c_str(), strLyrTitle.c_str(), fileType);

	// Is file name same with song file name?
	if (strSongTitle.size() > 0)
		isFileTitleSame = StrIsISame(FileGetTitle(szSongFile).c_str(), FileGetTitle(szLyricsFileName).c_str());

	// Continue ?
	if (!isFileTitleSame && matchValue == 0)
		return 0;

	// Is file dir same with song file name?
	if (strSongDir.size() > 0)
		isDirSame = StrIsISame(strSongDir.c_str(), szLyricsDir);

	if (matchValue > 0)
	{
		// Title same.
		if (isFileTitleSame) matchValue += 1;
		if (isDirSame) matchValue += 1;
	}

	if (isFileTitleSame && matchValue < MATCH_VALUE_OK)
	{
		// File name same.
		matchValue = __vArTiSame[FileTypeToMatchValueCol(fileType)];
		if (isDirSame)
			matchValue += 2;
	}

	return matchValue;
}

int CLyricsSearchParameter::CalMatchValueByTitle(LPCTSTR szLyrArtist, LPCTSTR szLyrTitle, MLFileType fileType)
{
	bool isTitleSame = false;
	bool isArtistSame = false;

	// Is Title same?
	CLyricsKeywordFilter::Filter(szLyrTitle, strLyrTiFiltered);
	isTitleSame = StrIsSame(strLyrTiFiltered.c_str(), strTitleFiltered.c_str());

	// Continue ?
	if (!isTitleSame)
		return 0;

	// Is Artist same?
	CLyricsKeywordFilter::Filter(szLyrArtist, strLyrArFiltered);
	isArtistSame = StrIsSame(strLyrArFiltered.c_str(), strArtistFiltered.c_str());

	int matchValue;
	if (isArtistSame)
		matchValue = __vArTiSame[FileTypeToMatchValueCol(fileType)];
	else
		matchValue = __vTiSame[FileTypeToMatchValueCol(fileType)];

	bool isTitleAllSame = StrIsISame(szTitle, szLyrTitle);
	bool isArtistAllSame = StrIsISame(szArtist, szLyrArtist);

	if (isTitleAllSame) matchValue += 1;
	if (isArtistAllSame) matchValue += 1;

	return matchValue;
}

bool CLyricsSearchParameter::IsBestMatchLyricsFound()
{
	return nMatchValueOfBest >= MATCH_VALUE_BETTER;
}

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
		if (bCheckExist && LyrSrcTypeFromName(result.strUrl.c_str()) == LST_FILE)
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

void V_LRCSEARCHRESULT::DeleteByOrder(int n)
{
	for (iterator it = begin(); it != end(); it++, n--)
	{
		if (n == 0)
		{
			erase(it);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(LyricsSearch)

class CTestCaseLyricsSearch : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseLyricsSearch);
	CPPUNIT_TEST(TestCLyricsSearchParameter);
	CPPUNIT_TEST(TestCLyricsSearchParameter2);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestCLyricsSearchParameter()
	{
		tstring testDir = GetUnitTestFolder();
		tstring strMedia = DirStringJoint(testDir.c_str(), _T("artist - title.mp3"));
		LPCTSTR szArtist = _T("artist");
		LPCTSTR szTitle = _T("title");

		LPCTSTR vLyrFileName[] = { _T("artist - title.lrc"), _T("artist - 01 title.txt"), _T("artist(who are you?) - 01 title.txt"), _T("title.lrc"),       _T("artist.lrc"), };
		int      vMatchValue[] = { __vArTiSame[1] + 2 + 2,   __vArTiSame[0] + 1 + 1,     __vArTiSame[0] + 1,                         __vTiSame[1] + 1 + 1,  0 };

		CLyricsSearchParameter searchParam(strMedia.c_str(), szArtist, szTitle);

		for (int i = 0; i < CountOf(vLyrFileName); i++)
		{
			tstring strLyrFile = DirStringJoint(testDir.c_str(), vLyrFileName[i]);
			int matchValue = searchParam.CalMatchValueByName(testDir.c_str(), vLyrFileName[i], MLGetFileType(strLyrFile.c_str()));
			CPPUNIT_ASSERT(matchValue == vMatchValue[i]);
		}
	}

	void TestCLyricsSearchParameter2()
	{
		tstring testDir = GetUnitTestFolder();
		tstring strMedia = DirStringJoint(testDir.c_str(), _T("just a media.mp3"));
		LPCTSTR szArtist = _T("artist");
		LPCTSTR szTitle = _T("title");

		CLyricsSearchParameter searchParam(strMedia.c_str(), szArtist, szTitle);

		tstring strLyrFile = DirStringJoint(testDir.c_str(), _T("just a media.lrc"));
		int matchValue = searchParam.CalMatchValueByName(testDir.c_str(), strLyrFile.c_str(), MLGetFileType(strLyrFile.c_str()));
		CPPUNIT_ASSERT(matchValue == __vArTiSame[1] + 2);
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseLyricsSearch);

#endif // _CPPUNIT_TEST
