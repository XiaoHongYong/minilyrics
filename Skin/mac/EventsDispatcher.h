#pragma once

struct	_EVENT_DISPATCHER;

class CEventsDispatcher : public CEventsDispatcherBase
{
public:
	int Init();
	void Quit();

	virtual void DispatchSyncEventByNoUIThread(IEvent *pEvent);
	virtual void DispatchUnsyncEvent(IEvent *pEvent);

};

void PostCustomCommandMsgMac(class CSkinWnd *pSkinWnd, int cmd);

