#pragma once

#include "MsgPipe.h"

class CPlayerServiceBase
{
public:
	//
	// Player service interface
	//
	bool Create();
	void Destroy();

	virtual void OnGetInfo(PlayerInfo &playerInfo) = 0;

	virtual void OnConnected() = 0;
	virtual void OnDisconnect() = 0;

	virtual void Play() = 0;
	virtual void Pause() = 0;
	virtual void PlayPause() = 0;
	virtual void Stop() = 0;
	virtual void Prev() = 0;
	virtual void Next() = 0;
	virtual void SeekTo(int nMsPos) = 0;

	virtual void SwitchToEmbeddedSkin() { }
	virtual void EndEmbeddedSkin() { }

public:
	enum
	{
		TIMER_GETPOS				= 1,
		TIMER_NEXT					= 10,
		TIME_OUT_UPDATE_POS			= 50,
		TIME_OUT_UPDATE_MEDIA_INFO	= 500,
	};

	CPlayerServiceBase(void);
	~CPlayerServiceBase(void);
	
protected:
	static LPVOID ThreadMsg(LPVOID param);
	
	bool OnMsg(uint32 msg, LPCVOID lpData, size_t lenData);

protected:
	CMsgPipe			m_msgPipe;

};
