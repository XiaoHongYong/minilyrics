//
//  ViewMacImp.h
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

class CBaseWnd;

inline void NSRectToRect(const NSRect &nsrc, RECT &rc) {
	rc.left = nsrc.origin.x;
	rc.top = nsrc.origin.y;
	rc.right = rc.left + nsrc.size.width;
	rc.bottom = rc.top + nsrc.size.height;
}

inline NSRect NSMakeRect(const RECT &rc) {
	NSRect nsrc;
	nsrc.origin.x = rc.left;
	nsrc.origin.y = rc.top;
	nsrc.size.width = rc.right - rc.left;
	nsrc.size.height = rc.bottom - rc.top;
	
	return nsrc;
}

@interface ViewMacImp : NSView {
	CBaseWnd		*mBaseWnd;
}

- (void)setOwnerBaseWnd:(CBaseWnd*)baseWnd;

@end
