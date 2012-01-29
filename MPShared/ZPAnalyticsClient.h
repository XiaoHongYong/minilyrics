#pragma once

#include "../MLProtocol/AppAnalyticsClient.h"

// Ping type
#define SZ_UD_SEARCH_LYRICS_DATA		_T("SLR")
#define SZ_UD_BATCH_SEARCH_DATA			_T("BSR")
#define SZ_UD_DOWNLOAD_LYRICS_DATA		_T("DLR")
#define SZ_UD_UPLOAD_LYRICS_DATA		_T("ULR")
#define SZ_UD_EMBEDDED_LYRICS_DATA		_T("ELR")
#define SZ_UD_PLAYER_RUN_TIME			_T("PR")

#define SZ_N_NUMBER_OF_RESULT			_T("NR")

class CZPAnalyticsClient : public CAppAnalyticsClient
{
public:
	CZPAnalyticsClient();
	~CZPAnalyticsClient();

	virtual void Quit();

	void IncreaseCmdClickCount(int nUID, CmdAccessMethod cam);

	void IncreaseNumberOfResult(LPCTSTR szType, int nResult);

#ifdef _DEBUG
	void TestSend();
#endif

protected:
	virtual tstring GetAnalyticsFile();

	virtual void PrepareProductInfo(MapString &ping, bool bInstallPing, bool bWeeklyPing);
	virtual void PrepareLicenseStatusData(MapString &ping);
	virtual void PrepareSettingsData(MapString &ping);
	virtual void PrepareOtherUsageData();

	void PrepareModeRelatedSettingsData(LPCTSTR szSection, MapString &ping, bool bFloatingLyr);

	virtual bool GetProxy(tstring &strServer, int &port, tstring &base64UserPass);

};
