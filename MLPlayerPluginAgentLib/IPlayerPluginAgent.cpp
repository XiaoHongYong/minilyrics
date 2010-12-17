#include "stdafx.h"
#include "../MLPluginLib/MLPlugin.h"
#include "IPlayerPluginAgent.h"

bool IPlayerPluginAgent::TurnOn()
{
	ASSERT(m_pPlayerService);

	return m_pPlayerService->Create(GetAppInstance());
}

void IPlayerPluginAgent::TurnOff()
{
	if (m_pPlayerService)
		m_pPlayerService->Destroy();
}

bool IPlayerPluginAgent::IsConnected()
{
	return g_MLService.m_hWndPlayerService == m_pPlayerService->GetHandle();
}
