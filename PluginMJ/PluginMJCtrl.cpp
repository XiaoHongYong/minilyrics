// PluginMJCtrl.cpp : Implementation of CPluginMJCtrl

#include "stdafx.h"
#include "PluginMJ.h"
#include "PluginMJCtrl.h"
#include "../MLPluginLib/MLPlugin.h"

/////////////////////////////////////////////////////////////////////////////
// CPluginMJCtrl
#define IDT_UPDATE				1
#define EWX_FORCEIFHUNG			0x00000010

void InitMJ(HINSTANCE hInstance, LPDISPATCH pDisp);

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

	InitMJ(_Module.GetModuleInstance(), pDisp);

	return S_OK;
}

LRESULT CPluginMJCtrl::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	g_MLService.OnDisconnected();

	return 0;
}
