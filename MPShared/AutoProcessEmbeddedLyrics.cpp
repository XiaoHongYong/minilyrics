// AutoProcessEmbeddedLyrics.cpp: implementation of the CAutoProcessEmbeddedLyrics class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricsTool.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "DlgBatchProcessEmbeddedLyrics.h"

CAutoProcessEmbeddedLyrics		g_autoProcessEmbeddedLyrics;

#define SZ_JOB_FILE_NAME		_T("process_embedded_lyrics.bxml")
#define VERSION_CUR				_T("1")
#define SZ_VERSION				_T("version")

//////////////////////////////////////////////////////////////////////

CAutoProcessEmbeddedLyrics::CAutoProcessEmbeddedLyrics()
{

}

CAutoProcessEmbeddedLyrics::~CAutoProcessEmbeddedLyrics()
{

}

#ifdef UNICODE
static void StrSplit(LPCTSTR szStr, TCHAR chSeperator, vector<string> &vAnsiStr)
{
	V_Str vStr;
	StrSplit(szStr, chSeperator, vStr);
	for (int i = 0; i < vStr.size(); i++)
	{
		string str;
		TStringToAnsi(vStr[i].c_str(), vStr[i].size(), str);
		vAnsiStr.push_back(str);
	}
}
#endif

bool CAutoProcessEmbeddedLyrics::Item::FromXML(SXNode *pNode)
{
	m_strSongFile = pNode->GetPropertySafe(_T("song_file"));
	m_uEmbeddedLyrType = pNode->GetPropertyInt(_T("embedded_lyr_type"), 0);
	m_bRemove = Tobool(pNode->GetPropertyInt(_T("remove"), false));
	m_nTryFailed = pNode->GetPropertyInt(_T("try_failed"), 0);

	string	binLyrics;
	if (pNode->GetPropertyBinData(_T("bin_lyrics"), binLyrics))
		m_strLyrics.insert(0, binLyrics.c_str(), binLyrics.size());

	return m_strSongFile.size() > 0;
}

void CAutoProcessEmbeddedLyrics::Item::ToXML(CXMLWriter &xmlWriter)
{
	xmlWriter.WriteStartElement(_T("item"));

	xmlWriter.WriteAttribute(_T("song_file"), m_strSongFile.c_str());
	xmlWriter.WriteAttribute(_T("embedded_lyr_type"), m_uEmbeddedLyrType);
	xmlWriter.WriteAttribute(_T("remove"), m_bRemove);
	xmlWriter.WriteAttribute(_T("try_failed"), m_nTryFailed);
	xmlWriter.WriteAttribute(_T("bin_lyrics"), m_strLyrics.c_str(), m_strLyrics.size());

	xmlWriter.WriteEndElement();
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

int CAutoProcessEmbeddedLyrics::RemoveEmbeddedLyrics(LPCTSTR szSongFile, UINT uEmbeddedLyrType, bool bAddToQueueOnFailed)
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

void CAutoProcessEmbeddedLyrics::Init()
{
	LoadJobs(true);
}

bool CAutoProcessEmbeddedLyrics::OnPostQuit()
{
	OnSongChanged();

	if (m_listJobs.empty())
		return true;

	CDlgBatchProcessEmbeddedLyrics	dlg;
	dlg.DoModal(NULL);

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
	nRet = id3v2.Open(item.m_strSongFile.c_str(), true, false);
	if (nRet != ERR_OK)
		return nRet;

	if (Id3v2Uslt)
		nRet = id3v2.RemoveUnsynchLyrics(NULL);

	if (Id3v2Sylt)
		nRet = id3v2.RemoveSynchLyrics(NULL);

	if (bId3v2LyricsUserDefined)
		nRet = id3v2.RemoveUserDefinedTextFrameByDesc(SZ_ID3V2_LYR_USER_DEFINED);

	nRet = id3v2.Save();
	if (nRet != ERR_OK)
		return nRet;
	id3v2.Close();

	return ERR_OK;
}

void CAutoProcessEmbeddedLyrics::SaveJobs()
{
	CMLBinXMLWriter		xmlWriter;
	CMutexAutolock		autolock(m_mutex);

	xmlWriter.WriteStartElement(_T("embedded_lyr_jobs"));
	xmlWriter.WriteAttribute(SZ_VERSION, VERSION_CUR);

	for (LIST_ITEMS::iterator it = m_listJobs.begin(); it != m_listJobs.end(); ++it)
	{
		Item		&item = *it;
		if (!item.m_bDealOK)
			item.ToXML(xmlWriter);
	}

	xmlWriter.WriteEndElement();

	tstring	strFile = GetWorkingFolder();
	strFile += SZ_JOB_FILE_NAME;
	xmlWriter.SaveAsFile(strFile.c_str());
}

void CAutoProcessEmbeddedLyrics::LoadJobs(bool bDeleteFile)
{
	tstring	strFile = GetWorkingFolder();
	strFile += SZ_JOB_FILE_NAME;

	m_listJobs.clear();

	if (!IsFileExist(strFile.c_str()))
		return;

	CSimpleXML	xml;
	if (xml.ParseFile(strFile.c_str())
		&& _tcscmp(xml.m_pRoot->GetProperty(SZ_VERSION), VERSION_CUR) == 0)
	{
		for (SXNode::LIST_CHILDREN::iterator it = xml.m_pRoot->listChildren.begin();
			it != xml.m_pRoot->listChildren.end(); ++it)
		{
			SXNode	*pNode = *it;
			if (_tcscmp(pNode->strName.c_str(), _T("item")) != 0)
				continue;

			Item item;
			if (item.FromXML(pNode))
				m_listJobs.push_back(item);
		}
	}

	if (bDeleteFile)
		DeleteFile(strFile.c_str());
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CAutoProcessEmbeddedLyrics)

class CTestCaseCAutoProcessEmbeddedLyrics : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCAutoProcessEmbeddedLyrics);
	CPPUNIT_TEST(Test_AutoProcessEmbeddedLyricsLoad);
	CPPUNIT_TEST_SUITE_END();

protected:

	void Test_AutoProcessEmbeddedLyricsLoad()
	{
		tstring	strFile = GetWorkingFolder();
		strFile += SZ_JOB_FILE_NAME;

		// Only run this test when the file doesn't exist.
		if (IsFileExist(strFile.c_str()))
			return;

		CAutoProcessEmbeddedLyrics	embedded_lyr_process;
		CAutoProcessEmbeddedLyrics::Item	item;

		item.m_strSongFile = _T("C:\\abc.mp3");
		item.m_strLyrics.append(";lksjd;gioew1230$/;%^&*");
		item.m_uEmbeddedLyrType = 2308273;
		item.m_bRemove = true;
		item.m_bDealOK = false;
		item.m_nTryFailed = 2;
		embedded_lyr_process.m_listJobs.push_back(item);

		item.m_strSongFile = _T("C:\\abcd.mp3");
		item.m_strLyrics.append(";lksjd;gssgeeggioew1230$/;%^&*");
		item.m_uEmbeddedLyrType = 21308273;
		item.m_bRemove = false;
		item.m_bDealOK = false;
		item.m_nTryFailed = 0;
		embedded_lyr_process.m_listJobs.push_back(item);

		CAutoProcessEmbeddedLyrics::LIST_ITEMS	listJobs = embedded_lyr_process.m_listJobs;

		embedded_lyr_process.SaveJobs();
		embedded_lyr_process.LoadJobs(true);

		CPPUNIT_ASSERT(listJobs.size() == embedded_lyr_process.m_listJobs.size());

		CAutoProcessEmbeddedLyrics::LIST_ITEMS::iterator it1 = embedded_lyr_process.m_listJobs.begin();
		for (CAutoProcessEmbeddedLyrics::LIST_ITEMS::iterator it2 = listJobs.begin(); 
			it2 != listJobs.end(); ++it1, ++it2)
		{
			CAutoProcessEmbeddedLyrics::Item &i1 = *it1;
			CAutoProcessEmbeddedLyrics::Item &i2 = *it2;

			CPPUNIT_ASSERT(i1.m_strSongFile == i2.m_strSongFile);
			CPPUNIT_ASSERT(i1.m_uEmbeddedLyrType == i2.m_uEmbeddedLyrType);
			CPPUNIT_ASSERT(i1.m_bRemove == i2.m_bRemove);
			CPPUNIT_ASSERT(i1.m_nTryFailed == i2.m_nTryFailed);

			CPPUNIT_ASSERT(i1.m_strLyrics.size() == i2.m_strLyrics.size());
			CPPUNIT_ASSERT(memcmp(i1.m_strLyrics.c_str(), i2.m_strLyrics.c_str(), i1.m_strLyrics.size()) == 0);
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCAutoProcessEmbeddedLyrics);

#endif // _CPPUNIT_TEST
