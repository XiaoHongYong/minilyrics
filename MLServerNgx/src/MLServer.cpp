// MLServer.cpp: implementation of the CMLServer class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLServer.h"
#include "MLHelperFun.h"
#include "LyricsDBMgr.h"
#include "../../MLLib/LrcParserHelper.h"
#include "../../MLLib/LyricsKeywordFilter.h"
#include "MLIDEncrypt.h"
#include "DBUser.h"
#include "LyricsUpdateMgr.h"


//////////////////////////////////////////////////////////////////////////
// Command processors

class CCmdProcessor
{
public:
	CCmdProcessor(CXMLMsgCmd *pMsgCmd, MLRetMsg *pRetMsg, LPCTSTR szCmd)
		{ m_pCmdMsg = pMsgCmd; m_pRetMsg = pRetMsg; m_szCmd = szCmd; m_pServer = NULL; }
	virtual ~CCmdProcessor() { }

	virtual int Init(MLServer *pSession, CSimpleXML &xml)
	{
		m_pServer = pSession;

		if (stricmp(xml.m_strEncoding.c_str(), SZ_UTF8) != 0)
			return ERR_BAD_XMLREQUEST;

		m_pRetMsg->strOrgCmd = xml.m_pRoot->strName;

		int nRet = m_pCmdMsg->FromXML(xml.m_pRoot);
		if (nRet != ERR_OK)
			return nRet;

		return ERR_OK;
	}

	virtual int Process() = 0;

public:
	MLRetMsg		*m_pRetMsg;

protected:
	CXMLMsgCmd		*m_pCmdMsg;
	LPCTSTR			m_szCmd;
	MLServer	*m_pServer;

};

class CCmdProcessorLogin : public CCmdProcessor
{
public:
	CCmdProcessorLogin() : CCmdProcessor(&m_cmd, &m_retMsg, _T("LI")) { }

	virtual int Process()
	{
		// Check user name and password.
		long	nUploaderId;
        int nRet = g_UserDB.LoginUser(m_cmd.strName.c_str(), m_cmd.strPwd.c_str(), nUploaderId);
        m_retMsg.result = nRet;
		return nRet;
	}

protected:
	MLMsgCmdLogin	m_cmd;
	MLMsgRetLogin	m_retMsg;

};


class CCmdProcessorSearchLyr : public CCmdProcessor
{
public:
	CCmdProcessorSearchLyr(bool bV0Protocol = false) : CCmdProcessor(&m_cmd, &m_retMsg, _T("SL"))
	{ m_retMsg.bV0Protocol = bV0Protocol; }

	virtual int Process()
	{
		if (m_retMsg.bV0Protocol)
		{
			// 2013/10/25: Check whether these clients are from ours, some third-party clients uses our server to search lyrics.
			// The rule is that: After 7.1 and later won't use SearchV0 protocol.
			if (m_cmd.strClient.empty())
				return ERR_OK;

			VersionNumber obsolte = VersionNumMake(7, 1, 0);
			VecString vClientInfo;
			VersionNumber ver = obsolte;
			StrSplit(m_cmd.strClient.c_str(), ' ', vClientInfo);
			if (vClientInfo.size() == 1 && StrIsISame(vClientInfo[0].c_str(), _T("MiniLyricsForAndroid")))
				ver = 0;
			else if (vClientInfo.size() > 1 && StrIsISame(vClientInfo[0].c_str(), _T("MiniLyrics")))
			{
				ver = VersionNumFrom(vClientInfo[1].c_str());
			}
			if (ver >= obsolte)
			{
				RetLyrInfo item;
				item.strArtist = "MiniLyrics";
				item.strTitle = "Please upgrade your MiniLyrics.";
				item.strLink = "http://www.google.com/";
				m_retMsg.listResultFiles.push_back(item);
				return ERR_OK;
			}
			m_retMsg.strMessage = "Please update your MiniLyrics to search lyrics.";

			return ERR_NOT_FOUND;
		}

		return m_pServer->ProcessSearchCmd(m_cmd, m_retMsg);
	}

protected:
	MLMsgCmdSearch	m_cmd;
	MLMsgRetSearch	m_retMsg;

};

class CCmdProcessorCanUpload : public CCmdProcessor
{
public:
	CCmdProcessorCanUpload() : CCmdProcessor(&m_cmd, &m_retMsg, _T("CU"))
	{ m_retMsg.bV0Protocol = true; }

	virtual int Process()
	{
		return ERR_NOT_FOUND;
	}

protected:
	MLMsgCmdSearch	m_cmd;
	MLMsgRetSearch	m_retMsg;

};

class CCmdProcessorBatchSearch : public CCmdProcessor
{
public:
	CCmdProcessorBatchSearch() : CCmdProcessor(&m_cmd, &m_retMsg, _T("BS")) { }

	virtual int Process()
	{
		return m_pServer->ProcessBatchSearchCmd(m_cmd, m_retMsg);
	}

protected:
	MLMsgCmdBatchSearch		m_cmd;
	MLMsgRetBatchSearch		m_retMsg;

};

class CCmdProcessorUploadV0 : public CCmdProcessor
{
public:
	CCmdProcessorUploadV0() : CCmdProcessor(&m_cmd, &m_retMsg, _T("UL"))
	{ m_retMsg.bV0Protocol = true; }

	virtual int Process()
	{
		return m_pServer->ProcessUploadCmdV0(m_cmd, m_retMsg);
	}

protected:
	MLMsgCmdUploadV0		m_cmd;
	MLMsgRetUploadV0		m_retMsg;

};

class CCmdProcessorUpload : public CCmdProcessor
{
public:
	CCmdProcessorUpload() : CCmdProcessor(&m_cmd, &m_retMsg, _T("U2")) { }

	virtual int Process()
	{
		return m_pServer->ProcessUploadCmd(m_cmd, m_retMsg);
	}

protected:
	MLMsgCmdUpload		m_cmd;
	MLMsgRetUpload		m_retMsg;

};

class CCmdProcessorActivate : public CCmdProcessor
{
public:
	CCmdProcessorActivate() : CCmdProcessor(&m_cmd, &m_retMsg, _T("AT")) { }

	virtual int Process()
	{
		return m_pServer->ProcessActivateLicenseCmd(m_cmd, m_retMsg);
	}

protected:
	MLMsgCmdActivateLicense		m_cmd;
	MLMsgRetActivateLicense		m_retMsg;

};

//////////////////////////////////////////////////////////////////////////

bool ParseArTiFormat(LPCSTR szArTi, string &strArtist, string &strTitle)
{
	LPCSTR		szPos;

	szPos = strstr(szArTi, " - ");
	if (szPos)
	{
		strTitle = strArtist = "";
		strArtist.append(szArTi, szPos);
		strTitle = szArTi + 3;

		return true;
	}

	return false;
}

void RetLyrInfoFromLyricsInfo(RetLyrInfo &file, CLyricsInfo	*plrcInfo)
{
	stringex		strTemp;

	file.nTimeLength = plrcInfo->wMediaLength;

	// 把链接转换为%XX%XX的链接
	UriQuote(plrcInfo->GetRelatedHttpLink(), file.strLink);

	if (plrcInfo->nRateCount)
	{
		file.fRate = (float)((double)plrcInfo->nRateTotal / plrcInfo->nRateCount);
		file.nRateCount = plrcInfo->nRateCount;
	}
	else
	{
		file.fRate = 0.0;
		file.nRateCount = 0;
	}
	file.nDownloads = plrcInfo->nDlCount;

	// direct
	file.strArtist = plrcInfo->strAr;
	file.strAlbum = plrcInfo->strAl;
	file.strTitle = plrcInfo->strTi;
	file.strUploader = plrcInfo->strEditedBy;
}

tstring GetProfileDir(LPCTSTR szValueName, LPCTSTR szDefaultSubDir)
{
	tstring strDir = g_Profile.GetString(szValueName, _T(""));
	if (strDir.empty())
	{
		strDir = DirStringJoint(GetAppResourceDir(), szDefaultSubDir);
		g_Profile.WriteString(szValueName, strDir.c_str());
	}

	return strDir;
}

//////////////////////////////////////////////////////////////////////////
// MLServer

MLServer::MLServer() : m_spamFilter("mlserver-") {
    srand((int)time(NULL));
}

MLServer::~MLServer()
{
}

int MLServer::Init(const MLServerNgxConf *conf)
{
    string dataPath;
    
    assert(conf->configDir.data != NULL);
    dataPath.assign((const char *)conf->configDir.data, conf->configDir.len);
    DirStringAddSlash(dataPath);
    
    assert(conf->lyricsDir.data != NULL);
    m_lyricsDir.assign((const char *)conf->lyricsDir.data, conf->lyricsDir.len);
    DirStringAddSlash(m_lyricsDir);
    
    assert(conf->relatedUploadDir.data != NULL);
    m_relatedUploadDir.assign((const char *)conf->relatedUploadDir.data, conf->relatedUploadDir.len);
    DirStringAddSlash(m_relatedUploadDir);
    
    assert(conf->lyricsHttpLinkBase.data != NULL);
    m_lyricsHttpLinkBase.assign((const char *)conf->lyricsHttpLinkBase.data, conf->lyricsHttpLinkBase.len);
    
    g_LyricsDBMgr.init(m_lyricsDir, m_relatedUploadDir);
    
    m_spamFilter.Load((dataPath + "data/").c_str());
    
    CLyricsKeywordFilter::Init((dataPath + "data/").c_str());

    string fn = dataPath + "/database/lyrics.db";
    int nRet = g_LyricsDB.Init(dataPath);
    if (nRet != ERR_OK) {
        printf("Failed to init lyrics db");
    }
    
    fn = dataPath + "/database/users.db";
    nRet = g_UserDB.Init(fn.c_str());
    if (nRet != ERR_OK) {
        printf("Failed to init users db");
    }
    
    CMLPacketAccountMgr *pMgr = CMLPacketAccountMgr::GetInstance();
    pMgr->AddAccount(4, "Mlv1clt4.0");
    pMgr->m_packFlag = MPV_V1_MD5_ID;
    
    return ERR_OK;
}


class CLyricsInfoSortCmp
{
public:
	bool operator()(const CLyricsInfo *item1, const CLyricsInfo *item2)
	{
		return _stricmp(item1->strTiCmp.c_str(), item2->strTiCmp.c_str()) < 0;
	}
};

#define ITEMS_PER_PAGE        50

int MLServer::ProcessSearchCmd(MLMsgCmdSearch &cmdSearch, MLMsgRetSearch &retSearch)
{
	retSearch.strServerUrl = m_lyricsHttpLinkBase;

	if (cmdSearch.strArtist.empty() && cmdSearch.strTitle.empty())
		return ERR_CMD_PARAM;

	int nRet = VerifyDeviceLicenseInfo(cmdSearch, retSearch);

	stringex strArCmp, strTiCmp;

	CLyricsKeywordFilter::Filter(cmdSearch.strArtist.c_str(), strArCmp);
	CLyricsKeywordFilter::Filter(cmdSearch.strTitle.c_str(), strTiCmp);

	if (strArCmp.size() && strTiCmp.size()) {
        // Only return matched lyrics? Artist and title are both match
		if (SearchMatchedLyricsOnly(strArCmp.c_str(), strTiCmp.c_str(), retSearch.listResultFiles) == ERR_OK)
			return ERR_OK;
	}

	if (strTiCmp.empty()) {
		// search by artist
		nRet = g_LyricsDB.SearchLyricsByArtist(strArCmp.c_str(), retSearch.listResultFiles);
	} else {
		// search by title
		nRet = g_LyricsDB.SearchLyricsByTitle(strTiCmp.c_str(), retSearch.listResultFiles);
	}

	if (retSearch.listResultFiles.empty())
		nRet = ERR_NOT_FOUND;
	else
		nRet = ERR_OK;
    
    return nRet;
}

int MLServer::ProcessBatchSearchCmd(MLMsgCmdBatchSearch &cmdSearch, MLMsgRetBatchSearch &retSearch)
{
	int nRet = VerifyDeviceLicenseInfo(cmdSearch, retSearch);
	if (cmdSearch.bOnlyActiveCanDownload && retSearch.licenseStatus != LS_ACTIVE)
		return ERR_REQUIRE_ACTIVE_LICENSE;

	CColonSeparatedValues csv;
	int i = 0;
	for (MLListSearchItems::iterator it = cmdSearch.listSearchItems.begin();
		it != cmdSearch.listSearchItems.end() && i < 50; ++it, ++i)
	{
		MLSearchItem &item = *it;
		MLLyricsInfoLite infoLite;

		int nRet = SearchBestMatchLyrics(item.strArtist.c_str(), item.strAlbum.c_str(),
			item.strTitle.c_str(), item.nMediaLength, infoLite);

		if (nRet != ERR_OK)
			infoLite.strFile.clear();
		retSearch.listLyricsInfo.push_back(infoLite);

		// SearchRet Code, Time cost, artist, title.
		csv.clear();
		csv.AddValue(item.strArtist.c_str());
		csv.AddValue(item.strTitle.c_str());
	}

	ASSERT(retSearch.listLyricsInfo.size() == cmdSearch.listSearchItems.size());

	return ERR_OK;
}

int MLServer::SearchMatchedLyricsOnly(LPCSTR szArCmp, LPCSTR szTiCmp, RetLyrInfoList &listLyr)
{
	int nRet = g_LyricsDB.SearchLyricsByArtistTitle(szArCmp, szTiCmp, listLyr);
	if (nRet != ERR_OK)
		return nRet;

	if (listLyr.empty())
		return ERR_NOT_FOUND;

	return ERR_OK;
}

int MLServer::SearchBestMatchLyrics(LPCSTR szArtist, LPCTSTR szAlbum, LPCSTR szTitle, int nMediaLength, MLLyricsInfoLite &infoLite)
{
	int				nRet = ERR_OK;
	stringex		strArtistCmp;
	stringex		strAlbumCmp;
	stringex		strTitleCmp;

	CLyricsKeywordFilter::Filter(szArtist, strArtistCmp);
	CLyricsKeywordFilter::Filter(szAlbum, strAlbumCmp);
	CLyricsKeywordFilter::Filter(szTitle, strTitleCmp);

	RetLyrInfoList	vLyrics;

	nRet = g_LyricsDB.SearchLyricsByArtistTitle(strArtistCmp.c_str(),
		strTitleCmp.c_str(), vLyrics);
	if (nRet != ERR_OK)
		return nRet;

	RetLyrInfo	*bestMatch = NULL;
	float		nBestMatchValue = 0.0;
	stringex	strAlbumCmp2;

    for (auto it = vLyrics.begin(); it != vLyrics.end(); ++it) {
		RetLyrInfo &info = *it;
		float nMatchValue = 60;
		if (info.contentType == LCT_TXT)
			nMatchValue = 60;
		else if (info.contentType == LCT_LRC)
			nMatchValue = 70;

		// increase rating
		if (info.nRateCount > 0) {
			float fRate = info.fRate;
			nMatchValue += fRate - 3;
			if (info.nRateCount > 5)
			{
				if (fRate >= 4.8)
					nMatchValue++;
				else if (fRate <= 3.0)
					nMatchValue--;
			}
			nMatchValue += info.nRateCount / (float)1000.0;
		}

		// Media Length equal?
		if (nMediaLength > 0 && abs(info.nTimeLength - nMediaLength) <= 3)
			nMatchValue += 1.0;

		// Album equal?
		CLyricsKeywordFilter::Filter(info.strAlbum.c_str(), strAlbumCmp2);
		if (_tcscmp(strAlbumCmp2.c_str(), strAlbumCmp.c_str()) == 0)
			nMatchValue += 1.0;

		if (nMatchValue > nBestMatchValue)
		{
			nBestMatchValue = nMatchValue;
			bestMatch = &info;
		}
	}

    if (!bestMatch) {
		return ERR_NOT_FOUND;
    }

	infoLite.strFile = m_lyricsDir;
	infoLite.strFile += bestMatch->strLink;
	strrep(infoLite.strFile, '/', '\\');

	// Set the file name to client, it used to save as file name.
	infoLite.strSaveName = bestMatch->strArtist;
	if (infoLite.strSaveName.size())
		infoLite.strSaveName += _T(" - ");
	infoLite.strSaveName += bestMatch->strTitle;
	infoLite.strSaveName = FileNameFilterInvalidChars(infoLite.strSaveName.c_str());
	infoLite.strSaveName += FileGetExt(bestMatch->strLink.c_str());

	return ERR_OK;
}

int MLServer::Process(uint8_t *data, size_t len, NgxOutputStream &outStream)
{
	CCharBuffer	buf;
    buf.append((char *)data, len);

    int ret = m_packetWrapper.Unwrapp(buf);
    if (ret != ERR_OK) {
        return ret;
    }

    CSimpleXML xml;
    if (!xml.ParseData(buf.data(), buf.size()))
        return ERR_PARSE_XML;

    MLMsgCmd msgCmd = CMLProtocol::GetMsgCommand(xml);
	CCmdProcessor *pCmdProcessor = NULL;
	switch (msgCmd) {
	case MC_LOGIN:
		pCmdProcessor = new CCmdProcessorLogin();
		break;
	case MC_SEARCH_V0:
		pCmdProcessor = new CCmdProcessorSearchLyr(true);
		break;
	case MC_SEARCH:
		pCmdProcessor = new CCmdProcessorSearchLyr();
		break;
	case MC_BATCH_SEARCH:
		// New version of upload lyrics command (since 7.1 or later)
		pCmdProcessor = new CCmdProcessorBatchSearch();
		break;
	case MC_CAN_UPLOAD:
		pCmdProcessor = new CCmdProcessorCanUpload();
		break;
	case MC_UPLOADV0:
		pCmdProcessor = new CCmdProcessorUploadV0();
		break;
	case MC_UPLOAD:
		// New version of upload lyrics command (since 7.1 or later)
		pCmdProcessor = new CCmdProcessorUpload();
		break;
	case MC_ACTIVATE:
		// Activate license command (since 7.1 or later)
		pCmdProcessor = new CCmdProcessorActivate();
		break;
	default:
		ERRLOG1(_T("unsupported command(%d) from client"), msgCmd);
		return ERR_FALSE;
	}

	ASSERT(pCmdProcessor);
	pCmdProcessor->Init(this, xml);

	ret = pCmdProcessor->Process();
	pCmdProcessor->m_pRetMsg->result = ret;

	CMLBinXMLWriter	xmlStream;
	pCmdProcessor->m_pRetMsg->ToXML(xmlStream);

	CCharBuffer &bufXml = xmlStream.GetBuffer();
    ret = m_packetWrapper.Wrapp(bufXml);

    outStream.write(bufXml.c_str(), bufXml.size());

	delete pCmdProcessor;

	return ret;
}


bool RemoveEncodingTagValue(string &strContentUtf8)
{
	// Remove LRC tag
	string	strPropName = SZ_LRC_ENCODING;
	int		nValPosBeg, nValPosEnd;

	if (SearchLrcPropTag(strContentUtf8.c_str(), strPropName, nValPosBeg, nValPosEnd))
	{
		nValPosBeg -= strPropName.size() + 1;
		nValPosEnd++;
	}
	else
	{
		strPropName = SZ_TXT_ENCODING;
		if (!SearchTxtPropTag(strContentUtf8.c_str(), strPropName, nValPosBeg, nValPosEnd))
			return false;
		nValPosBeg -= strPropName.size();
	}

	if (strContentUtf8[nValPosEnd] == '\n')
	{
		nValPosEnd++;
		if (strContentUtf8[nValPosEnd] == '\r')
			nValPosEnd++;
	}
	else if (strContentUtf8[nValPosEnd] == '\r' && strContentUtf8[nValPosEnd + 1] == '\n')
		nValPosEnd += 2;

	ASSERT(nValPosBeg >= 0 && nValPosEnd <= strContentUtf8.size());
	strContentUtf8.erase(nValPosBeg, nValPosEnd - nValPosBeg);

	return true;
}

int ConvertLyrToUtf8(LPBYTE byLyrData, int nLyrDataLen, string &strContentUtf8)
{
	CHAR_ENCODING encoding = GetLyricsTextEncoding(byLyrData, nLyrDataLen);

	if (encoding == ED_UNICODE)
		UCS2ToUtf8((LPCWSTR)byLyrData, nLyrDataLen / sizeof(WCHAR), strContentUtf8);
	else if (encoding == ED_UNICODE_BIG_ENDIAN)
	{
		UCS2BEToLE((LPWSTR)byLyrData, nLyrDataLen / sizeof(WCHAR));
		UCS2ToUtf8((LPCWSTR)byLyrData, nLyrDataLen / sizeof(WCHAR), strContentUtf8);
	}
	else if (encoding == ED_UTF8)
	{
		strContentUtf8.clear();
		strContentUtf8.append((LPCSTR)byLyrData, nLyrDataLen);
	}
	else
	{
		MbcsToUtf8((LPCSTR)byLyrData, nLyrDataLen, strContentUtf8, encoding);
		// Convert back to MBCS, make sure lyrics are converted correctly.
		stringex strMBCSVerify;
		Utf8ToMbcs(strContentUtf8.c_str(), strContentUtf8.size(), strMBCSVerify, encoding);
		if (memcmp(strMBCSVerify.c_str(), byLyrData, nLyrDataLen) != 0)
			return ERR_OBSOLETE_CLIENT_VER;
	}

	// Remove [encoding: ] tag
	RemoveEncodingTagValue(strContentUtf8);

	return ERR_OK;
}

int ConvertOldUploadLyrToUtf8(tstring &strContentBase64, string &strContentUtf8)
{
	string		strContent;
	if (strContentBase64.size() < 10)
		return ERR_BAD_FILE_CONTENT;

	if (!decodebase64(strContentBase64.c_str(), strContentBase64.size(), strContent))
		return ERR_BAD_BASE64CONTENT;

	return ConvertLyrToUtf8((LPBYTE)strContent.c_str(), strContent.size(), strContentUtf8);
}

int MLServer::ProcessUploadCmdV0(MLMsgCmdUploadV0 &cmdUpload, MLMsgRetUploadV0 &retMsg)
{
    retMsg.strMessage = _T("Failed to Upload lyrics, please visit http://www.crintsoft.com to upgrade your MiniLyrics.");
    return ERR_OBSOLETE_CLIENT_VER;
}

int MLServer::ProcessUploadCmd(MLMsgCmdUpload &cmdUpload, MLMsgRetUpload &retMsg)
{
	// Get user ID
	long	nUploaderId;
	int nRet = g_UserDB.LoginUser(cmdUpload.strLoginName.c_str(), cmdUpload.strPwdMask.c_str(), nUploaderId);
	if (nRet != ERR_OK)
		return nRet;

	return ProcessUploadCmd(cmdUpload.strFileContent, retMsg, nUploaderId, cmdUpload.strLoginName.c_str(), retMsg.strLyricsId);
}

int MLServer::ProcessUploadCmd(tstring &strLyrContent, MLMsgRetUploadV0 &retMsg,
									long nUploaderId, LPCTSTR szUploader, tstring &strLyrId)
{
	int				nRet;
	CLrcTag			tag(LTT_ALL);

	// strLyrContent is in UTF-8 encoding
	tag.SetEncoding(ED_UTF8);
	if (!tag.ReadFromText(strLyrContent.c_str(), (int)strLyrContent.size()))
		return ERR_LYR_NO_TAG_FOUND;
	if (tag.m_strTitle.empty())
		return ERR_LYR_NO_TAG_FOUND;

	// Check it with spam filter
	{
		int			nLevel;
		if (m_spamFilter.IsNameFiltered(tag.m_strArtist.c_str(), tag.m_strAlbum.c_str(),
			tag.m_strTitle.c_str(), NULL, nLevel, &retMsg.strMessage))
			return ERR_BAD_FILE_CONTENT;
		if (m_spamFilter.IsContentFiltered(tag.m_lyrContentType, FE_UTF8, strLyrContent.c_str(),
			(int)strLyrContent.size(), nLevel, &retMsg.strMessage))
			return ERR_BAD_FILE_CONTENT;
	}

	// If lyrics exists, return.
	uint nOrgLyricsId = 0;
	bool bRet = g_LyricsDBMgr.IsLyricsExists(tag, tag.GetMD5(), (int)nUploaderId, &nOrgLyricsId);
	if (bRet)
	{
		RetLyrInfo info;
		info.strArtist = tag.m_strArtist;
		info.strTitle = tag.m_strTitle;
		retMsg.listResultFiles.push_back(info);
		return ERR_UPLOAD_EXIST;
	}

	if (tag.m_strId.size())
		nOrgLyricsId = MLIDDecrypt(tag.m_strId.c_str());
	if (nOrgLyricsId != 0)
	{
		// Lyrics ID exist, check for updating.
        stringex strArtistCmp, strTitleCmp;

        CLyricsKeywordFilter::Filter(tag.m_strArtist.c_str(), strArtistCmp);
        CLyricsKeywordFilter::Filter(tag.m_strTitle.c_str(), strTitleCmp);

		RetLyrInfo lyrInfo;
        nRet = g_LyricsDB.SearchLyricsByArtistTitleID(strArtistCmp.c_str(), strTitleCmp.c_str(), nOrgLyricsId, lyrInfo);
        if (nRet == ERR_OK && lyrInfo.contentType == tag.m_lyrContentType) {
            // Enter here to correct lyrics.
            MLIDEncrypt(nOrgLyricsId, strLyrId);

            if (lyrInfo.uploaderID == nUploaderId) {
                // Update lyrics.
                nRet = g_LyricsDBMgr.UpdateLyrics(lyrInfo, tag, strLyrContent, szUploader);
                return nRet;
            }
            else
            {
                // Now we don't have time to review lyrics,
                // Just make it as a new upload, but we record which lyrics id it from
                // continue execute will be new upload
                if (false)
                {
                    // This lyrics are updated by another user, review is required.
//                    tstring    strFile;
//                    lyrInfo.GetFile(strFile);
//
//                    CCharBuffer    bufLyrData;
//                    ConvertUtf8ToProperFileData(strLyrContent.c_str(), strLyrContent.size(), bufLyrData);
//                    LyrUpdateItem item(szUploader, RS_REQUIRED, tag.GetMD5(), false);
//                    nRet = g_lyrUpdateMgr.AddUpdate(lyrInfo.nUniID,
//                        strFile.c_str(), item, bufLyrData);
//
//                    retMsg.strMessage = _T("Thank you for correct these lyrics, we will review your uploading.");

                    return nRet;
                }
            }
		}
	}

	//
	// This is a new upload of lyrics.
	//
	long			nLyricsId = 0;
	nRet = g_LyricsDBMgr.AddNewUploadedLyrics(nUploaderId, szUploader, tag, strLyrContent, nLyricsId, nOrgLyricsId);
	if (nRet != ERR_OK)
		return nRet;

	MLIDEncrypt((uint32_t)nLyricsId, strLyrId);

	return ERR_OK;
}

int MLServer::VerifyDeviceLicenseInfo(CXMLMsgCmd &cmd, MLRetMsg &retMsg)
{
//    if (cmd.strRegCode.empty())
//        return ERR_OK;
//
//    if (cmd.strDeviceID.empty())
//    {
//        // Old version of verify license code
//        LicenseCodeInfo info;
//        int nRet = g_licenseDB.GetCodeInfo(cmd.strRegCode.c_str(), info);
//            if (nRet != ERR_OK)
//            retMsg.licenseStatus = LS_TRIAL;
//        else
//            retMsg.licenseStatus = info.licenseStatus;
//        return ERR_OK;
///*        if (g_LyricsDB.IsBadRegCode(cmd.strRegCode.c_str()) == ERR_NOT_FOUND)
//        {
//            if (cmd.strRegCode.size() == 16)
//            {
//                // Verify Registration code
//                if (g_LyricsDB.VerifyRegCode(cmd.strRegCode.c_str()) == ERR_NOT_FOUND)
//                    retMsg.licenseStatus = LS_TRIAL;
//            }
//        }
//        else
//            retMsg.licenseStatus = LS_TRIAL;
//        return ERR_OK;
//        */
//    }
//
//    LicenseCodeInfo    licenseCodeInfo;
//    int nRet = g_licenseDB.VerifyDeviceLicenseCode(cmd.strDeviceID.c_str(), cmd.strRegCode.c_str(), licenseCodeInfo);
//    if (nRet != ERR_OK)
//    {
//        licenseCodeInfo.licenseStatus = LS_TRIAL;
//        return nRet;
//    }
//
//    retMsg.licenseStatus = licenseCodeInfo.licenseStatus;
//    if (licenseCodeInfo.licenseStatus != LS_TRIAL)
//    {
//        retMsg.strCodeStartDate = licenseCodeInfo.dateStart.ToString();
//        retMsg.nLicensedUser = licenseCodeInfo.nLicensedUsers;
//        retMsg.nCodeDurationDays = licenseCodeInfo.nDurationDays;
//    }

	return ERR_OK;
}

int MLServer::ProcessActivateLicenseCmd(MLMsgCmdActivateLicense &cmd, MLMsgRetActivateLicense &retMsg)
{
//    LicenseCodeInfo    licenseCodeInfo;
//    int nRet = g_licenseDB.Activate(cmd.strDeviceID.c_str(), cmd.strRegCode.c_str(), licenseCodeInfo);
//    if (nRet != ERR_OK)
//    {
//        licenseCodeInfo.licenseStatus = LS_TRIAL;
//        return nRet;
//    }
//
//    retMsg.licenseStatus = licenseCodeInfo.licenseStatus;
//    if (licenseCodeInfo.licenseStatus != LS_TRIAL)
//    {
//        retMsg.strCodeStartDate = licenseCodeInfo.dateStart.ToString();
//        retMsg.nLicensedUser = licenseCodeInfo.nLicensedUsers;
//        retMsg.nCodeDurationDays = licenseCodeInfo.nDurationDays;
//    }

    retMsg.licenseStatus = LS_ACTIVE;
    retMsg.strCodeStartDate = "2019-10-01";
    retMsg.nLicensedUser = 3;
    retMsg.nCodeDurationDays = 0;

	return ERR_OK;
}
