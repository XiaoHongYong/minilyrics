#pragma once

#include "../MLProtocol/AppAnalyticsClient.h"

class CZPAnalyticsClient : public CAppAnalyticsClient
{
public:
	CZPAnalyticsClient();
	~CZPAnalyticsClient();

	void IncreaseCmdClickCount(int nUID, CmdAccessMethod cam);

protected:
	virtual tstring GetAnalyticsFile();

	virtual void PrepareInstallPing(MapString &pingData);
	virtual void PrepareWeeklyPing(MapString &pingData);

	virtual bool GetProxy(tstring &strServer, int &port, tstring &base64UserPass);

};
