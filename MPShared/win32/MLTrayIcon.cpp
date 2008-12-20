// MLTrayIcon.cpp: implementation of the CMLTrayIcon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MLTrayIcon.h"
#include "MLCmd.h"
#include "ShellNotifyIcon.h"

#ifdef _WIN32_WCE

CMLTrayIcon::CMLTrayIcon() { }
CMLTrayIcon::~CMLTrayIcon() { }
void CMLTrayIcon::Init(CBaseWnd *pWnd) { }
void CMLTrayIcon::Quit() { }
void CMLTrayIcon::UpdateShowIconPos() { }
void CMLTrayIcon::UpdatePlayerSysTrayIcon() { }
void CMLTrayIcon::UpdateCaptionText() { }
void CMLTrayIcon::OnMyNotifyIcon(WPARAM wParam, LPARAM lParam) { }
void CMLTrayIcon::ForceShow(bool bForceShow) { }

#else // _WIN32_WCE

#define IDN_MINILYRICS			1
#define IDN_PLAYER_CTRL			2
#define IDN_PLAYER_CTRL_MAX		(2 + 5)

#define MAX_PLAYER_TRAY_ICON_CMD	4

SYSTRAY_ICON_CMD	g_SysTrayIconCmd[] = 
{
	{ CMD_NEXT,	_T("Next Track"), IDI_NEXT,	NULL, false },
	{ CMD_STOP, _T("Stop"), IDI_STOP, NULL, false },
	{ CMD_PLAYPAUSE, _T("Play/Pause"), IDI_PLAY, NULL, false },
	{ CMD_PREVIOUS, _T("Previous Track"), IDI_PREV, NULL, false },
	{ 0, NULL, 0, NULL }
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLTrayIcon::CMLTrayIcon()
{
// 	for (int i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++)
// 		g_SysTrayIconCmd[i] = NULL;

	m_pWnd = NULL;
	m_hIconTray = NULL;
	m_bForceShow = false;
}

CMLTrayIcon::~CMLTrayIcon()
{

}

void CMLTrayIcon::Init(CBaseWnd *pWnd)
{
	m_pWnd = pWnd;

	//
	// 更新按钮托盘图标
	for (int i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++)
	{
		TCHAR	szKey[128];

		wsprintf(szKey, _T("TrayIcon%d"), i);
		g_SysTrayIconCmd[i].bEnable = g_Profile.GetProfileBool(szKey, false);
		g_SysTrayIconCmd[i].hIcon = (HICON)::LoadImage(GetAppInstance(), MAKEINTRESOURCE(g_SysTrayIconCmd[i].uIconID), IMAGE_ICON, 16, 16, LR_VGACOLOR);
	}

	m_hIconTray = (HICON)::LoadImage(GetAppInstance(), MAKEINTRESOURCE(IDI_MPLAYER), IMAGE_ICON, 16, 16, LR_VGACOLOR);

	UpdatePlayerSysTrayIcon();
	UpdateShowIconPos();
}

void CMLTrayIcon::Quit()
{
	//
	// remove icon in tray bar
	CShellNotifyIcon::DelIcon(m_pWnd, IDN_MINILYRICS);

	if (m_hIconTray)
	{
		DestroyIcon(m_hIconTray);
		m_hIconTray = NULL;
	}

	//
	// 删除系统托盘栏上播放器的控制图标
	for (int i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++)
	{
		CShellNotifyIcon::DelIcon(m_pWnd, IDN_PLAYER_CTRL + i);

		if (g_SysTrayIconCmd[i].hIcon)
			DestroyIcon(g_SysTrayIconCmd[i].hIcon);
	}
}

void CMLTrayIcon::OnLanguageChanged()
{
}

void CMLTrayIcon::UpdateShowIconPos()
{
	tstring		strCaption;
	SHOW_ICON_ON	showIconOn;
	bool		bToolWindow = false;
	bool		bAddTrayIcon = true;

	showIconOn = (SHOW_ICON_ON)g_Profile.GetProfileInt(SZ_SECT_UI, _T("ShowIconOn"), SHOW_ICON_ON_BAR_TRAY);

	if (!IsEmptyString(g_Player.GetFullTitle()))
	{
		strCaption = g_Player.GetFullTitle();
		strCaption += _T(" - ");
		strCaption += SZ_APP_NAME;
	}
	else
		strCaption = SZ_ML_VERSION;

	switch (showIconOn)
	{
	case SHOW_ICON_ON_NONE:
		bAddTrayIcon = false;
		bToolWindow = true;
		break;
	case SHOW_ICON_ON_TASKBAR:
		bAddTrayIcon = false;
		bToolWindow = false;
		break;
	case SHOW_ICON_ON_SYSTRAY:
		bAddTrayIcon = true;
		bToolWindow = true;
		break;
	case SHOW_ICON_ON_BAR_TRAY:
		bAddTrayIcon = true;
		bToolWindow = false;
		break;
	}

	if (m_bForceShow)
		bAddTrayIcon = true;

	m_pWnd->SetToolWindow(bToolWindow);

	if (bAddTrayIcon)
	{
		// add icon in tray bar
		CShellNotifyIcon::AddIcon(m_pWnd, IDN_MINILYRICS, strCaption.c_str(), m_hIconTray, MPWM_TRAYICN);

		if (m_pWnd->IsIconic())
			m_pWnd->ShowWindow(SW_HIDE);
	}
	else
	{
		// remove icon in tray bar
		CShellNotifyIcon::DelIcon(m_pWnd, IDN_MINILYRICS);
	}
}

// COMMENT:
//		将
void CMLTrayIcon::UpdatePlayerSysTrayIcon()
{
	int		i;

	for (i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++)
	{
		CShellNotifyIcon::DelIcon(m_pWnd, IDN_PLAYER_CTRL + i);
	}
	for (i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++)
	{
		if (g_SysTrayIconCmd[i].bEnable)
		{
			// add icon in tray bar
			CShellNotifyIcon::AddIcon(m_pWnd, IDN_PLAYER_CTRL + i, 
				TOLOCAL(g_SysTrayIconCmd[i].szCmd), 
				g_SysTrayIconCmd[i].hIcon, MPWM_TRAYICN);
		}
	}
}

void CMLTrayIcon::UpdateTrayIconText(LPCTSTR szText)
{
	int			nShowPos;

	nShowPos = g_Profile.GetProfileInt(SZ_SECT_UI, _T("ShowIconOn"), SHOW_ICON_ON_SYSTRAY);

	if (nShowPos == SHOW_ICON_ON_BAR_TRAY || nShowPos == SHOW_ICON_ON_SYSTRAY)
		CShellNotifyIcon::ModifyIcon(m_pWnd, IDN_MINILYRICS, szText, NULL);
}

void CMLTrayIcon::OnMyNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDN_MINILYRICS)
	{
		if (lParam == WM_RBUTTONUP)
		{
			// show popup menu
			POINT pt;
			GetCursorPos(&pt);
			g_mpApp.GetMainWnd()->SetForeground();
			g_mpApp.GetMainWnd()->OnContexMenu(pt.x, pt.y);
		}
		else if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP)
		{
#ifndef _MPLAYER
			if (g_mpApp.IsEmbeddedMode()
				&& g_Player.GetPlayerType() == P_WINAMP5)
			{
				// buf fix for winamp5
				HWND	hWnd = g_Player.GetMainWnd();
				if (::IsIconic(hWnd))
				{
					::ShowWindow(hWnd, SW_RESTORE);
					if (g_mpApp.GetMainWnd()->IsTopmost())
						g_mpApp.GetMainWnd()->Topmost(g_mpApp.GetMainWnd()->IsTopmost());
				}
			}
			g_mpApp.GetMainWnd()->Activate();
#else
			ActivateWindow(g_mpApp.GetMainWnd()->GetHandle());
#endif
		}
	}
	else
	{
		int i = wParam - IDN_PLAYER_CTRL;
		assert(i >= 0 && i < MAX_PLAYER_TRAY_ICON_CMD);
		if (i >= 0 && i < MAX_PLAYER_TRAY_ICON_CMD)
		{
			if (lParam == WM_RBUTTONUP)
			{
// 				// show popup menu
// 				POINT	pt;
// 				HMENU	hMenuPlayer;
// 
// 				hMenuPlayer = GetSubMenu(m_hmenuContext, 4);
// 				GetCursorPos(&pt);
// 				m_pWnd->SetForeground();
// 
// 				TrackPopupMenu(hMenuPlayer, TPM_RIGHTBUTTON | TPM_LEFTALIGN, pt.x, pt.y, 0, m_pWnd->GetHandle(), NULL);
			}
			else if (lParam == WM_LBUTTONUP)
			{
				g_mpApp.GetMainWnd()->OnCustomCommand(g_SysTrayIconCmd[i].dwCmd);
			}
			else if (lParam == WM_LBUTTONDBLCLK)
			{
				// 激活播放器窗口
				::ActivateWindow(g_Player.GetMainWnd());
			}
		}
	}
}

void CMLTrayIcon::ForceShow(bool bForceShow)
{
	if (m_bForceShow != bForceShow)
	{
		m_bForceShow = bForceShow;

		UpdateShowIconPos();
	}
}

#endif // _WIN32_WCE
