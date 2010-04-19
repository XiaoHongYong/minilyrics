#ifndef _PREFER_PAGE_THEME_H_
#define _PREFER_PAGE_THEME_H_

#pragma once

#include "PreferencePageBase.h"

class CPagePfThemeRoot : public CPagePfBase
{
public:
	CPagePfThemeRoot(bool bFloatingLyrMode);

	static LPCTSTR GetPageID();

	void OnInitialUpdate();

	// bool OnCustomCommand(int nId);

};


#endif // _PREFER_PAGE_THEME_H_
