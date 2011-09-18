#pragma once

typedef uint		EventType;

class IEvent
{
public:
	virtual ~IEvent() {}
	EventType		eventType;
	tstring			strName;
	tstring			strValue;
};

class IEventHandler
{
public:
	virtual ~IEventHandler() { }
	virtual void OnEvent(const IEvent *pEvent) = 0;

};

class CEventsDispatcherBase
{
public:
	CEventsDispatcherBase();
	virtual ~CEventsDispatcherBase();

	virtual int Init();
	virtual void Quit();

	virtual void RegisterHanlder(EventType eventType, IEventHandler *pHandler);
	virtual void UnregisterHanlder(EventType eventType, IEventHandler *pHandler);
	virtual void UnregisterHanlder(IEventHandler *pHandler);

	// CEventsDispatcher will delete pEvent, after pass through it to all handlers.
	void DispatchSyncEvent(IEvent *pEvent);
	void DispatchSyncEvent(EventType eventType)
	{
		IEvent		*pEvent = new IEvent;
		pEvent->eventType = eventType;
		DispatchSyncEvent(pEvent);
	}

	virtual void DispatchSyncEventByNoUIThread(IEvent *pEvent) = 0;
	virtual void DispatchUnsyncEvent(IEvent *pEvent) = 0;
	void DispatchUnsyncEvent(EventType eventType)
	{
		IEvent		*pEvent = new IEvent;
		pEvent->eventType = eventType;
		DispatchUnsyncEvent(pEvent);
	}

protected:
	typedef list<IEventHandler *>			LIST_EVENTHANDLER;
	typedef vector<LIST_EVENTHANDLER>		VecListEventHandler;
	VecListEventHandler		m_vListEventHandler;
	Mutex					m_mutex;

};
