#include "StdAfx.h"
#include "iPodLyricsDownloaderApp.h"
#include "LDSkinWnd.h"

CLDSkinWnd::CLDSkinWnd(void)
{
}

CLDSkinWnd::~CLDSkinWnd(void)
{
}

void CLDSkinWnd::OnCommand(UINT uID, UINT nNotifyCode)
{
	CSkinWnd::OnCommand(uID, nNotifyCode);
}

bool CLDSkinWnd::OnCustomCommand(int nId)
{
	if (CSkinWnd::OnCustomCommand(nId))
		return true;

	switch (nId)
	{
	case CMD_CLOSE:
		Destroy();
		break;
	case CMD_QUIT:
		CiPodLyricsDownloaderApp::GetInstance()->PostQuitMessage();
		break;
	case CMD_MINIMIZE:
		ShowWindow(SW_SHOWMINIMIZED);
		break;
	case CMD_MAXIMIZE:
		if (IsZoomed())
			ShowWindow(SW_RESTORE);
		else
			ShowWindow(SW_MAXIMIZE);
		break;
	case CMD_MENU:
		break;
	}

	return true;
}
