// MLProtocol.cpp: implementation of the CMLProtocol class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLProtocol.h"


/***********************************************************************
History of MLProtocol:

ML 3.0:			Old XML message format, encoding: gb2312(system default).

ML 3.1 ~ 3.9:	HTTP tunnel XML message format, encoding: gb2312(system default).
				Deprecated: Protocol Version: "0.9"

ML 4.0 ~ 7.0:	HTTP tunnel Encrypted XML message format, encoding: UTF-8

ML 7.1 ~ :		HTTP tunnel Encrypted Binary XML message format, encoding: UTF-8
2011.03			support batch download, ML Binary XML message format

/***********************************************************************/

#define SZ_ML_POST_URL		"/searchlyrics.htm"

#define SZ_DATA_BLOCK_SIGN		"\0$"
#define LEN_DATA_BLOCK_SIGN		2

static ID_TO_STRING	__ErrID2Str[] = 
{
	{ ERR_UNKNOWN, _T("Unknown Error.")},			// 未翻译出来的结果
	{ ERR_BAD_XMLREQUEST, _T("Invalid xml request.")},
	{ ERR_CORRUPTTED_MSG, _T("The message is corrupted, maybe some data get currupted while receiving.")},

	{ ERR_UPLOAD_EXIST, _T("The uploaded lyrics already exist.") },	// 上传的内容已经存在
	{ ERR_USER_NOT_EXIST, _T("User name doesn't exist.") },	// 用户不存在
	{ ERR_BAD_USERPWD, _T("Wrong user name or password.") },		// 用户名或者密码错误
	{ ERR_BAD_CLIENT_VER, _T("Invliad client version.") },	// 客户端版本不正确, 无法登陆
	{ ERR_OBSOLETE_CLIENT_VER, _T("Obsoleted client. please download the latest version.") },	// 客户端版本太低, 无法登陆
	// { ERR_TOO_BUSY, _T("Server too busy, please try again later.") },		// 服务器太忙,暂时无法响应请求
	{ ERR_ACCESS_DENIED, _T("Access denied.") },	// 无权访问,被拒绝
	{ ERR_CMD_NOT_MATCH, _T("Received return command not match.") },
	{ ERR_CMD_PARAM, _T("Invalid message command parameter.") },
	{ ERR_UPLOAD_FILETYPE, _T("The file type can not be uploaded.") },	// 上传的文件类型错误
	{ ERR_BAD_BASE64CONTENT, _T("Incorrect base64 encoded content.") },
	{ ERR_BAD_FILE_CONTENT, _T("The file format is incorrect or contains spam information.") },
	{ ERR_NOT_AUTHORIZED, _T("Unauthorized action.") },
	{ ERR_NOT_LOGIN, _T("Not signed in, please sign in.") },
	{ ERR_LYR_NO_TAG_FOUND, _T("No lyrics tag is found in the lyrics.") },
	{ ERR_REQUIRE_ACTIVE_LICENSE, _T("An active license is required.") },

	{ 0, NULL}
};

static RegErr2Str	__addErr2Str(__ErrID2Str);

ID_TO_STRING	__MlCmdIDStr[] = 
{
	{ MC_LOGIN, MZ_LOGIN },
	{ MC_SEARCH, MZ_SEARCH },
	{ MC_BATCH_SEARCH, MZ_BATCH_SEARCH },
	{ MC_CAN_UPLOAD, MZ_CANUPLOAD},
	{ MC_SEARCH_V0, MZ_SEARCH_V0 },
	{ MC_UPLOADV0, MZ_UPLOADV0 },
	{ MC_UPLOAD, MZ_UPLOAD },
	{ MC_ACTIVATE, MZ_ACTIVATE },
	{ MC_GET_LYR_UPDATE, MZ_GET_LYR_UPDATE },
	{ MC_SUBMIT_LYR_REVIEW, MZ_SUBMIT_LYR_REVIEW },
	{ MC_EXE_SVR_CMD, MZ_EXE_SVR_CMD },
	{ MC_UNKNOWN, NULL},
};

MLMsgCmd MLMsgStr2Cmd(LPCTSTR szMLMsgCmd)
{
	return (MLMsgCmd)StringToID(__MlCmdIDStr, szMLMsgCmd, MC_UNKNOWN);
}

LPCTSTR MLMsgCmd2Str(MLMsgCmd msgCmd)
{
	return IDToString(__MlCmdIDStr, msgCmd, _T("Not found"));
}

ID_TO_STRING	__MlCmdResult2Str[] = 
{
	{ ERR_OK, _T("OK") },								// 0
	{ ERR_FAILED, _T("FAILED") },						// 1035
	{ ERR_BAD_XMLREQUEST, _T("BAD_XML_REQUEST") },	// 1036
	{ ERR_UPLOAD_EXIST, _T("UPLOAD_EXIST") },			// 1042
	{ ERR_USER_NOT_EXIST, _T("USER_NOT_FOUND") },		// 1043
	{ ERR_BAD_USERPWD, _T("BAD_USERPWD") },			// 1045
	{ ERR_BAD_CLIENT_VER, _T("BAD_CLIENT_VER") },		// 1047
	{ ERR_OBSOLETE_CLIENT_VER, _T("OBSOLETE_CLIENT_VER") },	// 1048
	{ ERR_NOT_FOUND, _T("NOT_FOUND") },				// 1049
	{ ERR_ACCESS_DENIED, _T("ACCESS_DENIED") },		// 1051
	{ ERR_CMD_NOT_MATCH, _T("CMD_NOT_MATCH") },		// 1053	
	{ ERR_CMD_PARAM, _T("BAD_CMD_PARAM") },			// 1054	
	{ ERR_UPLOAD_FILETYPE, _T("UPLOAD_FILETYPE") },	// 1056	
	{ ERR_BAD_BASE64CONTENT, _T("BAD_BASE64CONTENT") },	// 1058
	{ ERR_BAD_FILE_CONTENT, _T("BAD_FILE_CONTENT") },	// 1059
	{ ERR_NOT_AUTHORIZED, _T("NOT_AUTHORIZED") },
	{ ERR_NOT_LOGIN, _T("NOT_LOGIN") },
	{ ERR_LYR_NO_TAG_FOUND, _T("LYR_NO_TAG_FOUND") },
	{ ERR_REQUIRE_ACTIVE_LICENSE, _T("REQUIRE_ACTIVE_LICENSE") },
	{0, NULL},
};

int CXMLMsgCmd::FromXML(SXNode *pXmlNode)
{
	strClient = pXmlNode->GetPropertySafe(_T("client"));
	strRegCode = pXmlNode->GetPropertySafe(_T("RegCode"));
	strDeviceID = pXmlNode->GetPropertySafe(_T("did"));
	bOnlineVerified = Tobool(pXmlNode->GetPropertyInt(_T("ov")));
	nProductId = pXmlNode->GetPropertyInt(_T("pid"), PRODUCT_ID_MINILYRICS);
	ASSERT(nProductId != PRODUCT_ID_INVALID);

	return ERR_OK;
}

void CXMLMsgCmd::ToXMLAttribute(CXMLWriter &xmlStream)
{
	xmlStream.WriteAttribute(_T("client"), strClient.c_str());
	if (strRegCode.size())
		xmlStream.WriteAttribute(_T("RegCode"), strRegCode.c_str());
	if (strDeviceID.size())
		xmlStream.WriteAttribute(_T("did"), strDeviceID.c_str());
	if (bOnlineVerified)
		xmlStream.WriteAttribute(_T("ov"), bOnlineVerified);
	if (nProductId != PRODUCT_ID_MINILYRICS)
		xmlStream.WriteAttribute(_T("pid"), nProductId);
}

//////////////////////////////////////////////////////////////////////////
int MLMsgCmdLogin::FromXML(SXNode *pXmlNode)
{
	CXMLMsgCmd::FromXML(pXmlNode);

	strName = pXmlNode->GetPropertySafe(_T("user"));
	strPwd = pXmlNode->GetPropertySafe(_T("password"));

	return ERR_OK;
}

int MLMsgCmdLogin::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_LOGIN);
	xmlFile.WriteAttribute(_T("user"), strName.c_str());
	xmlFile.WriteAttribute(_T("password"), strPwd.c_str());
	CXMLMsgCmd::ToXMLAttribute(xmlFile);
	xmlFile.WriteEndElement();
	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
//
int MLRetMsg::FromXML(SXNode *pXmlNode)
{
	strOrgCmd = pXmlNode->GetPropertySafe(MZ_ORG_CMD);
	LPCTSTR		szValue = pXmlNode->GetProperty(MZ_CMD_RESULT);
	if (!szValue)
		return ERR_CMD_PARAM;

	result = StringToID(__MlCmdResult2Str, szValue, ERR_UNKNOWN);

	strMessage = pXmlNode->GetPropertySafe(_T("msg"));
	if (strMessage.empty())
		strMessage = pXmlNode->GetPropertySafe(_T("message"));

	licenseStatus = (LicenseStatus)pXmlNode->GetPropertyInt(_T("badrc"), LS_TRIAL);
	if (licenseStatus != LS_TRIAL)
	{
		strCodeStartDate = pXmlNode->GetPropertySafe(_T("ls_sd"));
		nLicensedUser = pXmlNode->GetPropertyInt(_T("ls_lu"), 1);
		nCodeDurationDays = pXmlNode->GetPropertyInt(_T("ls_dd"), LICENSE_DURATION_PERPETUAL);
	}

	return ERR_OK;
}

int MLRetMsg::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_RETURN);
	ToXMLAttribute(xmlFile);
	xmlFile.WriteEndElement();
	return ERR_OK;
}

void MLRetMsg::ToXMLAttribute(CXMLWriter &xmlFile)
{
	xmlFile.WriteAttribute(MZ_ORG_CMD, strOrgCmd.c_str());
	xmlFile.WriteAttribute(MZ_CMD_RESULT, IDToString(__MlCmdResult2Str, result, _T("UNTRANSLATED")));

	if (strMessage.size())
	{
		xmlFile.WriteAttribute(_T("msg"), strMessage.c_str());		// msg is for search and upload
		xmlFile.WriteAttribute(_T("message"), strMessage.c_str());	// message is for login
	}

	if (licenseStatus != LS_TRIAL)
	{
		xmlFile.WriteAttribute(_T("badrc"), licenseStatus);
		if (strCodeStartDate.size())
			xmlFile.WriteAttribute(_T("ls_sd"), strCodeStartDate.c_str());
		if (nLicensedUser > 1)
			xmlFile.WriteAttribute(_T("ls_lu"), nLicensedUser);
		if (nCodeDurationDays != LICENSE_DURATION_PERPETUAL)
			xmlFile.WriteAttribute(_T("ls_dd"), nCodeDurationDays);
	}
}

//////////////////////////////////////////////////////////////////////////
// 
int MLMsgCmdActivateLicense::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_ACTIVATE);

	CXMLMsgCmd::ToXMLAttribute(xmlFile);

	xmlFile.WriteEndElement();
	return ERR_OK;

}

// int MLMsgRetActivateLicense::FromXML(SXNode *pXmlNode)
// {
// 	int nRet = MLRetMsg::FromXML(pXmlNode);
// 	if (nRet != ERR_OK)
// 		return nRet;
// 
// 
// 	return ERR_OK;
// }
// 
// void MLMsgRetActivateLicense::ToXMLAttribute(CXMLWriter &xmlFile)
// {
// 	MLRetMsg::ToXMLAttribute(xmlFile);
// 
// }

//////////////////////////////////////////////////////////////////////////

int RetLyrInfo::FromXML(SXNode *pNode, LPCTSTR szServerUrl)
{
	ASSERT(szServerUrl);

	strArtist = pNode->GetPropertySafe(_T("artist"));
	strTitle = pNode->GetPropertySafe(_T("title"));
	strAlbum = pNode->GetPropertySafe(_T("album"));
	strUploader = pNode->GetPropertySafe(_T("uploader"));

	strLink = szServerUrl;
	strLink += pNode->GetPropertySafe(_T("link"));

	nTimeLength = pNode->GetPropertyInt(_T("timelength"));

	LPCTSTR szValue = pNode->GetProperty(_T("rate"));
	if (szValue)
		_stscanf(szValue, _T("%f"), &fRate);
	nRateCount = pNode->GetPropertyInt(_T("ratecount"));
	nDownloads = pNode->GetPropertyInt(_T("downloads"));

	return ERR_OK;
}

int RetLyrInfo::ToXML(CXMLWriter &xmlFile, tstring &strServerUrl, bool bV0Protocol)
{
	xmlFile.WriteStartElement(MZ_FILEINFO);

	if (bV0Protocol)
	{
		xmlFile.WriteAttribute(_T("filetype"), _T("lyrics")); // This must be set for old version (7.0.614 before).
		xmlFile.WriteAttribute(_T("link"), (strServerUrl + strLink).c_str());

		xmlFile.WriteAttribute(_T("filename"), GetFileName().c_str());
	}
	else
		xmlFile.WriteAttribute(_T("link"), strLink.c_str());

	if (!strArtist.empty())
		xmlFile.WriteAttribute(_T("artist"), strArtist.c_str());
	if (!strTitle.empty())
		xmlFile.WriteAttribute(_T("title"), strTitle.c_str());
	if (!strAlbum.empty())
		xmlFile.WriteAttribute(_T("album"), strAlbum.c_str());
	if (!strUploader.empty())
		xmlFile.WriteAttribute(_T("uploader"), strUploader.c_str());
	if (nTimeLength != 0)
		xmlFile.WriteAttribute(_T("timelength"), nTimeLength);
	if (fRate != 0.0)
	{
		TCHAR		szTemp[64];
		_stprintf_s(szTemp, CountOf(szTemp), _T("%.1f"), fRate);
		xmlFile.WriteAttribute(_T("rate"), szTemp);
	}
	if (nRateCount != 0)
		xmlFile.WriteAttribute(_T("ratecount"), nRateCount);
	if (nDownloads != 0)
		xmlFile.WriteAttribute(_T("downloads"), nDownloads);
	xmlFile.WriteEndElement();
	return ERR_OK;
}

tstring RetLyrInfo::GetFileName()
{
	ASSERT(strTitle.size());

	// Combine file name
	tstring	strFileName;
	strFileName = strArtist;
	if (strFileName.size())
		strFileName += _T(" - ");
	strFileName += strTitle;
	strFileName = FileNameFilterInvalidChars(strFileName.c_str());
	strFileName += FileGetExt(strLink.c_str());

	return strFileName;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgCmdSearch::FromXML(SXNode *pXmlNode)
{
	CXMLMsgCmd::FromXML(pXmlNode);

	strArtist = pXmlNode->GetPropertySafe(_T("artist"));
	strTitle = pXmlNode->GetPropertySafe(_T("title"));
	nRequestPage = pXmlNode->GetPropertyInt(_T("RequestPage"));
	bOnlyMatchedLyrics = Tobool(pXmlNode->GetPropertyInt(_T("OnlyMatched")));

	if (strArtist.empty() && strTitle.empty())
		return ERR_CMD_PARAM;

	return ERR_OK;
}

int MLMsgCmdSearch::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_SEARCH);

	CXMLMsgCmd::ToXMLAttribute(xmlFile);

	if (strArtist.size())
		xmlFile.WriteAttribute(_T("artist"), strArtist.c_str());
	if (strTitle.size())
		xmlFile.WriteAttribute(_T("title"), strTitle.c_str());
	if (nRequestPage != 0)
		xmlFile.WriteAttribute(_T("RequestPage"), nRequestPage);

	if (bOnlyMatchedLyrics)
		xmlFile.WriteAttribute(_T("OnlyMatched"), bOnlyMatchedLyrics);

	xmlFile.WriteEndElement();
	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgRetSearch::FromXML(SXNode *pNode)
{
	int nRet = MLRetMsg::FromXML(pNode);
	if (nRet != ERR_OK)
		return nRet;

	ASSERT(!bV0Protocol);

	strServerUrl = pNode->GetPropertySafe(_T("server_url"));
	nPageCount = pNode->GetPropertyInt(_T("PageCount"));
	nCurPage = pNode->GetPropertyInt(_T("CurPage"));

	SXNode::iterator	it, itEnd;

	itEnd = pNode->listChildren.end();
	for (it = pNode->listChildren.begin(); it != itEnd; ++it)
	{
		RetLyrInfo	Info;
		Info.FromXML(*it, strServerUrl.c_str());
		listResultFiles.push_back(Info);
	}

	return ERR_OK;
}

int MLMsgRetSearch::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_RETURN);

	MLRetMsg::ToXMLAttribute(xmlFile);

	if (bV0Protocol)
		xmlFile.WriteAttribute(_T("filetype"), _T("lyrics")); // This should be set for old version (7.0.614 before).
	else
		xmlFile.WriteAttribute(_T("server_url"), strServerUrl.c_str());

	if (nPageCount != 1)
		xmlFile.WriteAttribute(_T("PageCount"), nPageCount);

	if (nCurPage != 0)
		xmlFile.WriteAttribute(_T("CurPage"), nCurPage);

	for (list<RetLyrInfo>::iterator it = listResultFiles.begin(); it != listResultFiles.end(); ++it)
	{
		(*it).ToXML(xmlFile, strServerUrl, bV0Protocol);
	}

	xmlFile.WriteEndElement();

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////

int MLSearchItem::FromXML(SXNode *pXmlNode)
{
	strArtist = pXmlNode->GetPropertySafe(_T("ar"));
	strAlbum = pXmlNode->GetPropertySafe(_T("al"));
	strTitle = pXmlNode->GetPropertySafe(_T("ti"));

	nMediaLength = pXmlNode->GetPropertyInt(_T("length"));

	return ERR_OK;
}

int MLSearchItem::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_ITEM);

	if (strArtist.size())
		xmlFile.WriteAttribute(_T("ar"), strArtist.c_str());
	if (strAlbum.size())
		xmlFile.WriteAttribute(_T("al"), strAlbum.c_str());
	xmlFile.WriteAttribute(_T("ti"), strTitle.c_str());
	if (nMediaLength > 0)
		xmlFile.WriteAttribute(_T("length"), nMediaLength);

	xmlFile.WriteEndElement();
	return ERR_OK;
}

MLMsgCmdBatchSearch::MLMsgCmdBatchSearch()
{
	bOnlyActiveCanDownload = false;
}

int MLMsgCmdBatchSearch::FromXML(SXNode *pXmlNode)
{
	CXMLMsgCmd::FromXML(pXmlNode);

	bOnlyActiveCanDownload = Tobool(pXmlNode->GetPropertyInt(_T("oncd"), false));

	listSearchItems.reserve(pXmlNode->listChildren.size());

	for (SXNode::LIST_CHILDREN::iterator it = pXmlNode->listChildren.begin();
		it != pXmlNode->listChildren.end(); ++it)
	{
		SXNode	*p = *it;
		if (_tcscmp(p->strName.c_str(), MZ_ITEM) != 0)
			continue;
		listSearchItems.push_back(MLSearchItem());
		listSearchItems.back().FromXML(p);
	}
	if (listSearchItems.size() == 0)
		return ERR_CMD_PARAM;

	return ERR_OK;
}

int MLMsgCmdBatchSearch::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_BATCH_SEARCH);

	if (bOnlyActiveCanDownload)
		xmlFile.WriteAttribute(_T("oncd"), bOnlyActiveCanDownload);

	CXMLMsgCmd::ToXMLAttribute(xmlFile);

	for (MLListSearchItems::iterator it = listSearchItems.begin(); it != listSearchItems.end(); ++it)
	{
		(*it).ToXML(xmlFile);
	}

	xmlFile.WriteEndElement();
	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLLyricsInfoLite::FromXML(SXNode *pXmlNode)
{
	strSaveName = strFile = pXmlNode->GetPropertySafe(_T("fn"));
	pXmlNode->GetPropertyBinData(_T("content"), bufLyrContent);

	return ERR_OK;
}

int MLLyricsInfoLite::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_ITEM);

	if (strFile.size())
	{
		CCharBuffer		bufFile;

		if (OpenFileToBuffer(strFile.c_str(), bufFile) && bufFile.size())
		{
			xmlFile.WriteAttribute(_T("fn"), FileGetName(strSaveName.c_str()));
			xmlFile.WriteAttribute(_T("content"), bufFile.c_str(), bufFile.size());
		}
		else
			ERRLOG1(_T("Failed to open lyrics file: %s"), strFile.c_str());
	}

	xmlFile.WriteEndElement();
	return ERR_OK;
}

int MLMsgRetBatchSearch::FromXML(SXNode *pXmlNode)
{
	int nRet = MLRetMsg::FromXML(pXmlNode);
	if (nRet != ERR_OK)
		return nRet;

	for (SXNode::LIST_CHILDREN::iterator it = pXmlNode->listChildren.begin();
		it != pXmlNode->listChildren.end(); ++it)
	{
		SXNode	*p = *it;
		if (_tcscmp(p->strName.c_str(), MZ_ITEM) != 0)
			continue;
		listLyricsInfo.push_back(MLLyricsInfoLite());
		listLyricsInfo.back().FromXML(p);
	}

	return ERR_OK;
}

int MLMsgRetBatchSearch::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_RETURN);

	MLRetMsg::ToXMLAttribute(xmlFile);

	for (ListLyricsInfoLite::iterator it = listLyricsInfo.begin();
		it != listLyricsInfo.end(); ++it)
	{
		MLLyricsInfoLite &fi = *it;
		fi.ToXML(xmlFile);
	}

	xmlFile.WriteEndElement();

	// Append data block
	CCharBuffer &buf = xmlFile.GetBuffer();
	buf.append(SZ_DATA_BLOCK_SIGN, LEN_DATA_BLOCK_SIGN);

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgCmdUploadV0::FromXML(SXNode *pXmlNode)
{
	strLoginName = pXmlNode->GetPropertySafe(_T("username"));

	strFileContent = pXmlNode->GetPropertySafe(_T("FileDataB64"));

	return ERR_OK;
}

int MLMsgCmdUploadV0::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_UPLOADV0);

	xmlFile.WriteAttribute(_T("username"), strLoginName.c_str());

	xmlFile.WriteAttribute(_T("FileDataB64"), strFileContent.c_str());
	xmlFile.WriteEndElement();
	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgCmdUpload::FromXML(SXNode *pXmlNode)
{
	strLoginName = pXmlNode->GetPropertySafe(_T("username"));

	strPwdMask = pXmlNode->GetPropertySafe(_T("pwd"));

	LPCTSTR szContent = pXmlNode->GetPropertySafe(_T("FileData"));
	if (!szContent)
		return ERR_BAD_FILE_CONTENT;

	{
		int		nContentLen = _tcslen(szContent);
		// Convert FileData from base64
		if (nContentLen < 6)
			return ERR_BAD_FILE_CONTENT;

		// Convert Base64 data >> UTF8
#ifdef UNICODE
		string		utf8Content;
		if (!decodebase64(szContent, nContentLen, utf8Content))
			return ERR_BAD_BASE64CONTENT;

		UTF8ToTString(utf8Content.c_str(), utf8Content.size(), strFileContent);
#else
		if (!decodebase64(szContent, nContentLen, strFileContent))
			return ERR_BAD_BASE64CONTENT;
#endif
	}

	return ERR_OK;
}

int MLMsgCmdUpload::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_UPLOAD);

	xmlFile.WriteAttribute(_T("username"), strLoginName.c_str());
	xmlFile.WriteAttribute(_T("pwd"), strPwdMask.c_str());

	// Convert TString >> UTF8 >> Base64 str
	stringex	utf8Str;
	TStringToUTF8(strFileContent.c_str(), strFileContent.size(), utf8Str);
	encodeuu(utf8Str.c_str(), utf8Str.size(), strFileContent, true, false);

	xmlFile.WriteAttribute(_T("FileData"), strFileContent.c_str());
	xmlFile.WriteEndElement();
	return ERR_OK;
}

int MLMsgRetUpload::FromXML(SXNode *pXmlNode)
{
	int nRet = MLMsgRetUploadV0::FromXML(pXmlNode);
	if (nRet != ERR_OK)
		return nRet;

	strLyricsId = pXmlNode->GetPropertySafe(_T("LyricsId"));

	return ERR_OK;
}

void MLMsgRetUpload::ToXMLAttribute(CXMLWriter &xmlFile)
{
	MLRetMsg::ToXMLAttribute(xmlFile);
	if (strLyricsId.size())
		xmlFile.WriteAttribute(_T("LyricsId"), strLyricsId.c_str());
}


//////////////////////////////////////////////////////////////////////////
//

CMLPacketAccountMgr		*CMLPacketAccountMgr::m_pMLPacketWrapper = NULL;

CMLPacketAccountMgr::CMLPacketAccountMgr()
{
	m_packFlag = MPV_V1_MD5_ID;
	m_bEnableXmlPack = true;
	m_nInvalidV1PackCount = 0;
	m_nInvalidPackCount = 0;
	m_bServer = true;
}

const char* CMLPacketAccountMgr::GetPwd(long nAccountId)
{
	MAP_ACCOUNT::iterator	it;

	it = m_mapAccount.find(nAccountId);
	if (it == m_mapAccount.end())
		return NULL;
	else
		return (*it).second.c_str();
}

CMLPacketAccountMgr *CMLPacketAccountMgr::GetInstance()
{
	if (!m_pMLPacketWrapper)
		m_pMLPacketWrapper = new CMLPacketAccountMgr;

	return m_pMLPacketWrapper;
}

void CMLPacketAccountMgr::AddAccount(long nId, const char*szPwd)
{
	m_mapAccount[nId] = szPwd;
}

void CMLPacketAccountMgr::EraseAccount(long nId)
{
	m_mapAccount.erase(nId);
}

//////////////////////////////////////////////////////////////////////////
//
//	MPV_XML			= '<',
//			没有加密，直接为 XML 协议
//	MPV_V1_MD5_ID	= 1,
//			帐户 ID 和 Pwd 一一对应在服务器中
//			1、字段参考 MLEncodePacketV1
//			2、byXor 的生成： byDataOrg 的平均值 ^ 0x9D
//			3、byMd5 的生成： MD5String（byDataOrg + 用户Pwd）
//			4、byData --> byDataOrg: 逐字节与byXor进行XOR运算
//			服务器或者客户端可以根据MD5 值来校验通讯数据的真伪

CMLPacketWrapper::CMLPacketWrapper()
{
	m_pAccountMgr = CMLPacketAccountMgr::GetInstance();
	m_nCurrentId = 0;
	m_packFlag = m_pAccountMgr->m_packFlag;
}

int CMLPacketWrapper::Wrapp(CCharBuffer &buff)
{
	if (!m_pAccountMgr->m_bServer)
		m_packFlag = m_pAccountMgr->m_packFlag;

	if (m_packFlag == MPV_XML)
		return ERR_OK;

	if (m_packFlag == MPV_V1_MD5_ID)
	{
		const char *szPwd;
		MLEncodePacketV1	head;
		long		n = 0;
		size_t		nSizeOld;
		uint		i;

		szPwd = m_pAccountMgr->GetPwd(m_nCurrentId);
		if (!szPwd)
		{
			ERRLOG1(_T("Invalid Lyrics Search Account: %d"), m_nCurrentId);
			return ERR_BAD_MSG;
		}

		head.byVersion = MPV_V1_MD5_ID;
		head.SetID(m_nCurrentId);

		nSizeOld = buff.size();
		buff += szPwd;
		assert(sizeof(head.byMd5) >= 16);
		MDString(buff.c_str(), buff.size(), head.byMd5);
		buff.setSize(nSizeOld);

		for (i = 0; i < buff.size(); i++)
			n += (BYTE)(buff[i]);
		head.byXor = BYTE(n / buff.size());

		for (i = 0; i < buff.size(); i++)
			buff[i] = buff[i] ^ head.byXor;

		buff.insert(0, (const char *)&head, sizeof(head));
		return ERR_OK;
	}

	ERRLOG1(_T("CMLPacketWrapper::Wrapp() Not supported Packet Version: %d"), m_packFlag);

	return ERR_OK;
}

int CMLPacketWrapper::Unwrapp(CCharBuffer &buff)
{
	MLEncodePacketV1		*pHead;

	if (buff.size() <= sizeof(MLEncodePacketV1))
		return ERR_INVALID_PACK_VER;

	pHead = (MLEncodePacketV1 *)buff.data();

	if (pHead->byVersion == MPV_XML)
	{
		if (!m_pAccountMgr->m_bEnableXmlPack)
			return ERR_DISABLE_PACK_XML;

		// 在服务器端，需要根据客户端发送的packet类型，转换到对应的packet类型
		if (m_pAccountMgr->m_bServer)
			m_packFlag = MPV_XML;
		return ERR_OK;
	}

	if (pHead->byVersion == MPV_V1_MD5_ID)
	{
		const char		*szPwd;
		unsigned char	Md5[16];
		int				i;

		for (i = sizeof(MLEncodePacketV1); i < (int)buff.size(); i++)
			buff[(uint)i] = buff[(uint)i] ^ pHead->byXor;

		m_nCurrentId = pHead->GetID();
		szPwd = m_pAccountMgr->GetPwd(m_nCurrentId);
		if (!szPwd)
		{
			ERRLOG1(_T("Invalid Lyrics Search Account: %d"), m_nCurrentId);
			m_pAccountMgr->m_nInvalidV1PackCount++;
			return ERR_BAD_MSG;
		}

		int nSizeOld = buff.size();
		buff += szPwd;
		MDString(buff.c_str() + sizeof(MLEncodePacketV1), buff.size() - sizeof(MLEncodePacketV1), Md5);
		buff.resize(nSizeOld);

		for (i = 0; i < CountOf(pHead->byMd5); i++)
		{
			if (pHead->byMd5[i] != Md5[i])
			{
				ERRLOG2(_T("Invalid MD5 Value, Account Error: %d, %s"), m_nCurrentId, szPwd);
				m_pAccountMgr->m_nInvalidV1PackCount++;
				return ERR_PACK_BAD_MD5;
			}
		}
		buff.erase(0, sizeof(MLEncodePacketV1));

		// 在服务器端，需要根据客户端发送的packet类型，转换到对应的packet类型
		if (m_pAccountMgr->m_bServer)
			m_packFlag = MPV_V1_MD5_ID;

		return ERR_OK;
	}

	ERRLOG1(_T("CMLPacketWrapper::Unwrapp() Invalid Packet Version: %d"), pHead->byVersion);
	m_pAccountMgr->m_nInvalidPackCount++;

	return ERR_INVALID_PACK_VER;
}

//////////////////////////////////////////////////////////////////////////
//

#define ALLOC_BLOCK_LEN		(2 * 1024)

int CMLProtocol::ReadMsg(MLMsgType &msgType, CCharBuffer *pMsgBuff)
{
	ASSERT(m_pNetFile);
	ASSERT(pMsgBuff->size() == 0);
	pMsgBuff->clear();

	int		nRead;
	int		nRet = m_pNetFile->Read(*pMsgBuff, ALLOC_BLOCK_LEN, &nRead);
	if (nRet != ERR_OK)
		return nRet;
	if (pMsgBuff->size() < 4)
		return ERR_BAD_MSG;

	char	*pszMsg;
	pszMsg = (char *)pMsgBuff->data();

	if (StrIBeginWith(pszMsg, "HTTP"))
		msgType = MT_HTTP_RET;
	else if (StrIBeginWith(pszMsg, "GET"))
		msgType = MT_HTTP_GET;
	else if (StrIBeginWith(pszMsg, "POST"))
		msgType = MT_HTTP_POST;
	else
		return ReadOldXmlMsg(msgType, pMsgBuff);

	//
	// HTTP protocol message
	//
	CHttpProtocol	*httpProtocol = NULL;

	if (msgType == MT_HTTP_GET || msgType == MT_HTTP_POST)
		httpProtocol = &m_httpRequest;
	else if (msgType == MT_HTTP_RET)
		httpProtocol = &m_httpReturn;
	else
		ASSERT(0);

	httpProtocol->SetCacheData(pMsgBuff->data(), pMsgBuff->size());
	httpProtocol->SetNetFile(m_pNetFile);
	nRet = httpProtocol->ReadHead(TRUE);
	if (nRet != ERR_OK)
		return nRet;

	pMsgBuff->clear();
	nRet = httpProtocol->ReadData(pMsgBuff, ML_MSG_MAX_LENGTH);
	if (nRet != ERR_OK)
		return nRet;

	// Is XML or Binary XML?
	if ((msgType == MT_HTTP_POST && strcmp(m_httpRequest.GetUrl(), SZ_ML_POST_URL) == 0)
		|| msgType == MT_HTTP_RET)
	{
		nRet = m_packetWrapper.Unwrapp(*pMsgBuff);
		if (nRet != ERR_OK)
			return nRet;

		if (pMsgBuff->c_str()[0] == '<')
			msgType = MT_HTTP_XML;
		else if (pMsgBuff->c_str()[0] == 'M')
			msgType = MT_HTTP_BIN_XML;
		else
			return ERR_BAD_MSG;
	}

	return ERR_OK;
}

int CMLProtocol::ReadOldXmlMsg(MLMsgType &msgType, CCharBuffer *pMsgBuff)
{
	int nDataLength = 0;

	while (1)
	{
		if (pMsgBuff->size() < ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN)
		{
			int nRead = 0;
			int nRet = m_pNetFile->Read(*pMsgBuff, ALLOC_BLOCK_LEN, &nRead);
			if (nRet != ERR_OK)
				return nRet;
			if (nRead == 0)
				return ERR_NOT_RECIVED_ALL;
		}

		//
		// 如果没有接收到数据长度，则继续接收
		if (pMsgBuff->size() < ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN)
			continue;

		if (nDataLength == 0)
		{
			// Read message header
			LPCSTR pszMsg = NULL;
			pszMsg = pMsgBuff->data();
			if (!StrBeginWith(pszMsg, ML_MSG_XML))
				return ERR_BAD_MSG;

			char	szMsgDataLen[ML_MSG_XML_DATA_LEN + 1];
			strncpy_safe(szMsgDataLen, CountOf(szMsgDataLen), pszMsg + ML_MSG_XML_LEN, ML_MSG_XML_DATA_LEN);
			szMsgDataLen[ML_MSG_XML_DATA_LEN] = '\0';

			char	*pTemp = szMsgDataLen;
			while (*pTemp == ' ')
				pTemp++;
			nDataLength = atoi(pTemp);

			if (nDataLength > 0xFFFFFF || nDataLength <= 0)
			{
				pMsgBuff->clear();
				DBGLOG1(_T("INVALID MSG!! Length: %d Invalid! CMGCPCom::OnRecieve(), Drop it!"), nDataLength);
				return ERR_BAD_MSG;
			}

			// Erase header
			pMsgBuff->erase(0, ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN);
		}

		if (nDataLength <= (int)pMsgBuff->size())
		{
			//
			// 已经读完了一条消息
			ASSERT(nDataLength <= (int)pMsgBuff->size());
			break;
		}

		if (pMsgBuff->size() >= ML_MSG_MAX_LENGTH)
			return ERR_OUTOF_BUFF;
	}

	msgType = MT_XML;

	int nRet = m_packetWrapper.Unwrapp(*pMsgBuff);
	if (nRet != ERR_OK)
		return nRet;

	return ERR_OK;
}

int CMLProtocol::WriteOldXmlMsg(CCharBuffer *pMsgBuff)
{
	char	szHeader[256];
	int		nWrite;
	int		nRet;

	sprintf_s(szHeader, CountOf(szHeader), ML_MSG_XML_FMT, pMsgBuff->size());

	nRet = m_pNetFile->Write(szHeader, ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN, &nWrite);
	if (nRet != ERR_OK)
		return nRet;

	nRet = m_pNetFile->Write(pMsgBuff->data(), pMsgBuff->size(), &nWrite);
	if (nRet != ERR_OK)
		return nRet;

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLProtocol::CMLProtocol()
{
	m_bUseProxy = false;
	m_pNetFile = NULL;
}

CMLProtocol::~CMLProtocol()
{

}

int CMLProtocol::Init(CNetFile *pFile, LPCSTR szHost, int nPort)
{
	SetNetFile(pFile);

	m_httpReqWriteXmlCache.SetNetFile(m_pNetFile);
	m_httpReqWriteXmlCache.SetHost(szHost, nPort);
	m_httpReqWriteXmlCache.SetRequestType(CHttpRequestProtocol::RQ_POST);
	m_httpReqWriteXmlCache.SetUrl(SZ_ML_POST_URL);
	m_httpReqWriteXmlCache.SetAvailProp(CHttpRequestProtocol::RP_POST_MIN);
// 	m_httpReqWriteXmlCache.SetPropValue(CHttpRequestProtocol::RP_HOST, _T("www.minilyrics.com"));
	m_httpReqWriteXmlCache.SetPropValue(CHttpRequestProtocol::RP_USER_AGENT, "MiniLyrics");

	m_httpRetWriteXmlCache.SetNetFile(m_pNetFile);
	m_httpRetWriteXmlCache.SetAvailProp(CHttpReturnProtocol::RP_MIN_RET);
	m_httpRetWriteXmlCache.SetServer("MiniLyrics Server/1.1");
	m_httpRetWriteXmlCache.SetConnection("close");

	return ERR_OK;
}

int CMLProtocol::Init(CNetFile *pFile, LPCWSTR szHost, int nPort)
{
	stringex		strHost;

	ConvertStr(szHost, -1, strHost);

	return Init(pFile, strHost.c_str(), nPort);
}

void CMLProtocol::SetProxy(BOOL bEnableProxy, LPCSTR szProxyServer, int nProxyPort, LPCSTR szBase64ProxyUserPass)
{
	m_httpReqWriteXmlCache.SetProxyBase64Pass(bEnableProxy, szBase64ProxyUserPass);
}

// ERR_OK
int CMLProtocol::WriteXMLMsg(MLMsgType msgType, CCharBuffer *pMsgBuff)
{
	int		nRet;

	nRet = m_packetWrapper.Wrapp(*pMsgBuff);
	if (nRet != ERR_OK)
		return nRet;

	if (msgType == MT_HTTP_POST)
	{
#ifdef _DEBUG
		if (g_Profile.GetProfileInt(_T("PrintMLSend"), FALSE))
			DBGLOGDUMPS(pMsgBuff->data(), pMsgBuff->size());
#endif
		m_httpReqWriteXmlCache.SetNetFile(m_pNetFile);
		m_httpReqWriteXmlCache.SetContentLength(pMsgBuff->size());
		m_httpReqWriteXmlCache.WriteHead();
		return m_httpReqWriteXmlCache.WriteData(pMsgBuff->data(), pMsgBuff->size());
	}
	else if (msgType == MT_HTTP_RET)
	{
		m_httpRetWriteXmlCache.SetNetFile(m_pNetFile);
		m_httpRetWriteXmlCache.SetContentLength(pMsgBuff->size());
		m_httpRetWriteXmlCache.WriteHead();
		return m_httpRetWriteXmlCache.WriteData(pMsgBuff->data(), pMsgBuff->size());
	}
	else
	{
		return WriteOldXmlMsg(pMsgBuff);
	}
}

int CMLProtocol::ParseXMLMsg(CCharBuffer *pMsgBuff, CSimpleXML &xml)
{
	if (!xml.ParseData(pMsgBuff->data(), pMsgBuff->size()))
		return ERR_PARSE_XML;

	return ERR_OK;
}


MLMsgCmd CMLProtocol::GetMsgCommand(CSimpleXML &xml)
{
	SXNode	*pNode;
	pNode = xml.m_pRoot;
	if (pNode)
	{
		return (MLMsgCmd)StringToID(__MlCmdIDStr, pNode->strName.c_str(), MC_UNKNOWN);
	}
	return MC_UNKNOWN;
}


#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

IMPLEMENT_CPPUNIT_TEST_REG(CMLProtocol)


#define CPPUNIT_ASSERT_STR_EQUAL(str1, str2)	CPPUNIT_ASSERT(_tcscmp(str1, str2) == 0)

class CTestCaseCMLProtocol : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCMLProtocol);
	CPPUNIT_TEST(TestParseBinXml);
	CPPUNIT_TEST(TestMLMsgCmdSearch);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestParseBinXml()
	{
		MLMsgRetBatchSearch	msgRet;

		msgRet.result = ERR_CMD_NOT_MATCH;
		msgRet.strMessage = _T("Msg");
		msgRet.licenseStatus = LS_ACTIVE;
		msgRet.nCodeDurationDays = 10;
		msgRet.nLicensedUser = 2;
		msgRet.strCodeStartDate = _T("2011-12-11");

		tstring	strFile = GetWorkingFolder();
		strFile += _T("unittest_save.txt");
		string bufContent = ";alskge\1,.zsm,ng\n;ea<>>\n";
		CPPUNIT_ASSERT(SaveDataAsFile(strFile.c_str(), bufContent.c_str(), bufContent.size()));

		{
			MLLyricsInfoLite	lyrInfo;
			lyrInfo.strFile = strFile;
			lyrInfo.strSaveName = _T("artist - title.txt");
			msgRet.listLyricsInfo.push_back(lyrInfo);
		}
		MLLyricsInfoLite	lyrInfo;
		lyrInfo.strFile = strFile;
		lyrInfo.strSaveName = _T("artist - title2.txt");
		msgRet.listLyricsInfo.push_back(lyrInfo);

		CMLBinXMLWriter xmlStream;
		int ret = msgRet.ToXML(xmlStream);
		CPPUNIT_ASSERT(ret == ERR_OK);

		DeleteFile(strFile.c_str());

		CSimpleXML xml;
		ret = xml.ParseData(xmlStream.GetBuffer().c_str(), xmlStream.GetBuffer().size());
		CPPUNIT_ASSERT(ret != FALSE);
		MLMsgRetBatchSearch	msgFrom;
		ret = msgFrom.FromXML(xml.m_pRoot);
		CPPUNIT_ASSERT(ret == ERR_OK);

		CPPUNIT_ASSERT(msgFrom.result == msgRet.result);
		CPPUNIT_ASSERT(msgFrom.strMessage == msgRet.strMessage);
		CPPUNIT_ASSERT(msgFrom.licenseStatus == msgRet.licenseStatus);
		CPPUNIT_ASSERT(msgFrom.strCodeStartDate == msgRet.strCodeStartDate);
		CPPUNIT_ASSERT(msgFrom.nLicensedUser == msgRet.nLicensedUser);
		CPPUNIT_ASSERT(msgFrom.nCodeDurationDays == msgRet.nCodeDurationDays);
		CPPUNIT_ASSERT(msgFrom.listLyricsInfo.size() == msgRet.listLyricsInfo.size());
		ListLyricsInfoLite::iterator itFrom = msgFrom.listLyricsInfo.begin();
		for (ListLyricsInfoLite::iterator it = msgRet.listLyricsInfo.begin();
			it != msgRet.listLyricsInfo.end(); ++it, ++itFrom)
		{
			MLLyricsInfoLite &lyr = *it;
			MLLyricsInfoLite &lyrFrom = *itFrom;

			CPPUNIT_ASSERT(lyr.strSaveName == lyrFrom.strSaveName);
			CPPUNIT_ASSERT(lyr.strSaveName == lyrFrom.strFile);
			CPPUNIT_ASSERT(bufContent.size() == lyrFrom.bufLyrContent.size());
			CPPUNIT_ASSERT(memcmp(bufContent.c_str(), lyrFrom.bufLyrContent.c_str(), lyrFrom.bufLyrContent.size()) == 0);
		}
	}

	void TestMLMsgCmdSearch()
	{
		MLMsgCmdSearch	msgSearch;

		msgSearch.bOnlineVerified = false;
		msgSearch.nProductId = PRODUCT_ID_MINILYRICS;
		msgSearch.nRequestPage = 1;
		msgSearch.strArtist = _T("ar");
		msgSearch.strClient = _T("client xxx");
		msgSearch.strDeviceID = _T("did");
		msgSearch.strRegCode = _T("Regcode");
		msgSearch.strTitle = _T("title");

		CMLBinXMLWriter xmlStream;
		int ret = msgSearch.ToXML(xmlStream);
		CPPUNIT_ASSERT(ret == ERR_OK);

		CSimpleXML xml;
		ret = xml.ParseData(xmlStream.GetBuffer().c_str(), xmlStream.GetBuffer().size());
		CPPUNIT_ASSERT(ret != FALSE);
		MLMsgCmdSearch	msgFrom;
		ret = msgFrom.FromXML(xml.m_pRoot);
		CPPUNIT_ASSERT(ret == ERR_OK);

		CPPUNIT_ASSERT(msgFrom.bOnlineVerified == msgSearch.bOnlineVerified);
		CPPUNIT_ASSERT(msgFrom.nRequestPage == msgSearch.nRequestPage);
		CPPUNIT_ASSERT(msgFrom.strArtist == msgSearch.strArtist);
		CPPUNIT_ASSERT(msgFrom.strClient == msgSearch.strClient);
		CPPUNIT_ASSERT(msgFrom.strDeviceID == msgSearch.strDeviceID);
		CPPUNIT_ASSERT(msgFrom.strRegCode == msgSearch.strRegCode);
		CPPUNIT_ASSERT(msgFrom.strTitle == msgSearch.strTitle);
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCMLProtocol);

#endif // _CPPUNIT_TEST

