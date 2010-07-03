

#import "MiniLyricsAppDelegate.h"
#import "MainViewController.h"

@implementation MiniLyricsAppDelegate

@synthesize window, mainViewController;


- (void) applicationDidFinishLaunching: (UIApplication *) application {

	[window addSubview: [mainViewController view]];
    [window makeKeyAndVisible];
}


- (void)dealloc {

    [window release];
    [super dealloc];
}


@end
