#pragma once

#define SZ_SECT_APP_ANALYTICS	_T("AA")

class CAppAnalyticsClient
{
public:
	CAppAnalyticsClient(LPCTSTR szProductId, LPCTSTR szProductVer);
	virtual ~CAppAnalyticsClient();

	enum CmdAccessMethod
	{
		CAM_MENU,
		CAM_POP_MENU,
		CAM_BUTTON,
	};

	void Init();
	void Quit();

	void IncreaseValue(LPCTSTR szType, LPCTSTR szName, int nValue);

	void WriteValue(LPCTSTR szType, LPCTSTR szName, LPCTSTR szValue);
	void WriteValue(LPCTSTR szType, LPCTSTR szName, int nValue);

	tstring GetValue(LPCTSTR szType, LPCTSTR szName, LPCTSTR szDefValue = _T(""));
	int GetValueInt(LPCTSTR szType, LPCTSTR szName, int nDefValue = 0);

	void IncreaseCmdClickCount(LPCTSTR szCmd, CmdAccessMethod cam);

	void CheckSendTask();

protected:
	virtual tstring GetAnalyticsFile() = 0;

	virtual void PrepareBasePing(MapString &ping, LPCTSTR szPingType);

	virtual void PrepareInstallPing(MapString &pingData) { }
	virtual void PrepareWeeklyPing(MapString &pingData) { }

	virtual bool GetProxy(tstring &strServer, int &port, tstring &base64UserPass) { return false; }

	MapString *GetPing(LPCTSTR szType);

	void SavePingStatus();

	void OnSavePingStatusCheck();

	int Send(const CColonSeparatedValues &csvValues);

	static void ThreadSend(void * lpParam);

	struct SendParam
	{
		CAppAnalyticsClient *pAAClient;
		CColonSeparatedValues installPing, weeklyPing;
	};

protected:
	typedef map<tstring, MapString *> MapPing;

	MapPing					m_mapPing;

	CBaseDate				m_lastSendDate, m_lastWeeklyPingDate, m_lastSaveTime;
	MapString				m_weeklyPing;

	CThread					m_threadSend;
	LPCTSTR					m_szProductId, m_szProductVer;

};
