#ifndef _PREFER_PAGE_UI_H_
#define _PREFER_PAGE_UI_H_

#pragma once

#include "PreferencePageBase.h"

class CPagePfUIRoot : public CPagePfBase
{
public:
	CPagePfUIRoot();

	static LPCTSTR GetPageID();

	void OnInitialUpdate();

};


#endif // _PREFER_PAGE_UI_H_
