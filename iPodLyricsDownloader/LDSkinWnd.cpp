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

	return true;
}
