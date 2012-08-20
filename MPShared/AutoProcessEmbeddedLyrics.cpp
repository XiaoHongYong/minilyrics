// AutoProcessEmbeddedLyrics.cpp: implementation of the CAutoProcessEmbeddedLyrics class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "DlgBatchProcessEmbeddedLyrics.h"

CAutoProcessEmbeddedLyrics		g_autoProcessEmbeddedLyrics;

#define SZ_JOB_FILE_NAME		_T("process_embedded_lyrics.bxml")
#define VERSION_CUR				_T("2")
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
	for (uint i = 0; i < vStr.size(); i++)
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
	StrSplit(pNode->GetPropertySafe(_T("embedded_lyr_names")), '|', m_vLyrNames);
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
	xmlWriter.WriteAttribute(_T("embedded_lyr_names"), 
		StrJoin(m_vLyrNames.begin(), m_vLyrNames.end(), _T("|")).c_str());
	xmlWriter.WriteAttribute(_T("remove"), m_bRemove);
	xmlWriter.WriteAttribute(_T("try_failed"), m_nTryFailed);
	xmlWriter.WriteAttribute(_T("bin_lyrics"), m_strLyrics.c_str(), m_strLyrics.size());

	xmlWriter.WriteEndElement();
}

int CAutoProcessEmbeddedLyrics::SaveEmbeddedLyrics(LPCTSTR szSongFile, LPCTSTR szLyricsFile, CCharBuffer *pBufLyrics, VecString &vEmbeddedLyrNames, bool bAddToQueueOnFailed, int *succeededCount)
{
	if (succeededCount)
		*succeededCount = 0;

	if (vEmbeddedLyrNames.empty())
		return ERR_OK;

	ASSERT(szLyricsFile || pBufLyrics);
	if (!CTagLib::IsEmbeddedLyricsSupported(szSongFile))
		return ERR_NOT_SUPPORT_FILE_FORMAT;

	CMutexAutolock		autolock(m_mutex);

	CCharBuffer		bufLyrFile;

	// Open lyrics file to bufLyrFile
	if (!IsEmptyString(szLyricsFile))
	{
		LRC_SOURCE_TYPE	lst = LyrSrcTypeFromName(szLyricsFile);
		if (lst == LST_FILE)
		{
			if (!OpenFileToBuffer(szLyricsFile, bufLyrFile))
				return ERR_OPEN_FILE;
		}
		else
		{
			CMLData		lyrics;
			int nRet = lyrics.OpenLyrics(szSongFile, 0, szLyricsFile);
			if (nRet != ERR_OK)
				return nRet;

			tstring str;
			lyrics.ToString(str, FT_LYRICS_LRC, true);
			TStringToProperFileData(str.c_str(), bufLyrFile);
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
			item.m_vLyrNames = vEmbeddedLyrNames;
			item.m_nTryFailed = 0;
			nRet = DealEmbeddedLyrics(item);
			if (succeededCount)
				*succeededCount = item.m_nSucceededCount;
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
	newItem.m_vLyrNames = vEmbeddedLyrNames;
	newItem.m_nTryFailed = 0;
	nRet = DealEmbeddedLyrics(newItem);
	if (succeededCount)
		*succeededCount = newItem.m_nSucceededCount;
	if (nRet != ERR_OK && bAddToQueueOnFailed)
		m_listJobs.push_back(newItem);

	return nRet;
}

int CAutoProcessEmbeddedLyrics::RemoveEmbeddedLyrics(LPCTSTR szSongFile, VecString &vEmbeddedLyrNames, bool bAddToQueueOnFailed)
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
			item.m_vLyrNames = vEmbeddedLyrNames;
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
	newItem.m_vLyrNames = vEmbeddedLyrNames;
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

	tstring strSongFileCur = g_Player.GetSrcMedia();

	for (LIST_ITEMS::iterator it = m_listJobs.begin(); it != m_listJobs.end(); )
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
	if (!CTagLib::IsEmbeddedLyricsSupported(item.m_strSongFile.c_str()))
		return ERR_NOT_SUPPORT_FILE_FORMAT;

	SetFileNoReadOnly(item.m_strSongFile.c_str());

	int				nRet;
	if (item.m_bRemove)
		nRet = DealRemoveEmbeddedLyrics(item);
	else
		nRet = DealSaveEmbeddedLyrics(item);

	if (nRet == ERR_OK)
		item.m_bDealOK = true;

	CMPlayerApp::GetAppAnalytics().IncreaseNumberOfResult(SZ_UD_EMBEDDED_LYRICS_DATA, nRet);

	return nRet;
}

int CAutoProcessEmbeddedLyrics::DealSaveEmbeddedLyrics(Item &item)
{
	uint32 uAllLstFlag = 0;
	for (uint i = 0; i < item.m_vLyrNames.size(); i++)
		uAllLstFlag |= LyrSrcTypeFromName(item.m_vLyrNames[i].c_str());

	int nRet;

	// Save lyrics3v2 lyrics
	if (IsFlagSet(uAllLstFlag, LST_LYRICS3V2)
		&& CTagLib::IsID3v2TagSupported(item.m_strSongFile.c_str()))
	{
		CLyrics3v2	lyrics3v2;
		nRet = lyrics3v2.WriteLyrcsInfo(item.m_strSongFile.c_str(), item.m_strLyrics.c_str(), item.m_strLyrics.size());
		if (nRet != ERR_OK && nRet != ERR_NOT_FIND_LRC3V2)
			return nRet;
		item.m_nSucceededCount++;
	}

	// Save ID3v2 embedded lyrics
	if ((uAllLstFlag & LST_ID3V2) > 0
		 && CTagLib::IsID3v2TagSupported(item.m_strSongFile.c_str()))
	{
		// save lyrics as id3v2 sylt and uslt
		CMLData		lyricsData;

		nRet = lyricsData.OpenLyrics(item.m_strSongFile.c_str(), 0, (LPBYTE)item.m_strLyrics.c_str(), item.m_strLyrics.size());
		if (nRet == ERR_OK)
		{
			nRet = lyricsData.SaveLyricsInSongOfID3v2(item.m_vLyrNames);
			if (nRet != ERR_OK)
				return nRet;
			item.m_nSucceededCount++;
		}
	}

	// Save M4a embedded lyrics
	if ((uAllLstFlag & LST_M4A_LYRICS) > 0
		 && CTagLib::IsM4aTagSupported(item.m_strSongFile.c_str()))
	{
		CMLData		lyricsData;

		nRet = lyricsData.OpenLyrics(item.m_strSongFile.c_str(), 0, (LPBYTE)item.m_strLyrics.c_str(), item.m_strLyrics.size());
		if (nRet == ERR_OK)
		{
			nRet = lyricsData.SaveLyricsInSongOfM4a();
			if (nRet != ERR_OK)
				return nRet;
			item.m_nSucceededCount++;
		}
	}

	if ((uAllLstFlag & (LST_ID3V2_SYLT | LST_LYRICS3V2)) > 0)
		g_LyricSearch.CancelAssociate(item.m_strSongFile.c_str());

	return ERR_OK;
}

int CAutoProcessEmbeddedLyrics::DealRemoveEmbeddedLyrics(Item &item)
{
	ASSERT(item.m_bRemove);

	return CTagLib::RemoveEmbeddedLyrics(item.m_strSongFile.c_str(), item.m_vLyrNames, &item.m_nSucceededCount);
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

	tstring	strFile = GetAppDataDir();
	strFile += SZ_JOB_FILE_NAME;
	xmlWriter.SaveAsFile(strFile.c_str());
}

void CAutoProcessEmbeddedLyrics::LoadJobs(bool bDeleteFile)
{
	tstring	strFile = GetAppDataDir();
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
		tstring	strFile = GetAppDataDir();
		strFile += SZ_JOB_FILE_NAME;

		// Only run this test when the file doesn't exist.
		if (IsFileExist(strFile.c_str()))
			return;

		CAutoProcessEmbeddedLyrics	embedded_lyr_process;
		CAutoProcessEmbeddedLyrics::Item	item;

		item.m_strSongFile = _T("C:\\abc.mp3");
		item.m_strLyrics.append(";lksjd;gioew1230$/;%^&*");
		item.m_vLyrNames.push_back(_T("abc"));
		item.m_vLyrNames.push_back(_T("de"));
		item.m_bRemove = true;
		item.m_bDealOK = false;
		item.m_nTryFailed = 2;
		embedded_lyr_process.m_listJobs.push_back(item);

		item.m_strSongFile = _T("C:\\abcd.mp3");
		item.m_strLyrics.append(";lksjd;gssgeeggioew1230$/;%^&*");
		item.m_vLyrNames.push_back(_T("a"));
		item.m_vLyrNames.push_back(_T("def"));
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
			CPPUNIT_ASSERT(i1.m_vLyrNames == i2.m_vLyrNames);
			CPPUNIT_ASSERT(i1.m_bRemove == i2.m_bRemove);
			CPPUNIT_ASSERT(i1.m_nTryFailed == i2.m_nTryFailed);

			CPPUNIT_ASSERT(i1.m_strLyrics.size() == i2.m_strLyrics.size());
			CPPUNIT_ASSERT(memcmp(i1.m_strLyrics.c_str(), i2.m_strLyrics.c_str(), i1.m_strLyrics.size()) == 0);
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCAutoProcessEmbeddedLyrics);

#endif // _CPPUNIT_TEST
