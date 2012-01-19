#include "StdAfx.h"
#include "HttpClient.h"
#include "AppAnalyticsClient.h"

#define SZ_APP_ANALYTICS_DATA_VER	_T("AAV1")
#define SZ_APP_ANALYTICS_SERVER		_T("http://search.crintsoft.com/analytics/v1/put/")
// #define SZ_APP_ANALYTICS_SERVER		_T("http://192.168.1.107/analytics/v1/put/")


void MapStringToColonSeparatedValues(const MapString &mapNameValues, CColonSeparatedValues &csvValues)
{
	csvValues.clear();
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
bool CAppAnalyticsClient::Rule::SaveFromXml(SXNode *pNode)
{
	SXNode *pNodeBaseRule = pNode->GetChild(_T("BaseRule"));
	if (!pNodeBaseRule)
		return false;

	LPCTSTR vNames[] = { _T("SplitVer"), _T("NewVerEnabledAA"), _T("NewVerDisabledAA"), _T("OldVerEnabledAA"), _T("OldVerDisabledAA") };

	for (int i = 0; i < CountOf(vNames); i++)
		g_Profile.EncryptWriteProfileString(SZ_SECT_APP_ANALYTICS, vNames[i], 
			pNodeBaseRule->GetPropertySafe(vNames[i]));

	return true;
}

void AAProfileGetStringSet(LPCTSTR szValueName, LPCTSTR szDefault, SetTStrNCase &setStrings)
{
	StrSplit(g_Profile.EncryptGetProfileString(SZ_SECT_APP_ANALYTICS, szValueName, szDefault).c_str(),
		'|', setStrings);
}

void CAppAnalyticsClient::Rule::LoadFromProfile()
{
	verSeparator = VersionNumFrom(g_Profile.EncryptGetProfileString(SZ_SECT_APP_ANALYTICS, _T("SplitVer"), _T("")).c_str());
	AAProfileGetStringSet(_T("NewVerEnabledAA"), _T("*"), newVerEnabledAA);
	AAProfileGetStringSet(_T("NewVerDisabledAA"), _T(""), newVerDisabledAA);
	AAProfileGetStringSet(_T("OldVerEnabledAA"), _T("PI|BU|LS|CU|PR"), oldVerEnabledAA);
	AAProfileGetStringSet(_T("OldVerDisabledAA"), _T(""), oldVerDisabledAA);
}

//////////////////////////////////////////////////////////////////////////

CAppAnalyticsClient::CAppAnalyticsClient(LPCTSTR szProductId, LPCTSTR szProductVer)
{
	m_szProductId = szProductId;
	m_szProductVer = szProductVer;
	m_lastWeeklyPingDate = m_lastSendDate = GetCurrentDateOnly();
	m_lastSaveTime = 0;
	m_bUsageDataModified = false;
}

CAppAnalyticsClient::~CAppAnalyticsClient()
{
}

void CAppAnalyticsClient::Init()
{
	m_nAppStartTime = GetTickCount();

	m_lastSendDate.FromString(g_Profile.EncryptGetProfileString(SZ_SECT_APP_ANALYTICS, _T("LastTaskDate"), _T("1977:12:12")).c_str());

	if (!m_lastWeeklyPingDate.FromString(g_Profile.EncryptGetProfileString(SZ_SECT_APP_ANALYTICS, _T("LastWeeklyDate"), _T("")).c_str()))
	{
		m_lastWeeklyPingDate = GetCurrentDateOnly();
		g_Profile.EncryptWriteProfileString(SZ_SECT_APP_ANALYTICS, _T("LastWeeklyDate"), m_lastWeeklyPingDate.ToUtcDateString().c_str());
	}

	m_lastSaveTime = GetTickCount();
	m_bUsageDataModified = false;

	CTextFile file;
	int nRet = file.Open(GetAnalyticsFile().c_str(), CTextFile::OM_READ, ED_UTF8);
	if (nRet == ERR_OK)
	{
		tstring line;

		// Load last status
		if (file.ReadLine(line) && _tcsicmp(line.c_str(), SZ_APP_ANALYTICS_DATA_VER) == 0)
		{
			// Weekly Ping
			while (file.ReadLine(line))
			{
				CColonSeparatedValues csv;
				MapString *usageData = new MapString();
				csv.Split(line.c_str(), *usageData, '=');
				MapString::iterator it = usageData->find(SZ_N_USAGE_DATA_TYPE);
				if (it == usageData->end())
					delete usageData;
				else
					m_mapUsageData[(*it).second] = usageData;
			}
		}
	}
	else
		InitUDC();
}

void CAppAnalyticsClient::Quit()
{
	CMutexAutolock lock(m_mutex);

	MapString &pingBasicUsage = GetUsageData(SZ_UD_BASIC_USAGE);

	uint32 nAppRunningTimeSec = (GetTickCount() - m_nAppStartTime) / 1000;
	if (nAppRunningTimeSec < 2 * 60)
		IncreaseValue(pingBasicUsage, SZ_N_BU_CLOSED_TIMES, 1);
	else
	{
		IncreaseValue(pingBasicUsage, SZ_N_BU_RUN_TIMES, 1);
		IncreaseValue(pingBasicUsage, SZ_N_BU_RUN_DURATION, nAppRunningTimeSec);
	}

	SaveUDCStatus();

	Free();
}

void CAppAnalyticsClient::IncreaseValue(LPCTSTR szType, LPCTSTR szName, int nValue)
{
	m_bUsageDataModified = true;

	CMutexAutolock lock(m_mutex);
	MapString &usageData = GetUsageData(szType);

	IncreaseValue(usageData, szName, nValue);
}

void CAppAnalyticsClient::WriteValue(LPCTSTR szType, LPCTSTR szName, LPCTSTR szValue)
{
	m_bUsageDataModified = true;

	CMutexAutolock lock(m_mutex);
	MapString &usageData = GetUsageData(szType);
	usageData[szName] = szValue;
}

void CAppAnalyticsClient::WriteValue(LPCTSTR szType, LPCTSTR szName, int nValue)
{
	m_bUsageDataModified = true;

	CMutexAutolock lock(m_mutex);
	MapString &usageData = GetUsageData(szType);
	usageData[szName] = StringFromInt(nValue);
}

tstring CAppAnalyticsClient::GetValue(LPCTSTR szType, LPCTSTR szName, LPCTSTR szDefValue)
{
	CMutexAutolock lock(m_mutex);
	MapUsageData::iterator it = m_mapUsageData.find(szType);
	if (it == m_mapUsageData.end())
		return szDefValue;

	MapString *usageData = (*it).second;
	return usageData->GetValue(szName, szDefValue);
}

int CAppAnalyticsClient::GetValueInt(LPCTSTR szType, LPCTSTR szName, int nDefValue)
{
	CMutexAutolock lock(m_mutex);
	MapUsageData::iterator it = m_mapUsageData.find(szType);
	if (it == m_mapUsageData.end())
		return nDefValue;

	MapString *usageData = (*it).second;
	MapString::iterator itValue = usageData->find(szName);
	if (itValue == usageData->end())
		return nDefValue;
	else
		return StringToInt((*itValue).second.c_str());
}

void CAppAnalyticsClient::IncreaseCmdClickCount(LPCTSTR szCmd, CmdAccessMethod cam)
{
	CMutexAutolock lock(m_mutex);
	CStrPrintf name(_T("%s#%c"), szCmd, (TCHAR)cam);

	IncreaseValue(SZ_UD_CMD_USAGE, name.c_str(), 1);
}

void CAppAnalyticsClient::IncreaseCrashTimes()
{
	IncreaseValue(SZ_UD_BASIC_USAGE, SZ_N_BU_CRASH_TIMES, 1);

	SaveUDCStatus();
}

MapString &CAppAnalyticsClient::GetUsageData(LPCTSTR szType)
{
	MapUsageData::iterator it = m_mapUsageData.find(szType);
	if (it == m_mapUsageData.end())
	{
		MapString *usageData = new MapString();
		m_mapUsageData[szType] = usageData;
		(*usageData)[SZ_N_USAGE_DATA_TYPE] = szType;
		return *usageData;
	}

	return *((*it).second);
}

void CAppAnalyticsClient::IncreaseValue(MapString &usageData, LPCTSTR szName, int nValue)
{
	MapString::iterator it = usageData.find(szName);
	if (it == usageData.end())
		usageData[szName] = StringFromInt(nValue);
	else
		(*it).second = StringFromInt(nValue + StringToInt((*it).second.c_str()));
}

void CAppAnalyticsClient::SaveUDCStatus()
{
	m_lastSaveTime = GetTickCount();
	m_bUsageDataModified = false;

	CTextFile file;
	int nRet = file.Open(GetAnalyticsFile().c_str(), CTextFile::OM_WRITE, ED_UTF8);
	if (nRet == ERR_OK)
	{
		file.WriteLine(SZ_APP_ANALYTICS_DATA_VER);

		CColonSeparatedValues csvValues;
		tstring strUsageData;
		GetAllUsageData(strUsageData);
		file.WriteLine(strUsageData.c_str());
	}
	else
		ERRLOG1(_T("Failed to save AA status."), Error2Str(nRet).c_str());
}

#ifdef _WIN32
void GetOSInfo(tstring &os, tstring &osLang)
{
	OSVERSIONINFO osinfo;
	memset(&osinfo, 0, sizeof(osinfo));

	osinfo.dwOSVersionInfoSize = sizeof(osinfo);
	if (!GetVersionEx(&osinfo))
	{
		os = _T("Unknown");
		return;
	}

	os = _T("Win ");

	SYSTEM_INFO si;
	memset(&si, 0, sizeof(si));
	GetSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		os += _T("64 ");

	os += CStrPrintf(_T("%d.%d"), osinfo.dwMajorVersion, osinfo.dwMinorVersion).c_str();

	// Get OS language
	tstring strLang;
	GetUserDefaultLang(strLang, osLang);
}
#endif // #ifdef _WIN32

#ifdef _MAC_OSX
void GetOSInfo(tstring &os, tstring &osLang)
{
	os = _T("Mac OS X");
	osLang = _T("Unknown");
}
#endif

void CAppAnalyticsClient::Free()
{
	for (MapUsageData::iterator it = m_mapUsageData.begin(); it != m_mapUsageData.end(); ++it)
	{
		MapString *usageData = (*it).second;
		delete usageData;
	}
	m_mapUsageData.clear();
}

void CAppAnalyticsClient::InitUDC()
{
	Free();

	MapString &usageData = GetUsageData(SZ_UD_BASIC_USAGE);
	usageData[SZ_N_BU_START_TIME] = CStrPrintf(_T("%u"), CBaseDate::GetCurrentDate().GetTime()).c_str();

	SaveUDCStatus();
}

void CAppAnalyticsClient::PrepareProductInfo(MapString &usageData, bool bInstallPing, bool bWeeklyPing)
{
	if (bInstallPing)
		usageData[SZ_N_PI_INSTALL] = SZ_1;
	if (bWeeklyPing)
		usageData[SZ_N_PI_WEEKLY] = SZ_1;

	tstring os, osLang;
	GetOSInfo(os, osLang);
	usageData[SZ_N_PI_OS] = os;
	usageData[SZ_N_PI_OS_LANGUAGE] = osLang;

	usageData[SZ_N_PI_PRODUCT_ID] = m_szProductId;
	usageData[SZ_N_PI_PRODUCT_VER] = m_szProductVer;
}

void CAppAnalyticsClient::PrepareBasicUsageData(MapString &usageData)
{
	usageData[SZ_N_BU_END_TIME] = CStrPrintf(_T("%u"), CBaseDate::GetCurrentDate().GetTime()).c_str();
}

void CAppAnalyticsClient::CheckSendTask()
{
	// Try to save Usage Data status
	if (GetTickCount() - m_lastSaveTime >= 5 * 60 * 1000)
	{
		CMutexAutolock lock(m_mutex);
		SaveUDCStatus();
	}

	// Try to send when one day past.
	CBaseDate	dateNow = GetCurrentDateOnly();
	if (abs((int)dateNow.GetTime() - (int)m_lastSendDate.GetTime()) < CBaseDate::SECOND_IN_ONE_DAY)
		return;

	if (m_threadSend.IsRunning())
		return;

	SendParam *pSendParam = new SendParam;
	pSendParam->pAAClient = this;

	pSendParam->bInstallPing = g_Profile.GetProfileBool(SZ_SECT_APP_ANALYTICS, _T("NIS"), true);
	pSendParam->bWeeklyPing = false;

	if (abs((int)dateNow.GetTime() - (int)m_lastWeeklyPingDate.GetTime()) >= CBaseDate::SECOND_IN_ONE_DAY * 7)
	{
		// More than 7 days, send weekly usageData
		pSendParam->bWeeklyPing = true;
	}

	if (pSendParam->bInstallPing || pSendParam->bWeeklyPing)
	{
		CMutexAutolock lock(m_mutex);

		// Prepare Usage Data
		PrepareProductInfo(GetUsageData(SZ_UD_PRODUCT_INFO), pSendParam->bInstallPing, pSendParam->bWeeklyPing);
		PrepareBasicUsageData(GetUsageData(SZ_UD_BASIC_USAGE));
		PrepareLicenseStatusData(GetUsageData(SZ_UD_LICENSE_STATUS));
		PrepareSettingsData(GetUsageData(SZ_UD_SETTINGS));
		PrepareOtherUsageData();

		tstring strUsageData;
		GetAllUsageData(strUsageData, true);
		TStringToUTF8(strUsageData.c_str(), strUsageData.size(), pSendParam->strUsageData);
		m_threadSend.Create(ThreadSend, pSendParam);
	}
	else
		delete pSendParam;

	g_Profile.EncryptWriteProfileString(SZ_SECT_APP_ANALYTICS, _T("LastTaskDate"), dateNow.ToUtcDateString().c_str());
}

void CAppAnalyticsClient::GetAllUsageData(tstring &strUsageData, bool bForSending)
{
	Rule rule;
	if (bForSending)
		rule.LoadFromProfile();

	// Write product info first
	CColonSeparatedValues csv;
	MapString &bu = GetUsageData(SZ_UD_PRODUCT_INFO);
	MapStringToColonSeparatedValues(bu, csv);
	strUsageData = csv.c_str();

	for (MapUsageData::iterator it = m_mapUsageData.begin(); it != m_mapUsageData.end(); ++it)
	{
		MapString *usageData = (*it).second;
		if (_tcscmp((*it).first.c_str(), SZ_UD_PRODUCT_INFO) == 0)
			continue;
		if (bForSending && !CanSendUsageDataType(*usageData, rule))
			continue;

		MapStringToColonSeparatedValues(*usageData, csv);
		strUsageData += '\n';
		strUsageData += csv.c_str();
	}
}

bool IsRuleMatch(SetTStrNCase &setEnabled, SetTStrNCase &setDisabled, LPCTSTR str)
{
	// Is Disabled?
	if (setDisabled.find(_T("*")) != setDisabled.end())
		return false;
	if (setDisabled.find(str) != setDisabled.end())
		return false;

	// Is Enabled?
	if (setEnabled.find(_T("*")) != setEnabled.end())
		return true;
	if (setEnabled.find(str) != setEnabled.end())
		return true;
	return false;
}

bool CAppAnalyticsClient::CanSendUsageDataType(MapString &usageData, Rule &rule)
{
	MapString::iterator it = usageData.find(SZ_N_USAGE_DATA_TYPE);
	if (it == usageData.end())
		return false;

	LPCTSTR szType = (*it).second.c_str();

	bool bCanSend = true;
	VersionNumber verCur = VersionNumFrom(m_szProductVer);
	if (verCur >= rule.verSeparator)
	{
		// New version
		bCanSend = IsRuleMatch(rule.newVerEnabledAA, rule.newVerDisabledAA, szType);
	}
	else
	{
		// Old version
		bCanSend = IsRuleMatch(rule.oldVerEnabledAA, rule.oldVerDisabledAA, szType);
	}

	if (!bCanSend)
		DBGLOG1(_T("The Usage Data Type is disabled to send: %s."), szType);

	return bCanSend;
}

int CAppAnalyticsClient::Send(const string &strUsageData)
{
	// Connect to server and send data
	CHttpClient	httpClient;

#ifdef _DEBUG
	tstring strServer = g_Profile.GetProfileString(_T("aaserverurl"), SZ_APP_ANALYTICS_DATA_VER);
	httpClient.Init(strServer.c_str(), NULL);
	DBGLOG1(_T("AppAnalytics server: %s"), strServer.c_str());
#else
	httpClient.Init(SZ_APP_ANALYTICS_SERVER, NULL);
#endif
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

	nRet = httpClient.SendPostRequest(strUsageData.c_str(), strUsageData.size());
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

	CColonSeparatedValues csv;
	MapString mapRetValues;
	tstring strResponse;
	UTF8ToTString(bufResponse.c_str(), bufResponse.size(), strResponse);
	csv.Split(strResponse.c_str(), mapRetValues);
	MapString::iterator it = mapRetValues.find(_T("Result"));
	if (it == mapRetValues.end()
		|| _tcscmp((*it).second.c_str(), _T("OK")) != 0)
		ERRLOG1(_T("AA Response is NOT OK: %s"), strResponse.c_str());
	else
	{
		// Succeeded, need to clear old status
		DBGLOG0(_T("Succeeded to submit AppAnalytics Data."));

		CMutexAutolock lock(m_mutex);
		InitUDC();
	}

	return ERR_OK;
}

void CAppAnalyticsClient::ThreadSend(void * lpParam)
{
	SendParam *pSendParam = (SendParam *)lpParam;

	int nRet = pSendParam->pAAClient->Send(pSendParam->strUsageData);
	if (nRet == ERR_OK)
	{
		if (pSendParam->bInstallPing)
			g_Profile.WriteProfileInt(SZ_SECT_APP_ANALYTICS, _T("NIS"), false);
		pSendParam->pAAClient->m_lastWeeklyPingDate = GetCurrentDateOnly();
		g_Profile.EncryptWriteProfileString(SZ_SECT_APP_ANALYTICS, _T("LastWeeklyDate"),
			pSendParam->pAAClient->m_lastWeeklyPingDate.ToUtcDateString().c_str());
	}

	delete pSendParam;
}
