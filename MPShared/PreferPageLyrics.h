#ifndef _PREFER_PAGE_LYRICS_H_
#define _PREFER_PAGE_LYRICS_H_

#pragma once

#include "PreferencePageBase.h"

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
