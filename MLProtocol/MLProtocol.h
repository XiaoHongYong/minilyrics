// MLProtocol.h: interface for the CMLProtocol class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MLPROTOCOL_H__BB1B1F2A_F36F_47C4_B264_AA43E755E09C__INCLUDED_)
#define AFX_MLPROTOCOL_H__BB1B1F2A_F36F_47C4_B264_AA43E755E09C__INCLUDED_

/***********************************************************************
History of MLProtocol:

ML 3.0:			Old XML message format, encoding: gb2312(system default).

ML 3.1 ~ 3.9:	HTTP tunnel XML message format, encoding: gb2312(system default).
				Deprecated: Protocol Version: "0.9"

ML 4.0 ~ 7.0:	HTTP tunnel Encrypted XML message format, encoding: UTF-8

ML 7.1 ~ :		HTTP tunnel Encrypted Binary XML message format, encoding: UTF-8
2011.03			support batch download, ML Binary XML message format

/***********************************************************************/

#pragma once

#include "NetServer.h"
#include "HttpProtocol.h"
#include "MLUtils.h"

DECLAR_CPPUNIT_TEST_REG(CMLProtocol)

typedef list<tstring>		ListString;

enum ML_PROTOCAL_ERROR
{
	ERR_UNKNOWN				= 1035, // 未翻译出来的结果
	ERR_BAD_XMLREQUEST		= 1036,
	ERR_CORRUPTTED_MSG		= 1037,	// The message is corrupted
	ERR_REDIRECT			= 1038,	// 重定向连接
	ERR_REG_EMAIL_EXIST		= 1040,
	ERR_REG_USER_EXIST		= 1041,
	ERR_UPLOAD_EXIST		= 1042,	// 上传的内容已经存在
	ERR_USER_NOT_EXIST		= 1043,	// 用户不存在
	ERR_USER_EXPIRED		= 1044,	// 用户已经过期
	ERR_BAD_USERPWD			= 1045, // 用户名或者密码错误
	ERR_BAD_CLIENT_VER		= 1047,	// 客户端版本不正确, 无法登陆
	ERR_OBSOLETE_CLIENT_VER = 1048,	// 客户端版本太低, 无法登陆
	ERR_ACCESS_DENIED		= 1051,	// 无权访问,被拒绝
	ERR_NOT_SUPPORT_CMD		= 1052,	// 还不支持的命令
	ERR_CMD_NOT_MATCH		= 1053,	// 命令不匹配
	ERR_CMD_PARAM			= 1054,	// 参数错误
	ERR_UPLOAD_FILETYPE		= 1056,	// 上传的文件类型错误
	ERR_BAD_BASE64CONTENT	= 1058,	// Base64 编码错误
	ERR_BAD_FILE_CONTENT	= 1059,	// 文件内容有错误，或者包含不允许的内容
	ERR_REG_USER_NAME_SHORT	= 1060,	// 用户名太短
	ERR_NOT_AUTHORIZED		= 1061,	// 未经授权的行为
	ERR_NOT_LOGIN			= 1062,	// 未登陆，请登陆
	ERR_UPDATE_LRC_INFO_SHOULD_SAME	= 1063, // 更新歌词时，前后的歌词标题和作者必须一致
	ERR_LYR_NO_TAG_FOUND	= 1064,	// No lyrics tag is found in content.
};

#define SERVER_PORT_BEGIN			8000
#ifdef _IPHONE
#define SERVER_PORT_END				8010
#else
#define SERVER_PORT_END				8050
#endif

#define ML_SERVER_PORT				1212


enum MLMsgType
{
	MT_UNKNOWN,			// 未知格式
	MT_XML,				// xml的消息格式

	MT_HTTP_GET,		// Http get request
	MT_HTTP_POST,		// Http post request
	MT_HTTP_RET,		// Http return

	MT_HTTP_XML,		// Minilyrics 请求的HTTP tunnel的消息格式,
						// translated from MT_HTTP_*
	MT_HTTP_BIN_XML,	// Similar with MT_HTTP_XML, but it's binary xml format.
						// See CMLBinXMLParser
};

//
// Command send from client to server
//
enum MLMsgCmd
{
	MC_UNKNOWN,				// Unknown
	MC_LOGIN,				// Log in
	MC_SEARCH,				// Search lyrics
	MC_BATCH_SEARCH,		// Search lyrics by batch
	MC_CAN_UPLOAD,			// Deprecated command. return ERR_NOT_FOUND dirrectly.
	MC_UPLOADV0,			// Old version of upload lyrics.
	MC_UPLOAD,				// Upload lyrics command

	// Admin messages
	MC_GET_LYR_UPDATE,
	MC_SUBMIT_LYR_REVIEW,
	MC_EXE_SVR_CMD,
};

MLMsgCmd MLMsgStr2Cmd(LPCTSTR szMLMsgCmd);


#define MZ_LOGIN			_T("login")
#define MZ_RETURN			_T("return")
#define MZ_SEARCH			_T("search")
#define MZ_BATCH_SEARCH		_T("batchsearch")		// MiniLyrics 7.1 or later
#define MZ_CANUPLOAD		_T("canupload")
#define MZ_UPLOADV0			_T("upload")
#define MZ_UPLOAD			_T("uploadV1")

#define MZ_GET_LYR_UPDATE	_T("GetLyrUpdate")
#define MZ_SUBMIT_LYR_REVIEW _T("SubmitLyrReview")
#define MZ_EXE_SVR_CMD		_T("ExeSvrCmd")

#define MZ_FILEINFO			_T("fileinfo")
#define MZ_ITEM				_T("item")
#define MZ_ORG_CMD			_T("orgcmd")
#define MZ_CMD_RESULT		_T("result")
#define MZ_SESSIONID		_T("sessionid")

class CXMLMsgCmd
{
public:
	virtual ~CXMLMsgCmd();

	virtual int FromXML(SXNode *pXmlNode) = 0;
	virtual int ToXML(CXMLWriter &xmlFile) = 0;

};


// Returned message
class MLRetMsg : public CXMLMsgCmd
{
public:
	tstring		strOrgCmd;		// 原来的请求命令
	int			result;

public:
	virtual int FromXML(SXNode *pXmlNode);
	virtual int ToXML(CXMLWriter &xmlFile);
	void ToXMLAttribute(CXMLWriter &xmlFile);

};

// Login command
class MLMsgCmdLogin : public CXMLMsgCmd
{
public:
	tstring		strName;
	tstring		strPwd;
	tstring		strClient;
	tstring		strRegCode;

public:
	virtual int FromXML(SXNode *pXmlNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

// Return of log in command
class MLMsgRetLogin : public MLRetMsg
{
public:
	tstring		strMessage;				// 消息

public:
	virtual int FromXML(SXNode *pXmlNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

// Returned search result lyrics info
class RetLyrInfo
{
public:
	tstring		strArtist;
	tstring		strTitle;
	tstring		strAlbum;
	tstring		strFileName;
	tstring		strUploader;
	tstring		strLink;
	int			nTimeLength;			// Media Length in second
	float		fRate;					// rate info of lyrics
	int			nRateCount;				// 
	int			nDownloads;				// total downloads of lyrics

public:
	RetLyrInfo()
	{
		nTimeLength = 0;
		fRate = 0.0;
		nRateCount = 0;
		nDownloads = 0;
	}
	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

typedef list<RetLyrInfo>	RetLyrInfoList;

enum ML_EDITION
{
	ME_NOTSET	= 0,
	ME_ENGLISH	= 1,
	ME_CHINESE	= 2,
};

// Search lyrics command
class MLMsgCmdSearch : public CXMLMsgCmd
{
public:
	tstring		strArtist;
	tstring		strTitle;
	tstring		strClient;			// client name + client ver
	tstring		strRegCode;
	int			nRequestPage;
	bool		bOnlyMatchedLyrics; // Return only matched lyrics

public:
	MLMsgCmdSearch()
	{
		nRequestPage = 0;
		bOnlyMatchedLyrics = false;
	}
	virtual int FromXML(SXNode *pXmlNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

enum REGCODE_VERIFY_VALUE
{
	RVV_OK			= 0,
	RVV_TRIAL		= 1,
	RVV_INVALID		= 2,
};

// Return of search lyrics command
class MLMsgRetSearch : public MLRetMsg
{
public:
	REGCODE_VERIFY_VALUE	RegVerifyValue;
	tstring					strMessage;
	int						nPageCount, nCurPage;

	list<RetLyrInfo>		listResultFiles;

public:
	MLMsgRetSearch()
	{
		RegVerifyValue = RVV_OK;
		nPageCount = 1;
		nCurPage = 0;
	}
	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

// Batch lyrics search item
class MLSearchItem
{
public:
	MLSearchItem() : nMediaLength(0) { }

	int FromXML(SXNode *pXmlNode);
	int ToXML(CXMLWriter &xmlFile);

	tstring		strArtist, strTitle;
	int			nMediaLength;			// in seconds

	// Fields below are used on in CLIENT of MiniLyrics.
	tstring		strMediaFile, strLyrFile;
	int			nIndex;

};
typedef vector<MLSearchItem>		MLListSearchItems;

// Client sent to server to search lyrics by batch.
class MLMsgCmdBatchSearch : public CXMLMsgCmd
{
public:
	tstring		strClient;		// client name + client ver
	tstring		strRegCode;
	MLListSearchItems	listSearchItems;

public:
	MLMsgCmdBatchSearch();

	virtual int FromXML(SXNode *pXmlNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

// Returned lyrics info for batch lyrics search.
class MLLyricsInfoLite
{
public:
	tstring			strFile; // strArtist, strTitle, 
	int				nIndex;
	string			bufLyrContent;

public:
	virtual int FromXML(SXNode *pXmlNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

typedef list<MLLyricsInfoLite>		ListLyricsInfoLite;

// The return message of batch lyrics search
class MLMsgRetBatchSearch : public MLRetMsg
{
public:
	REGCODE_VERIFY_VALUE	RegVerifyValue;
	tstring					strMessage;
	ListLyricsInfoLite		listLyricsInfo;		// Lyrics info: artist, title, file name

public:
	MLMsgRetBatchSearch()
	{
		RegVerifyValue = RVV_OK;
	}
	virtual int FromXML(SXNode *pXmlNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};


// Old lyrics upload command
//  7.0 and before.
class MLMsgCmdUploadV0 : public CXMLMsgCmd
{
public:
	tstring		strLoginName;
	tstring		strFileContent;			// Uploaded lyrics content, encoded in base64.

public:
	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};
typedef MLMsgRetSearch MLMsgRetUploadV0;

// Upload lyrics command 7.1 or later
class MLMsgCmdUpload : public CXMLMsgCmd
{
public:
	tstring		strLoginName;
	tstring		strPwdMask;
	tstring		strFileContent;			// FileContentMust be in UTF-8 encoding, NO BOM.

public:
	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};
typedef MLMsgRetSearch MLMsgRetUploadV0;

// New version of upload lyrics command (since 7.1 or later)
class MLMsgRetUpload : public MLMsgRetUploadV0
{
public:
	tstring					strLyricsId;	// Returned lyrics, this will be saved in the uploaded lyrics.

public:
	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

// Old xml message (3.0)
#define ML_MSG_MAX_LENGTH		(1024 * 1024)
#define ML_MSG_XML				"XML"
#define ML_MSG_XML_FMT			"XML%6d"
#define ML_MSG_XML_LEN			3
#define ML_MSG_XML_DATA_LEN		6


enum ML_PACK_VERSION
{
	MPV_INVALID			= 0,
	MPV_XML				= '<',		// 没有加密，直接为 XML 协议
	MPV_V1_MD5_ID		= 2,		// 
};

#pragma pack(push, 1)
struct MLEncodePacketV1
{
	BYTE		byVersion;		// 协议的版本：
	BYTE		byXor;			// XOR 解密的字段, byDataOrg 的平均值 ^ 0x9D
	BYTE		byID[4];		// 客户端使用的MD5验证码ID
								// nID = byID[0] | byID[1] << 8 | byID[2] << 16 | byID[3] << 24
	BYTE		byMd5[16];		// 校验 消息包正确性的MD5值

	void SetID(long nId)
	{
		byID[0] = (BYTE)(nId & 0xFF);
		byID[1] = (BYTE)((nId >> 8) & 0xFF);
		byID[2] = (BYTE)((nId >> 16) & 0xFF);
		byID[3] = (BYTE)((nId >> 24) & 0xFF);
	}

	long GetID()
	{
		return byID[0] | (byID[1] << 8) | (byID[2] << 16) | (byID[3] << 24);
	}
};
#pragma pack(pop)

class CMLPacketAccountMgr
{
public:
	static CMLPacketAccountMgr *GetInstance();

	void AddAccount(long nId, const char*szPwd);
	void EraseAccount(long nId);

	const char* GetPwd(long nAccountId);

	void SetMode(bool bServer, ML_PACK_VERSION packVer)
	{
		m_bServer = bServer;
		m_packFlag = packVer;
	}

protected:
	CMLPacketAccountMgr();

	typedef map<long, string>	MAP_ACCOUNT;

	MAP_ACCOUNT					m_mapAccount;

	static CMLPacketAccountMgr	*m_pMLPacketWrapper;

public:
	ML_PACK_VERSION				m_packFlag;
	bool						m_bEnableXmlPack;		// 是否允许未加密的 XML packet
	bool						m_bServer;				// 是否为服务器端

	// 统计数据
	long						m_nInvalidV1PackCount;	// 无效的V1 packet 数目
	long						m_nInvalidPackCount;	// 无效的 packet 数目

};

class CMLPacketWrapper
{
public:
	CMLPacketWrapper();

	int Wrapp(CCharBuffer &buff);
	int Unwrapp(CCharBuffer &buff);

	void SetId(int nId)
		{ m_nCurrentId = nId; }

	ML_PACK_VERSION GetPackVersion() { return m_packFlag; }

protected:
	long						m_nCurrentId;
	CMLPacketAccountMgr			*m_pAccountMgr;
	ML_PACK_VERSION				m_packFlag;

};

class CMLProtocol
{
public:
	CMLProtocol();
	virtual ~CMLProtocol();

	int Init(CNetFile *pFile, LPCSTR szHost, int nPort);

	int Init(CNetFile *pFile, LPCWSTR szHost, int nPort);

	int SetNetFile(CNetFile *pFile) { m_pNetFile = pFile; return ERR_OK; }

	void SetProxy(BOOL bEnableProxy, LPCSTR szProxyServer, int nProxyPort, LPCSTR szBase64ProxyUserPass = NULL);

	int ReadMsg(MLMsgType &msgType, CCharBuffer *pMsgBuff);

	int ReadOldXmlMsg(MLMsgType &msgType, CCharBuffer *pMsgBuff);
	int WriteOldXmlMsg(CCharBuffer *pMsgBuff);

	int WriteXMLMsg(MLMsgType msgType, CCharBuffer *pMsgBuff);

	int ParseXMLMsg(CCharBuffer *pMsgBuff, CSimpleXML &xml);

	MLMsgCmd GetMsgCommand(CSimpleXML &xml);

	void SetClientId(int nId)
	{
		m_packetWrapper.SetId(nId);
	}

protected:
	//
	// proxy
	BOOL					m_bUseProxy;
	tstring					m_strBase64ProxyUserPass;

	// 为了提高效率，而设置的缓存区
	CHttpRequestProtocol	m_httpReqWriteXmlCache;
	CHttpReturnProtocol		m_httpRetWriteXmlCache;

	CNetFile				*m_pNetFile;

public:
	CHttpRequestProtocol	m_httpRequest;
	CHttpReturnProtocol		m_httpReturn;

	CMLPacketWrapper		m_packetWrapper;

};

class CMLServerProtocol : public CMLProtocol
{
public:

};

#endif // !defined(AFX_MLPROTOCOL_H__BB1B1F2A_F36F_47C4_B264_AA43E755E09C__INCLUDED_)
