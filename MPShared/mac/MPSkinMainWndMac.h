// MPSkinMainWnd.h: interface for the CMPSkinMainWnd class.
#pragma once

#include "MPSkinWnd.h"
#include "MLTrayIcon.h"
#include "MPSkinMainWnd.h"

class CMPSkinMainWnd : public CMPSkinMainWndBase
{
public:
	CMPSkinMainWnd();
	virtual ~CMPSkinMainWnd();

	virtual bool OnCreate();
	virtual void OnDestroy();

	// IEventHandler
	virtual void OnEvent(const IEvent *pEvent);

	void UpdatePlayerSysTrayIcon() { m_mlTrayIcon.UpdatePlayerSysTrayIcon(); }

	CMLTrayIcon &GetTrayIcon() { return m_mlTrayIcon; }

#ifndef _MPLAYER
public:
	//
	// functions for embedded into other windows...
	//
	virtual void Topmost(bool bTopmost);
	virtual void Minimize();
	virtual void Activate();
#endif

protected:
	CMLTrayIcon		m_mlTrayIcon;

};
