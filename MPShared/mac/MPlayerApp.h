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

	BOOL IsRunning() { return m_bRunning; }
	
	void RestartToAppMode(AppMode appMode);

protected:
	AppMode					m_appModeQuitToStart;
	bool					m_bRunning;

};
