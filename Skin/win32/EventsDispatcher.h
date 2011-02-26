#pragma once

class CEventsDispatcher : public CEventsDispatcherBase, public CBaseWnd
{
public:
	int Init();
	void Quit();

	virtual void DispatchSyncEventByNoUIThread(IEvent *pEvent);
	virtual void DispatchUnsyncEvent(IEvent *pEvent);

protected:
	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

};
