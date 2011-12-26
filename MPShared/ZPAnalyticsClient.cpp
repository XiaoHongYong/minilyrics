#include "StdAfx.h"
#include "ZPAnalyticsClient.h"
#include "MPlayerAppBase.h"
#include "MLVersionStr.h"

#ifdef _MPLAYER
#define SZ_APP_ANALYTICS_PRODUCT_ID		_T("ZP")
#else
#define SZ_APP_ANALYTICS_PRODUCT_ID		_T("ML")
#endif

CZPAnalyticsClient::CZPAnalyticsClient() :
	CAppAnalyticsClient(SZ_APP_ANALYTICS_PRODUCT_ID, _T(ML_VERSION_STR))
{
}

CZPAnalyticsClient::~CZPAnalyticsClient()
{
}

tstring CZPAnalyticsClient::GetAnalyticsFile()
{
	tstring file = GetAppDataDir();
	file += _T("MiniLyrics.aa");
	return file;
}

bool CZPAnalyticsClient::GetProxy(tstring &strServer, int &port, tstring &base64UserPass)
{
	int					nProxyPort = 80;
	TCHAR				szProxyServer[256];

	if (CMLProfile::InetGetProxy(szProxyServer, nProxyPort))
	{
		strServer = szProxyServer;
		base64UserPass = CMLProfile::InetGetBase64ProxyUserPass();
		return true;
	}

	return false;
}

void CZPAnalyticsClient::PrepareInstallPing(MapString &pingData)
{
}

void CZPAnalyticsClient::PrepareWeeklyPing(MapString &pingData)
{
}
