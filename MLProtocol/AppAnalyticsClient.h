#pragma once

#define SZ_SECT_APP_ANALYTICS	_T("AA")

class CAppAnalyticsClient
{
public:
	CAppAnalyticsClient(LPCTSTR szProductId, LPCTSTR szProductVer);
	virtual ~CAppAnalyticsClient();

	void Init();

	void WriteValue(LPCTSTR szName, LPCTSTR szValue);
	void WriteValue(LPCTSTR szName, int nValue);

	tstring GetValue(LPCTSTR szName, LPCTSTR szDefValue = _T(""));
	int GetValueInt(LPCTSTR szName, int szValueDef = 0);

	void CheckSendTask();

protected:
	virtual tstring GetAnalyticsFile() = 0;

	virtual void PrepareBasePing(MapString &ping, LPCTSTR szPingType);

	virtual void PrepareInstallPing(MapString &pingData) { }
	virtual void PrepareWeeklyPing(MapString &pingData) { }

	virtual bool GetProxy(tstring &strServer, int &port, tstring &base64UserPass) { return false; }

	void SavePingStatus();

	int Send(const CColonSeparatedValues &csvValues);

	static void ThreadSend(void * lpParam);

	struct SendParam
	{
		CAppAnalyticsClient *pAAClient;
		CColonSeparatedValues installPing, weeklyPing;
	};

protected:
	CBaseDate				m_lastSendDate, m_lastWeeklyPingDate;
	MapString				m_weeklyPing;

	CThread					m_threadSend;
	LPCTSTR					m_szProductId, m_szProductVer;

};
