//
//  ViewMacImp.h
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

class CBaseWnd;

@interface ViewMacImp : NSView {
    NSImage *circleImage;
    NSImage *pentagonImage;
    BOOL showingPentagon;
	
	CBaseWnd		*mBaseWnd;
}

- (void)setOwnerBaseWnd:(CBaseWnd*)baseWnd;

@end
