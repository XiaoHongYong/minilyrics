#include "StdAfx.h"
#include "MLPlayerPluginAgentLib.h"
#include "../MLPluginLib/MLPlugin.h"
#include "MPC\PlayerImp.h"

CMLService		g_MLService;

MPCAgent::CMPCPluginAgent		g_mpcPluginAgent;

void RegisteredPlayerPluginAgent(VecPlayerPluginAgent &vAgent)
{
	vAgent.push_back(&g_mpcPluginAgent);
}

int MbcsToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut, int nCodePage)
{
	int		n;

	n = MultiByteToWideChar(nCodePage, 0,
		str, nLen, strOut, nOut);
	if (n == 0 && GetLastError() == ERROR_INVALID_PARAMETER)
	{
		n = MultiByteToWideChar(CP_ACP, 0,
			str, nLen, strOut, nOut);
	}

	if (n < nOut)
		strOut[n] = '\0';

	return n;
}
