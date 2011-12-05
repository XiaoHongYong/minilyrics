#include "StdAfx.h"
#include "EventsDispatcherBase.h"

#define MAX_EVENT_ID		100

IEventHandler::~IEventHandler()
{
	if (m_pEventDispatcher)
		m_pEventDispatcher->UnRegisterHandler(this);
}

void IEventHandler::RegisterHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1)
{
	m_pEventDispatcher = pEventDispatcher;
	m_pEventDispatcher->RegisterHandler(ev1, this);
}

void IEventHandler::RegisterHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1, EventType ev2)
{
	m_pEventDispatcher = pEventDispatcher;
	m_pEventDispatcher->RegisterHandler(ev1, this);
	m_pEventDispatcher->RegisterHandler(ev2, this);
}

void IEventHandler::RegisterHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1, EventType ev2, EventType ev3)
{
	m_pEventDispatcher = pEventDispatcher;
	m_pEventDispatcher->RegisterHandler(ev1, this);
	m_pEventDispatcher->RegisterHandler(ev2, this);
	m_pEventDispatcher->RegisterHandler(ev3, this);
}

void IEventHandler::RegisterHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1, EventType ev2, EventType ev3, EventType ev4)
{
	m_pEventDispatcher = pEventDispatcher;
	m_pEventDispatcher->RegisterHandler(ev1, this);
	m_pEventDispatcher->RegisterHandler(ev2, this);
	m_pEventDispatcher->RegisterHandler(ev3, this);
	m_pEventDispatcher->RegisterHandler(ev4, this);
}

void IEventHandler::UnregisterHandler()
{
	if (m_pEventDispatcher)
	{
		m_pEventDispatcher->UnRegisterHandler(this);
		m_pEventDispatcher = NULL;
	}
}

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

void CEventsDispatcherBase::RegisterHandler(EventType eventType, IEventHandler *pHandler)
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

void CEventsDispatcherBase::UnRegisterHandler(EventType eventType, IEventHandler *pHandler)
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

void CEventsDispatcherBase::UnRegisterHandler(IEventHandler *pHandler)
{
	CMutexAutolock		lock(m_mutex);

	for (int i = 0; i < m_vListEventHandler.size(); i++)
	{
		LIST_EVENTHANDLER	&listHandler = m_vListEventHandler[i];
		LIST_EVENTHANDLER::iterator	it, itEnd;

		itEnd = listHandler.end();
		for (it = listHandler.begin(); it != itEnd; ++it)
		{
			if (*it == pHandler)
			{
				listHandler.erase(it);
				break;
			}
		}
	}
}

void CEventsDispatcherBase::DispatchSyncEvent(IEvent *pEvent)
{
	EventType	eventType = pEvent->eventType;

	if (eventType >= m_vListEventHandler.size())
	{
		DBGLOG1(_T("No Event Handler for: %d"), (int)eventType);
		// ASSERT(0 && "No handler for this event");
		delete pEvent;
		return;
	}

	CMutexAutolock		lock(m_mutex);
	LIST_EVENTHANDLER		&listHandler = m_vListEventHandler[eventType];

	for (LIST_EVENTHANDLER::iterator it = listHandler.begin(); it != listHandler.end(); ++it)
	{
		IEventHandler	*pHandler = *it;
		pHandler->OnEvent(pEvent);
	}

	delete pEvent;
}
