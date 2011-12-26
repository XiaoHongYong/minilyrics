#pragma once

#include "../MLProtocol/AppAnalyticsClient.h"

class CZPAnalyticsClient : public CAppAnalyticsClient
{
public:
	CZPAnalyticsClient();
	~CZPAnalyticsClient();

protected:
	virtual void PrepareSend();

};
