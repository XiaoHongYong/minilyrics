
#pragma once

class CPlayerServiceBase;

class IPlayerPluginAgent
{
public:
	IPlayerPluginAgent(CPlayerServiceBase *pPlayerServiceBase)
		{ m_pPlayerService = pPlayerServiceBase; }
	virtual ~IPlayerPluginAgent() { }

	virtual bool IsPlayerRunning() = 0;

	virtual bool TurnOn();
	virtual void TurnOff();

	virtual bool IsConnected();

protected:
	CPlayerServiceBase			*m_pPlayerService;

};

typedef vector<IPlayerPluginAgent*>	VecPlayerPluginAgent;