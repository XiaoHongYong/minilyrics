#include "StdAfx.h"
#include "EventsDispatcherBase.h"

#define MAX_EVENT_ID		100

//////////////////////////////////////////////////////////////////////

CEventsDispatcherBase::CEventsDispatcherBase()
{

}

CEventsDispatcherBase::~CEventsDispatcherBase()
{

}

int CEventsDispatcherBase::Init()
{
	return ERR_OK;
}

void CEventsDispatcherBase::Quit()
{
	CMutexAutolock		lock(m_mutex);

	m_vListEventHandler.clear();
}

void CEventsDispatcherBase::RegisterHanlder(EventType eventType, IEventHandler *pHandler)
{
	ASSERT(eventType >= 0 && eventType <= MAX_EVENT_ID);
	CMutexAutolock		lock(m_mutex);

	if (eventType >= m_vListEventHandler.size())
		m_vListEventHandler.resize(eventType + 1);

	LIST_EVENTHANDLER		&listHandler = m_vListEventHandler[eventType];
	LIST_EVENTHANDLER::iterator	it, itEnd;

	itEnd = listHandler.end();
	for (it = listHandler.begin(); it != itEnd; ++it)
	{
		if (*it == pHandler)
			return;
	}
	listHandler.push_back(pHandler);
}

void CEventsDispatcherBase::UnregisterHanlder(EventType eventType, IEventHandler *pHandler)
{
	ASSERT(eventType >= 0 && eventType <= MAX_EVENT_ID);
	if (eventType >= m_vListEventHandler.size())
		return;

	CMutexAutolock		lock(m_mutex);
	LIST_EVENTHANDLER	&listHandler = m_vListEventHandler[eventType];
	LIST_EVENTHANDLER::iterator	it, itEnd;

	itEnd = listHandler.end();
	for (it = listHandler.begin(); it != itEnd; ++it)
	{
		if (*it == pHandler)
		{
			listHandler.erase(it);
			return;
		}
	}
}

void CEventsDispatcherBase::DispatchSyncEvent(IEvent *pEvent)
{
	EventType	eventType = pEvent->eventType;

	if (eventType < 0 || eventType >= m_vListEventHandler.size())
	{
		ASSERT(0 && "No handler for this event");
		delete pEvent;
		return;
	}

	CMutexAutolock		lock(m_mutex);
	LIST_EVENTHANDLER		&listHandler = m_vListEventHandler[eventType];
	LIST_EVENTHANDLER::iterator	it, itEnd;

	itEnd = listHandler.end();
	for (it = listHandler.begin(); it != itEnd; ++it)
	{
		IEventHandler	*pHandler = *it;
		pHandler->OnEvent(pEvent);
	}

	delete pEvent;
}
