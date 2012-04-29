#include "StdAfx.h"
#include "HelperFun.h"
#ifdef WIN32
#ifdef _WINCE_
#define _mbschr		strchr
#else
#include "mbstring.h"
#endif
#endif
#include "LrcTag.h"

//////////////////////////////////////////////////////////////////////////
//

void AnalysisLyricsFileNameEx(tstring &strArtist, tstring &strTitle, LPCTSTR szFile)
{
	tstring fileTitle = FileGetTitle(szFile);

	if (!StrSplit(fileTitle.c_str(), _T(" - "), strArtist, strTitle)
		&& !StrSplit(fileTitle.c_str(), '-', strArtist, strTitle))
	{
		strArtist.resize(0);
		strTitle = fileTitle;
	}

	TrimStr(strArtist);
	TrimStr(strTitle);
}

tstring FormatMediaTitle(LPCTSTR szArtist, LPCTSTR szTitle)
{
	tstring mediaTitle = szArtist;
	if (mediaTitle.size() > 0 && !IsEmptyString(szTitle))
		mediaTitle += _T(" - ");
	mediaTitle += szTitle;

	return mediaTitle;
}

//////////////////////////////////////////////////////////////////////////
//

void AnalysisProxySetting(LPCTSTR szProxySetting, TCHAR szServer[], int nMaxSize, int &nPort)
{
	LPCTSTR		szColon;

	szColon = _tcschr(szProxySetting, ':');
	if (szColon == NULL)
	{
		_tcscpy_safe(szServer, nMaxSize, szProxySetting);
		nPort = 80;
	}
	else
	{
		_tcsncpy_safe(szServer, nMaxSize, szProxySetting,
			int(szColon - szProxySetting));
		nPort = _ttoi(szColon + 1);
	}
}

#ifdef WIN32
bool GetMLEncriptyData(stringex &strData)
{
	TCHAR			szFile[MAX_PATH];
	stringex		str;
	TCHAR			szMD5[64];

	GetModuleFileName(GetAppInstance(), szFile, CountOf(szFile));

	if (!OpenFileToBuffer(szFile, str))
		return false;

	MD5ToString(str.c_str() + 0x100, str.size() - 0x100, szMD5);

	// 0x51 ~ 0x78
	// decodebase64(str.c_str() + 0x51, 0x78 - 0x51, strData);
	strData.append(str.c_str() + 0x51, 0x78 - 0x51);

	for (uint i = 0; i < 32; i++)
		strData[i] = strData[i] ^ szMD5[i];

	// 解密之后的数据了，这里存储的是核心数据

	return true;
}
#endif

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(HelperFun)

class CTestCaseStringEx : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseStringEx);
	CPPUNIT_TEST(TestAnalysisLyricsFileNameEx);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestAnalysisLyricsFileNameEx()
	{
		tstring artist, title;
		LPCTSTR szFile;

		szFile = _T("c:\\abc\ar - ti-a.mp3");
		AnalysisLyricsFileNameEx(artist, title, szFile);
		CPPUNIT_ASSERT(artist == _T("ar"));
		CPPUNIT_ASSERT(title == _T("ti-a"));

		szFile = _T("c:\\abc\ar-ti-a.mp3");
		AnalysisLyricsFileNameEx(artist, title, szFile);
		CPPUNIT_ASSERT(artist == _T("ar"));
		CPPUNIT_ASSERT(title == _T("ti-a"));

		szFile = _T("c:\\abc\ti.mp3");
		AnalysisLyricsFileNameEx(artist, title, szFile);
		CPPUNIT_ASSERT(artist == _T(""));
		CPPUNIT_ASSERT(title == _T("ti"));
	}

};

#endif