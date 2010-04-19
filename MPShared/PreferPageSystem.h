#ifndef _PREFER_PAGE_SYSTEM_H_
#define _PREFER_PAGE_SYSTEM_H_

#pragma once

#include "PreferencePageBase.h"


class CPagePfSystemRoot : public CPagePfBase
{
public:
	CPagePfSystemRoot();

	static LPCTSTR GetPageID();

	void OnInitialUpdate();

	// bool OnCustomCommand(int nId);

};

#endif // _PREFER_PAGE_SYSTEM_H_
