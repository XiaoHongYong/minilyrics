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

	virtual void OnCopyData(HWND hWnd, PCOPYDATASTRUCT pCopyData);

	void UpdatePlayerSysTrayIcon() { m_mlTrayIcon.UpdatePlayerSysTrayIcon(); }

	CMLTrayIcon &GetTrayIcon() { return m_mlTrayIcon; }

#ifndef _MPLAYER
public:
	//
	// functions for embedded into other windows...
	//
	virtual bool IsTopmost();
	virtual bool IsIconic();
	virtual void Topmost(bool bTopmost);
	virtual void Minimize();
	virtual void Activate();

	HWND GetRootParentWnd();
#endif

protected:
	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	CMLTrayIcon		m_mlTrayIcon;

};
