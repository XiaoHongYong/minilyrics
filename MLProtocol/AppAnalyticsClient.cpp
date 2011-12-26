#include "StdAfx.h"
#include "AppAnalyticsClient.h"

#define SZ_APP_ANALYTICS_DATA_VER	_T("AAV1")

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
	m_lastWeeklyPingDate.FromTime(g_Profile.GetProfileInt(SZ_SECT_APP_ANALYTICS, _T("LastWeeklyDate"), 0));
	if (m_lastWeeklyPingDate.GetTime() == 0)
	{
		m_lastWeeklyPingDate = CBaseDate::GetCurrentDate();
		g_Profile.WriteProfileInt(SZ_SECT_APP_ANALYTICS, _T("LastWeeklyDate"), m_lastWeeklyPingDate.GetTime());
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

	bool bNewInst = g_Profile.GetProfileBool(SZ_SECT_APP_ANALYTICS, _T("NIS"), true);
	if (bNewInst)
	{
		// Prepare for install ping
		MapString installPing;
		PrepareInstallPing(installPing);

		CColonSeparatedValues csvValues;
		MapStringToColonSeparatedValues(installPing, csvValues);

		//m_threadSend.Create(, );
	}

	if (abs(dateNow.GetTime() - m_lastWeeklyPingDate.GetTime()) >= CBaseDate::MILLIS_IN_ONE_DAY * 7)
	{
		// More than 7 days, send weekly ping
		PrepareWeeklyPing(m_weeklyPing);

		CColonSeparatedValues csvValues;
		MapStringToColonSeparatedValues(m_weeklyPing, csvValues);

		//m_threadSend.Create(, );
	}
}

int CAppAnalyticsClient::Send(const CColonSeparatedValues &csvValues)
{
	// Connect to server and send data

	// Send new install ping
	g_Profile.WriteProfileInt(SZ_SECT_APP_ANALYTICS, _T("NIS"), false);

	return ERR_OK;
}
