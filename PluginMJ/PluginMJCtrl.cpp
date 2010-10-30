// PluginMJCtrl.cpp : Implementation of CPluginMJCtrl

#include "stdafx.h"
#include "PluginMJ.h"
#include "PluginMJCtrl.h"
#include "PlayerImp.h"

/////////////////////////////////////////////////////////////////////////////
// CPluginMJCtrl
#define IDT_UPDATE				1
#define EWX_FORCEIFHUNG			0x00000010

CPluginMJCtrl::CPluginMJCtrl()
{
	m_bWindowOnly = TRUE;
}

LRESULT CPluginMJCtrl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

STDMETHODIMP CPluginMJCtrl::Init(LPDISPATCH pDisp)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	g_Player.m_pMJ = pDisp;


	if (!InitMiniLyricsMsgPlugin(_Module.GetModuleInstance(), &g_Player))
		return S_OK;	// we must return 0!

// 	if (!InitMiniLyricsPlugin(_Module.GetModuleInstance(), &g_Player))
// 		return S_OK;	// we must return 0!
// 
	// now we can use other g_MLService functions.
	char	szMsg[256];

	wsprintfA(szMsg, "Start MJ Player plugin for MiniLyrics. HINSTANCE: %#X", g_hInstance);
	g_MLService.Log(szMsg);

	return S_OK;
}

LRESULT CPluginMJCtrl::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	g_MLService.Quit();

	return 0;
}
