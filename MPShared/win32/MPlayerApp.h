#pragma once

#include "../MPlayerAppBase.h"

///////////////////////////////////////////////////////////////////////////////
// Messages to wake up Main Wnd
const int	WM_MM_NOTIFY		= WM_USER;

// WM_MM_NOTIFY消息的子消息的定义
enum WM_MM_NOTIFY_SUB_MSG
{
	MM_WAKEUP				= 4,
};

class CMPlayerApp : public CMPlayerAppBase
{
public:
	CMPlayerApp();
	virtual ~CMPlayerApp();

	bool SetRunningFlag();

	virtual bool Init();
	virtual void Quit();

	BOOL IsRunning() { return m_hMutexRuning != NULL; }

#ifndef _MPLAYER
	//
	// Embedded skin mode support.
	//
	bool IsSupportEmbedded();
	bool IsEmbeddedImmovable();
	bool IsEmbeddedMode() { return !m_bMPSkinMode; }

	int OnSwitchToEmbeddedSkinMode(bool bReloadTheme);
	void EndEmbeddedSkin();

	int LoadGeneralEmbeddedSkin(bool bLoadSkinColorTheme = true);
	int LoadWinamp2Skin(bool bLoadSkinColorTheme = true);
#endif

protected:
	HANDLE					m_hMutexRuning;

};
