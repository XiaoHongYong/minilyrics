//
//  PlayerEventDispatcher.m
//  MiniLyricsMac
//
//  Created by Xiao Hongyong on 8/1/13.
//
//

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

CPlayerEventDispatcher::CPlayerEventDispatcher()
{
	m_nTimeOutUpdateLyr = 40;
	m_pInternal = [[_PlayerEventDispatcherInternal alloc] init];
}

void CPlayerEventDispatcher::StartLyrDrawUpdate()
{
	[(_PlayerEventDispatcherInternal*)m_pInternal StartLyrDrawUpdate];
}

void CPlayerEventDispatcher::StopLyrDrawUpdate()
{
	[(_PlayerEventDispatcherInternal*)m_pInternal StopLyrDrawUpdate];
}
