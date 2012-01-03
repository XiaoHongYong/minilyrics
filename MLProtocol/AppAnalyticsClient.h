#pragma once

#define SZ_SECT_APP_ANALYTICS	_T("AA")

#define SZ_N_USAGE_DATA_TYPE	_T("AA")	// Name it AA to make sure it's the first property in log

// Define Usage Data types
#define SZ_UD_PRODUCT_INFO		_T("PI")
#define SZ_UD_BASIC_USAGE		_T("BU")
#define SZ_UD_LICENSE_STATUS	_T("LS")
#define SZ_UD_CMD_USAGE			_T("CU")
#define SZ_UD_SETTINGS			_T("ST")

// Product info properties
#define SZ_N_PI_PRODUCT_ID		_T("PID")
#define SZ_N_PI_PRODUCT_VER		_T("PV")
#define SZ_N_PI_WEEKLY			_T("WK")
#define SZ_N_PI_INSTALL			_T("IS")
#define SZ_N_PI_DEVICE_ID		_T("DID")
#define SZ_N_PI_OS				_T("OS")
#define SZ_N_PI_OS_VER			_T("OSV")
#define SZ_N_PI_OS_LANGUAGE		_T("OL")

// Basic Usage Data properties
#define SZ_N_BU_START_TIME		_T("ST")
#define SZ_N_BU_END_TIME		_T("ST")
#define SZ_N_BU_RUN_DURATION	_T("RD")	// Total running durations, in seconds
#define SZ_N_BU_RUN_TIMES		_T("RT")
#define SZ_N_BU_CLOSED_TIMES	_T("CT")	// After running, app is closed almost immediately(less than 2 minUtes)
#define SZ_N_BU_CRASH_TIMES		_T("CRT")	// After running, app is closed almost immediately(less than 2 minUtes)

// License status properties
#define SZ_N_LS_LICENSE_STATUS	_T("LS")
#define SZ_N_LS_CODE			_T("C")
#define SZ_N_LS_NAME			_T("N")
#define SZ_N_LS_START_DATE		_T("SD")
#define SZ_N_LS_DURATION_DAYS	_T("DD")
#define SZ_N_LS_LICENSED_USERS	_T("LU")
#define SZ_N_LS_ONLINE_VERIFIED	_T("OV")

#ifndef SZ_1
#define SZ_1					_T("1")
#endif

class CAppAnalyticsClient
{
public:
	CAppAnalyticsClient(LPCTSTR szProductId, LPCTSTR szProductVer);
	virtual ~CAppAnalyticsClient();

	struct Rule
	{
		VersionNumber		verSeparator;
		SetTStrNCase		newVerEnabledAA, newVerDisabledAA;
		SetTStrNCase		oldVerEnabledAA, oldVerDisabledAA;

		static bool SaveFromXml(SXNode *pNode);
		void LoadFromProfile();

	};

	enum CmdAccessMethod
	{
		CAM_MENU		= 'M',
		CAM_POP_MENU	= 'P',
		CAM_BUTTON		= 'B',
	};

	void Init();
	void Quit();

	void IncreaseValue(LPCTSTR szType, LPCTSTR szName, int nValue);

	void WriteValue(LPCTSTR szType, LPCTSTR szName, LPCTSTR szValue);
	void WriteValue(LPCTSTR szType, LPCTSTR szName, int nValue);

	tstring GetValue(LPCTSTR szType, LPCTSTR szName, LPCTSTR szDefValue = _T(""));
	int GetValueInt(LPCTSTR szType, LPCTSTR szName, int nDefValue = 0);

	void IncreaseCmdClickCount(LPCTSTR szCmd, CmdAccessMethod cam);
	void IncreaseCrashTimes();

	void CheckSendTask();

protected:
	void IncreaseValue(MapString &usageData, LPCTSTR szName, int nValue);

	void Free();
	virtual void InitUDC();

	virtual tstring GetAnalyticsFile() = 0;

	virtual void PrepareProductInfo(MapString &usageData, bool bInstallPing, bool bWeeklyPing);
	virtual void PrepareBasicUsageData(MapString &usageData);
	virtual void PrepareLicenseStatusData(MapString &usageData) { }
	virtual void PrepareSettingsData(MapString &usageData) { }
	virtual void PrepareOtherUsageData() { }

	virtual bool GetProxy(tstring &strServer, int &port, tstring &base64UserPass) { return false; }

	MapString &GetUsageData(LPCTSTR szType);

	bool CanSendUsageDataType(MapString &usageData, Rule &rule);

	void SaveUDCStatus();

	void GetAllUsageData(tstring &strUsageData, bool bForSending = false);

	int Send(const string &strUsageData);

	static void ThreadSend(void * lpParam);

	struct SendParam
	{
		CAppAnalyticsClient *pAAClient;
		bool				bWeeklyPing, bInstallPing;
		string				strUsageData;
	};

protected:
	typedef map<tstring, MapString *> MapUsageData;

	MapUsageData					m_mapUsageData;
	Mutex					m_mutex;

	CBaseDate				m_lastSendDate, m_lastWeeklyPingDate;

	CThread					m_threadSend;
	LPCTSTR					m_szProductId, m_szProductVer;

	uint32					m_nAppStartTime;

	bool					m_bUsageDataModified;
	uint32					m_lastSaveTime;

};
