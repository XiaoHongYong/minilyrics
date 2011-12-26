#include "StdAfx.h"
#include "HttpClient.h"
#include "AppAnalyticsClient.h"

#define SZ_APP_ANALYTICS_DATA_VER	_T("AAV1")
#define SZ_APP_ANALYTICS_SERVER		_T("http://search.crintsoft.com/analytics/v1/put/")

#define SZ_NAME_PING_TYPE	_T("PT")
#define SZ_NAME_PRODUCT_ID	_T("PID")
#define SZ_NAME_PRODUCT_VER	_T("PV")

#define SZ_VALUE_PT_INSTALL	_T("is")
#define SZ_VALUE_PT_WEEKLY	_T("wk")

void MapStringToColonSeparatedValues(const MapString &mapNameValues, CColonSeparatedValues &csvValues)
{
	for (MapString::const_iterator it = mapNameValues.begin(); it != mapNameValues.end(); ++it)
	{
		csvValues.AddNameValue((*it).first.c_str(), (*it).second.c_str());
	}
}

CBaseDate GetCurrentDateOnly()
{
	CBaseDate cur = CBaseDate::GetCurrentDate();
	cur.ms = cur.hour = cur.minute = cur.second = 0;
	return cur;
}

//////////////////////////////////////////////////////////////////////////

CAppAnalyticsClient::CAppAnalyticsClient(LPCTSTR szProductId, LPCTSTR szProductVer)
{
	m_szProductId = szProductId;
	m_szProductVer = szProductVer;
	m_lastWeeklyPingDate = m_lastSendDate = GetCurrentDateOnly();
}

CAppAnalyticsClient::~CAppAnalyticsClient()
{
}

void CAppAnalyticsClient::Init()
{
	m_lastSendDate.FromString(g_Profile.GetProfileString(SZ_SECT_APP_ANALYTICS, _T("LastTaskDate"), _T("1977:12:12")));

	if (!m_lastWeeklyPingDate.FromString(g_Profile.GetProfileString(SZ_SECT_APP_ANALYTICS, _T("LastWeeklyDate"), _T(""))))
	{
		m_lastWeeklyPingDate = GetCurrentDateOnly();
		g_Profile.WriteProfileString(SZ_SECT_APP_ANALYTICS, _T("LastWeeklyDate"), m_lastWeeklyPingDate.ToUtcDateString().c_str());
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

void CAppAnalyticsClient::SavePingStatus()
{
	CTextFile file;
	int nRet = file.Open(GetAnalyticsFile().c_str(), CTextFile::OM_WRITE, ED_UTF8);
	if (nRet == ERR_OK)
	{
		file.WriteLine(SZ_APP_ANALYTICS_DATA_VER);

		CColonSeparatedValues csvValues;
		MapStringToColonSeparatedValues(m_weeklyPing, csvValues);
		file.WriteLine(csvValues.c_str());
	}
}

void CAppAnalyticsClient::PrepareBasePing(MapString &ping, LPCTSTR szPingType)
{
	ping[SZ_NAME_PING_TYPE] = szPingType;
	ping[SZ_NAME_PRODUCT_ID] = m_szProductId;
	ping[SZ_NAME_PRODUCT_VER] = m_szProductVer;
}

void CAppAnalyticsClient::CheckSendTask()
{
	// Try to send when one day past.
	CBaseDate	dateNow = GetCurrentDateOnly();
	if (abs(dateNow.GetTime() - m_lastSendDate.GetTime()) < CBaseDate::SECOND_IN_ONE_DAY)
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

		PrepareBasePing(installPing, SZ_VALUE_PT_INSTALL);
		PrepareInstallPing(installPing);

		MapStringToColonSeparatedValues(installPing, pSendParam->installPing);
	}

	if (abs(dateNow.GetTime() - m_lastWeeklyPingDate.GetTime()) >= CBaseDate::SECOND_IN_ONE_DAY * 7)
	{
		// More than 7 days, send weekly ping
		PrepareBasePing(m_weeklyPing, SZ_VALUE_PT_WEEKLY);
		PrepareWeeklyPing(m_weeklyPing);

		MapStringToColonSeparatedValues(m_weeklyPing, pSendParam->weeklyPing);
	}

	if (pSendParam->installPing.size() > 0 || pSendParam->weeklyPing.size() > 0)
		m_threadSend.Create(ThreadSend, pSendParam);
	else
		delete pSendParam;

	g_Profile.WriteProfileString(SZ_SECT_APP_ANALYTICS, _T("LastTaskDate"), dateNow.ToUtcDateString().c_str());
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
		httpClient.SetProxy(true, proxyServer.c_str(), proxyPort, base64UserPass.c_str());

	int nRet = httpClient.Connect();
	if (nRet != ERR_OK)
	{
		DBGLOG1(_T("Failed to connect to AppAnalytics server: %s"), Error2Str(nRet).c_str());
		return nRet;
	}

	string strUtf8Data;
	TStringToUTF8(csvValues.c_str(), csvValues.size(), strUtf8Data);
	nRet = httpClient.SendPostRequest(strUtf8Data.c_str(), strUtf8Data.size());
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
			g_Profile.WriteProfileString(SZ_SECT_APP_ANALYTICS, _T("LastWeeklyDate"),
				GetCurrentDateOnly().ToUtcDateString().c_str());
		}
	}

	delete pSendParam;
}
