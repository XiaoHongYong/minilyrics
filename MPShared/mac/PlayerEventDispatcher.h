
#pragma once


///////////////////////////////////////////////////////////////////////////////
// create window to get playback pos of player timely.

class CPlayerEventDispatcher
{
public:
	CPlayerEventDispatcher();
	~CPlayerEventDispatcher() { }
	
	void Init()
	{
	}
	
	void Quit()
	{
	}
	
	long GetMasterVolume()
	{
		return 0;
	}
	
	void SetMasterVolume(long volume)
	{
	}

	void SetLyrDrawUpdateFast(bool bFast)
	{
		if (bFast)
			m_nTimeOutUpdateLyr = 20;
		else
			m_nTimeOutUpdateLyr = 40;
		
		StartLyrDrawUpdate();
	}
	
	void StartLyrDrawUpdate();
	
	void StopLyrDrawUpdate();

protected:
	int			m_nTimeOutUpdateLyr;
	void *		m_pInternal;
	
};

extern CPlayerEventDispatcher		g_playerEventDispatcher;
