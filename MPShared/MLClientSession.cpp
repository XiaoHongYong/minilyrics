// MLClientSession.cpp: implementation of the CMLClientSession class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MPlayerApp.h"
#include "MLClientSession.h"


#define CUR_ACCOUNT_ID		4
#define CUR_ACCOUNT_NAME	"Mlv1clt4.0"
#define CUR_MSG_TYPE		MT_HTTP_POST

ServerCfg		g_LyrServers[SERVER_COUNT] = 
{
	{ _T(""), 80 },
};

int					g_nCurLyrServer = 0;
int					g_nPortSearchServer = 80;
int					g_nPortDownloadServer = 80;

void LoadServerInfo()
{
	string		strEng;

	if (IsEmptyString(g_LyrServers[0].szServer))
	{
		int			nEngIndex = 0;
		g_LyrServers[nEngIndex].nPort = 80;
#if !defined(_DEBUG) && !defined(_MPLAYER) && defined(_WIN32)
		TCHAR			szFile[MAX_PATH];
		string			strChs;

		GetModuleFileName(GetAppInstance(), szFile, CountOf(szFile));
		if (!MLDecriptV2(szFile, strEng, strChs))
		{
			for (int i = 0; i < CountOf(g_LyrServers); i++)
				_tcscpy(g_LyrServers[i].szServer, _T("x"));
		}
		else
		{
			ConvertStr2(strEng.c_str(), strEng.size(), 
				g_LyrServers[nEngIndex].szServer, CountOf(g_LyrServers[nEngIndex].szServer));
		}
#else
		_tcscpy_safe(g_LyrServers[nEngIndex].szServer, 
			CountOf(g_LyrServers[nEngIndex].szServer), _T("search.crintsoft.com"));
#endif

#ifdef _DEBUG
		//
		// Load server config from Install.ini
		//
		CProfile	file;
		tstring		str;
		file.Init(_T("install.ini"), _T("INSTALL"));

		str = file.GetProfileString(_T("server1"), _T(""));
		if (!str.empty())
			_tcscpy_safe(g_LyrServers[0].szServer, CountOf(g_LyrServers[0].szServer), str.c_str());
		str = file.GetProfileString(_T("server1port"), _T(""));
		if (!str.empty())
			g_LyrServers[0].nPort = _ttoi(str.c_str());
#endif
	}
}

void MD5Gen16(LPCWSTR szSource, WCHAR szMD5[])
{
	stringex	strSource;
	wstringex	strMd5;
	char			szTemp[128];

	ConvertStr(szSource, -1, strSource);

	MD5Gen16(strSource.c_str(), szTemp);

	ConvertStr(szTemp, -1, strMd5);

	wcscpy_safe(szMD5, 64, strMd5.c_str());
}

//////////////////////////////////////////////////////////////////////

CMLClientSession::CMLClientSession()
{
	m_bLogined = FALSE;
	m_bEnableProxy = false;
	m_nProxyPort = 80;
	m_bInitialzed = false;
}

CMLClientSession::~CMLClientSession()
{

}

void CMLClientSession::Init(LPCTSTR szAppName)
{
	if (m_bInitialzed)
		return;
	m_bInitialzed = true;

	m_strClient = szAppName;
	LoadServerInfo();

	m_strLoginName = g_Profile.GetProfileString(_T("LoginName"), _T(""));
	m_strPwdMask = g_Profile.GetProfileString(_T("LoginPwdMask"), _T(""));

	m_MsgProtocol.Init(&m_NetFile, GetDefaultServer(), GetDefaultServerPort());

	CMLPacketAccountMgr *pMgr = CMLPacketAccountMgr::GetInstance();
	pMgr->AddAccount(CUR_ACCOUNT_ID, CUR_ACCOUNT_NAME);
	m_MsgProtocol.SetClientId(CUR_ACCOUNT_ID);
	pMgr->SetMode(FALSE, MPV_V1_MD5_ID);

}

int CMLClientSession::Connect()
{
	TCHAR	szProxyServer[128];
	int		nProxyPort;
	BOOL	bUserProxy;
	stringex	strProxyServer, strProxyPwd;
	int		nRet;

	m_bClosing = false;

	if (m_ClientCom.IsValid())
		m_ClientCom.Close();

	bUserProxy = CMLProfile::InetGetProxy(szProxyServer, nProxyPort);
	ConvertStr(szProxyServer, -1, strProxyServer);
	ConvertStr(CMLProfile::InetGetBase64ProxyUserPass(), -1, strProxyPwd);
	SetProxy(bUserProxy, strProxyServer.c_str(), nProxyPort, strProxyPwd.c_str());

	if (m_bEnableProxy)
		nRet = m_ClientCom.Connect(m_strProxyServer.c_str(), m_nProxyPort);
	else
	{
		// Try to connect to server: Latest port, 80, 8000 ~ 8080
		nRet = m_ClientCom.Connect(GetDefaultServer(), g_nPortSearchServer);
		if (nRet != ERR_OK && !m_bClosing)
		{
			// 80
			if (g_nPortSearchServer != 80)
				nRet = m_ClientCom.Connect(GetDefaultServer(), 80);
			if (nRet == ERR_OK)
				g_nPortSearchServer = 80;
			else
			{
				// 8000 ~ 8080
				if (g_nPortSearchServer < SERVER_PORT_BEGIN)
					g_nPortSearchServer = SERVER_PORT_BEGIN - 1;
				for (int i = SERVER_PORT_BEGIN; !m_bClosing && i <= SERVER_PORT_END; i++)
				{
					g_nPortSearchServer++;
					if (g_nPortSearchServer > SERVER_PORT_END)
						g_nPortSearchServer = SERVER_PORT_BEGIN;
					nRet = m_ClientCom.Connect(GetDefaultServer(), g_nPortSearchServer);
					if (nRet == ERR_OK || nRet == ERR_NET_HOST_NOT_FOUND)
						break;
				}
			}
		}
	}
	if (nRet != ERR_OK)
		return nRet;

	m_NetFile.Attach(m_ClientCom.GetSocket());
	nRet = m_NetFile.SetTimeOut(20 * 1000);

/*	nRet = m_MsgProtocol.Init(&m_NetFile);
	if (nRet != ERR_OK)
		return nRet;*/

	return ERR_OK;
}

void CMLClientSession::SetUploader(LPCTSTR szUser, LPCTSTR szPwd, bool bSavePwd)
{
	if (_tcscmp(szPwd, m_strPwdMask.c_str()) != 0
		|| _tcscmp(szUser, m_strLoginName.c_str()) != 0)
	{
		char		szPwdMaskAnsi[50];
		stringex	strPwdAnsi;
		TStringToAnsi(szPwd, -1, strPwdAnsi);
		MD5Gen16(strPwdAnsi.c_str(), szPwdMaskAnsi);
		AnsiToTString(szPwdMaskAnsi, -1, m_strPwdMask);

		m_strLoginName = szUser;
	}

	LPCTSTR szPwdSaved = g_Profile.GetProfileString(_T("LoginPwdMask"), _T(""));
	LPCTSTR szPwdToSave;
	if (bSavePwd)
		szPwdToSave = m_strPwdMask.c_str();
	else
		szPwdToSave = _T("");

	if (_tcscmp(szPwdToSave, szPwdSaved) != 0)
		g_Profile.WriteProfileString(_T("LoginPwdMask"), szPwdToSave);

	if (_tcscmp(szPwdToSave, g_Profile.GetProfileString(_T("LoginName"), _T(""))) != 0)
		g_Profile.WriteProfileString(_T("LoginName"), szUser);
}

void CMLClientSession::SetProxy(BOOL bEnableProxy, LPCSTR szProxyServer, int nPort, LPCSTR szBase64ProxyUserPass)
{
	if (bEnableProxy && !IsEmptyString(szProxyServer))
	{
		m_strProxyServer = szProxyServer;
		m_nProxyPort = nPort;
		m_bEnableProxy = true;
	}
	else
		m_bEnableProxy = false;

	m_MsgProtocol.SetProxy(bEnableProxy, szProxyServer, nPort, szBase64ProxyUserPass);
}

int CMLClientSession::Login(tstring &strMsgRet)
{
	MLMsgCmdLogin	cmdLogin;
	int				nRet;

	ASSERT(m_strLoginName.size() && m_strPwdMask.size());
	cmdLogin.strName = m_strLoginName;
	cmdLogin.strPwd = m_strPwdMask;
	PrepareClientInfo(cmdLogin);

	m_bLogined = FALSE;
	nRet = SendXmlCommand(cmdLogin);
	if (nRet != ERR_OK)
		return nRet;

	MLMsgRetLogin		retLogin;
	nRet = ProcessCommonRetMsg(&retLogin, MC_LOGIN);

	if (nRet == ERR_OK)
		m_bLogined = TRUE;

	strMsgRet = retLogin.strMessage;

	return nRet;
}

int CMLClientSession::ActivateLicense(tstring &strMsgRet, MLMsgRetActivateLicense &retMsg)
{
	MLMsgCmdActivateLicense	cmd;

	// The license code has been stored in the profile.
	PrepareClientInfo(cmd, true);

	int nRet = SendXmlCommand(cmd);
	if (nRet != ERR_OK)
		return nRet;

	nRet = ProcessCommonRetMsg(&retMsg, MC_ACTIVATE);
	strMsgRet = retMsg.strMessage;

	ERRLOG1(_T("ActivateResult: %d"), nRet);

	if (nRet == ERR_OK)
		g_licenseManager.SaveActivateResult(retMsg);

	return nRet;
}

// 返回值：
//	ERR_NOT_FOUND
//  
int CMLClientSession::Search(bool bOnlyMatched, LPCTSTR szArtist, LPCTSTR szTitle, int nRequestPage, MLMsgRetSearch &retSearch)
{
	int			nRet;
	MLMsgCmdSearch	cmdSearch;
	tstringex		strEncoding;

	retSearch.strMessage = _T("");

	ConvertStr(GetSysDefaultCharEncoding().szEncoding, -1, strEncoding);
	cmdSearch.strTitle = szTitle;
	cmdSearch.strArtist = szArtist;
	cmdSearch.nRequestPage = nRequestPage;
	cmdSearch.bOnlyMatchedLyrics = bOnlyMatched;
	PrepareClientInfo(cmdSearch);

	nRet = SendXmlCommand(cmdSearch);
	if (nRet != ERR_OK)
		return nRet;

	nRet = ProcessCommonRetMsg(&retSearch, MC_SEARCH);

	g_licenseManager.SaveActivateResult(retSearch);

	if (nRet != ERR_OK)
		return nRet;

	return ERR_OK;
}

int CMLClientSession::BatchSearch(MLListSearchItems &listToSearch, MLMsgRetBatchSearch &retSearch)
{
	int			nRet;
	MLMsgCmdBatchSearch	cmdSearch;

	retSearch.strMessage = _T("");

	cmdSearch.bOnlyActiveCanDownload = true;
	cmdSearch.listSearchItems = listToSearch;
	PrepareClientInfo(cmdSearch);

	nRet = SendXmlCommand(cmdSearch);
	if (nRet != ERR_OK)
		return nRet;

	nRet = ProcessCommonRetMsg(&retSearch, MC_BATCH_SEARCH);

	return nRet;
}

int CMLClientSession::Upload(const tstring &lyricsContent, tstring &strLyricsId, tstring &strMsg)
{
	MLMsgCmdUpload	cmdUpload;

	ASSERT(m_strLoginName.size() && m_strPwdMask.size());
	cmdUpload.strLoginName = m_strLoginName;
	cmdUpload.strPwdMask = m_strPwdMask;
	cmdUpload.strFileContent = lyricsContent;

	int nRet = SendXmlCommand(cmdUpload);
	if (nRet != ERR_OK)
		return nRet;

	MLMsgRetUpload	retUpload;

	nRet = ProcessCommonRetMsg(&retUpload, MC_UPLOAD);
	if (nRet == ERR_BAD_USERPWD)
		m_bLogined = false;

	strLyricsId = retUpload.strLyricsId;
	strMsg = retUpload.strMessage;

	return nRet;
}

void CMLClientSession::PrepareClientInfo(CXMLMsgCmd &cmd, bool bActivate)
{
	cmd.strClient = m_strClient;
	cmd.nProductId = PRODUCT_ID_MINILYRICS;
#ifndef _IPHONE
	if (g_licenseManager.GetLicenseStatus() == LS_ACTIVE || bActivate)
	{
		CLicenseManager::LicenseInfo	licenseInfo;
		g_licenseManager.GetLicenseInfo(licenseInfo);

		cmd.strRegCode = licenseInfo.strCode;
		strrep(cmd.strRegCode, _T("-"), _T(""));
		cmd.strDeviceID = licenseInfo.strDeviceID;
		cmd.bOnlineVerified = licenseInfo.bOnlineVerified;
	}
#endif // #ifdef _IPHONE
}

int CMLClientSession::SendXmlCommand(CXMLMsgCmd &cmd)
{
	// CXMLWriter		xmlStream;
	CMLBinXMLWriter		xmlStream;

	int nRet = cmd.ToXML(xmlStream);
	if (nRet != ERR_OK)
		return nRet;

	return m_MsgProtocol.WriteXMLMsg(CUR_MSG_TYPE, &xmlStream.GetBuffer());
}

int CMLClientSession::ProcessCommonRetMsg(MLRetMsg *pretMsg, MLMsgCmd cmdOrgSend)
{
	ASSERT(pretMsg);
	MLMsgType		msgType;
	CCharBuffer		buffMsg;
	int				nRet;
	buffMsg.reserve(2048);
	nRet = m_MsgProtocol.ReadMsg(msgType, &buffMsg);
	if (nRet != ERR_OK)
	{
		DBGLOG0(_T("READ MSG FAILED!"));
		return nRet;
	}

	CSimpleXML		xml;
	nRet = m_MsgProtocol.ParseXMLMsg(&buffMsg, xml);
	if (nRet != ERR_OK)
	{
		return nRet;
	}

	if (_tcscmp(xml.m_pRoot->strName.c_str(), MZ_RETURN) != 0)
		return ERR_CMD_NOT_MATCH;

	nRet = pretMsg->FromXML(xml.m_pRoot);
	if (nRet != ERR_OK)
		return nRet;

	MLMsgCmd		cmdOrg = MLMsgStr2Cmd(pretMsg->strOrgCmd.c_str());
	if (cmdOrg != cmdOrgSend)
		return ERR_CMD_NOT_MATCH;

	return pretMsg->result;
}

/*

//////////////////////////////////////////////////////////////////////////
// 所有函数的返回结果说明：
// 返回0，表示该函数调用成功；
// 返回其他值，表示该函数调用失败；使用GetErrorDesc，可以取得具体的错误描述
//

#ifndef ERR_OK
#define ERR_OK		0
#endif

struct LyricsInfo
{
	char szFileName[MAX_PATH];
	char szArtist[MAX_PATH];
	char szTitle[MAX_PATH];
};

extern "C"
{

// 登陆
int Login(LPCTSTR szName, LPCTSTR szPwd);

// 搜索，传入文件名，返回搜索结果句柄，和对应的结果个数
// 如果没有歌词，仍然返回成功
int SearchLyrics(LPCTSTR szFileName, HANDLE &hResult, int &nCount);

// 根据结果索引，返回每一个结果的具体信息
int GetLyricsInfo(HANDLE hLyrics, int nIndex, LyricsInfo &lyricsInfo);

// 在不使用结果句柄之后，必须调用此函数，以释放内存
int FreeLyricsHandle(HANDLE hLyrics);

// 下载歌词, 
// 参数：
//		szDownloadDir	: 歌词下载到的目录
//		szLyricsFile	：下载之后的文件名，包括路径
int DownloadLyrics(HANDLE hLyrics, int nIndex, LPCTSTR szDownloadDir, LPTSTR szLyricsFile);

// 根据返回的错误ID，返回错误描述
LPCTSTR GetErrorDesc(int nRet);

}
//
// example:

// 初始化
int Init()
{
	int		nRet;

	// 登陆
	nRet = Login(_T("name"), _T("password"))
	if (nRet != ERR_OK)
	{
		CString		str;
		str.Format(_T("登陆失败，原因: %s"), GetErrorDesc(nRet));
		AfxMessageBox(str);
	}
}

int YourSearchLyrics(LPCTSTR szFile)
{
	HANDLE	hResult;
	int		nRet, nCount;
	CString		str;

	nRet = SearchLyrics(szFile, hResult, nCount);
	if (nRet != ERR_OK)
	{
		str.Format(_T("搜索失败，原因: %s"), GetErrorDesc(nRet));
		AfxMessageBox(str);
		return 0;
	}

	for (int i = 0; i < nCount; i++)
	{
		LyricsInfo		lyricsInfo;
		nRet = GetLyricsInfo(hResult, i, lyricsInfo);
		if (nRet == ERR_OK)
		{
			// 显示这些信息，添加到列表中，由用户选择
			//....
		}
	}

	char		szLyricsFile[MAX_PATH];
	char		szSaveDir[MAX_PATH] = _T("C:\\Download");

	// 选择下载第一首
	nRet = DownloadLyrics(hResult, 0, szSaveDir, szLyricsFile);
	if (nRet != ERR_OK)
	{
		str.Format(_T("下载失败，原因: %s"), GetErrorDesc(nRet));
		AfxMessageBox(str);
		return 0;
	}

	// 释放内存
	FreeLyricsHandle(hResult);

}

 */
