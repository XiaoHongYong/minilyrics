//
//  main.m
//  MiniLyrics
//
//  Created by Hongyong Xiao on 11/13/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "stdafx.h"
#import "MPlayerApp.h"

bool IsLyricsDownloader(int argc, char *argv[])
{
	for (int i = 0; i < argc; i++)
	{
		if (_tcsicmp(argv[i], _T("/iPodLyricsDownloader")) == 0)
			return true;
	}
	
	return false;
}

void RestartToLyricsDownloader()
{
	NSString *appExe = [[[NSProcessInfo processInfo] arguments] objectAtIndex:0];

	pid_t child = fork();
	if (child == 0)
	{
		int ret = execl("/usr/bin/open", "-n", "-a", [appExe UTF8String], "--args", "/iPodLyricsDownloader", NULL);
		if (ret == 0)
			ERRLOG0("Succeeded to start MiniLyrics.");
		else
			ERRLOG1("Failed to start MiniLyrics: %s", strerror(errno));
	}
	else if (child == -1)
		return;
	else {
		int status = 0;
		waitpid(child, &status, WNOHANG);
	}
}

int main(int argc, char *argv[])
{
	InitBaseFrameWork(argc, argv, "MiniLyrics.log", "MiniLyrics.ini", SZ_SECT_UI);

	CMPlayerApp *pApp = CMPlayerApp::GetInstance();
	if (IsLyricsDownloader(argc, argv))
	{
		pApp->m_appMode = SA_IPOD_LYRICS_DOWNLOADER;
	}
	else
	{
		pApp->m_appMode = SA_LYRICS_PLAYER;
		if (!g_playerMgr.Init(argc, argv))
			return 0;
	}

	return NSApplicationMain(argc, (const char **)argv);
}
