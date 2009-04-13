#ifndef _PREFER_PAGE_SYSTEM_H_
#define _PREFER_PAGE_SYSTEM_H_

#pragma once

#include "PreferencePageBase.h"


//
// Internet settings
//
class CPreferPageInternet : public CPreferencePageBase
{
public:
	CPreferPageInternet();
	~CPreferPageInternet();

	BOOL OnInitDialog();

	void OnCommand(UINT uID, UINT nNotifyCode);
	void OnDestroy();
	void SaveProxyServerUserPwd();

protected:

};


//
// Startup options
//
class CPPageStartup : public CPreferencePageBase
{
public:
	CPPageStartup();
	~CPPageStartup();

	BOOL OnInitDialog();
	void OnCommand(UINT uID, UINT nNotifyCode);
	void OnDestroy();

protected:

};


#endif // _PREFER_PAGE_SYSTEM_H_
