// MLProtocol.h: interface for the CMLProtocol class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MLPROTOCOL_H__BB1B1F2A_F36F_47C4_B264_AA43E755E09C__INCLUDED_)
#define AFX_MLPROTOCOL_H__BB1B1F2A_F36F_47C4_B264_AA43E755E09C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetServer.h"
#include "HttpProtocol.h"
#include "MLUtils.h"

enum ML_PROTOCAL_ERROR
{
	ERR_UNKNOWN				= 1035, // 未翻译出来的结果
	ERR_BAD_XMLREQUEST		= 1036,
	ERR_REDIRECT			= 1038,	// 重定向连接
	ERR_REG_EMAIL_EXIST		= 1040,
	ERR_REG_USER_EXIST		= 1041,
	ERR_UPLOAD_EXIST		= 1042,	// 上传的内容已经存在
	ERR_USER_NOT_EXIST		= 1043,	// 用户不存在
	ERR_USER_EXPIRED		= 1044,	// 用户已经过期
	ERR_BAD_USERPWD			= 1045,		// 用户名或者密码错误
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
};

#define SERVER_PORT_BEGIN			8000
#define SERVER_PORT_END				8050

#define ML_SERVER_PORT				1212

#define ML_PROTOCOL_VERION_V8		_T("0.8")
#define ML_PROTOCOL_VERION			_T("0.9")

enum MLMsgType
{
	MT_UNKNOWN,			// 未知格式
	MT_XML,				// xml的消息格式
	MT_BIN,				// 二进制的消息格式
	MT_HTTP_GET,
	MT_HTTP_POST,		// Http post 的消息格式
	MT_HTTP_POST_XML,	// Minilyrics 请求的HTTP tunnel的消息格式
	MT_HTTP_RET,		// Http return的消息格式
};

enum MLMsgCmd
{
	//
	// 从客户端发送到服务器的消息
	MC_UNKNOWN,				// 未知
	MC_LOGIN,				// 登陆
	MC_SEARCH,				// 搜索（歌词……）
	MC_CAN_UPLOAD,			// 是否能够上传（歌词……），服务器返回结果，如果不能上传会返回具体的原因
	MC_UPLOAD,				// 上传（歌词……）
	MC_POST_BUG,			// 发送Bug

	//
	// 从服务器发送到客户端的消息
	MC_RETURN,				// 服务器返回客户端的应答消息
};

MLMsgCmd MLMsgStr2Cmd(LPCTSTR szMLMsgCmd);


#define MZ_LOGIN			_T("login")
#define MZ_RETURN			_T("return")
#define MZ_SEARCH			_T("search")
#define MZ_CANUPLOAD		_T("canupload")
#define MZ_UPLOAD			_T("upload")
#define MZ_POST_BUG			_T("bug")

#define MZ_FILEINFO			_T("fileinfo")
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


// 命令请求的执行结果
class MLRetMsg : public CXMLMsgCmd
{
public:
	tstring		strOrgCmd;		// 原来的请求命令
	int			result;

public:
	virtual int FromXML(SXNode *pXmlNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

// 客户端登陆服务器
//
// 返回值：
//		ERR_OK			-	成功
//		ERR_REDIRECT		-	重定向连接
//		ERR_USER_NOT_EXIST-	用户不存在
//		ERR_BAD_USERPWD	-	用户名或者密码错误
//		ERR_BAD_CLIENT_VER,
//		ERR_OBSOLETE_CLIENT_VER,
class MLMsgCmdLogin : public CXMLMsgCmd
{
public:
	// 登陆时的信息
	tstring		strProtocolVer;		// 使用的协议版本
	tstring		strName;
	tstring		strPwd;
	tstring		strClient;
	tstring		strRegCode;

public:
	virtual int FromXML(SXNode *pXmlNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

class MLMsgRetLogin : public MLRetMsg
{
public:
	tstring		strMessage;				// 消息

public:
	virtual int FromXML(SXNode *pXmlNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

// 搜索结果，上传信息中的文件信息
class MLFileInfo
{
public:
	tstring		strArtist;
	tstring		strTitle;
	tstring		strAlbum;
	tstring		strFileName;
	tstring		strMD5;
	tstring		strUploader;
	tstring		strEncoding;
	tstring		strLink;
	int			nMatchValue;			// 搜索结果比较得分
	int			nTimeLength;			// 歌曲（歌词）长度，单位s
	float		fRate;					// rate info of lyrics
	int			nRateCount;				// 
	int			nDownloads;				// total downloads of lyrics

public:
	MLFileInfo()
	{
		nMatchValue = 0;
		nTimeLength = 0;
		fRate = 0.0;
		nRateCount = 0;
		nDownloads = 0;
	}
	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};

typedef list<MLFileInfo>	MLFileInfoList;

enum ML_EDITION
{
	ME_NOTSET	= 0,
	ME_ENGLISH	= 1,
	ME_CHINESE	= 2,
};

// 客户端向服务器请求搜索命令
//
// 返回值:
//		ERR_OK			-	成功
//		ERR_REDIRECT		-	重定向连接
//		ERR_NOT_FOUND
//		ERR_ACCESS_DENIED
class MLMsgCmdSearch : public CXMLMsgCmd
{
public:
	tstring		strArtist;
	tstring		strTitle;
	tstring		strProtocolVer;		// 使用的协议版本
	tstring		strClient;	// client name + client ver
	tstring		strRegCode;
	tstring		strClientCharEncoding;		// 客户的使用的字符编码，服务器用户将utf-8转换为客户端的编码，搜索mbcs的旧歌词
	int			nRequestPage;
	bool		bReturnMatchedLyrics;

public:
	MLMsgCmdSearch()
	{
		nRequestPage = 0;
		bReturnMatchedLyrics = false;
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

// 客户端向服务器请求搜索命令执行的结果
class MLMsgRetSearch : public MLRetMsg
{
public:
	REGCODE_VERIFY_VALUE	RegVerifyValue;
	tstring					strMessage;
	int						nPageCount, nCurPage;

	list<MLFileInfo>		listResultFiles;

public:
	MLMsgRetSearch()
	{
		// bInvalidRegCode = FALSE;
		RegVerifyValue = RVV_OK;
		nPageCount = 1;
		nCurPage = 0;
	}
	virtual int FromXML(SXNode *pXmlNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};


// 客户端向服务器请求判断是否能够上传
//
// 返回值:
//		ERR_OK			-	成功
//		ERR_REDIRECT		-	重定向连接
class MLMsgCmdCanUpload : public CXMLMsgCmd
{
public:
	tstring		strLoginName;
	tstring		strArtist;
	tstring		strAlbum;
	tstring		strTitle;
	int			nTimeLength;		// 歌曲（歌词）长度，单位ms
	tstring		strFileName;
	tstring		strMD5;				// 文件的MD5值
	tstring		strClientCharEncoding;

public:
	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};
typedef MLMsgRetSearch MLMsgRetCanUpload;


// 客户端向服务器请求上传
//
// 返回值:
//		ERR_OK			-	成功
//		ERR_REDIRECT		-	重定向连接
class MLMsgCmdUpload : public CXMLMsgCmd
{
public:
	tstring		strLoginName;
	tstring		strArtist;
	tstring		strAlbum;
	tstring		strTitle;
	tstring		strFileName;
	int			nTimeLength;			// 歌曲（歌词）长度，单位ms
	tstring		strMD5;					// 文件的MD5值
	tstring		strFileContent;			// 此字段有效，存储的文件内容
	tstring		strFileContentOld;		// 此字段有效，存储的文件内容
	tstring		strContentEncoding;
	tstring		strClientCharEncoding;

public:
	virtual int FromXML(SXNode *pNode);
	virtual int ToXML(CXMLWriter &xmlFile);

};
typedef MLMsgRetSearch MLMsgRetUpload;


// 客户端向服务器发送bug
//
// 返回值:
//		ERR_OK			-	成功
//		ERR_REDIRECT		-	重定向连接
class MLMsgCmdPostBug : public CXMLMsgCmd
{
public:
	tstring		strBugType;			// 建议？一般问题？严重问题……
	tstring		strBugComment;		// Bug描述
	tstring		strBugProduce;		// 该bug如何重现？
	tstring		strLoginName;

};

#define ML_MSG_MAX_LENGTH		(1024 * 1024)
#define ML_MSG_XML				"XML"
#define ML_MSG_XML_LEN			3
#define ML_MSG_XML_DATA_LEN		6

enum ML_PACK_VERSION
{
	MPV_INVALID			= 0,
	// MPV_AUTO			= 1,		// 
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
//	BYTE		byData[1];

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
	long						m_nXmlPackCount;		// 未加密的Xml packet数
	long						m_nV1PackCount;			// V1 packet 的数目
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
	{
		m_nCurrentId = nId;
	}

protected:
	long						m_nCurrentId;
	CMLPacketAccountMgr			*m_pAccountMgr;
	ML_PACK_VERSION				m_packFlag;

};

class CMsgProtocol
{
public:
	CMsgProtocol();
	~CMsgProtocol();

	int SetNetFile(CNetFile *pFile) { m_pNetFile = pFile; return ERR_OK; }

	//
	int ReadMsg(MLMsgType &msgType, CCharBuffer *pMsgBuff, int nMaxBuff, BOOL *pbMLAgent = NULL);
//	int ContinueReadMsg(int nDataLength, CCharBuffer *pBuffer, int nMaxBuff);

	int WriteMsg(const char *szMsgType, CCharBuffer *pMsgBuff);

protected:
	CNetFile	*m_pNetFile;

	CCharBuffer		m_buffMsg;

public:
	CHttpRequestProtocol	m_httpRequest;
	CHttpReturnProtocol		m_httpReturn;

};

class CMLProtocol : public CMsgProtocol  
{
public:
	CMLProtocol();
	virtual ~CMLProtocol();

	int Init(CNetFile *pFile, LPCSTR szHost, int nPort);

	int Init(CNetFile *pFile, LPCWSTR szHost, int nPort);

	void SetProxy(BOOL bEnableProxy, LPCSTR szProxyServer, int nProxyPort, LPCSTR szBase64ProxyUserPass = NULL);

	// ERR_OK
	int ReadMsg(MLMsgType &msgType, CCharBuffer *pMsgBuff, BOOL *pbMLAgent = NULL);

	// ERR_OK
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
	BOOL						m_bUseProxy;
	tstring						m_strBase64ProxyUserPass;

	// 为了提高效率，而设置的缓存区
	CHttpRequestProtocol		m_httpReqWriteXmlCache;
	CHttpReturnProtocol			m_httpRetWriteXmlCache;

public:
	CMLPacketWrapper			m_packetWrapper;

};

class CMLServerProtocol : public CMLProtocol
{
public:

};

#endif // !defined(AFX_MLPROTOCOL_H__BB1B1F2A_F36F_47C4_B264_AA43E755E09C__INCLUDED_)
