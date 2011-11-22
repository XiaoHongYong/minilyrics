//
//  BaseWnd.cpp
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/15/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "BaseWndInc.h"
#import "BaseWnd.h"
#import "WindowMacImp.h"
#import "ViewMacImp.h"

struct _BaseWndInfo
{
	WindowMacImp		*window;
	ViewMacImp			*view;
};

CBaseWnd::CBaseWnd()
{
	m_info = new _BaseWndInfo();
}

CBaseWnd::~CBaseWnd()
{
	delete m_info;
}

bool CBaseWnd::Create(int x, int y, int nWidth, int nHeight)
{
	NSRect frame = NSMakeRect(0, 0, 200, 200);
	WindowMacImp* w = [[WindowMacImp alloc] initWithContentRect:frame
											  styleMask:NSBorderlessWindowMask
												backing:NSBackingStoreBuffered
												  defer:NO];
	
	ViewMacImp* view = [[ViewMacImp alloc] initWithFrame:frame];
	[w setContentView:view];

	[w setBackgroundColor:[NSColor blueColor]];
	[w orderFront:NSApp];

	m_info->window = w;

	return true;
}
