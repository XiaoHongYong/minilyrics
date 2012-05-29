//
//  ViewMacImp.m
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "stdafx.h"
#import "ViewMacImp.h"
#import "WidgetLite.h"

@implementation ViewMacImp

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
		circleImage = [NSImage imageNamed:@"circle"];
		pentagonImage = [NSImage imageNamed:@"pentagon"];
    }
    
    return self;
}

- (void)dealloc {
    [circleImage release];
    [pentagonImage release];
    [super dealloc];
}

/*
 When it's time to draw, this routine is called. This view is inside the window, the window's opaqueness has been turned off, and the window's styleMask has been set to NSBorderlessWindowMask on creation, so this view draws the all the visible content. The first two lines below fill the view with "clear" color, so that any images drawn also define the custom shape of the window.  Furthermore, if the window's alphaValue is less than 1.0, drawing will use transparency.
 */
- (void)drawRect:(NSRect)rect {
    // Clear the drawing rect.
    [[NSColor clearColor] set];
    NSRectFill([self frame]);
    // A boolean tracks the previous shape of the window. If the shape changes, it's necessary for the
    // window to recalculate its shape and shadow.
/*    BOOL shouldDisplayWindow = NO;
    // If the window transparency is > 0.7, draw the circle, otherwise, draw the pentagon. 
    if ([[self window] alphaValue] > 0.7) {
        shouldDisplayWindow = (showingPentagon == YES);
        showingPentagon = NO;
        [circleImage compositeToPoint:NSZeroPoint operation:NSCompositeSourceOver];
    } else {
        shouldDisplayWindow = (showingPentagon == NO);
        showingPentagon = YES;
        [pentagonImage compositeToPoint:NSZeroPoint operation:NSCompositeSourceOver];
    }
    // Reset the window shape and shadow.
    if (shouldDisplayWindow) {
        [[self window] display];
        [[self window] setHasShadow:NO];
        [[self window] setHasShadow:YES];
    }*/

	CMLGraphics * graph = mBaseWnd->GetGraphics();
	CRect rcClip;
	NSRectToRect(rect, rcClip);

	mBaseWnd->OnPaint(graph, rcClip);

	mBaseWnd->ReleaseGraphics(graph);
}

- (void)setOwnerBaseWnd:(CBaseWnd*)baseWnd {
	mBaseWnd = baseWnd;
}

- (void)keyDown:(NSEvent *)theEvent {
	mBaseWnd->OnKeyDown([theEvent keyCode], 0);
	
	int keyCode = [theEvent keyCode];
	if (keyCode == NSUpArrowFunctionKey) {
		keyCode = 1;
	} else if (keyCode == NSPageUpFunctionKey) {
		keyCode = 1;
	} else if (keyCode == NSF1FunctionKey) {
		keyCode = 1;
	}
}

///////////////////////////////////////////////////////////////////////////
// View messages

/*- (void)viewWillStartLiveResize {
	NSRect rc = [self frame];
	mBaseWnd->OnSize(rc.size.width, rc.size.height);
}*/

- (void)viewDidEndLiveResize {
	NSRect rc = [self frame];
	mBaseWnd->OnSize(rc.size.width, rc.size.height);
}

@end
