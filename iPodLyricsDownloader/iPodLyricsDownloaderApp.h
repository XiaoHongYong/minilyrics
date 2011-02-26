#pragma once

#include "resource.h"

enum LyricsDownloaderCustomCommand
{
	CMD_CLOSE,
	CMD_QUIT,
	CMD_MINIMIZE,
	CMD_MAXIMIZE,
	CMD_MENU,
	CMD_FILE_LIST,
};

extern STRING_ID_MAP g_StringIdMap[];

class CiPodLyricsDownloaderApp : public CSkinApp
{
public:
	CiPodLyricsDownloaderApp(void);
	~CiPodLyricsDownloaderApp(void);

	static CiPodLyricsDownloaderApp *GetInstance();

	virtual bool Init();
	virtual void Quit();

	virtual CSkinFactory *NewSkinFactory();

};
