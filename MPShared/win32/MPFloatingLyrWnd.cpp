// MPFloatingLyrWnd.cpp: implementation of the CMPFloatingLyrWnd class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "MPlayerApp.h"
#include "MPFloatingLyrWnd.h"
#include "LyricShowAgentObj.h"

CMPFloatingLyrWnd		g_wndFloatingLyr;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPFloatingLyrWnd::CMPFloatingLyrWnd()
{
	m_bManageBySkinFactory = false;
}

CMPFloatingLyrWnd::~CMPFloatingLyrWnd()
{

}

int CMPFloatingLyrWnd::Create()
{
	m_rcReal.left = g_Profile.GetProfileInt(SZ_SECT_FLOATING_LYR, _T("wnd_left"), m_rcReal.left);
	m_rcReal.top = g_Profile.GetProfileInt(SZ_SECT_FLOATING_LYR, _T("wnd_top"), m_rcReal.top);
	m_rcReal.right = m_rcReal.left + g_Profile.GetProfileInt(SZ_SECT_FLOATING_LYR, _T("wnd_width"), m_rcReal.Width());
	m_rcReal.bottom = m_rcReal.top + g_Profile.GetProfileInt(SZ_SECT_FLOATING_LYR, _T("wnd_height"), m_rcReal.Height());

	SkinWndStartupInfo skinWndStartupInfo(_T("FloatingLyr"), _T("Floating Lyrics"), 
		_T("floatinglyr.xml"), NULL);
	CMPSkinWnd::Create(skinWndStartupInfo, CMPlayerAppBase::GetMPSkinFactory());

	m_cursor.LoadCursorFromRes(IDC_MLHAND);

	return ERR_OK;
}

bool CMPFloatingLyrWnd::OnCustomCommand(int nId)
{
	if (nId == CMD_CLOSE)
		g_Profile.WriteProfileInt(_T("FloatingLyr"), false);

	return CMPSkinWnd::OnCustomCommand(nId);
}

LRESULT CMPFloatingLyrWnd::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SYSCOMMAND:
		if (wParam == SC_CLOSE)
		{
			g_Profile.WriteProfileInt(_T("FloatingLyr"), false);
		}
		break;
	}

	return CMPSkinWnd::WndProc(message, wParam, lParam);
}

bool CMPFloatingLyrWnd::SettingGetTopmost()
{
	return g_Profile.GetProfileBool(SZ_SECT_FLOATING_LYR, _T("topmost"), true);
}

int CMPFloatingLyrWnd::SettingGetOpaquePercent()
{
	return g_Profile.GetProfileInt(SZ_SECT_FLOATING_LYR, _T("WindowOpaquePercent"), 100);
}

bool CMPFloatingLyrWnd::SettingGetClickThrough()
{
	return g_Profile.GetProfileBool(SZ_SECT_FLOATING_LYR, _T("ClickThrough"), false);
}

void CMPFloatingLyrWnd::SettingReverseTopmost()
{
	bool		bTopmost = !IsTopmost();

	g_Profile.WriteProfileInt(SZ_SECT_FLOATING_LYR, _T("topmost"), bTopmost);
	SetTopmost(bTopmost);
}

void CMPFloatingLyrWnd::SettingSetOpaquePercent(int nPercent)
{
	g_Profile.WriteProfileInt(SZ_SECT_UI, _T("WindowOpaquePercent"), nPercent);
	SetTransparent(OpaquePercentToAlpha(nPercent), m_bClickThrough);
}

void CMPFloatingLyrWnd::SettingReverseClickThrough()
{
	g_Profile.WriteProfileInt(SZ_SECT_FLOATING_LYR, _T("ClickThrough"), !m_bClickThrough);
	m_bClickThrough = !m_bClickThrough;
	SetTransparent(OpaquePercentToAlpha(SettingGetOpaquePercent()),
		m_bClickThrough);
}
