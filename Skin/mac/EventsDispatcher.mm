#include "StdAfx.h"
#import <Foundation/Foundation.h>
#import <AppKit/NSApplication.h>
#include "EventsDispatcherBase.h"
#include "EventsDispatcher.h"
#include "Skin.h"

@interface EventsDispatchItem : NSObject {
	IEvent		*m_pEvent;
	CEventsDispatcher *m_pDispacher;
}

- (id) initWithEvent:(IEvent *)pEvent dispatcher:(CEventsDispatcher *)pDispacher;

- (void) dispatch;

@end

@implementation EventsDispatchItem

- (id) initWithEvent:(IEvent *)pEvent dispatcher:(CEventsDispatcher *)pDispacher {
	m_pEvent = pEvent;
	m_pDispacher = pDispacher;
	return self;
}

- (void) dispatch {
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
	[dispatchItem release];
}

void CEventsDispatcher::DispatchUnsyncEvent(IEvent *pEvent)
{
	EventsDispatchItem *dispatchItem = [[EventsDispatchItem alloc] initWithEvent:pEvent
																	  dispatcher:this];
	
	[dispatchItem performSelectorOnMainThread:@selector(dispatch)
								   withObject:nil
								waitUntilDone:false];
	[dispatchItem release];
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

@interface CustomCmdPoster : NSObject {
	CSkinWnd		*mSkinWnd;
	int mCmdId;
	
}

- (void) OnCustomCommand;
- (id) init:(CSkinWnd *)pWnd cmd:(int)cmdId;

@end

@implementation CustomCmdPoster

- (void) OnCustomCommand {
	mSkinWnd->OnCustomCommand(mCmdId);
}

- (id) init:(CSkinWnd *)pWnd cmd:(int)cmdId {
	mSkinWnd = pWnd;
	mCmdId = cmdId;
	return self;
}

@end

void PostCustomCommandMsgMac(CSkinWnd *pSkinWnd, int cmd)
{
	CustomCmdPoster *poster = [[CustomCmdPoster alloc] init:pSkinWnd cmd:cmd];
	[poster performSelectorOnMainThread:@selector(OnCustomCommand)
							 withObject:nil
						  waitUntilDone:NO];
	[poster release];
}

void PostQuitMessageMac()
{
	[NSApp performSelectorOnMainThread:@selector(terminate:) withObject:nil waitUntilDone:NO];
//	[NSApp terminate:nil];
}


