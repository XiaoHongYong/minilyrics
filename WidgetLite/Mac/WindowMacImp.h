//
//  WindowMacImp.h
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>

typedef map<int, NSTimer *>	MapNSTimer;

class CBaseWnd;

@interface WindowMacImp : NSWindow <NSWindowDelegate> {
	NSPoint			mInitPt;
	
	MapNSTimer		mMapTimer;
	CBaseWnd		*mBaseWnd;
}

- (void)setTimer:(int)idTimer duration:(int)duration;
- (void)killTimer:(int)idTimer;

- (void)onTimer:(NSTimer*)theTimer;

- (void)setOwnerBaseWnd:(CBaseWnd*)baseWnd;

- (void)onUserMsg:(NSArray*)msg ;

@end
