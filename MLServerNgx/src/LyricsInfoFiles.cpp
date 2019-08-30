#include "StdAfx.h"
#include "LyricsInfoFiles.h"

//////////////////////////////////////////////////////////////////////////
/*
// CLyrHTFile:
//
Format:		<xml ....>
			<History current="id">
				<item id="50" lastUpdateId="49" uploader="T-uploader2" rs="0" md5="CEA297C2EE514AFCDB2609D286EC5752"/>
				<item id="49" lastUpdateId="0" uploader="T-uploader" rs="0" md5="B79FB166F782B5D58F72834F12BA36F6"/>
			</History>
//
*//////////////////////////////////////////////////////////////////////////

class CLyrHTFile
{
public:
	CLyrHTFile() { m_nCurrentUpdateId = LYR_UPDATE_ID_ORG; m_nDerivedFromId = 0; m_bModified = false; }
	~CLyrHTFile()
	{
		if (m_bModified)
			Save();
	}

	int Open(LPCTSTR szFile);

	int AddUpdate(LyrUpdateItem &lyrUpdateItem, long nDerivedFromId = 0);

	int ChangeReviewStatus(LyrUpdateItem &update, ReviewStatus reviewStatus, LPCTSTR szReviewer);
	// int ChangeReviewStatus(long nUpdateId, ReviewStatus reviewStatus);
	// int GetReviewStatus(long nUpdateId, ReviewStatus &reviewStatus);

	ListLyrUpdateItem::iterator GetUpdate(long nUpdateId);

	ListLyrUpdateItem &GetUpdateHistory() { return m_listUpdateHistory; }
	long GetCurrentUpdateId() { return m_nCurrentUpdateId; }

protected:
	int Save();

private:
	ListLyrUpdateItem		m_listUpdateHistory;
	long					m_nCurrentUpdateId;
	long					m_nDerivedFromId;
	tstring					m_strFile;
	bool					m_bModified;

};

void LyrUpdateItem::FromXML(SXNode *pNode)
{
	nUpdateId = pNode->GetPropertyInt(_T("id"));
	nLastUpdateId = pNode->GetPropertyInt(_T("lastUpdateId"));
	dateUpdate = (DWORD)pNode->GetPropertyInt(_T("dateUpdate"));
	strUploader = pNode->GetPropertySafe(_T("uploader"));
	reviewStatus = (ReviewStatus)pNode->GetPropertyInt(_T("rs"));
	strReviewer = pNode->GetPropertySafe(_T("reviewer"));
	strMD5 = pNode->GetPropertySafe(_T("md5"));
}

void LyrUpdateItem::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(_T("item"));

	xmlFile.WriteAttribute(_T("id"), nUpdateId);
	if (nLastUpdateId != LYR_UPDATE_ID_INVALID)
		xmlFile.WriteAttribute(_T("lastUpdateId"), nLastUpdateId);
	xmlFile.WriteAttribute(_T("dateUpdate"), dateUpdate);
	if (strUploader.size())
		xmlFile.WriteAttribute(_T("uploader"), strUploader.c_str());
	xmlFile.WriteAttribute(_T("rs"), reviewStatus);
	if (strReviewer.size())
		xmlFile.WriteAttribute(_T("reviewer"), strReviewer.c_str());
	xmlFile.WriteAttribute(_T("md5"), strMD5.c_str());

	xmlFile.WriteEndElement();
}

int CLyrHTFile::Open(LPCTSTR szFile)
{
	m_strFile = szFile;
	m_strFile += SZ_LYR_HST_EXT;

	m_listUpdateHistory.clear();

	// Empty ?
	if (!IsFileExist(m_strFile.c_str()))
		return ERR_OK;

	CSimpleXML	xml;

	if (!xml.ParseFile(m_strFile.c_str()))
		return ERR_PARSE_XML;

	SXNode	*pRoot = xml.m_pRoot;

	if (_tcscmp(pRoot->strName.c_str(), _T("History")) != 0)
		return ERR_BAD_FILE_CONTENT;

	m_nCurrentUpdateId = pRoot->GetPropertyInt(_T("current"));
	m_nDerivedFromId = pRoot->GetPropertyInt(_T("DerivedFrom"));

	for (SXNode::iterator it = pRoot->listChildren.begin(); it != pRoot->listChildren.end(); ++it)
	{
		SXNode	*p = *it;
		ASSERT(p->strName == _T("item"));
		LyrUpdateItem	update;

		update.FromXML(p);
		m_listUpdateHistory.push_back(update);
	}

	return ERR_OK;
}

int CLyrHTFile::AddUpdate(LyrUpdateItem &lyrUpdateItem, long nDerivedFromId)
{
	m_bModified = true;
	if (nDerivedFromId != 0)
		m_nDerivedFromId = nDerivedFromId;

	if (lyrUpdateItem.reviewStatus == RS_ACCEPTED
		|| lyrUpdateItem.reviewStatus == RS_OPTIONAL)
	{
		lyrUpdateItem.nLastUpdateId = m_nCurrentUpdateId;
		m_nCurrentUpdateId = lyrUpdateItem.nUpdateId;
	}
	else
		lyrUpdateItem.nLastUpdateId = LYR_UPDATE_ID_INVALID;

	m_listUpdateHistory.insert(m_listUpdateHistory.begin(), lyrUpdateItem);

	return ERR_OK;
}

/*
int CLyrHTFile::ChangeReviewStatus(long nUpdateId, ReviewStatus reviewStatus)
{
	ListLyrUpdateItem::iterator it = GetUpdate(nUpdateId);

	if (it == m_listUpdateHistory.end())
		return ERR_NOT_FOUND;

	LyrUpdateItem &update = *it;
	ChangeReviewStatus(update, reviewStatus);
}*/

int CLyrHTFile::ChangeReviewStatus(LyrUpdateItem &update, ReviewStatus reviewStatus, LPCTSTR szReviewer)
{
	if (reviewStatus == RS_ACCEPTED)
	{
		if (m_nCurrentUpdateId == update.nUpdateId && update.reviewStatus == RS_ACCEPTED)
			return ERR_OK;

		m_nCurrentUpdateId = update.nUpdateId;
		if (update.nLastUpdateId != LYR_UPDATE_ID_INVALID)
			update.nLastUpdateId = m_nCurrentUpdateId;
	}
	else if (reviewStatus == RS_REJECTED)
	{
		// Recover current update id.
		if (m_nCurrentUpdateId == update.nUpdateId)
			m_nCurrentUpdateId = update.nLastUpdateId;
		else if (update.reviewStatus == RS_REJECTED)
			return ERR_OK;
	}
	else if (update.reviewStatus == reviewStatus)
		return ERR_OK;

	m_bModified = true;


	update.strReviewer = szReviewer;
	update.reviewStatus = reviewStatus;

	return ERR_OK;
}
/*
int CLyrHTFile::GetReviewStatus(long nUpdateId, ReviewStatus &reviewStatus)
{
	ListLyrUpdateItem::iterator it = GetUpdate(nUpdateId);

	if (it == m_listUpdateHistory.end())
		return ERR_NOT_FOUND;

	LyrUpdateItem &update = *it;
	reviewStatus = update.reviewStatus;

	return ERR_OK;
}*/

ListLyrUpdateItem::iterator CLyrHTFile::GetUpdate(long nUpdateId)
{
	for (ListLyrUpdateItem::iterator it = m_listUpdateHistory.begin(); it != m_listUpdateHistory.end(); ++it)
	{
		if ((*it).nUpdateId == nUpdateId)
			return it;
	}

	return m_listUpdateHistory.end();
}

int CLyrHTFile::Save()
{
	CXMLWriter	xmlFile;

	xmlFile.WriteStartElement(_T("History"));

	xmlFile.WriteAttribute(_T("current"), m_nCurrentUpdateId);

	for (ListLyrUpdateItem::iterator it = m_listUpdateHistory.begin(); it != m_listUpdateHistory.end(); ++it)
		(*it).ToXML(xmlFile);

	xmlFile.WriteEndElement();

	if (!xmlFile.SaveAsFile(m_strFile.c_str()))
		return ERR_WRITE_FILE;

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// CLyrHDFile:
//
// FOOTER DATA FOOTER
// FOOTER DATA FOOTER
//
//////////////////////////////////////////////////////////////////////////

#define LYR_HD_FOOTER_TAG		"@HD#"
#define LYR_HD_FOOTER_TAG_LEN	4

class CLyrHDFile
{
public:
#pragma pack (push,1)
	struct ItemHeaderFooter
	{
		char	szTag[4];
		char	szUpdateId[12];
		char	szDataLen[8];	// < 1M

		long GetUpdateId();
		int GetDataLen();
		void Set(long nUpdateId, int nDataLen);
	};
#pragma pack (pop)

	void Open(LPCTSTR szFile);
	void OpenRejected(LPCTSTR szFile);

	bool IsExist() { return ::IsFileExist(m_strHDFile.c_str()); }

	int AddUpdate(long nUpdateId, CCharBuffer &bufLyrData);

	int GetUpdateData(long nUpdateId, CCharBuffer &bufLyrData);

protected:
	int AppendItem(long nUpdateId, CCharBuffer &bufLyrData);

protected:
	tstring				m_strHDFile, m_strLyrFile;

};

long CLyrHDFile::ItemHeaderFooter::GetUpdateId()
{
	return atoi(szUpdateId);
}

int CLyrHDFile::ItemHeaderFooter::GetDataLen()
{
	return atoi(szDataLen);
}

void CLyrHDFile::ItemHeaderFooter::Set(long nUpdateId, int nDataLen)
{
	memset(szUpdateId, 0, sizeof(szUpdateId));
	memset(szDataLen, 0, sizeof(szDataLen));

	char		szBuf[256];
	sprintf(szBuf, "%d", nUpdateId);
	strcpy_safe(szUpdateId, CountOf(szUpdateId), szBuf);

	sprintf(szBuf, "%d", nDataLen);
	strcpy_safe(szDataLen, CountOf(szDataLen), szBuf);
}

void CLyrHDFile::Open(LPCTSTR szFile)
{
	m_strLyrFile = szFile;
	m_strHDFile = szFile;
	m_strHDFile += SZ_LYR_HD_EXT;
}

void CLyrHDFile::OpenRejected(LPCTSTR szFile)
{
	m_strLyrFile = szFile;
	m_strHDFile = szFile;
	m_strHDFile += _T(".rj.hd");
}

int CLyrHDFile::AddUpdate(long nUpdateId, CCharBuffer &bufLyrData)
{
	if (!IsFileExist(m_strHDFile.c_str()))
	{
		// HD file NOT exists, so for first update, backup old data.
		CCharBuffer	bufOldLyrData;
		if (OpenFileToBuffer(m_strLyrFile.c_str(), bufOldLyrData))
		{
			CLyrHTFile	htFile;

			int nRet = htFile.Open(m_strLyrFile.c_str());

			AppendItem(htFile.GetCurrentUpdateId(), bufLyrData);
		}
		else
			ERRLOG1(_T("Failed to open old lyrics data: %s"), m_strLyrFile.c_str());
	}

	// Add new record.
	return AppendItem(nUpdateId, bufLyrData);
}

int CLyrHDFile::AppendItem(long nUpdateId, CCharBuffer &bufLyrData)
{
	ItemHeaderFooter	headerFooter;

	memcpy(headerFooter.szTag, LYR_HD_FOOTER_TAG, LYR_HD_FOOTER_TAG_LEN);
	headerFooter.Set(nUpdateId, bufLyrData.size());

	FILE	*fp = _tfopen(m_strHDFile.c_str(), _T("a+b"));
	if (!fp)
		return ERR_OPEN_FILE;

	// Write header
	int n = fwrite(&headerFooter, 1, sizeof(headerFooter), fp);
	if (n != sizeof(headerFooter))
		goto E_WRITE_FILE;

	// Write lyrics data.
	n = fwrite(bufLyrData.c_str(), 1, bufLyrData.size(), fp);
	if (n != bufLyrData.size())
		goto E_WRITE_FILE;

	// Write footer
	n = fwrite(&headerFooter, 1, sizeof(headerFooter), fp);
	if (n != sizeof(headerFooter))
		goto E_WRITE_FILE;

	fclose(fp);
	return ERR_OK;

E_WRITE_FILE:
	fclose(fp);
	return ERR_WRITE_FILE;
}

int CLyrHDFile::GetUpdateData(long nUpdateId, CCharBuffer &bufLyrData)
{
	ItemHeaderFooter	headerFooter;

	// Search from down to up
	FILE	*fp = _tfopen(m_strHDFile.c_str(), _T("rb"));
	if (!fp)
		return ERR_OPEN_FILE;

	fseek(fp, 0 - sizeof(headerFooter), SEEK_END);

    int nDataLen, nRead;
    
	while (1)
	{
		// read footer
		int n = fread(&headerFooter, 1, sizeof(headerFooter), fp);
		if (n != sizeof(headerFooter))
			goto E_ERROR;

		if (memcmp(headerFooter.szTag, LYR_HD_FOOTER_TAG, LYR_HD_FOOTER_TAG_LEN) != 0)
			goto E_ERROR;

		if (headerFooter.GetUpdateId() == nUpdateId)
			break;

		// 3 = 2 of this item + 1 of previous item footer.
		if (fseek(fp, 0 - (headerFooter.GetDataLen() + sizeof(headerFooter) * 3), SEEK_CUR) != 0)
			goto E_ERROR;
	}

	// Read lyrics data.
	nDataLen = headerFooter.GetDataLen();
	bufLyrData.reserve(nDataLen + 2);
	if (fseek(fp, 0 - nDataLen - sizeof(headerFooter), SEEK_CUR) != 0)
		goto E_ERROR;
	nRead = fread(bufLyrData.data(), 1, nDataLen, fp);
	if (nRead != nDataLen)
		goto E_ERROR;

	bufLyrData.resize(nDataLen);
	bufLyrData.append("\0\0", 2);	// Ensure data is ended with double zero
	bufLyrData.resize(nDataLen);

	fclose(fp);
	return ERR_OK;

E_ERROR:
	fclose(fp);
	return ERR_READ_FILE;
}


//////////////////////////////////////////////////////////////////////////

CLyrUpdateHistory::CLyrUpdateHistory(void)
{
}

CLyrUpdateHistory::~CLyrUpdateHistory(void)
{
}

// Specify the lyrics file name to open lyrics update history.
int CLyrUpdateHistory::Open(LPCTSTR szFile)
{
	m_strLyrFile = szFile;

	if (!IsFileExist(szFile))
		return ERR_FILE_NOT_EXIST;
	return ERR_OK;
}

int CLyrUpdateHistory::Add(LyrUpdateItem &lyrUpdateItem, CCharBuffer &bufNewLyrData, long nDerivedFromId)
{
	//
	// Add history file
	//
	CLyrHTFile	htFile;
	htFile.Open(m_strLyrFile.c_str());

	if (!lyrUpdateItem.bUpload)
	{
		// Only "Updating" action will record the lyrics data. 
		CLyrHDFile	hdFile;
		hdFile.Open(m_strLyrFile.c_str());

		// Add history data
		hdFile.AddUpdate(lyrUpdateItem.nUpdateId, bufNewLyrData);
	}

	htFile.AddUpdate(lyrUpdateItem, nDerivedFromId);

	return ERR_OK;
}

int CLyrUpdateHistory::GetUpdateHistory(ListLyrUpdateItem &listLyrUpdateItem, long &nCurrentUpdateId, bool bIncludingLyrData)
{
	CLyrHTFile	htFile;

	int nRet = htFile.Open(m_strLyrFile.c_str());
	if (nRet != ERR_OK)
		return nRet;

	listLyrUpdateItem = htFile.GetUpdateHistory();
	nCurrentUpdateId = htFile.GetCurrentUpdateId();

	if (bIncludingLyrData)
		return FillInLyricsData(listLyrUpdateItem);

	return ERR_OK;
}

int CLyrUpdateHistory::ReviewLyrics(long nLyricsId, long nStartUpdateId, long nUpdateIdAccepted, long nUpdateIdRejected, LPCTSTR szReviewer)
{
	ASSERT(nUpdateIdAccepted != nUpdateIdRejected);
	if (nUpdateIdAccepted == nUpdateIdRejected)
		return ERR_FALSE;

	CLyrHTFile	htFile;

	int nRet = htFile.Open(m_strLyrFile.c_str());
	if (nRet != ERR_OK)
		return nRet;

	int			nCurrentUpdateIdOrg = htFile.GetCurrentUpdateId();

	ListLyrUpdateItem &listUpdates = htFile.GetUpdateHistory();
	for (ListLyrUpdateItem::iterator it = listUpdates.begin();
		it != listUpdates.end(); ++it)
	{
		LyrUpdateItem &item = *it;
		if (item.nUpdateId == nUpdateIdAccepted)
			htFile.ChangeReviewStatus(item, RS_ACCEPTED, szReviewer);
		else if (item.nUpdateId == nUpdateIdRejected)
			htFile.ChangeReviewStatus(item, RS_REJECTED, szReviewer);
		else if (item.nUpdateId >= nStartUpdateId && IsUnReviewed(item.reviewStatus))
			htFile.ChangeReviewStatus(item, RS_REVIEWED, szReviewer);
	}

	CLyrHDFile	hdFile;
	hdFile.Open(m_strLyrFile.c_str());

	if (nCurrentUpdateIdOrg == nUpdateIdRejected
		&& nUpdateIdAccepted == LYR_UPDATE_ID_INVALID)
	{
		// Revert to previous lyrics data?
		nUpdateIdAccepted = htFile.GetCurrentUpdateId();

		// Delete this lyrics file, should go another way.
		ASSERT(!(htFile.GetUpdateHistory().size() == 1 
			&& htFile.GetUpdateHistory().front().bUpload));
	}

	if (nCurrentUpdateIdOrg != nUpdateIdAccepted
		&& nUpdateIdAccepted != LYR_UPDATE_ID_INVALID)
	{
		// Rewrite lyrics data with the "Accepted update".
		CCharBuffer	bufLyrData;
		nRet = hdFile.GetUpdateData(nUpdateIdAccepted, bufLyrData);
		if (nRet != ERR_OK)
			return nRet;

		if (SaveDataAsFile(m_strLyrFile.c_str(), bufLyrData.c_str(), bufLyrData.size()))
			return ERR_OK;
		else
			return ERR_WRITE_FILE;
	}

	return ERR_OK;
}

/*

//
// Accept Review will write updated data to file.
//
int CLyrUpdateHistory::AcceptReview(long nUpdateId)
{
	CLyrHTFile	htFile;

	int nRet = htFile.Open(m_strLyrFile.c_str());
	if (nRet != ERR_OK)
		return nRet;

	nRet = htFile.ChangeReviewStatus(nUpdateId, RS_ACCEPTED);
	if (nRet != ERR_OK)
		return nRet;

	CLyrHDFile	hdFile;
	hdFile.Open(m_strLyrFile.c_str());

	CCharBuffer	bufLyrData;
	nRet = hdFile.GetUpdateData(nUpdateId, bufLyrData);
	if (nRet != ERR_OK)
		return nRet;

	if (SaveDataAsFile(m_strLyrFile.c_str(), bufLyrData.c_str(), bufLyrData.size()))
		return ERR_OK;
	else
		return ERR_WRITE_FILE;
}

int CLyrUpdateHistory::RejectReview(long nUpdateId)
{
	CLyrHTFile	htFile;

	int nRet = htFile.Open(m_strLyrFile.c_str());
	if (nRet != ERR_OK)
		return nRet;

	ReviewStatus	reviewStatus;

	nRet = htFile.GetReviewStatus(nUpdateId, reviewStatus);
	if (nRet != ERR_OK)
		return nRet;
	if (reviewStatus == RS_REJECTED)
		return ERR_OK;

	if (htFile.GetCurrentUpdateId() != nUpdateId)
		return htFile.ChangeReviewStatus(nUpdateId, RS_REJECTED);

	//
	// Restore old lyrics
	//
	nRet = htFile.ChangeReviewStatus(nUpdateId, RS_REJECTED);
	if (nRet != ERR_OK)
		return nRet;

	CLyrHDFile	hdFile;
	hdFile.Open(m_strLyrFile.c_str());

	CCharBuffer	bufLyrData;
	nRet = hdFile.GetUpdateData(htFile.GetCurrentUpdateId(), bufLyrData);
	if (nRet != ERR_OK)
		return nRet;

	if (SaveDataAsFile(m_strLyrFile.c_str(), bufLyrData.c_str(), bufLyrData.size()))
		return ERR_OK;
	else
		return ERR_WRITE_FILE;
}

int CLyrUpdateHistory::GetUnreviewedUpdateHistory(ListLyrUpdateItem &listLyrUpdateItem, bool bIncludingLyrData)
{
	CLyrHTFile	htFile;

	int nRet = htFile.Open(m_strLyrFile.c_str());
	if (nRet != ERR_OK)
		return nRet;

	ListLyrUpdateItem &listUpdate = htFile.GetUpdateHistory();

	for (ListLyrUpdateItem::iterator it = listUpdate.begin(); it != listUpdate.end(); ++it)
	{
		LyrUpdateItem &item = *it;
		if (item.reviewStatus == RS_OPTIONAL || item.reviewStatus == RS_REQUIRED)
			listLyrUpdateItem.push_back(item);
	}

	if (bIncludingLyrData)
		return FillInLyricsData(listLyrUpdateItem);

	return ERR_OK;
}
*/

bool CLyrUpdateHistory::IsUpdateExists(LPCTSTR szMd5)
{
	CLyrHTFile	htFile;

	int nRet = htFile.Open(m_strLyrFile.c_str());
	if (nRet != ERR_OK)
		return false;

	ListLyrUpdateItem &listUpdate = htFile.GetUpdateHistory();

	for (ListLyrUpdateItem::iterator it = listUpdate.begin(); it != listUpdate.end(); ++it)
	{
		LyrUpdateItem &item = *it;
		if (_tcscmp(szMd5, item.strMD5.c_str()) == 0)
			return true;
	}

	return false;
}

int CLyrUpdateHistory::FillInLyricsData(ListLyrUpdateItem &listLyrUpdateItem)
{
	CLyrHDFile	hdFile;

	hdFile.Open(m_strLyrFile.c_str());

	for (ListLyrUpdateItem::iterator it = listLyrUpdateItem.begin();
		it != listLyrUpdateItem.end(); ++it)
	{
		LyrUpdateItem &item = *it;

		int nRet = hdFile.GetUpdateData(item.nUpdateId, item.bufLyrData);
		if (nRet != ERR_OK)
			return nRet;
	}

	return ERR_OK;
}

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(LyricsInfoFiles)

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

class CTestCaseLyricsInfoFiles : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseLyricsInfoFiles);
	CPPUNIT_TEST(Test_CLyrHTFile);
	CPPUNIT_TEST(Test_CLyrHDFile);
	CPPUNIT_TEST_SUITE_END();

protected:
#define		SZ_TEST_LRC_FILE	"12.lrc"

public:
	void setUp()
	{
	}
	void tearDown() 
	{
		tstring			strFile = GetAppResourceDir();
		strFile += SZ_TEST_LRC_FILE;
		DeleteFile(strFile.c_str());
		DeleteFile((strFile + SZ_LYR_HST_EXT).c_str());
		DeleteFile((strFile + SZ_LYR_HD_EXT).c_str());
	}

protected:
	void Test_CLyrHTFile()
	{
		CLyrHTFile		htFile;
		tstring			strFile = GetAppResourceDir();
		strFile += SZ_TEST_LRC_FILE;
		// ReviewStatus	reviewStatus;
		tstring		strLyricsNew = "[id:12]\n[ti:L.O.V.E]\n[ar:Ashlee Simpson dou]\n[al:I Am Me]\n[by:FlowerSpider]\n[00:00.01]L.O.V.E Ashlee Simpson\n[00:04.86]I'm talkin' 'bout love\n[00:10.71]All my girls stand in a ssgcircle New";
#define TEST_UPLOADER	"T-uploader"
#define TEST_UPLOADER2	"T-uploader2"

		CPPUNIT_ASSERT(SaveDataAsFile(strFile.c_str(), strLyricsNew.c_str(), strLyricsNew.size()));

		DeleteFile((strFile + SZ_LYR_HST_EXT).c_str());
		CPPUNIT_ASSERT(htFile.Open(strFile.c_str()) == ERR_OK);

		LyrUpdateItem lyrUpdateItem1(TEST_UPLOADER, RS_OPTIONAL, _T("xxxxMD5xxx"), true);
		LyrUpdateItem lyrUpdateItem2(TEST_UPLOADER2, RS_OPTIONAL, _T("xxxxMD5xxx"), true);
		lyrUpdateItem1.nUpdateId = 1;
		lyrUpdateItem2.nUpdateId = 2;
		CPPUNIT_ASSERT(htFile.AddUpdate(lyrUpdateItem1) == ERR_OK);
		CPPUNIT_ASSERT(htFile.AddUpdate(lyrUpdateItem2) == ERR_OK);

		ListLyrUpdateItem::iterator it = htFile.GetUpdate(1);
		CPPUNIT_ASSERT(it != htFile.GetUpdateHistory().end());
		CPPUNIT_ASSERT(htFile.ChangeReviewStatus(*it, RS_ACCEPTED, _T("test1")) == ERR_OK);

		it = htFile.GetUpdate(2);
		CPPUNIT_ASSERT(it != htFile.GetUpdateHistory().end());
		CPPUNIT_ASSERT(htFile.ChangeReviewStatus(*it, RS_ACCEPTED, _T("test1")) == ERR_OK);

		//
		// Verify data in list is OK
		//
		ListLyrUpdateItem	listUpdate = htFile.GetUpdateHistory();
		CPPUNIT_ASSERT(listUpdate.size() == 2);

		{
			LyrUpdateItem &update = listUpdate.front();
			CPPUNIT_ASSERT(update.nUpdateId == 2);
			CPPUNIT_ASSERT(_tcscmp(update.strUploader.c_str(), TEST_UPLOADER2) == 0);
			CPPUNIT_ASSERT(update.reviewStatus == RS_ACCEPTED);
		}

		{
			LyrUpdateItem &update = listUpdate.back();
			CPPUNIT_ASSERT(update.nUpdateId == 1);
			CPPUNIT_ASSERT(_tcscmp(update.strUploader.c_str(), TEST_UPLOADER) == 0);
			CPPUNIT_ASSERT(update.reviewStatus == RS_ACCEPTED);
		}

		DeleteFile(strFile.c_str());
	}

	void Test_CLyrHDFile()
	{
		CLyrHDFile		hdFile;
		tstring			strFile = GetAppResourceDir();
		strFile += SZ_TEST_LRC_FILE;

		hdFile.Open(strFile.c_str());

		CCharBuffer		strLyr1, strLyr2;
		strLyr1 = "[id:12]\n[ti:L.O.V.E]\n[ar:Ashlee Simpson dou]\n[al:I Am Me]\n[by:FlowerSpider]\n[00:00.01]L.O.V.E Ashlee Simpson\n[00:04.86]I'm talkin' 'bout love\n[00:10.71]All my girls stand in a ssgcircle New";
		strLyr2 = "[id:12]\n[ti:L.O.V.E]\n[ar:Ashlee Simpson dou]\n[al:I Am Me]\n[by:22]\n[00:00.01]2222  Simpson\n[00:04.86]I'm talkin' 'bout love\n[00:10.71]All my girls stand in a ssgcircle New";

		CPPUNIT_ASSERT(hdFile.AddUpdate(1, strLyr1) == ERR_OK);
		CPPUNIT_ASSERT(hdFile.AddUpdate(2, strLyr2) == ERR_OK);

		CCharBuffer		str;
		CPPUNIT_ASSERT(hdFile.GetUpdateData(1, str) == ERR_OK);
		CPPUNIT_ASSERT(strcmp(str.c_str(), strLyr1.c_str()) == 0);

		CPPUNIT_ASSERT(hdFile.GetUpdateData(2, str) == ERR_OK);
		CPPUNIT_ASSERT(strcmp(str.c_str(), strLyr2.c_str()) == 0);

		DeleteFile(strFile.c_str());
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseLyricsInfoFiles);

#endif // _CPPUNIT_TEST
