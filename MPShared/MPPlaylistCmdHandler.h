#pragma once

class CMPPlaylistCmdHandler : public ISkinCmdHandler  
{
public:
	CMPPlaylistCmdHandler();
	virtual ~CMPPlaylistCmdHandler();
	
	// if the command id is processed, return true.
	virtual bool OnCommand(int nId);
	virtual bool OnCustomCommand(int nId);
	virtual bool OnUIObjNotify(IUIObjNotify *pNotify);

};
