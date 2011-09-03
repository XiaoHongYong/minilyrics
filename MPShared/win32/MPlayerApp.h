#pragma once

#include "../MPlayerAppBase.h"

class CMPlayerApp : public CMPlayerAppBase
{
public:
	CMPlayerApp();
	virtual ~CMPlayerApp();

	bool IsAnotherInstanceRunning();
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

	void RestartToAppMode(AppMode appMode);

protected:
	AppMode					m_appModeQuitToStart;

#endif

protected:
	HANDLE					m_hMutexRuning;

};
