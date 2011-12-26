#include "StdAfx.h"
#include "AppAnalyticsClient.h"

#define SZ_APP_ANALYTICS_DATA_VER	_T("AAV1")
#define SZ_APP_ANALYTICS_SERVER		_T("http://search.crintsoft.com/analytics/v1/put/")

void MapStringToColonSeparatedValues(const MapString &mapNameValues, CColonSeparatedValues &csvValues)
{
	
}

CAppAnalyticsClient::CAppAnalyticsClient(LPCTSTR szProductId, LPCTSTR szProductVer)
{
	m_lastWeeklyPingDate = m_lastSendDate = CBaseDate::GetCurrentDate();
}

CAppAnalyticsClient::~CAppAnalyticsClient()
{
}

void CAppAnalyticsClient::Init()
{
	m_lastSendDate.FromTime(g_Profile.GetProfileInt(SZ_SECT_APP_ANALYTICS, _T("LastTaskDate"), 0));
	if (!m_lastWeeklyPingDate.FromString(g_Profile.GetProfileString(SZ_SECT_APP_ANALYTICS, _T("LastWeeklyDate"), _T(""))))
	{
		m_lastWeeklyPingDate = CBaseDate::GetCurrentDate();
		m_lastWeeklyPingDate.hour = m_lastWeeklyPingDate.minute = m_lastWeeklyPingDate.second = m_lastWeeklyPingDate.ms = 0;
		g_Profile.WriteProfileInt(SZ_SECT_APP_ANALYTICS, _T("LastWeeklyDate"), m_lastWeeklyPingDate.ToUtcDateString().c_str());
	}

	CTextFile file;
	int nRet = file.Open(GetAnalyticsFile().c_str(), CTextFile::OM_READ, ED_UTF8);
	if (nRet == ERR_OK)
	{
		tstring line;

		// Load last status
		if (file.ReadLine(line) && _tcsicmp(line.c_str(), SZ_APP_ANALYTICS_DATA_VER) == 0)
		{
			// Weekly Ping
			if (file.ReadLine(line))
			{
				CColonSeparatedValues csv;
				csv.Split(line.c_str(), m_weeklyPing, '=');
			}
		}
	}
}

tstring CAppAnalyticsClient::GetAnalyticsFile()
{
	tstring file = GetAppDataDir();
	file += _T("MiniLyrics.aa");
	return file;
}

void CAppAnalyticsClient::SavePingStatus()
{

}

void CAppAnalyticsClient::CheckSendTask()
{
	// Try to send when one day past.
	CBaseDate	dateNow = CBaseDate::GetCurrentDate();
	if (abs(dateNow.GetTime() - m_lastSendDate.GetTime()) < CBaseDate::MILLIS_IN_ONE_DAY)
		return;

	if (m_threadSend.IsRunning())
		return;

	SendParam *pSendParam = new SendParam;
	pSendParam->pAAClient = this;

	bool bNewInst = g_Profile.GetProfileBool(SZ_SECT_APP_ANALYTICS, _T("NIS"), true);
	if (bNewInst)
	{
		// Prepare for install ping
		MapString installPing;
		PrepareInstallPing(installPing);

		MapStringToColonSeparatedValues(installPing, pSendParam->installPing);
	}

	if (abs(dateNow.GetTime() - m_lastWeeklyPingDate.GetTime()) >= CBaseDate::MILLIS_IN_ONE_DAY * 7)
	{
		// More than 7 days, send weekly ping
		PrepareWeeklyPing(m_weeklyPing);

		MapStringToColonSeparatedValues(m_weeklyPing, pSendParam->weeklyPing);
	}

	m_threadSend.Create(ThreadSend, pSendParam);
}

int CAppAnalyticsClient::Send(const CColonSeparatedValues &csvValues)
{
	// Connect to server and send data
	CHttpClient	httpClient;

	httpClient.Init(SZ_APP_ANALYTICS_SERVER, NULL);
	httpClient.SetUserAgent(_T("AppAnalytics"));

	tstring proxyServer, base64UserPass;
	int proxyPort;
	if (GetProxy(proxyServer, proxyPort, base64UserPass))
		httpClient.SetProxy(true, proxyServer.c_str(), proxyPort, base64UserPass);

	tstring strUtf8Data;
	TStringToUTF8(csvValues.c_str(), csvValues.size(), strUtf8Data);
	httpClient.SetPost(strUtf8Data.c_str(), strUtf8Data.size());

	int nRet = httpClient.Connect();
	if (nRet != ERR_OK)
	{
		DBGLOG1(_T("Failed to connect to AppAnalytics server: %s"), Error2Str(nRet).c_str());
		return nRet;
	}

	nRet = httpClient.SendRequest();
	if (nRet != ERR_OK)
	{
		DBGLOG1(_T("Failed to send AppAnalytics data: %s"), Error2Str(nRet).c_str());
		return nRet;
	}

	int httpCode;
	CCharBuffer bufResponse;
	nRet = httpClient.GetRespond(httpCode, bufResponse);
	if (nRet != ERR_OK)
	{
		DBGLOG1(_T("Failed to get response of AppAnalytics server: %s"), Error2Str(nRet).c_str());
		return nRet;
	}

	// Send new install ping
	DBGLOG1(_T("Succeeded to submit AppAnalytics Data: %s"), csvValues.c_str());

	return ERR_OK;
}

void CAppAnalyticsClient::ThreadSend(void * lpParam)
{
	SendParam *pSendParam = (SendParam *)lpParam;

	if (pSendParam->installPing.size() > 0)
	{
		int nRet = pSendParam->pAAClient->Send(pSendParam->installPing);
		if (nRet == ERR_OK)
			g_Profile.WriteProfileInt(SZ_SECT_APP_ANALYTICS, _T("NIS"), false);
	}

	if (pSendParam->weeklyPing.size() > 0)
	{
		int nRet = pSendParam->pAAClient->Send(pSendParam->weeklyPing);
		if (nRet == ERR_OK)
		{
			g_Profile.WriteProfileInt(SZ_SECT_APP_ANALYTICS, _T("LastWeeklyDate"),
				CBaseDate.GetCurrentDate().ToUtcDateString().c_str());
		}
	}

	delete pSendParam;
}
