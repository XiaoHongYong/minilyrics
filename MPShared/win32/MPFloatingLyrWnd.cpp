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

	CSkinWnd::Create(_T("FloatingLyr"), _T("Floating Lyrics"), g_mpApp.GetSkinFactory(), SZ_SECT_FLOATING_LYR);

	m_cursor.LoadCursorFromRes(IDC_MLHAND);

	SetToolWindow(true);

	return ERR_OK;
}

void CMPFloatingLyrWnd::OnCommand(UINT uID, UINT nNotifyCode)
{
	if (uID == IDC_CLICK_THROUGH)
	{
		g_Profile.WriteProfileInt(SZ_SECT_FLOATING_LYR, _T("ClickThrough"), !m_bClickThrough);
	}

	switch (uID)
	{
	case IDC_CLOSE:
		g_Profile.WriteProfileInt(_T("FloatingLyr"), false);
		Destroy();
		break;
	default:
		CMPSkinWnd::OnCommand(uID, nNotifyCode);
		break;
	}
}

int CMPFloatingLyrWnd::OpenDefaultSkin()
{
	CloseSkin();

	m_bSkinOpened = true;
	m_bIndividualProperties = true;

	SetProperty(_T("ContextMenu"), _T("LyrBarMenu"));
	SetProperty(_T("LyrDisplayStylePropName"), _T("FloatingLyrDispStyle"));
	SetProperty(_T("ContextMenu"), _T("FloatingLyrMenu"));
	SetProperty(_T("MinWidth"), _T("280"));
	SetProperty(_T("MinHeight"), _T("10"));
	SetProperty(_T("CmdHandler"), _T("ch_floating_lyr"));
	SetProperty(_T("TranslucencyAlpha"), _T("0"));

	EnableTranslucencyLayered(true);

	// Create lyrics show object
	CUIObject		*pObj;

	pObj = m_pSkinFactory->CreateUIObject(this, CLyricShowAgentObj::ClassName());
	ASSERT(pObj);
	if (pObj)
	{
		pObj->SetProperty(SZ_PN_LEFT, _T("0"));
		pObj->SetProperty(SZ_PN_TOP, _T("0"));
		pObj->SetProperty(SZ_PN_WIDTH, _T("w"));
		pObj->SetProperty(SZ_PN_HEIGHT, _T("h"));
		pObj->SetProperty(_T("EnableAdjustVertAlign"), _T("FALSE"));
		pObj->SetProperty(_T("EnableStaticTextStyle"), _T("FALSE"));
		pObj->SetProperty(_T("LyrDisplayStylePropName"), _T("FloatingLyrDispStyle"));
		pObj->SetProperty(_T("FloatingLyr"), _T("1"));
		pObj->SetProperty(_T("MaxShowLines"), _T("4"));
		pObj->SetProperty(_T("FadeOutBorder"), _T("TRUE"));
		pObj->SetProperty(_T("EnableToolbar"), _T("FALSE"));
		pObj->SetProperty(_T("AlwaysAutoWidth"), _T("TRUE"));
		pObj->SetProperty(_T("UseAlbumArtAsBg"), _T("FALSE"));

		pObj->SetProperty(_T("BlendingLyrText"), _T("TRUE"));

		pObj->SetProperty(_T("ThickOutline"), _T("TRUE"));

		m_rootConainter.AddUIObject(pObj);
	}

	// load window size of skin
	OnLoadWndSizePos();

	UpdateSkinProperty();

	m_bClickThrough = g_Profile.GetProfileBool(SZ_SECT_FLOATING_LYR, _T("ClickThrough"), false);

	OnSkinLoaded();

	//if (!g_Profile.GetProfileBool(GetCfgSectName(), _T("TransparentBkColor"), FALSE))
	//	g_Profile.WriteProfileInt(GetCfgSectName(), _T("TransparentBkColor"), TRUE);

	//LoadTransparentSettings();

	return ERR_OK;
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
