//
//  main.m
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/13/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "stdafx.h"
#import "MPlayerApp.h"

int main(int argc, char *argv[])
{
	InitBaseFrameWork(argc, argv, "ZikiPlayer.log", "ZikiPlayer.ini", SZ_SECT_UI);

	return NSApplicationMain(argc, (const char **)argv);
}
