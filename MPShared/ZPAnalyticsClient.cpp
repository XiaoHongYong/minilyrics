#include "StdAfx.h"
#include "ZPAnalyticsClient.h"
#include "MLVersionStr.h"

#ifdef _MPLAYER
#define SZ_APP_ANALYTICS_PRODUCT_ID		_T("ZP")
#else
#define SZ_APP_ANALYTICS_PRODUCT_ID		_T("ML")
#endif

CZPAnalyticsClient::CZPAnalyticsClient() :
	CAppAnalyticsClient(SZ_APP_ANALYTICS_PRODUCT_ID, _T(ML_VERSION_STR))
{
}

CZPAnalyticsClient::~CZPAnalyticsClient()
{
}

void CZPAnalyticsClient::PrepareSend()
{

}
