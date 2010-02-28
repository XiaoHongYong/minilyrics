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
	m_rcReal.top = m_rcReal.left = 0;
	m_rcReal.right = 640;
	m_rcReal.bottom = 150;
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

	CSkinWnd::Create(_T("FloatingLyr"), _T("Floating Lyrics"), g_mpApp.GetSkinFactory(), SZ_SECT_FLOATING_LYR, NULL, true);

	m_bClickThrough = g_Profile.GetProfileBool(SZ_SECT_FLOATING_LYR, _T("ClickThrough"), false);
	LoadTransparentSettings();

	m_cursor.LoadCursorFromRes(IDC_MLHAND);

	return ERR_OK;
}

void CMPFloatingLyrWnd::OnCommand(UINT uID, UINT nNotifyCode)
{
	if (uID == IDC_CLICK_THROUGH)
	{
		g_Profile.WriteProfileInt(SZ_SECT_FLOATING_LYR, _T("ClickThrough"), !m_bClickThrough);
		m_bClickThrough = !m_bClickThrough;
		LoadTransparentSettings();
		return;
	}

	CMPSkinWnd::OnCommand(uID, nNotifyCode);
}

bool CMPFloatingLyrWnd::OnCustomCommand(int nId)
{
	if (nId == CMD_CLOSE)
		g_Profile.WriteProfileInt(_T("FloatingLyr"), false);

	return CMPSkinWnd::OnCustomCommand(nId);
}

int CMPFloatingLyrWnd::OpenDefaultSkin()
{
	CloseSkin();

	CSimpleXML	xml;
	TCHAR		szFloatingLyrXML[MAX_PATH];

	//
	// Load floating lyrics window xml file.
	//
	if (!g_mpApp.GetSkinFactory()->GetResourceMgr()->GetResourcePathName(
		szFloatingLyrXML, CountOf(szFloatingLyrXML), _T("floatinglyr.xml")))
		return ERR_NOT_FOUND;

	if (xml.ParseFile(szFloatingLyrXML))
	{
		return FromXML(xml.m_pRoot);
	}
	else
	{
		ERRLOG1(_T("Failed to load floating lyrics: %s"), szFloatingLyrXML);
		return ERR_PARSE_XML;
	}
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
