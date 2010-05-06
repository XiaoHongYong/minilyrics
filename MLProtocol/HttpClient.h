// HttpClient.h: interface for the CHttpClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPCLIENT_H__0474F34E_B025_4BFB_9D8F_09D927ED8690__INCLUDED_)
#define AFX_HTTPCLIENT_H__0474F34E_B025_4BFB_9D8F_09D927ED8690__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetServer.h"
#include "MLProtocol.h"

class CHttpClient  
{
public:
	CHttpClient();
	virtual ~CHttpClient();

	int Init(LPCTSTR szHost, LPCTSTR szUri, LPCTSTR szReferUrl, int nPort = __HTTP_PORT);

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

#endif // !defined(AFX_HTTPCLIENT_H__0474F34E_B025_4BFB_9D8F_09D927ED8690__INCLUDED_)
