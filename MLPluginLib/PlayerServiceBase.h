#pragma once

class CPlayerServiceBase : public CTinyBaseWnd
{
public:
	//
	// Player service interface
	//

	virtual void OnGetInfo(PlayerInfo &playerInfo) = 0;

	virtual void OnConnectedDone() = 0;
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
		TIMER_CHECK_MEDIA			= 2,
		TIMER_NEXT					= 10,
		TIME_OUT_UPDATE_POS			= 50,
		TIME_OUT_UPDATE_MEDIA_INFO	= 500,
	};

	CPlayerServiceBase(void);
	~CPlayerServiceBase(void);

	bool Create(HINSTANCE hInstance)
	{
		return CTinyBaseWnd::Create(hInstance, SZ_PLUGIN_SERVICE_CLASS_NAME);
	}

	virtual bool OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds);

};
