

#import <UIKit/UIKit.h>

@class MainViewController;

@interface MiniLyricsAppDelegate : NSObject <UIApplicationDelegate> {

    UIWindow					*window;
	IBOutlet MainViewController	*mainViewController;
}

@property (nonatomic, retain) IBOutlet UIWindow				*window;
@property (nonatomic, retain) IBOutlet MainViewController	*mainViewController;

@end

