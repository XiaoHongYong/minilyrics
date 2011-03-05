#pragma once

#include "resource.h"

enum LyricsDownloaderCustomCommand
{
	CMD_FILE_LIST = CMD_BASE_END,
};

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
