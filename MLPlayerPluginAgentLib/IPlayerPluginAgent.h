
#pragma once

class CPlayerServiceBase;

class IPlayerPluginAgent
{
public:
	virtual bool IsPlayerRunning() = 0;

	virtual bool TurnOn() = 0;
	virtual void TurnOff() = 0;

	virtual bool IsConnected() = 0;

protected:
	bool OnTurnOn(CPlayerServiceBase *pPlayerService);
	void OnTurnOff(CPlayerServiceBase *pPlayerService);

};

typedef vector<IPlayerPluginAgent*>	VecPlayerPluginAgent;