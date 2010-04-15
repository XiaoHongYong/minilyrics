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

//
// Internet Lyrics Search Options
//
class CPreferPageLyricsDownload : public CPreferencePageBase
{
public:
	CPreferPageLyricsDownload();
	~CPreferPageLyricsDownload();

	void OnCommand(UINT uID, UINT nNotifyCode);
	BOOL OnInitDialog();

};

#endif // _PREFER_PAGE_LYRICS_H_
