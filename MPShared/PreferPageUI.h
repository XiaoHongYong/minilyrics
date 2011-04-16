#pragma once

#include "PreferencePageBase.h"

class CPagePfUIRoot : public CPagePfBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
	CPagePfUIRoot();

	void OnInitialUpdate();

};

void RegisterPfUIPages(CSkinFactory *pSkinFactory);
