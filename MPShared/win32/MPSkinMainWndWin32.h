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

	virtual void OnCopyData(WPARAM wParam, PCOPYDATASTRUCT pCopyData);

	virtual void OnSkinLoaded();

	virtual void OnTimer(UINT nIDEvent);

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
#endif
	virtual void Activate();

	HWND GetRootParentWnd();

protected:
	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	void SetEmbedded();

protected:
	CMLTrayIcon		m_mlTrayIcon;

	UINT			m_nTimerFixWndFocus;

};
