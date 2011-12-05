#include "StdAfx.h"
#import <Foundation/Foundation.h>
#include "EventsDispatcherBase.h"
#include "EventsDispatcher.h"

@interface EventsDispatchItem : NSObject {
	IEvent		*m_pEvent;
	CEventsDispatcher *m_pDispacher;
}

- (id) initWithEvent:(IEvent *)pEvent dispatcher:(CEventsDispatcher *)pDispacher;

- (void) dispatch:(id)param;

@end

@implementation EventsDispatchItem

- (id) initWithEvent:(IEvent *)pEvent dispatcher:(CEventsDispatcher *)pDispacher {
	m_pEvent = pEvent;
	m_pDispacher = pDispacher;
	return self;
}

- (void) dispatch:(id)param {
	m_pDispacher->DispatchSyncEvent(m_pEvent);
}

@end

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	EventsDispatchItem *dispatchItem = [[EventsDispatchItem alloc] initWithEvent:pEvent
																	  dispatcher:this];

	[dispatchItem performSelectorOnMainThread:@selector(dispatch)
									 withObject:nil
								  waitUntilDone:true];
}

void CEventsDispatcher::DispatchUnsyncEvent(IEvent *pEvent)
{
	EventsDispatchItem *dispatchItem = [[EventsDispatchItem alloc] initWithEvent:pEvent
																	  dispatcher:this];
	
	[dispatchItem performSelectorOnMainThread:@selector(dispatch)
								   withObject:nil
								waitUntilDone:false];
}
