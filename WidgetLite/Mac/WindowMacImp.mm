//
//  WindowMacImp.m
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "StdAfx.h"
#import "WidgetLite.h"
#import "WindowMacImp.h"

@implementation WindowMacImp

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle
				  backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag {
	self = [super initWithContentRect:contentRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
	if (self != nil) {
		// [self setAlphaValue:1.0];
		[self setOpaque:NO];
	}
	return self;
}

- (void) mouseDown:(NSEvent *)theEvent {
	mInitPt = [theEvent locationInWindow];
}

- (void)mouseDragged:(NSEvent *)theEvent {
	NSRect screenVisibleFrame = [[NSScreen mainScreen] visibleFrame];
	NSRect windowFrame = [self frame];
	NSPoint newOrigin = windowFrame.origin;

	// Get the mouse location in window coordinates.
	NSPoint currentLocation = [theEvent locationInWindow];
	// Update the origin with the difference between the new mouse location and the old mouse location.
	newOrigin.x += (currentLocation.x - mInitPt.x);
	newOrigin.y += (currentLocation.y - mInitPt.y);

	// Don't let window get dragged up under the menu bar
	if ((newOrigin.y + windowFrame.size.height) > (screenVisibleFrame.origin.y + screenVisibleFrame.size.height)) {
		newOrigin.y = screenVisibleFrame.origin.y + (screenVisibleFrame.size.height - windowFrame.size.height);
	}

	// Move the window to the new location
	[self setFrameOrigin:newOrigin];
}

- (void)setTimer:(int)idTimer duration:(int)duration {
	[self killTimer:idTimer];
	
	NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:duration
													  target:self
													selector:@selector(onTimer)
													userInfo:[NSNumber numberWithInt:idTimer]
													 repeats:YES];

	mMapTimer[idTimer] = timer;
}

- (void)killTimer:(int)idTimer {
	MapNSTimer::iterator it = mMapTimer.find(idTimer);
	
	if (it != mMapTimer.end()) {
		NSTimer *timer = (*it).second;
		[timer invalidate];
		mMapTimer.erase(it);
	}
}

- (void)onTimer:(NSTimer*)theTimer {
	NSNumber *idTimer = [theTimer userInfo];
	
	mBaseWnd->OnTimer([idTimer intValue]);
}

- (void)setOwnerBaseWnd:(CBaseWnd*)baseWnd {
	mBaseWnd = baseWnd;
}

- (void)dealloc {
	for (MapNSTimer::iterator it = mMapTimer.begin(); it != mMapTimer.end(); ++it) {
		NSTimer *timer = (*it).second;
		[timer invalidate];
	}
	mMapTimer.clear();

	[super dealloc];
}

- (void)onUserMsg:(NSArray*)msg {
	NSNumber *msgId = [msg objectAtIndex:0];
	NSNumber *param = [msg objectAtIndex:1];
	mBaseWnd->OnUserMessage([msgId intValue], (LPARAM)[param longValue]);
}

@end
