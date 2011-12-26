#pragma once

#include "NetServer.h"
#include "MLProtocol.h"

class CHttpClient  
{
public:
	CHttpClient();
	virtual ~CHttpClient();

	int Init(LPCTSTR szUrl, LPCTSTR szReferUrl);

	void SetPost(LPCVOID lpPostData, size_t len);

	void SetHostPort(int nPort);
	int GetHostPort() const { return m_nHostPort; }

	int Connect();

	int SendRequest();

	int GetRespond(int &nRetCode, CCharBuffer &buffRet);

	int Close() { return m_ClientCom.Close(); }

	LPCSTR GetServer() { return m_strServer.c_str(); }
	int GetServerPort() { return m_nServerPort; }

	void SetProxy(BOOL bEnableProxy, LPCSTR szProxyServer, int nPort, LPCSTR szBase64ProxyUserPass = NULL);
	void SetUserAgent(LPCTSTR szUserAgent);

	CHttpRequestProtocol &GetHttpRequestProtocol() { return m_httpRequest; }

protected:
	CClientCom		m_ClientCom;
	CHttpRequestProtocol		m_httpRequest;
	CHttpReturnProtocol			m_httpReturn;
	CNetFile		m_NetFile;

	string			m_strHost;
	int				m_nHostPort;
	string			m_strServer;
	int				m_nServerPort;
	string			m_strUserAgent;

	//
	// proxy
	BOOL			m_bUseProxy;
	string			m_strProxyServer;
	string			m_strBase64ProxyUserPass;

};
