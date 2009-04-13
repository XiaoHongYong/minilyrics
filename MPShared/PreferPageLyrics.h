#ifndef _PREFER_PAGE_LYRICS_H_
#define _PREFER_PAGE_LYRICS_H_

#pragma once

#include "PreferencePageBase.h"

//
// Lyrics search folder
//
class CPreferPageLyrics : public CPreferencePageBase
{
public:
	CPreferPageLyrics();
	~CPreferPageLyrics();

	BOOL OnInitDialog();
	void OnCommand(UINT uID, UINT nNotifyCode);
	void OnDestroy();
	BOOL OnNotify(WIDGET_NOTIFY_INFO *info);

protected:
	BOOL IsDirAdded(TCHAR szDir[]);

	CWidgetListCtrl		m_listCtrl;

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
