#ifndef _PREFER_PAGE_LYRICS_H_
#define _PREFER_PAGE_LYRICS_H_

#pragma once

#include "PreferencePageBase.h"

class CPagePfLyricsRoot : public CPagePfBase
{
public:
	CPagePfLyricsRoot();

	static LPCTSTR GetPageID();

	void OnInitialUpdate();

	// bool OnCustomCommand(int nId);

};

#endif // _PREFER_PAGE_LYRICS_H_
