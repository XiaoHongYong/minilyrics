// MLProtocol.cpp: implementation of the CMLProtocol class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLProtocol.h"

#define SZ_ML_POST_URL		"/searchlyrics.htm"


static ID_TO_STRING	__ErrID2Str[] = 
{
	{ ERR_UNKNOWN, _T("Unknown Error.")},			// 未翻译出来的结果
	{ ERR_BAD_XMLREQUEST, _T("Bad xml request.")},
	{ ERR_REDIRECT, _T("Redirect to other server.") },		// 重定向连接

	{ ERR_REG_EMAIL_EXIST, _T("The email already exists.") },
	{ ERR_REG_USER_EXIST, _T("The user already exists.") },	// 用户已经存在, 无法申请
	{ ERR_UPLOAD_EXIST, _T("The content uploaded already exists.") },	// 上传的内容已经存在
	{ ERR_USER_NOT_EXIST, _T("User name doesn't exist.") },	// 用户不存在
	{ ERR_BAD_USERPWD, _T("Wrong user name or password.") },		// 用户名或者密码错误
	{ ERR_BAD_CLIENT_VER, _T("Invliad client version.") },	// 客户端版本不正确, 无法登陆
	{ ERR_OBSOLETE_CLIENT_VER, _T("Obsoleted client. please download the latest version.") },	// 客户端版本太低, 无法登陆
	// { ERR_TOO_BUSY, _T("Server too busy, please try again later.") },		// 服务器太忙,暂时无法响应请求
	{ ERR_ACCESS_DENIED, _T("Access denied.") },	// 无权访问,被拒绝
	{ ERR_NOT_SUPPORT_CMD, _T("Not supported command.") },	// 还不支持的命令
	{ ERR_CMD_NOT_MATCH, _T("Received return command not match.") },
	{ ERR_CMD_PARAM, _T("Invalid message command parameter.") },
	{ ERR_UPLOAD_FILETYPE, _T("The file type can not be uploaded.") },	// 上传的文件类型错误
	{ ERR_BAD_BASE64CONTENT, _T("Incorrect base64 encoded content.") },
	{ ERR_BAD_FILE_CONTENT, _T("The file content is incorrect or contains forbidden words.") },
	{ ERR_REG_USER_NAME_SHORT, _T("The username must contains at least 3 characters.") },
	{ ERR_NOT_AUTHORIZED, _T("Unauthorized action.") },
	{ ERR_NOT_LOGIN, _T("Not logined, please login first!") },
	{ ERR_UPDATE_LRC_INFO_SHOULD_SAME, _T("The newly updated and old lyrics must have the same title and artist info.") },

	{ 0, NULL}
};

static RegErr2Str	__addErr2Str(__ErrID2Str);


#ifdef WIN32
UINT GetTempFileName(
  LPTSTR lpTempFileName 
                       // pointer to buffer that receives the new 
                       // filename
)
{
	int		nRet;
	nRet = GetTempPath(MAX_PATH, lpTempFileName);
	ASSERT(nRet > 0 && nRet <= MAX_PATH);
	if (nRet == 0 || nRet > MAX_PATH)
		return 0;

	return GetTempFileName(lpTempFileName, _T("ML"), 0, lpTempFileName);
}

BOOL SaveBufferAsTempFile(LPTSTR szTempFile, LPVOID lpBuffer, int nLen)
{
	if (GetTempFileName(szTempFile) == 0)
	{
		LOG1(LOG_LVL_ERROR, _T("GetTempFileName() FAILED! LastError: %d"), GetLastError());
		return FALSE;
	}

	return SaveStringAsFile(szTempFile, (LPCTSTR)lpBuffer, nLen / sizeof(TCHAR));
}
#endif

ID_TO_STRING	__MlCmdIDStr[] = 
{
	{MC_APPLY_ACCOUNT, MZ_APPLY_ACCOUNT },
	{MC_MODIFY_ACCOUNT, MZ_MODIFY_ACCOUNT },
	{MC_QUERY_ACCOUNT_INFO, MZ_QUERY_ACCOUNT_INFO },
	{MC_LOGIN, MZ_LOGIN },
	{MC_LOGOUT, MZ_LOGOUT },
	{MC_RETURN, MZ_RETURN },
	{MC_SEARCH, MZ_SEARCH },
	{MC_CAN_UPLOAD, MZ_CANUPLOAD},
	{MC_UPLOAD, MZ_UPLOAD },
	{MC_DOWNLOADBEST, MZ_DOWNLOADBEST},
	{MC_GET_STATISTICTYPE, MZ_GETSTATISTICTYPE},
	{MC_STATISTICQUERY, MZ_STATISTICQUERY },
	{MC_POST_BUG, MZ_POST_BUG },
	{MC_UNKNOWN, NULL},
//	{, },
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
	{ERR_OK, _T("OK") },								// 0
	{ERR_FAILED, _T("FAILED") },						// 1035
	{ERR_BAD_XMLREQUEST, _T("BAD_XML_REQUEST") },	// 1036
	{ERR_REDIRECT, _T("REDIRECT") },					// 1038
	{ERR_REG_EMAIL_EXIST, _T("EMAIL_EXIST") },		// 1040
	{ERR_REG_USER_EXIST, _T("USER_EXIST") },			// 1041
	{ERR_UPLOAD_EXIST, _T("UPLOAD_EXIST") },			// 1042
	{ERR_USER_NOT_EXIST, _T("USER_NOT_FOUND") },		// 1043
	{ERR_BAD_USERPWD, _T("BAD_USERPWD") },			// 1045
	{ERR_BAD_CLIENT_VER, _T("BAD_CLIENT_VER") },		// 1047
	{ERR_OBSOLETE_CLIENT_VER, _T("OBSOLETE_CLIENT_VER") },	// 1048
	{ERR_NOT_FOUND, _T("NOT_FOUND") },				// 1049
	// {ERR_TOO_BUSY, _T("SERVER_TOO_BUSY") },			// 1050
	{ERR_ACCESS_DENIED, _T("ACCESS_DENIED") },		// 1051
	{ERR_NOT_SUPPORT_CMD, _T("NOT_SUPPORT_CMD") },	// 1052	
	{ERR_CMD_NOT_MATCH, _T("CMD_NOT_MATCH") },		// 1053	
	{ERR_CMD_PARAM, _T("BAD_CMD_PARAM") },			// 1054	
	{ERR_UPLOAD_FILETYPE, _T("UPLOAD_FILETYPE") },	// 1056	
	{ERR_BAD_BASE64CONTENT, _T("BAD_BASE64CONTENT") },	// 1058
	{ERR_BAD_FILE_CONTENT, _T("BAD_FILE_CONTENT") },	// 1059
	{0, NULL},
//	{, },
};
//
//MLMsgCmdResultCode ErrorToMLCmdResult(int nErr)
//{
//	if (nErr == ERR_OK)
//		return ERR_OK;
//	else if (nErr < ERR_UNKNOWN)
//		return ERR_FAILED;
//	else
//		return (MLMsgCmdResultCode)nErr;
//}
//
//int MLCmdResultToError(MLMsgCmdResultCode result)
//{
//	return result;
///*
//	switch (result)
//	{
//	case ERR_OK: return ERR_OK;
//	case ERR_UNKNOWN: return ERR_FALSE;
//	case ERR_BAD_XMLREQUEST: return ERR_BAD_XMLREQUEST;
////	case : return ;
////	case : return ;
////	case : return ;
////	case : return ;
////	case : return ;
////	case : return ;
//	default:
//		return ERR_FALSE;
//	}
//	return ERR_FALSE;*/
//
//}

ID_TO_STRING	__MlLinkTypeIDStr[] = 
{
	{MLT_URL, _T("URL") },
	{MLT_MLL, _T("MLL") },
	{MLT_NONE, NULL},
};

LPCTSTR LinkTypeToStr(MLLinkType linkType)
{
	return IDToString(__MlLinkTypeIDStr, MLT_NONE, _T(""));
}

MLLinkType LinkTypeFromStr(LPCTSTR szLinkType)
{
	return (MLLinkType)StringToID(__MlLinkTypeIDStr, szLinkType, MLT_NONE);
}

CXMLMsgCmd::~CXMLMsgCmd()
{
}

//////////////////////////////////////////////////////////////////////////
int MLMsgCmdLogin::FromXML(SXNode *pXmlNode)
{
	LPCTSTR		szValue;
	szValue = pXmlNode->GetProperty(_T("user"));
	strName = SURETEXT(szValue);
	szValue = pXmlNode->GetProperty(_T("password"));
	strPwd = SURETEXT(szValue);
	// szValue = pXmlNode->GetProperty(_T("clientversion"));
	// strClientVer = SURETEXT(szValue);
	// szValue = pXmlNode->GetProperty(_T("clientname"));
	// strClientName = SURETEXT(szValue);
	szValue = pXmlNode->GetProperty(_T("client"));
	strClient = SURETEXT(szValue);
	szValue = pXmlNode->GetProperty(_T("protocolversion"));
	strProtocolVer = SURETEXT(szValue);
	szValue = pXmlNode->GetProperty(_T("regcode"));
	strRegCode = SURETEXT(szValue);

	return ERR_OK;
}

int MLMsgCmdLogin::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_LOGIN);
	xmlFile.WriteAttribute(_T("user"), strName.c_str());
	xmlFile.WriteAttribute(_T("password"), strPwd.c_str());
	// xmlFile.WriteAttribute(_T("clientversion"), strClientVer.c_str());
	// xmlFile.WriteAttribute(_T("clientname"), strClientName.c_str());
	xmlFile.WriteAttribute(_T("client"), strClient.c_str());
	xmlFile.WriteAttribute(_T("protocolversion"), strProtocolVer.c_str());
	if (strRegCode.size())
		xmlFile.WriteAttribute(_T("regcode"), strRegCode.c_str());
	xmlFile.WriteEndElement();
	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgRetLogin::FromXML(SXNode *pXmlNode)
{
	LPCTSTR		szValue;
	szValue = pXmlNode->GetProperty(MZ_ORG_CMD);
	strOrgCmd = SURETEXT(szValue);
	szValue = pXmlNode->GetProperty(MZ_CMD_RESULT);
	if (!szValue)
		return ERR_CMD_PARAM;

	result = StringToID(__MlCmdResult2Str, szValue, ERR_UNKNOWN);

	szValue = pXmlNode->GetProperty(_T("message"));
	strMessage = SURETEXT(szValue);

	return ERR_OK;
}

int MLMsgRetLogin::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_RETURN);
	xmlFile.WriteAttribute(MZ_ORG_CMD, strOrgCmd.c_str());
	xmlFile.WriteAttribute(MZ_CMD_RESULT, IDToString(__MlCmdResult2Str, result, _T("UNTRANSLATED")));

	// 为了兼容之前的版本，需要返回 MZ_SESSIONID
	xmlFile.WriteAttribute(MZ_SESSIONID, _T("1"));
	if (strMessage.size())
		xmlFile.WriteAttribute(_T("message"), strMessage.c_str());
	xmlFile.WriteEndElement();
	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
//
int MLRetMsg::FromXML(SXNode *pXmlNode)
{
	LPCTSTR		szValue;
	szValue = pXmlNode->GetProperty(MZ_ORG_CMD);
	strOrgCmd = SURETEXT(szValue);
	szValue = pXmlNode->GetProperty(MZ_CMD_RESULT);
	if (!szValue)
		return ERR_CMD_PARAM;

	result = StringToID(__MlCmdResult2Str, szValue, ERR_UNKNOWN);

	return ERR_OK;
}

int MLRetMsg::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_RETURN);
	xmlFile.WriteAttribute(MZ_ORG_CMD, strOrgCmd.c_str());
	xmlFile.WriteAttribute(MZ_CMD_RESULT, IDToString(__MlCmdResult2Str, result, _T("UNTRANSLATED")));
	xmlFile.WriteEndElement();
	return ERR_OK;
}
//////////////////////////////////////////////////////////////////////////
MLFileType StrToMLFileType(LPCTSTR szFileType)
{
	if (_tcsicmp(szFileType, _T("lyrics")) == 0)
		return FT_LYRICS;
	else if (_tcsicmp(szFileType, _T("song")) == 0)
		return FT_LYRICS;
	else 
		return FT_UNKNOWN;
}

LPCTSTR MLFileTypeToStr(MLFileType fileType)
{
	if (fileType == FT_LYRICS)
		return _T("lyrics");
	else if (fileType == FT_SONG)
		return _T("song");
	else 
		return _T("");
}

int MLFileInfo::FromXML(SXNode *pNode)
{
	LPCTSTR		szValue;
	szValue = pNode->GetProperty(_T("filetype"));
	if (szValue == NULL)
		return ERR_CMD_PARAM;
	fileType = StrToMLFileType(szValue);
	if (fileType == FT_UNKNOWN)
		return ERR_CMD_PARAM;
//
//	szValue = pNode->GetProperty(_T("ID"));
//	nLyricsID = _ttoi(SURETEXT(szValue));

	szValue = pNode->GetProperty(_T("artist"));
	strArtist = SURETEXT(szValue);

	szValue = pNode->GetProperty(_T("title"));
	strTitle = SURETEXT(szValue);

	szValue = pNode->GetProperty(_T("album"));
	strAlbum = SURETEXT(szValue);

	szValue = pNode->GetProperty(_T("comment"));
	strComment = SURETEXT(szValue);

	szValue = pNode->GetProperty(_T("filename"));
	strFileName = SURETEXT(szValue);

	szValue = pNode->GetProperty(_T("uploader"));
	strUploader = SURETEXT(szValue);

	szValue = pNode->GetProperty(_T("linkType"));
	strLink = LinkTypeFromStr(SURETEXT(szValue));

	szValue = pNode->GetProperty(_T("link"));
	strLink = SURETEXT(szValue);

	nMatchValue = pNode->GetPropertyInt(_T("matchvalue"));

	nTimeLength = pNode->GetPropertyInt(_T("timelength"));

	szValue = pNode->GetProperty(_T("encoding"));
	strEncoding = SURETEXT(szValue);

	szValue = pNode->GetProperty(_T("rate"));
	if (szValue)
		_stscanf(szValue, _T("%f"), &fRate);
	nRateCount = pNode->GetPropertyInt(_T("ratecount"));
	nDownloads = pNode->GetPropertyInt(_T("downloads"));

	return ERR_OK;
}

int MLFileInfo::ToXML(CXMLWriter &xmlFile)
{
	tstring		strFileType;
	strFileType = MLFileTypeToStr(fileType);
	ASSERT(!strFileType.empty());
	if (strFileType.empty())
		return ERR_CMD_PARAM;
	xmlFile.WriteStartElement(MZ_FILEINFO);
	xmlFile.WriteAttribute(_T("filetype"), strFileType.c_str());

//	xmlFile.WriteAttribute(_T("ID"), nLyricsID);
	if (!strArtist.empty())
		xmlFile.WriteAttribute(_T("artist"), strArtist.c_str());
	if (!strTitle.empty())
		xmlFile.WriteAttribute(_T("title"), strTitle.c_str());
	if (!strAlbum.empty())
		xmlFile.WriteAttribute(_T("album"), strAlbum.c_str());
	if (!strComment.empty())
		xmlFile.WriteAttribute(_T("comment"), strComment.c_str());
	if (!strFileName.empty())
		xmlFile.WriteAttribute(_T("filename"), strFileName.c_str());
	if (!strUploader.empty())
		xmlFile.WriteAttribute(_T("uploader"), strUploader.c_str());
	if (linkType != MLT_NONE)
		xmlFile.WriteAttribute(_T("linkType"), LinkTypeToStr(linkType));
	if (!strLink.empty())
		xmlFile.WriteAttribute(_T("link"), strLink.c_str());
	if (nMatchValue != 0)
		xmlFile.WriteAttribute(_T("matchvalue"), nMatchValue);
	if (nTimeLength != 0)
		xmlFile.WriteAttribute(_T("timelength"), nTimeLength);
	if (!strEncoding.empty())
		xmlFile.WriteAttribute(_T("encoding"), strEncoding.c_str());
	if (fRate != 0.0)
	{
		TCHAR		szTemp[64];
		_stprintf(szTemp, _T("%.1f"), fRate);
		xmlFile.WriteAttribute(_T("rate"), szTemp);
	}
	if (nRateCount != 0)
		xmlFile.WriteAttribute(_T("ratecount"), nRateCount);
	if (nDownloads != 0)
		xmlFile.WriteAttribute(_T("downloads"), nDownloads);
	xmlFile.WriteEndElement();
	return ERR_OK;
}


//////////////////////////////////////////////////////////////////////////
int MLMsgCmdSearch::FromXML(SXNode *pXmlNode)
{
	LPCTSTR		szValue;
	szValue = pXmlNode->GetProperty(_T("filetype"));
	if (szValue == NULL)
		return ERR_CMD_PARAM;
	searchType = StrToMLFileType(szValue);
	if (searchType == FT_UNKNOWN)
		return ERR_CMD_PARAM;

	szValue = pXmlNode->GetProperty(_T("artist"));
	strArtist = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("title"));
	strTitle = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("ProtoVer"));
	strProtocolVer = SURETEXT(szValue);

	// szValue = pXmlNode->GetProperty(_T("ClientVer"));
	// strClientVer = SURETEXT(szValue);
	//
	// szValue = pXmlNode->GetProperty(_T("ClientName"));
	// strClientName = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("client"));
	strClient = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("RegCode"));
	strRegCode = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("ClientCharEncoding"));
	strClientCharEncoding = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("RequestPage"));
	if (szValue)
		nRequestPage = _ttoi(szValue);
	else
		nRequestPage = 0;

	if (strArtist.empty() && strTitle.empty())
		return ERR_CMD_PARAM;

	return ERR_OK;
}

int MLMsgCmdSearch::ToXML(CXMLWriter &xmlFile)
{
	tstring		strSearchType;
	strSearchType = MLFileTypeToStr(searchType);
	if (strSearchType.empty())
		return ERR_CMD_PARAM;
	xmlFile.WriteStartElement(MZ_SEARCH);
	xmlFile.WriteAttribute(_T("filetype"), strSearchType.c_str());

	xmlFile.WriteAttribute(_T("artist"), strArtist.c_str());
	xmlFile.WriteAttribute(_T("title"), strTitle.c_str());
	if (strProtocolVer.size())
		xmlFile.WriteAttribute(_T("ProtoVer"), strProtocolVer.c_str());
//	if (strClientVer.size())
//		xmlFile.WriteAttribute(_T("ClientVer"), strClientVer.c_str());
//	if (strClientName.size())
//		xmlFile.WriteAttribute(_T("ClientName"), strClientName.c_str());
	if (strClient.size())
		xmlFile.WriteAttribute(_T("client"), strClient.c_str());
	if (strRegCode.size())
		xmlFile.WriteAttribute(_T("RegCode"), strRegCode.c_str());
	if (strClientCharEncoding.size())
		xmlFile.WriteAttribute(_T("ClientCharEncoding"), strClientCharEncoding.c_str());
	if (nRequestPage != 0)
		xmlFile.WriteAttribute(_T("RequestPage"), nRequestPage);

	xmlFile.WriteEndElement();
	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgRetSearch::FromXML(SXNode *pXmlNode)
{
	LPCTSTR		szValue;
	SXNode		*pNode;
	pNode = pXmlNode;
	szValue = pXmlNode->GetProperty(MZ_ORG_CMD);
	strOrgCmd = SURETEXT(szValue);
	szValue = pXmlNode->GetProperty(MZ_CMD_RESULT);
	if (!szValue)
		return ERR_CMD_PARAM;

	result = StringToID(__MlCmdResult2Str, szValue, ERR_UNKNOWN);

	szValue = pNode->GetProperty(_T("filetype"));
	if (szValue == NULL)
		return ERR_CMD_PARAM;
	searchType = StrToMLFileType(szValue);
	if (searchType == FT_UNKNOWN)
		return ERR_CMD_PARAM;

	szValue = pNode->GetProperty(_T("artist"));
	strArtist = SURETEXT(szValue);

	szValue = pNode->GetProperty(_T("title"));
	strTitle = SURETEXT(szValue);

	szValue = pNode->GetProperty(_T("badrc"));
	if (szValue)
		RegVerifyValue = (REGCODE_VERIFY_VALUE)_ttoi(szValue);
	else
		RegVerifyValue = RVV_OK;

	szValue = pNode->GetProperty(_T("msg"));
	strMessage = SURETEXT(szValue);

	mledition = ME_NOTSET;
	szValue = pXmlNode->GetProperty(_T("MLEdition"));
	if (szValue)
	{
		if (_tcsicmp(szValue, _T("ME_ENGLISH")) == 0)
			mledition = ME_ENGLISH;
		else if (_tcsicmp(szValue, _T("ME_CHINESE")) == 0)
			mledition = ME_CHINESE;
	}

	szValue = pNode->GetProperty(_T("PageCount"));
	if (szValue)
		nPageCount = _ttoi(szValue);
	else
		nPageCount = 1;

	szValue = pNode->GetProperty(_T("CurPage"));
	if (szValue)
		nCurPage = _ttoi(szValue);
	else
		nCurPage = 0;

	SXNode::iterator	it, itEnd;

	itEnd = pNode->listChildren.end();
	for (it = pNode->listChildren.begin(); it != itEnd; ++it)
	{
		MLFileInfo	Info;
		Info.FromXML(*it);
		listResultFiles.push_back(Info);
	}

	return ERR_OK;
}

int MLMsgRetSearch::ToXML(CXMLWriter &xmlFile)
{
	tstring		strSearchType;
	strSearchType = MLFileTypeToStr(searchType);
	if (strSearchType.empty())
		return ERR_CMD_PARAM;
	xmlFile.WriteStartElement(MZ_RETURN);
	xmlFile.WriteAttribute(MZ_ORG_CMD, strOrgCmd.c_str());
	xmlFile.WriteAttribute(MZ_CMD_RESULT, IDToString(__MlCmdResult2Str, result, _T("UNTRANSLATED")));
	xmlFile.WriteAttribute(_T("filetype"), strSearchType.c_str());

	xmlFile.WriteAttribute(_T("artist"), strArtist.c_str());
	xmlFile.WriteAttribute(_T("title"), strTitle.c_str());

	if (RegVerifyValue)
		xmlFile.WriteAttribute(_T("badrc"), RegVerifyValue);
	if (strMessage.size())
		xmlFile.WriteAttribute(_T("msg"), strMessage.c_str());

	if (mledition == ME_ENGLISH)
		xmlFile.WriteAttribute(_T("MLEdition"), _T("ME_ENGLISH"));
	else if (mledition == ME_CHINESE)
		xmlFile.WriteAttribute(_T("MLEdition"), _T("ME_CHINESE"));

	if (nPageCount != 1)
		xmlFile.WriteAttribute(_T("PageCount"), nPageCount);

	if (nCurPage != 0)
		xmlFile.WriteAttribute(_T("CurPage"), nCurPage);

	for (list<MLFileInfo>::iterator it = listResultFiles.begin(); it != listResultFiles.end(); ++it)
	{
		(*it).ToXML(xmlFile);
	}
	xmlFile.WriteEndElement();
	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgCmdCanUpload::FromXML(SXNode *pXmlNode)
{
	LPCTSTR		szValue;
	szValue = pXmlNode->GetProperty(_T("filetype"));
	if (szValue == NULL)
		return ERR_CMD_PARAM;
	fileType = StrToMLFileType(szValue);
	if (fileType == FT_UNKNOWN)
		return ERR_CMD_PARAM;

	szValue = pXmlNode->GetProperty(_T("username"));
	strLoginName = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("artist"));
	strArtist = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("album"));
	strAlbum = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("title"));
	strTitle = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("timelength"));
	nTimeLength = _ttoi(SURETEXT(szValue));

	szValue = pXmlNode->GetProperty(_T("filename"));
	strFileName = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("md5"));
	strMD5 = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("ClientCharEncoding"));
	strClientCharEncoding = SURETEXT(szValue);

	if (strArtist.empty() && strTitle.empty() || strMD5.empty())
	{
		DBGLOG3(_T("Canupload command param error, Artist: %S, Title: %S, MD5: %S"),
			strArtist.c_str(), strTitle.c_str(), strMD5.c_str());
		return ERR_CMD_PARAM;
	}

	return ERR_OK;
}

int MLMsgCmdCanUpload::ToXML(CXMLWriter &xmlFile)
{
	tstring		strFileType;
	strFileType = MLFileTypeToStr(fileType);
	if (strFileType.empty())
		return ERR_CMD_PARAM;
	xmlFile.WriteStartElement(MZ_CANUPLOAD);
	xmlFile.WriteAttribute(_T("username"), strLoginName.c_str());
	xmlFile.WriteAttribute(_T("filetype"), strFileType.c_str());

	xmlFile.WriteAttribute(_T("artist"), strArtist.c_str());
	xmlFile.WriteAttribute(_T("album"), strAlbum.c_str());
	xmlFile.WriteAttribute(_T("title"), strTitle.c_str());
	xmlFile.WriteAttribute(_T("timelength"), nTimeLength);
	xmlFile.WriteAttribute(_T("filename"), strFileName.c_str());
	xmlFile.WriteAttribute(_T("md5"), strMD5.c_str());
	if (strClientCharEncoding.size())
		xmlFile.WriteAttribute(_T("ClientCharEncoding"), strClientCharEncoding.c_str());
	xmlFile.WriteEndElement();
	return ERR_OK;
}


//////////////////////////////////////////////////////////////////////////
int MLMsgCmdUpload::FromXML(SXNode *pXmlNode)
{
	LPCTSTR		szValue;

	szValue = pXmlNode->GetProperty(_T("username"));
	strLoginName = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("filetype"));
	if (szValue == NULL)
		return ERR_CMD_PARAM;
	fileType = StrToMLFileType(szValue);
	if (fileType == FT_UNKNOWN)
		return ERR_CMD_PARAM;

	szValue = pXmlNode->GetProperty(_T("artist"));
	strArtist = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("album"));
	strAlbum = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("title"));
	strTitle = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("comment"));
	strComment = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("encoding"));
	strContentEncoding = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("timelength"));
	nTimeLength = _ttoi(SURETEXT(szValue));

	szValue = pXmlNode->GetProperty(_T("filename"));
	strFileName = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("md5"));
	strMD5 = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("FileData"));
	strFileContentOld = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("FileDataB64"));
	strFileContent = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("ClientCharEncoding"));
	strClientCharEncoding = SURETEXT(szValue);

	if (strArtist.empty() && strTitle.empty() || strMD5.empty())
		return ERR_CMD_PARAM;

	return ERR_OK;
}

int MLMsgCmdUpload::ToXML(CXMLWriter &xmlFile)
{
	tstring		strFileType;
	strFileType = MLFileTypeToStr(fileType);
	if (strFileType.empty())
		return ERR_CMD_PARAM;
	xmlFile.WriteStartElement(MZ_UPLOAD);
	xmlFile.WriteAttribute(_T("username"), strLoginName.c_str());
	xmlFile.WriteAttribute(_T("filetype"), strFileType.c_str());

	xmlFile.WriteAttribute(_T("artist"), strArtist.c_str());
	xmlFile.WriteAttribute(_T("album"), strAlbum.c_str());
	xmlFile.WriteAttribute(_T("title"), strTitle.c_str());
	if (strComment.size())
	xmlFile.WriteAttribute(_T("comment"), strComment.c_str());
	if (strContentEncoding.size())
		xmlFile.WriteAttribute(_T("encoding"), strContentEncoding.c_str());
	if (nTimeLength > 0)
		xmlFile.WriteAttribute(_T("timelength"), nTimeLength);
	xmlFile.WriteAttribute(_T("filename"), strFileName.c_str());
	xmlFile.WriteAttribute(_T("md5"), strMD5.c_str());
	xmlFile.WriteAttribute(_T("FileDataB64"), strFileContent.c_str());
	if (strClientCharEncoding.size())
		xmlFile.WriteAttribute(_T("ClientCharEncoding"), strClientCharEncoding.c_str());
	xmlFile.WriteEndElement();
	return ERR_OK;
}


//////////////////////////////////////////////////////////////////////////
int MLDataList::FromXML(SXNode *pXmlNode)
{
	//
	// property
	strClassID = SURETEXT(pXmlNode->GetProperty(_T("classid")));

	//
	// title
	// <title> <colum>name</colum> <colum>desc<colum> </title>
	{
		SXNode	*pNodeTitle;
		pNodeTitle = pXmlNode->GetChild(_T("title"));
		if (!pNodeTitle)
		{
			DBGLOG0(_T("Can't get title node of DataList!"));
			return ERR_CMD_PARAM;
		}

		SXNode::iterator	it, itEnd;

		itEnd = pNodeTitle->listChildren.end();
		for (it = pNodeTitle->listChildren.begin(); it != itEnd; ++it)
		{
			SXNode *pNodeTemp = *it;

			if (_tcsicmp(pNodeTemp->strName.c_str(), _T("column")) == 0)
			{
				vTitles.push_back(pNodeTemp->strContent);
			}
		}
	}

	//
	// data
	// <data><r><c>apple</c><c>apple's good</c></r></data>
	{
		SXNode	*pNodeData;
		pNodeData = pXmlNode->GetChild(_T("data"));
		if (!pNodeData)
		{
			DBGLOG0(_T("DataList has no data!"));
			// 允许数据为空！
			return ERR_OK;
		}

		SXNode::iterator	it, itEnd;
		itEnd = pNodeData->listChildren.end();
		for (it = pNodeData->listChildren.begin(); it != itEnd; ++it)
		{
			SXNode *pNodeRow = *it;
			if (_tcsicmp(pNodeRow->strName.c_str(), _T("r")) == 0)
			{
				//
				// row
				listRowData.push_back(RowData_t());
				RowData_t		&rowData = listRowData.back();

				SXNode::iterator	itCol, itColEnd;
				itColEnd = pNodeRow->listChildren.end();
				for (itCol = pNodeRow->listChildren.begin(); itCol != itColEnd; ++itCol)
				{
					// column
					SXNode *pNodeCol = *itCol;
					if (_tcsicmp(pNodeCol->strName.c_str(), _T("c")) == 0)
					{
						rowData.push_back(pNodeCol->strContent);
					}
				}
			}
		}
	}

	return ERR_OK;
}

int MLDataList::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(_T("DataList"));

	// property
	xmlFile.WriteAttribute(_T("classid"), strClassID.c_str());

	// <title> <column>name</colum> <colum>desc<colum> </title>
	{
		xmlFile.WriteStartElement(_T("title"));
		for (int i = 0; i < (int)vTitles.size(); i++)
			xmlFile.WriteAttribute(_T("column"), vTitles[i].c_str());
		xmlFile.WriteEndElement();
	}

	//
	// data
	// <data><r><c>apple</c><c>apple's good</c></r></data>
	{
		if (!listRowData.empty())
		{
			xmlFile.WriteStartElement(_T("data"));
			list<RowData_t>::iterator	it;
			for (it = listRowData.begin(); it != listRowData.end(); ++it)
			{
				xmlFile.WriteStartElement(_T("r"));
				for (RowData_t::iterator itRow = (*it).begin(); itRow != (*it).end(); itRow++)
					xmlFile.WriteAttribute(_T("c"), (*itRow).c_str());
				xmlFile.WriteEndElement();
			}
			xmlFile.WriteEndElement();
		}
	}

	xmlFile.WriteEndElement();

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgCmdPostSongInfo::SongInfo::FromXML(SXNode *pXmlNode)
{
	LPCTSTR		szValue;

	szValue = pXmlNode->GetProperty(_T("md5Song"));
	strMd5Song = SURETEXT(szValue);

	szValue = pXmlNode->GetProperty(_T("md5Lyrics"));
	strMd5Lyrics = SURETEXT(szValue);

//	szValue = pXmlNode->GetProperty(_T("album"));
//	strAlbum = SURETEXT(szValue);
//
//	szValue = pXmlNode->GetProperty(_T("title"));
//	strTitle = SURETEXT(szValue);

	return ERR_OK;
}

int MLMsgCmdPostSongInfo::SongInfo::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(_T("songinfo"));
	xmlFile.WriteAttribute(_T("md5Song"), strMd5Song.c_str());
	xmlFile.WriteAttribute(_T("md5Lyrics"), strMd5Lyrics.c_str());
//	xmlFile.WriteAttribute(_T("album"), strAlbum.c_str());
//	xmlFile.WriteAttribute(_T("title"), strTitle.c_str());
	xmlFile.WriteEndElement();

	return ERR_OK;
}

int MLMsgCmdPostSongInfo::FromXML(SXNode *pXmlNode)
{
	SXNode::iterator	it, itEnd;

	itEnd = pXmlNode->listChildren.end();
	for (it = pXmlNode->listChildren.begin(); it != itEnd; ++it)
	{
		if (_tcsicmp((*it)->strName.c_str(), _T("songinfo")) == 0)
		{
			SongInfo	songInfo;
			songInfo.FromXML(*it);
			vSongInfo.push_back(songInfo);
		}
	}

	return ERR_OK;
}

int MLMsgCmdPostSongInfo::ToXML(CXMLWriter &xmlFile)
{
	xmlFile.WriteStartElement(MZ_POST_SONGINFO);

	for (int i = 0; i < (int)vSongInfo.size(); i++)
		vSongInfo[i].ToXML(xmlFile);

	xmlFile.WriteEndElement();

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
//

CMLPacketAccountMgr		*CMLPacketAccountMgr::m_pMLPacketWrapper = NULL;

CMLPacketAccountMgr::CMLPacketAccountMgr()
{
	m_packFlag = MPV_V1_MD5_ID;
	m_bEnableXmlPack = true;
	m_nXmlPackCount = 0;
	m_nV1PackCount = 0;
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
	MAP_ACCOUNT::iterator	it;

	it = m_mapAccount.find(nId);
	if (it == m_mapAccount.end())
		return;
	else
		m_mapAccount.erase(it);
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
		int			i;

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

		for (i = 0; i < (int)buff.size(); i++)
			n += (BYTE)(buff[i]);
		head.byXor = BYTE(n / buff.size());

		for (i = 0; i < (int)buff.size(); i++)
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

		m_pAccountMgr->m_nXmlPackCount++;

		// 在服务器端，需要根据客户端发送的packet类型，转换到对应的packet类型
		if (m_pAccountMgr->m_bServer)
			m_packFlag = MPV_XML;
		return ERR_OK;
	}

	if (pHead->byVersion == MPV_V1_MD5_ID)
	{
		m_pAccountMgr->m_nV1PackCount++;
		const char		*szPwd;
		unsigned char	Md5[16];
		int				i;

		for (i = sizeof(MLEncodePacketV1); i < (int)buff.size(); i++)
			buff[i] = buff[i] ^ pHead->byXor;

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
		return ERR_OK;
	}

	ERRLOG1(_T("CMLPacketWrapper::Unwrapp() Invalid Packet Version: %d"), pHead->byVersion);
	m_pAccountMgr->m_nInvalidPackCount++;

	return ERR_INVALID_PACK_VER;
}

//////////////////////////////////////////////////////////////////////////
//

#define ALLOC_BLOCK_LEN		(20 * 1024)


//////////////////////////////////////////////////////////////////////////
CMsgProtocol::CMsgProtocol()
{
}

CMsgProtocol::~CMsgProtocol()
{
}


int CMsgProtocol::ReadMsg(MLMsgType &msgType, CCharBuffer *pMsgBuff, int nMaxBuff, BOOL *pbMLAgent)
{
	ASSERT(m_pNetFile);
	int		nDataLength;

	nDataLength = 0;

	int		nCapaLen;		// 可用容量
	int		nRead;
	char	*pszMsg;
	int		nRet;

	ASSERT(pMsgBuff->size() == 0);
	if (m_buffMsg.size() > 0)
	{
		pMsgBuff->append(m_buffMsg.c_str(), m_buffMsg.size());
		m_buffMsg.clear();
	}

	if (pMsgBuff->size() < 4)
	{
		nCapaLen = pMsgBuff->capacity();
		pszMsg = (char *)pMsgBuff->data();
		pszMsg += pMsgBuff->size();
		nRet = m_pNetFile->Read(pszMsg, nCapaLen - pMsgBuff->size(), &nRead);
		if (nRet != ERR_OK)
			return nRet;

		pMsgBuff->setSize(pMsgBuff->size() + nRead);
		if (pMsgBuff->size() < 4)
			return ERR_BAD_MSG;
	}
	pszMsg = (char *)pMsgBuff->data();
	if (_strnicmp(pszMsg, "POST", 4) == 0)
	{
		// HTTP 消息
		m_httpRequest.SetCacheData(pMsgBuff->data(), pMsgBuff->size());
		m_httpRequest.SetNetFile(m_pNetFile);
		nRet = m_httpRequest.ReadHead(TRUE);
		if (nRet != ERR_OK)
			return nRet;

		ASSERT(m_httpRequest.GetRequestType() == CHttpRequestProtocol::RQ_POST);
		pMsgBuff->clear();
		nRet = m_httpRequest.ReadData(pMsgBuff, nMaxBuff);
		if (nRet != ERR_OK)
			return nRet;

		if (pbMLAgent)
		{
			if (_strnicmp("MiniLyrics", m_httpRequest.GetPropValue(CHttpRequestProtocol::RP_USER_AGENT), 10) == 0)
				*pbMLAgent = TRUE;
			else
				*pbMLAgent = FALSE;
		}

		// continue...
		msgType = MT_HTTP_POST;
		return ERR_OK;
	}
	else if (_strnicmp(pszMsg, "HTTP", 4) == 0)
	{
		// HTTP 消息
		m_httpReturn.SetCacheData(pMsgBuff->data(), pMsgBuff->size());
		m_httpReturn.SetNetFile(m_pNetFile);
		nRet = m_httpReturn.ReadHead(TRUE);
		if (nRet != ERR_OK)
			return nRet;

		pMsgBuff->clear();
		nRet = m_httpReturn.ReadData(pMsgBuff, nMaxBuff);
		if (nRet != ERR_OK)
			return nRet;

		// continue...
		msgType = MT_HTTP_RET;
		return ERR_OK;
	}
	else if (_strnicmp(pszMsg, "GET", 3) == 0)
	{
		// HTTP 消息
		m_httpRequest.SetCacheData(pMsgBuff->data(), pMsgBuff->size());
		m_httpRequest.SetNetFile(m_pNetFile);
		nRet = m_httpRequest.ReadHead(TRUE);
		if (nRet != ERR_OK)
			return nRet;

		ASSERT(m_httpRequest.GetRequestType() == CHttpRequestProtocol::RQ_GET);
		pMsgBuff->clear();
		nRet = m_httpRequest.ReadData(pMsgBuff, nMaxBuff);
		if (nRet != ERR_HTTP_GET_NODATA)
			return nRet;

/*
		// continue...
		m_httpReturn.SetNetFile(m_pNetFile);
		m_httpReturn.SetAvailProp(CHttpReturnProtocol::RP_MIN_RET);
		m_httpReturn.SetServer("MiniLyrics Server/1.1");
		m_httpReturn.SetConnection("close");
		m_httpReturn.WriteHead();
		m_httpReturn.WriteData(SZ_ML_SERVER_HTTP, sizeof(SZ_ML_SERVER_HTTP));
*/
		msgType = MT_HTTP_GET;
		return ERR_OK;

		// return ERR_HTTP_401;
	}

	nCapaLen = pMsgBuff->capacity();
	nDataLength = 0;
	char	szMsgType[ML_MSG_XML_LEN + 1];
	char	szMsgDataLen[ML_MSG_XML_DATA_LEN + 1];

	while (1)
	{
		// 需要继续接收
//		if (pMsgBuff->GetLength() < 10)	// 每条消息的最小长度
//		{
//			DBGLOG0(_T("收到的消息长度小于最小长度 10 BYTE"));
//			return FALSE;
//		}
		//
		// 如果没有接收到数据长度，则继续接收
		if (nDataLength == 0 && pMsgBuff->size() > ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN)
		{
			// DBGLOGDUMPS(pMsgBuff->GetBuffer(), pMsgBuff->GetLength());

			//
			// 读入数据长度和消息类型
//			char	*szMsgHeader;
			// 消息类型
//			szMsgHeader = (char *)m_buffMsgHeader.GetBuffer();

			pszMsg = pMsgBuff->data();
			strncpy(szMsgType, pszMsg, ML_MSG_XML_LEN);
			szMsgType[ML_MSG_XML_LEN] = '\0';

			strncpy(szMsgDataLen, pszMsg + ML_MSG_XML_LEN, ML_MSG_XML_DATA_LEN);
			szMsgDataLen[ML_MSG_XML_DATA_LEN] = '\0';

			char	*pTemp = szMsgDataLen;
			while (*pTemp == ' ')
				pTemp++;
			nDataLength = atoi(pTemp);

			if (nDataLength > 0xFFFFFF || nDataLength <= 0)
			{
				// 收到的是非法的
				pMsgBuff->clear();
				LOG1(LOG_LVL_ERROR, _T("INVALID MSG!! Length: %d Invalid! CMGCPCom::OnRecieve(), Drop it!"), nDataLength);
				return ERR_BAD_MSG;
			}

			// 去掉头部
			pMsgBuff->erase(ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN, pMsgBuff->size() - (ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN));
		}

		// int		nCurMsgLen = ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN + nDataLength;
		if (nDataLength > 0 && nDataLength <= (int)pMsgBuff->size())
		{
			//
			// 已经读完了一条消息
			if (nDataLength > (int)pMsgBuff->size())
				m_buffMsg.append(pMsgBuff->data() + nDataLength, pMsgBuff->size() - nDataLength);
			// pMsgBuff->Cut(ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN, pMsgBuff->GetLength() - ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN);
			break;
		}

		if (nCapaLen <= ALLOC_BLOCK_LEN + (int)pMsgBuff->size())
		{
			//
			// 当前分配的空间太小
			if (nCapaLen >= nMaxBuff)
				return ERR_OUTOF_BUFF;
			nCapaLen += ALLOC_BLOCK_LEN;
			if (nCapaLen > nMaxBuff)
				nCapaLen = nMaxBuff;

			pMsgBuff->reserve(nCapaLen);
		}

		pszMsg = (char *)pMsgBuff->data();
		pszMsg += pMsgBuff->size();
		nRet = m_pNetFile->Read(pszMsg, nCapaLen - pMsgBuff->size(), &nRead);
		if (nRet == ERR_OK)
		{
			// 判断是否已经关闭socket
			if (nRead == 0)
				return ERR_NOT_RECIVED_ALL;
			// 继续
			pMsgBuff->setSize(pMsgBuff->size() + nRead);
		}
		else
			return nRet;
	}

	if (strcmp(szMsgType, ML_MSG_XML) == 0)
		msgType = MT_XML;

	return ERR_OK;
}
//
//int CMsgProtocol::ContinueReadMsg(int nDataLength, CCharBuffer *pBuffer, int nMaxBuff)
//{
//	return ERR_OK;
//}

int CMsgProtocol::WriteMsg(char *szMsgType, CCharBuffer *pMsgBuff)
{
	char	szHeader[256];
	char	szFormat[32];
	int		nWrite;
	int		nRet;

	// %3s%6d
	sprintf(szFormat, "%%%ds%%%dd", ML_MSG_XML_LEN, ML_MSG_XML_DATA_LEN);

	sprintf(szHeader, szFormat, szMsgType, pMsgBuff->size());

	nRet = m_pNetFile->Write(szHeader, ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN, &nWrite);
	if (nRet != ERR_OK)
		return nRet;

	// DBGLOG1("WriteMsg: %s", szHeader);
	// DBGLOGDUMPS(pMsgBuff->GetBuffer(), pMsgBuff->GetLength());
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
int CMLProtocol::ReadMsg(MLMsgType &msgType, CCharBuffer *pMsgBuff, BOOL *pbMLAgent)
{
//	TrackFunctionEx("CMLProtocol::ReadMsg");

	int		nRet;

	pMsgBuff->clear();
	nRet = CMsgProtocol::ReadMsg(msgType, pMsgBuff, ML_MSG_MAX_LENGTH, pbMLAgent);
	if (nRet == ERR_OK)
	{
#ifdef _DEBUG
		if (g_Profile.GetProfileInt(_T("PrintMLRecv"), FALSE))
			DBGLOGDUMPS(pMsgBuff->c_str(), pMsgBuff->size());
#endif
		if ((msgType == MT_HTTP_POST && strcmp(m_httpRequest.GetUrl(), SZ_ML_POST_URL) == 0)
			 || msgType == MT_HTTP_RET)
		{
			// 在此对消息类型进行转换
			msgType = MT_HTTP_POST_XML;

			nRet = m_packetWrapper.Unwrapp(*pMsgBuff);
			if (nRet != ERR_OK)
				return nRet;
		}
//
//		if (strcmp(szMsgType, ML_MSG_XML) == 0)
//			msgType = MT_XML;
//		else if (strcmp(szMsgType, ML_MSG_BIN) == 0)
//			msgType = MT_BIN;
//		else if (strcmp(szMsgType, ML_MSG_HTTP) == 0)
//			msgType = MT_HTTP;
//		else
//			msgType = MT_UNKNOWN;
	}

	return nRet;
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
//	m_httpReqWriteXmlCache.SetNetFile(m_pNetFile);
//	m_httpReqWriteXmlCache.SetHost(_T("www.minilyrics.com"), 80);
//
//	m_httpReqWriteXmlCache.SetRequestType(CHttpRequestProtocol::RQ_GET);
//	m_httpReqWriteXmlCache.SetAvailProp(CHttpRequestProtocol::RP_BASE_GET);
//
//	m_httpReqWriteXmlCache.SetUrl(_T("/"));
//
//	m_httpReqWriteXmlCache.SetPropValue(CHttpRequestProtocol::RP_CONNECTION, _T(""));
//	m_httpReqWriteXmlCache.SetProxyBase64Pass(FALSE);
//	m_httpReqWriteXmlCache.SetPropValue(CHttpRequestProtocol::RP_HOST, _T("www.getlyrics.com"));
//	m_httpReqWriteXmlCache.SetPropValue(CHttpRequestProtocol::RP_ACCEPT, _T("*/*"));
//	m_httpReqWriteXmlCache.SetPropValue(CHttpRequestProtocol::RP_REFERER, _T("http://www.minilyrics.com"));
//	m_httpReqWriteXmlCache.SetPropValue(CHttpRequestProtocol::RP_USER_AGENT, _T("Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)"));
//
//	m_httpReqWriteXmlCache.WriteHead();
/*

	m_httpReqWriteXmlCache.SetRequestType(CHttpRequestProtocol::RP_BASE_GET);
	m_httpReqWriteXmlCache.SetUrl(_T(SZ_ML_POST_URL));
	m_httpReqWriteXmlCache.SetAvailProp(CHttpRequestProtocol::RP_POST_MIN);
// 	m_httpReqWriteXmlCache.SetPropValue(CHttpRequestProtocol::RP_HOST, _T("www.minilyrics.com"));
	m_httpReqWriteXmlCache.SetPropValue(CHttpRequestProtocol::RP_USER_AGENT, _T("MiniLyrics 3.0"));


	m_httpRetWriteXmlCache.SetNetFile(m_pNetFile);
	m_httpRetWriteXmlCache.ReadHead(TRUE);
*/
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
		return WriteMsg(ML_MSG_XML, pMsgBuff);
	}
}

int CMLProtocol::ParseXMLMsg(CCharBuffer *pMsgBuff, CSimpleXML &xml)
{
//	TrackFunctionEx("CMLProtocol::ParseXMLMsg");

/*	char	szTempXmlFile[MAX_PATH];


	if (!SaveBufferAsTempFile(szTempXmlFile, pMsgBuff->GetBuffer(), pMsgBuff->GetLength()))
		return ERR_FALSE;

	if (!ParseXmlFile(xml, szTempXmlFile))
	{
		DeleteFile(szTempXmlFile);
		return ERR_FALSE;
	}
	DeleteFile(szTempXmlFile);
*/
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


