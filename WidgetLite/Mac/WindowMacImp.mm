//
//  WindowMacImp.m
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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

@end
