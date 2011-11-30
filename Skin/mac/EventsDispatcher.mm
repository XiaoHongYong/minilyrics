#include "StdAfx.h"
#include "EventsDispatcherBase.h"
#include "EventsDispatcher.h"

int CEventsDispatcher::Init()
{

	return CEventsDispatcherBase::Init();
}

void CEventsDispatcher::Quit()
{
	CEventsDispatcherBase::Quit();

}

void CEventsDispatcher::DispatchSyncEventByNoUIThread(IEvent *pEvent)
{
	NSArray *msg = [NSArray arrayWithObjects:[NSNumber numberWithInt:nMessageID], [NSNumber numberWithLong:param], nil];
	
	[m_info->window performSelectorOnMainThread:@selector(onUserMsg)
									 withObject:msg
								  waitUntilDone:false];

	::SendMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
}

void CEventsDispatcher::DispatchUnsyncEvent(IEvent *pEvent)
{
	::PostMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
}
