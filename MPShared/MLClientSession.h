#pragma once

#include "../MLProtocol/NetServer.h"
#include "../MLProtocol/MLProtocol.h"


struct ServerCfg
{
	TCHAR		szServer[32];
	int			nPort;
};

#define SERVER_COUNT			1

extern ServerCfg			g_LyrServers[SERVER_COUNT];
extern int					g_nCurLyrServer;

extern int					g_nPortSearchServer;
extern int					g_nPortDownloadServer;

/*
***** To prevent DDos attach: *****
1. Open as many port as possible on server, If any connection error occurs,
   so MiniLyrics client tries connect port from 80 to 180, about 100 ports.

*/

class CMLClientSession  
{
public:
	CMLClientSession();
	virtual ~CMLClientSession();

	void Init(LPCTSTR szAppName);

	void SetUploader(LPCTSTR szUser, LPCTSTR szPwd, bool bSavePwd);
	LPCTSTR GetLoginName() const { return m_strLoginName.c_str(); }
	LPCTSTR GetLoginPwd() const { return m_strPwdMask.c_str(); }

	void SetProxy(BOOL bEnableProxy, LPCSTR szProxyServer, int nPort, LPCSTR szBase64ProxyUserPass = NULL);

	int Connect();

	int Close() { m_bClosing = true; return m_ClientCom.Close(); }

	bool IsLogined() { return m_bLogined; }

	int Login(tstring &strMsgRet);

	// Activate license
	int ActivateLicense(tstring &strMsgRet, MLMsgRetActivateLicense &retMsg);

	// Search lyrics
	int Search(bool bOnlyMatched, LPCTSTR szArtist, LPCTSTR szTitle, int nRequestPage, MLMsgRetSearch &retSearch);

	// Search lyrics by batch
	int BatchSearch(MLListSearchItems &listToSearch, MLMsgRetBatchSearch &retSearch);

	// Upload lyrics
	int Upload(const tstring &lyricsContent, tstring &strLyricsId, tstring &strMsg);

protected:
	void PrepareClientInfo(CXMLMsgCmd &cmd, bool bActivate = false);

	int SendXmlCommand(CXMLMsgCmd &cmd);

	// Process common return message
	int ProcessCommonRetMsg(MLRetMsg *pretMsg, MLMsgCmd cmdOrgSend);

public:
	static LPCTSTR GetDefaultServer() { return g_LyrServers[g_nCurLyrServer].szServer; }
	static int GetDefaultServerPort() { return g_nPortSearchServer; }

protected:
	CClientCom		m_ClientCom;
	CMLProtocol		m_MsgProtocol;
	CNetFile		m_NetFile;
	bool			m_bInitialzed;

	tstring			m_strLoginName;
	tstring			m_strPwdMask;
	tstring			m_strClient;

	bool			m_bLogined;

	bool			m_bClosing;

	//
	// proxy
	tstring			m_strBase64ProxyUserPass;
	string			m_strProxyServer;
	int				m_nProxyPort;
	bool			m_bEnableProxy;

};
