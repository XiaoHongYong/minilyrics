//
//  PlayerEventDispatcher.m
//  MiniLyricsMac
//
//  Created by Xiao Hongyong on 8/1/13.
//
//
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import "PlayerEventDispatcher.h"

@interface _PlayerEventDispatcherInternal : NSObject
{
	NSTimer *timer;
}

@end

#import "stdafx.h"
#import "Player.h"
#import "MPlayerApp.h"

@implementation _PlayerEventDispatcherInternal

-(void)StartLyrDrawUpdate
{
	timer = [NSTimer scheduledTimerWithTimeInterval:((double)40) / 1000
													  target:self
													selector:@selector(onTimer)
													userInfo:nil
													 repeats:YES];
}

-(void)StopLyrDrawUpdate
{
	if (timer != nil)
	{
		[timer invalidate];
		timer = nil;
	}
}

-(void)onTimer
{
	if (g_Player.IsUseSeekTimeAsPlayingTime())
		return;
	
	int nPlayPos = g_Player.GetPlayPos();
	g_LyricData.SetPlayElapsedTime(nPlayPos);
	
	CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);
	
	DispatchPlayPosEvent(nPlayPos);
}

@end

struct InternalInfoHolder {
    _PlayerEventDispatcherInternal      *eventDispatcher;
};

CPlayerEventDispatcher::CPlayerEventDispatcher()
{
	m_nTimeOutUpdateLyr = 40;
    m_pInternal = new InternalInfoHolder();
    m_pInternal->eventDispatcher = [[_PlayerEventDispatcherInternal alloc] init];
}

CPlayerEventDispatcher::~CPlayerEventDispatcher() {
    delete m_pInternal;
}

void CPlayerEventDispatcher::StartLyrDrawUpdate()
{
	[m_pInternal->eventDispatcher StartLyrDrawUpdate];
}

void CPlayerEventDispatcher::StopLyrDrawUpdate()
{
	[m_pInternal->eventDispatcher StopLyrDrawUpdate];
}
