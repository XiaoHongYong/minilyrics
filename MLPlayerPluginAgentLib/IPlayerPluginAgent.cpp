#include "stdafx.h"
#include "../MLPluginLib/MLPlugin.h"
#include "IPlayerPluginAgent.h"

bool IPlayerPluginAgent::OnTurnOn(CPlayerServiceBase *pPlayerService)
{
	ASSERT(pPlayerService);

	return pPlayerService->Create(GetAppInstance());
}

void IPlayerPluginAgent::OnTurnOff(CPlayerServiceBase *pPlayerService)
{
	if (pPlayerService)
		pPlayerService->Destroy();
}
