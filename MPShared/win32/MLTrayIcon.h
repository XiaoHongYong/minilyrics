// MLTrayIcon.h: interface for the CMLTrayIcon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MLTRAYICON_H__43C46452_1848_4A21_BE3D_AB1CD8EBE0F8__INCLUDED_)
#define AFX_MLTRAYICON_H__43C46452_1848_4A21_BE3D_AB1CD8EBE0F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MPWM_TRAYICN		(WM_USER + 10)

enum SHOW_ICON_ON
{
	SHOW_ICON_ON_TASKBAR = 0,
	SHOW_ICON_ON_SYSTRAY,
	SHOW_ICON_ON_BAR_TRAY,
	SHOW_ICON_ON_NONE
};

struct SYSTRAY_ICON_CMD
{
	DWORD	dwCmd;
	LPCTSTR	szCmd;
	UINT	uIconID;
	HICON	hIcon;
	bool	bEnable;
};

extern SYSTRAY_ICON_CMD	g_SysTrayIconCmd[];
extern int				MAX_PLAYER_TRAY_ICON_CMD;

class CMLTrayIcon  
{
public:
	CMLTrayIcon();
	virtual ~CMLTrayIcon();

	void Init(CBaseWnd *pWnd);
	void Quit();

	void UpdateShowIconPos();
	void UpdatePlayerSysTrayIcon();

	void UpdateTrayIconText(LPCTSTR szText);

	void OnMyNotifyIcon(WPARAM wParam, LPARAM lParam);

	void ForceShow(bool bForceShow);

protected:
#ifndef _WIN32_WCE
	CBaseWnd		*m_pWnd;
	HICON			m_hIconTray;
#endif

	bool			m_bForceShow;

};

#endif // !defined(AFX_MLTRAYICON_H__43C46452_1848_4A21_BE3D_AB1CD8EBE0F8__INCLUDED_)
