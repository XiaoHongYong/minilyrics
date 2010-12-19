#include "StdAfx.h"
#include "MLPlayerPluginAgentLib.h"
#include "../MLPluginLib/MLPlugin.h"
#include "MPC\MpcPlayerService.h"
// #include "HMM\HmmPlayerService.h"
#include "AlbumPlayer\ApPlayerService.h"

CMLService		g_MLService;

void RegisteredPlayerPluginAgent(VecPlayerPluginAgent &vAgent)
{
	vAgent.push_back(new MPCAgent::CMPCPluginAgent());
	vAgent.push_back(new AlbumPlayerAgent::CApPluginAgent());
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
