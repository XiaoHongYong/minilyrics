//
//  main.m
//  MiniLyricsTest
//
//  Created by Hongyong Xiao on 10/20/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "CMLApp.h"
#import "../../../base/cppunit-utils.h"

int main(int argc, char *argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	RunUnitTest();

	int retVal = UIApplicationMain(argc, argv, nil, nil);
	[pool release];
	return retVal;
}
