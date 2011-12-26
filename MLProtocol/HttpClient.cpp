// HttpClient.cpp: implementation of the CHttpClient class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "HttpClient.h"

//////////////////////////////////////////////////////////////////////

CHttpClient::CHttpClient()
{
	m_strUserAgent = "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3 ( .NET CLR 3.5.30729)";
	m_bUseProxy = false;
	m_nHostPort = 80;
	m_nServerPort = 80;
}

CHttpClient::~CHttpClient()
{

}

int CHttpClient::Init(LPCTSTR szUrl, LPCTSTR szReferUrl)
{
	tstring scheme, domain, path;

	if (!UrlParse(szUrl, scheme, domain, m_nHostPort, path))
		return ERR_HTTP_BAD_URL;

	if (_tcsicmp(scheme.c_str(), _T("http")) != 0)
		return ERR_HTTP_BAD_URL;

	TStringToAnsi(domain.c_str(), domain.size(), m_strHost);

	m_httpRequest.SetNetFile(&m_NetFile);
	m_httpRequest.SetHost(m_strHost.c_str(), m_nHostPort);
	m_httpRequest.SetRequestType(CHttpRequestProtocol::RQ_GET);

	stringex	strAnsi;
	TStringToAnsi(path.c_str(), path.size(), strAnsi);
	m_httpRequest.SetUrl(strAnsi.c_str());

	m_httpRequest.SetAvailProp(CHttpRequestProtocol::RP_BASE_GET | CHttpRequestProtocol::RP_PRAGMA | CHttpRequestProtocol::RP_CONNECTION | CHttpRequestProtocol::RP_CACHE_CONTROL);
 	m_httpRequest.SetPropValue(CHttpRequestProtocol::RP_HOST, m_strHost.c_str());
 	m_httpRequest.SetPropValue(CHttpRequestProtocol::RP_ACCEPT, "*/*");
	if (szReferUrl)
	{
		TStringToAnsi(szReferUrl, -1, strAnsi);
		m_httpRequest.SetPropValue(CHttpRequestProtocol::RP_REFERER, strAnsi.c_str());
	}
	m_httpRequest.SetPropValue(CHttpRequestProtocol::RP_USER_AGENT, m_strUserAgent.c_str());

	m_httpReturn.SetNetFile(&m_NetFile);
	m_httpReturn.SetAvailProp(CHttpReturnProtocol::RP_MIN_RET);
	m_httpReturn.SetServer("MiniLyrics Server/1.1");
	m_httpReturn.SetConnection("close");

	SetProxy(false, NULL, 80);

	return ERR_OK;
}

void CHttpClient::SetHostPort(int nPort)
{
	m_nHostPort = nPort;
	if (!m_bUseProxy)
		m_nServerPort = nPort;
}

int CHttpClient::Connect()
{
	if (m_ClientCom.IsValid())
		m_ClientCom.Close();

//	if (g_MLProfile.InetIsUseProxy())
//		SetProxy(TRUE, g_MLProfile.InetGetProxyServer(), g_MLProfile.InetGetBase64ProxyUserPass());
//	else
//		SetProxy(FALSE, "", "");

	int		nRet;
	nRet = m_ClientCom.Connect(m_strServer.c_str(), m_nServerPort);
	if (nRet != ERR_OK)
		return nRet;

	m_NetFile.Attach(m_ClientCom.GetSocket());
	nRet = m_NetFile.SetTimeOut(20000);		// 10 second

	return ERR_OK;
}

void CHttpClient::SetProxy(BOOL bEnableProxy, LPCSTR szProxyServer, int nPort, LPCSTR szBase64ProxyUserPass)
{
	if (bEnableProxy && !IsEmptyString(szProxyServer))
	{
		ASSERT(strchr(szProxyServer, ':') == NULL);
//		char			szServer[256];
//		const char		*szColon;
//
//		szColon = strchr(szProxyServer, ':');
//		if (szColon == NULL)
//		{
//			strcpy_safe(szServer, CountOf(szServer), szProxyServer);
//			m_strServer = szServer;
//			m_nServerPort = __HTTP_PORT;
//		}
//		else
//		{
//			strncpysz_safe(szServer, CountOf(szServer), szProxyServer, int(szColon - szProxyServer));
//			m_strServer = szServer;
//			m_nServerPort = atoi(szColon + 1);
//		}
		m_strServer = szProxyServer;
		m_nServerPort = nPort;
		m_httpRequest.SetProxyBase64Pass(TRUE, szBase64ProxyUserPass);
	}
	else
	{
		m_strServer = m_strHost;
		m_nServerPort = m_nHostPort;
	}
}

int CHttpClient::SendRequest()
{
	m_httpRequest.SetNetFile(&m_NetFile);
//	m_httpRequest.SetContentLength();
	return m_httpRequest.WriteHead();
//	return m_httpRequest.WriteData(pMsgBuff->GetBuffer(), pMsgBuff->GetLength());
}

int CHttpClient::GetRespond(int &nRetCode, CCharBuffer &buffRet)
{
	int		nRet;

	buffRet.clear();

	// HTTP ÏûÏ¢
	m_httpReturn.SetNetFile(&m_NetFile);
	nRet = m_httpReturn.ReadHead(TRUE);
	if (nRet != ERR_OK)
		return nRet;

	nRet = m_httpReturn.ReadData(&buffRet, 1024 * 1000);
	if (nRet == ERR_OK)
		nRetCode = m_httpReturn.GetResultCode();

	return nRet;
}

void CHttpClient::SetUserAgent(LPCTSTR szUserAgent)
{
	stringex	strOut;

	ConvertStr(szUserAgent, -1, strOut);

	m_strUserAgent = strOut.c_str();
}
