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
	tstring GetAnalyticsFile();

	void SavePingStatus();

	virtual void PrepareInstallPing(MapString &pingData) { }
	virtual void PrepareWeeklyPing(MapString &pingData) { }


	// This method will be called before send data to server
	virtual void PrepareSend(bool bInstallPing, bool bWeeklyPing) { }

	int Send(const CColonSeparatedValues &csvValues);

protected:
	CBaseDate				m_lastSendDate, m_lastWeeklyPingDate;
	MapString				m_weeklyPing;

	CThread					m_threadSend;

};
