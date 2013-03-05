
#import "MainViewController.h"
#import <Foundation/Foundation.h>
#import "LyricsSearchViewController.h"

#import "../../base/base.h"
#import "../../Utils/Utils.h"
#import "../../MLLib/MLLib.h"

#import "CMLApp.h"

@implementation MainViewController

@synthesize userMediaItemCollection;	// the media item collection created by the user, using the media item picker	
@synthesize musicPlayer;				// the music player, which plays media items from the iPod library
@synthesize addMusicButton;
@synthesize navigationBar;				// the application's Navigation bar
@synthesize playedMusicOnce;			// A flag indicating if the user has played iPod library music at least one time
										//		since application launch.
@synthesize artistLabel;
@synthesize titleLabel;
@synthesize albumLabel;
@synthesize messageLabel;
@synthesize searchingIndicator;

@synthesize advancedCtrlView;
@synthesize repeatButton;
@synthesize shuffleButton;
@synthesize playingSlider;
@synthesize playedTimeLable;
@synthesize leftTimeLable;
@synthesize playingProgressTimer;
@synthesize mediaLength;


@synthesize albumArtImage;
@synthesize staticLyrView;
@synthesize lyrVerticalView;

@synthesize	playPauseButton;

@synthesize mlApp;
@synthesize threadInit;

#ifdef _FREE
@synthesize banner;
#endif // #ifdef _FREE

#pragma mark Music control________________________________

extern "C" NSString *formatPlayingTime(int n);

- (void) updatePlaybackTime:(int)currentPlaybackTime
{
	[playedTimeLable setText:
	 [NSString stringWithFormat:@"%@", formatPlayingTime(currentPlaybackTime)]];
	[leftTimeLable setText:
	 [NSString stringWithFormat:@"-%@", 
	  formatPlayingTime(int(self.mediaLength - [musicPlayer currentPlaybackTime] + 0.5))]];
}

// A toggle control for playing or pausing iPod library music playback, invoked
//		when the user taps the 'playBarButton' in the Navigation bar. 
- (IBAction) playOrPauseMusic: (id)sender {
	if (self.userMediaItemCollection == nil) {
		[self ShowMusic:sender];
		return;
	}

	MPMusicPlaybackState playbackState = [musicPlayer playbackState];

	if (playbackState == MPMusicPlaybackStateStopped || playbackState == MPMusicPlaybackStatePaused) {
		[musicPlayer play];
	} else if (playbackState == MPMusicPlaybackStatePlaying) {
		[musicPlayer pause];
	}
}

- (IBAction)	rewindMusic:			(id) sender
{
	if (self.userMediaItemCollection == nil) {
		[self ShowMusic:sender];
		return;
	}
	
	[musicPlayer skipToPreviousItem];
}

- (IBAction)	forwardMusic:			(id) sender
{
	if (self.userMediaItemCollection == nil) {
		[self ShowMusic:sender];
		return;
	}
	
	[musicPlayer skipToNextItem];
}

- (IBAction)	shuffle:				(id) sender
{
	MPMusicShuffleMode shuffleMode = [musicPlayer shuffleMode];
	UIImage *imgShuffle;

	if (shuffleMode == MPMusicShuffleModeOff) {
		shuffleMode = MPMusicShuffleModeSongs;
		imgShuffle = [UIImage imageNamed:@"shuffle_on.png"];
	}
	else {
		shuffleMode = MPMusicShuffleModeOff;
		imgShuffle = [UIImage imageNamed:@"shuffle_off.png"];
	}

	[musicPlayer setShuffleMode: shuffleMode];
	[shuffleButton setImage:imgShuffle forState:UIControlStateNormal];
}

- (IBAction)	repeat:					(id) sender
{
	MPMusicRepeatMode repeatMode = [musicPlayer repeatMode];
	UIImage *imgRepeat;
	
	if (repeatMode == MPMusicRepeatModeNone) {
		repeatMode = MPMusicRepeatModeAll;
		imgRepeat = [UIImage imageNamed:@"repeat_on.png"];
	}
	else if (repeatMode == MPMusicRepeatModeAll) {
		repeatMode = MPMusicRepeatModeOne;
		imgRepeat = [UIImage imageNamed:@"repeat_on_1.png"];
	}
	else {
		repeatMode = MPMusicRepeatModeNone;
		imgRepeat = [UIImage imageNamed:@"repeat_off.png"];
	}
	
	[musicPlayer setRepeatMode: repeatMode];
	[repeatButton setImage:imgRepeat forState:UIControlStateNormal];
}

- (IBAction)	beginSeek:				(id) sender
{
	lyrVerticalView.inSeekingLyrics = YES;
}

- (IBAction)	endSeek:				(id) sender
{
	if (lyrVerticalView.inSeekingLyrics) {
		lyrVerticalView.inSeekingLyrics = NO;
		[musicPlayer setCurrentPlaybackTime: [playingSlider value]];
	}
}

- (IBAction)	playingPosSeek:			(id) sender
{
	int playbackTime = int(([playingSlider value] +  + 0.5) * 1000);
	CMLApp::GetInstance()->GetLyricsData().SetPlayElapsedTime(playbackTime);
	[self updatePlaybackTime:playbackTime / 1000];
}

- (IBAction)	forwardLyrics:			(id) sender
{
	CMLData &lyrData = CMLApp::GetInstance()->GetLyricsData();
	lyrData.SetOffsetTime(lyrData.GetOffsetTime() - 500);
}

- (IBAction)	backwardLyrics:			(id) sender
{
	CMLData &lyrData = CMLApp::GetInstance()->GetLyricsData();
	lyrData.SetOffsetTime(lyrData.GetOffsetTime() + 500);
}

// If there is no selected media item collection, display the media item picker. If there's 
// already a selected collection, display the list of selected songs.
- (IBAction) ShowMusic: (id) sender {
	if (self.userMediaItemCollection == nil ||
		[[self.userMediaItemCollection items] count] == 0)
	{
		MPMediaPickerController *picker =
		[[MPMediaPickerController alloc] initWithMediaTypes: MPMediaTypeMusic];
		if (picker == nil) {
			ERRLOG0("Failed to alloc MPMediaPickerController");
			return;
		}
		
		picker.delegate						= self;
		picker.allowsPickingMultipleItems	= YES;
		picker.prompt						= NSLocalizedString (@"Add songs to play", "Prompt in media item picker");
		
		// The media item picker uses the default UI style, so it needs a default-style
		//		status bar to match it visually
		[[UIApplication sharedApplication] setStatusBarStyle: UIStatusBarStyleDefault animated: YES];
		
		picker.modalTransitionStyle = UIModalTransitionStyleCrossDissolve;
		[self presentModalViewController: picker animated: YES];
		[picker release];
		
		UIImage	*imgPlaylist = [UIImage imageNamed:@"NowPlayingAlbumInfo.png"];
		[addMusicButton setImage:imgPlaylist forState:UIControlStateNormal];
	}
	else {
		MusicTableViewController *controller = [[MusicTableViewController alloc] initWithNibName: @"MusicTableView" bundle: nil];
		controller.delegate = self;
		controller.musicPlayer = musicPlayer;
		controller.modalTransitionStyle = UIModalTransitionStyleCrossDissolve;
		
		[self presentModalViewController: controller animated: YES];
		[controller release];
	}
}

- (IBAction) ShowLyrSearchView: (id) sender {
	LyricsSearchViewController *controller = [[LyricsSearchViewController alloc] initWithNibName: @"LyricsSearchView" bundle: nil];
	controller.modalTransitionStyle = UIModalTransitionStyleCrossDissolve;
	controller.parentViewController = self;
	
	[self presentModalViewController: controller animated: YES];
	[controller release];
}

// Invoked by the delegate of the media item picker when the user is finished picking music.
//		The delegate is either this class or the table view controller, depending on the 
//		state of the application.
- (void) updatePlayerQueueWithMediaCollection: (MPMediaItemCollection *) mediaItemCollection {

	// Configure the music player, but only if the user chose at least one song to play
	if (!mediaItemCollection) {

		[self setUserMediaItemCollection:nil];
		
		[musicPlayer stop];

		// Apply the new media item collection as a playback queue for the music player.
		[musicPlayer setQueueWithItemCollection: userMediaItemCollection];
		
		// Restore the now-playing item and its current playback time.
		musicPlayer.nowPlayingItem			= nil;
		return;
	}

	// If there's no playback queue yet...
	if (userMediaItemCollection == nil) {
	
		// apply the new media item collection as a playback queue for the music player
		[self setUserMediaItemCollection: mediaItemCollection];
		[musicPlayer setQueueWithItemCollection: userMediaItemCollection];
		[self setPlayedMusicOnce: YES];
		[musicPlayer play];

	// Obtain the music player's state so it can then be
	//		restored after updating the playback queue.
	} else {

		// Take note of whether or not the music player is playing. If it is
		//		it needs to be started again at the end of this method.
		BOOL wasPlaying = NO;
		if (musicPlayer.playbackState == MPMusicPlaybackStatePlaying) {
			wasPlaying = YES;
		}
		
		// Save the now-playing item and its current playback time.
		MPMediaItem *nowPlayingItem			= musicPlayer.nowPlayingItem;
		NSTimeInterval currentPlaybackTime	= musicPlayer.currentPlaybackTime;

		// Combine the previously-existing media item collection with the new one
		NSMutableArray *combinedMediaItems	= [[userMediaItemCollection items] mutableCopy];
		NSArray *newMediaItems				= [mediaItemCollection items];
		[combinedMediaItems addObjectsFromArray: newMediaItems];
		
		[self setUserMediaItemCollection: [MPMediaItemCollection collectionWithItems: (NSArray *) combinedMediaItems]];
		[combinedMediaItems release];

		// Apply the new media item collection as a playback queue for the music player.
		[musicPlayer setQueueWithItemCollection: userMediaItemCollection];
		
		// Restore the now-playing item and its current playback time.
		musicPlayer.nowPlayingItem			= nowPlayingItem;
		musicPlayer.currentPlaybackTime		= currentPlaybackTime;
		
		// If the music player was playing, get it playing again.
		if (wasPlaying) {
			[musicPlayer play];
		}
	}
	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
														 NSUserDomainMask, YES);
	NSString *documentPath = [paths objectAtIndex:0];
	NSString *mediaFile = [documentPath stringByAppendingPathComponent:@"playlist.plist"];

	NSMutableData *data = [[NSMutableData alloc] init];
	NSKeyedArchiver *archiver = [[NSKeyedArchiver alloc] initForWritingWithMutableData:data];

	[archiver encodeObject:self.userMediaItemCollection forKey:@"playlist"];
	
	[archiver finishEncoding];
	BOOL sucess = [data writeToFile:mediaFile atomically:YES];
	if (!sucess)
		ERRLOG0("Failed to save playlist.");
	[archiver release];
	[data release];

/*	stringex buff;
	if (OpenFileToBuffer([mediaFile UTF8String], buff))
	{
		NSArray *items = [NSArray arrayWithContentsOfFile:mediaFile];
		if (items)
			self.userMediaItemCollection = [[MPMediaItemCollection alloc] initWithItems:items];
	}*/
}

// If the music player was paused, leave it paused. If it was playing, it will continue to
//		play on its own. The music player state is "stopped" only if the previous list of songs
//		had finished or if this is the first time the user has chosen songs after app 
//		launch--in which case, invoke play.
- (void) restorePlaybackState {

	if (musicPlayer.playbackState == MPMusicPlaybackStateStopped && userMediaItemCollection) {

		if (playedMusicOnce == NO) {
		
			[self setPlayedMusicOnce: YES];
			[musicPlayer play];
		}
	}

}



#pragma mark Media item picker delegate methods________

// Invoked when the user taps the Done button in the media item picker after having chosen
//		one or more media items to play.
- (void) mediaPicker: (MPMediaPickerController *) mediaPicker didPickMediaItems: (MPMediaItemCollection *) mediaItemCollection {
  
	// Dismiss the media item picker.
	[self dismissModalViewControllerAnimated: YES];
	
	// Apply the chosen songs to the music player's queue.
	[self updatePlayerQueueWithMediaCollection: mediaItemCollection];

	[[UIApplication sharedApplication] setStatusBarStyle: UIStatusBarStyleBlackOpaque animated: YES];
}

// Invoked when the user taps the Done button in the media item picker having chosen zero
//		media items to play
- (void) mediaPickerDidCancel: (MPMediaPickerController *) mediaPicker {

	[self dismissModalViewControllerAnimated: YES];
	
	[[UIApplication sharedApplication] setStatusBarStyle: UIStatusBarStyleBlackOpaque animated: YES];
}



#pragma mark Music notification handlers__________________

// When the now-playing item changes, update the media item artwork and the now-playing label.
- (void) handle_NowPlayingItemChanged: (id) notification {

	MPMediaItem *currentItem = [musicPlayer nowPlayingItem];
	
	[artistLabel setText: [currentItem valueForProperty: MPMediaItemPropertyArtist]];
	[albumLabel setText: [currentItem valueForProperty: MPMediaItemPropertyAlbumTitle]];
	[titleLabel setText: [currentItem valueForProperty: MPMediaItemPropertyTitle]];
	
	if (musicPlayer.playbackState == MPMusicPlaybackStateStopped) {
		// Provide a suitable prompt to the user now that their chosen music has 
		//		finished playing.
		[artistLabel setText: @""];
		[albumLabel setText: @""];
		[titleLabel setText: @""];
	}
	
	NSNumber	*length = [currentItem valueForProperty: MPMediaItemPropertyPlaybackDuration];
	[self setMediaLength: [length doubleValue]];
	
	[playingSlider setMaximumValue: (float)[length doubleValue]];
	[playingSlider setMinimumValue: 0.0];
	
	[self onUpdatePlayingProgress];
	
	[mlApp onMediaChanged];
}

// When the playback state changes, set the play/pause button in the Navigation bar
//		appropriately.
- (void) handle_PlaybackStateChanged: (id) notification {

	MPMusicPlaybackState playbackState = [musicPlayer playbackState];
	
	if (playbackState == MPMusicPlaybackStatePaused) {
		UIImage *pauseImage = [UIImage imageNamed:@"play.png"];
		[playPauseButton setImage:pauseImage forState:UIControlStateNormal];
	} else if (playbackState == MPMusicPlaybackStatePlaying) {
		UIImage *playImage = [UIImage imageNamed:@"pause.png"];
		[playPauseButton setImage:playImage forState:UIControlStateNormal];
	} else if (playbackState == MPMusicPlaybackStateStopped) {

		UIImage *playImage = [UIImage imageNamed:@"play.png"];
		[playPauseButton setImage:playImage forState:UIControlStateNormal];

		// Even though stopped, invoking 'stop' ensures that the music player will play  
		//		its queue from the start.
		[musicPlayer stop];

	}

	[lyrVerticalView onPlaybackStateChanged:playbackState];
}

- (void) handle_iPodLibraryChanged: (id) notification {

	// Implement this method to update cached collections of media items when the 
	// user performs a sync while your application is running. This sample performs 
	// no explicit media queries, so there is nothing to update.
}

#pragma mark Table view delegate methods________________

// Invoked when the user taps the Done button in the table view.
- (void) musicTableViewControllerDidFinish: (MusicTableViewController *) controller {
	
	[self dismissModalViewControllerAnimated: YES];
	[self restorePlaybackState];
}



#pragma mark Application setup____________________________

#if TARGET_IPHONE_SIMULATOR
#warning *** Simulator mode: iPod library access works only when running on a device.
#endif

// To learn about notifications, see "Notifications" in Cocoa Fundamentals Guide.
- (void) registerForMediaPlayerNotifications {

	NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];

	[notificationCenter addObserver: self
						   selector: @selector (handle_NowPlayingItemChanged:)
							   name: MPMusicPlayerControllerNowPlayingItemDidChangeNotification
							 object: musicPlayer];
	
	[notificationCenter addObserver: self
						   selector: @selector (handle_PlaybackStateChanged:)
							   name: MPMusicPlayerControllerPlaybackStateDidChangeNotification
							 object: musicPlayer];

/*
	// This sample doesn't use libray change notifications; this code is here to show how
	//		it's done if you need it.
	[notificationCenter addObserver: self
						   selector: @selector (handle_iPodLibraryChanged:)
							   name: MPMediaLibraryDidChangeNotification
							 object: musicPlayer];
	
	[[MPMediaLibrary defaultMediaLibrary] beginGeneratingLibraryChangeNotifications];
*/

	[musicPlayer beginGeneratingPlaybackNotifications];
}

- (void) registerGestrueRecognizer:(UIView *)view
{
	UIPinchGestureRecognizer *pinchGesture = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(scaleLyrics:)];
	[pinchGesture setDelegate:self];
	[view addGestureRecognizer:pinchGesture];
	[pinchGesture release];
	
	UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(hideAdvancedCtrl:)];
	[tapGesture setDelegate:self];
	[view addGestureRecognizer:tapGesture];
	[tapGesture release];
}

#ifdef _FREE

-(void)createADBannerView
{
	// --- WARNING ---
	// If you are planning on creating banner views at runtime in order to support iOS targets that don't support the iAd framework
	// then you will need to modify this method to do runtime checks for the symbols provided by the iAd framework
	// and you will need to weaklink iAd.framework in your project's target settings.
	// See the iPad Programming Guide, Creating a Universal Application for more information.
	// http://developer.apple.com/iphone/library/documentation/general/conceptual/iPadProgrammingGuide/Introduction/Introduction.html
	// --- WARNING ---

	// Depending on our orientation when this method is called, we set our initial content size.
	// If you only support portrait or landscape orientations, then you can remove this check and
	// select either ADBannerContentSizeIdentifier320x50 (if portrait only) or ADBannerContentSizeIdentifier480x32 (if landscape only).
	NSString *contentSize = UIInterfaceOrientationIsPortrait(self.interfaceOrientation) ? ADBannerContentSizeIdentifier320x50 : ADBannerContentSizeIdentifier480x32;

	// Calculate the intial location for the banner.
	// We want this banner to be at the bottom of the view controller, but placed
	// offscreen to ensure that the user won't see the banner until its ready.
	// We'll be informed when we have an ad to show because -bannerViewDidLoadAd: will be called.
	CGRect frame;
	frame.size = [ADBannerView sizeFromBannerContentSizeIdentifier:contentSize];
	frame.origin = CGPointMake(0.0, CGRectGetMaxY(self.view.bounds));

	// Now to create and configure the banner view
	ADBannerView *bannerView = [[ADBannerView alloc] initWithFrame:frame];
	// Set the delegate to self, so that we are notified of ad responses.
	bannerView.delegate = self;
	// Set the autoresizing mask so that the banner is pinned to the bottom
	bannerView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleTopMargin;
	// Since we support all orientations in this view controller, support portrait and landscape content sizes.
	// If you only supported landscape or portrait, you could remove the other from this set.
	bannerView.requiredContentSizeIdentifiers = [NSSet setWithObjects:ADBannerContentSizeIdentifier320x50, ADBannerContentSizeIdentifier480x32, nil];

	// At this point the ad banner is now be visible and looking for an ad.
	[self.view addSubview:bannerView];
	self.banner = bannerView;
	[bannerView release];
}

-(void)layoutForCurrentOrientation:(BOOL)animated
{
    CGFloat animationDuration = animated ? 0.2 : 0.0;
    // by default content consumes the entire view area
    CGRect contentFrame = self.view.bounds;
	contentFrame.size.height -= 96; // Ad is at the lyrics displaying area.
    // the banner still needs to be adjusted further, but this is a reasonable starting point
    // the y value will need to be adjusted by half the banner height to get the final position
    CGPoint bannerCenter = CGPointMake(CGRectGetMidX(contentFrame), CGRectGetMaxY(contentFrame));
    CGFloat bannerHeight = 0.0;
    
    // First, setup the banner's content size and adjustment based on the current orientation
    if(UIInterfaceOrientationIsLandscape(self.interfaceOrientation))
    {
        banner.currentContentSizeIdentifier = ADBannerContentSizeIdentifier480x32;
        bannerHeight = 32.0;
    }
    else
    {
        banner.currentContentSizeIdentifier = ADBannerContentSizeIdentifier320x50;
        bannerHeight = 50.0;
    }
    
    // Depending on if the banner has been loaded, we adjust the content frame and banner location
    // to accomodate the ad being on or off screen.
    // This layout is for an ad at the bottom of the view.
    if(banner.bannerLoaded)
    {
        contentFrame.size.height -= bannerHeight;
        bannerCenter.y -= bannerHeight / 2.0;
    }
    else
    {
        bannerCenter.y += bannerHeight / 2.0;
    }
    
    // And finally animate the changes, running layout for the content view if required.
/*    [UIView animateWithDuration:animationDuration
                     animations:^{
                         contentView.frame = contentFrame;
                         [contentView layoutIfNeeded];
                         
                         banner.center = bannerCenter;
                     }];*/
	banner.center = bannerCenter;
}

#endif // #ifdef _FREE

- (void) initThreadRoute {
	// Instantiate the music player. If you specied the iPod music player in the Settings app,
	//		honor the current state of the built-in iPod app.
	[self setMusicPlayer: [MPMusicPlayerController iPodMusicPlayer]];
	
	[self performSelectorOnMainThread:@selector(onInitThreadDone) withObject:nil waitUntilDone:false];
}

// Configure the application.
- (void) viewDidLoad {

    [super viewDidLoad];
	
#ifdef _FREE
	[self createADBannerView];
#endif // #ifdef _FREE

	[artistLabel setText:@""];
	[albumLabel setText:@""];
	[titleLabel setText:@""];
	
	// threadInit = [[NSThread alloc] initWithTarget:self selector:@selector(initThreadRoute) object:nil];
	// [threadInit start];
	[self initThreadRoute];
}

- (void) onInitThreadDone {
	[self setPlayingProgressTimer: [NSTimer scheduledTimerWithTimeInterval: 0.5
															   target: self
															 selector: @selector (onUpdatePlayingProgress)
															 userInfo: nil
															  repeats: YES]];	

	[self setPlayedMusicOnce: NO];

	if ([musicPlayer nowPlayingItem]) {
		// Update the UI to reflect the now-playing item. 
		[self handle_NowPlayingItemChanged: nil];

		if ([musicPlayer playbackState] == MPMusicPlaybackStatePaused) {
			[self handle_PlaybackStateChanged: nil];
		}
	}

	[self registerForMediaPlayerNotifications];

	//
	// Init MLApp.
	//
	mlApp = [[MLApp alloc] init:musicPlayer];
	[mlApp setDelegate:self];

	//
	// Init set static text lyrics view.
	//
	[staticLyrView InitLyrView];
	[staticLyrView onLyricsChanged];
	[staticLyrView setHidden:FALSE];

	//
	// Init set vertical lyrics view.
	//
	[lyrVerticalView setMusicPlayer:self.musicPlayer];
	[lyrVerticalView InitLyrView];
	[lyrVerticalView setHidden:TRUE];

	//
	// Register Gesture Recognizer
	//
	[self registerGestrueRecognizer:lyrVerticalView];
	[self registerGestrueRecognizer:staticLyrView];

/*	UIPanGestureRecognizer *panGesture = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(moveLyrics:)];
	[panGesture setMaximumNumberOfTouches:2];
	[panGesture setDelegate:self];
	[lyrVerticalView addGestureRecognizer:panGesture];
	[panGesture release];

    UILongPressGestureRecognizer *longPressGesture = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(showResetMenu:)];
    [piece addGestureRecognizer:longPressGesture];
    [longPressGesture release];*/

	//
	// Set shuffle button status
	//
	MPMusicShuffleMode shuffleMode = [musicPlayer shuffleMode];
	UIImage *imgShuffle;
	if (shuffleMode == MPMusicShuffleModeOff) {
		imgShuffle = [UIImage imageNamed:@"shuffle_off.png"];
	}
	else {
		imgShuffle = [UIImage imageNamed:@"shuffle_on.png"];
	}
	[shuffleButton setImage:imgShuffle forState:UIControlStateNormal];

	//
	// Set repeat button status
	//
	MPMusicRepeatMode repeatMode = [musicPlayer repeatMode];
	UIImage *imgRepeat;
	if (repeatMode == MPMusicRepeatModeNone) {
		imgRepeat = [UIImage imageNamed:@"repeat_off.png"];
	}
	else if (repeatMode == MPMusicRepeatModeAll) {
		imgRepeat = [UIImage imageNamed:@"repeat_on.png"];
	}
	else {
		imgRepeat = [UIImage imageNamed:@"repeat_on_1.png"];
	}
	[repeatButton setImage:imgRepeat forState:UIControlStateNormal];
	
	//
	// Set seekbar slider images
	//
	playingSlider.backgroundColor = [UIColor clearColor];	
	UIImage *stetchLeftTrack = [[UIImage imageNamed:@"playing_slider_left.png"]
								stretchableImageWithLeftCapWidth:10.0 topCapHeight:0.0];
	UIImage *stetchRightTrack = [[UIImage imageNamed:@"playing_slider_right.png"]
								 stretchableImageWithLeftCapWidth:10.0 topCapHeight:0.0];
	UIImage *sliderBall = [UIImage imageNamed:@"playing_slider_ball.png"];
	UIImage *sliderBallPressed = [UIImage imageNamed:@"playing_slider_ball.png"];
	[playingSlider setThumbImage: sliderBall forState:UIControlStateNormal];
	[playingSlider setThumbImage: sliderBallPressed forState:UIControlStateHighlighted];
	[playingSlider setThumbImage: sliderBallPressed forState:UIControlStateSelected];
	[playingSlider setMinimumTrackImage:stetchLeftTrack forState:UIControlStateNormal];
	[playingSlider setMaximumTrackImage:stetchRightTrack forState:UIControlStateNormal];

	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
														 NSUserDomainMask, YES);
	NSString *documentPath = [paths objectAtIndex:0];
	NSString *mediaFile = [documentPath stringByAppendingPathComponent:@"playlist.plist"];

	NSData *data = [[NSData alloc] initWithContentsOfFile:mediaFile];
	if (data) {
		NSKeyedUnarchiver *unarchiver = [[NSKeyedUnarchiver alloc] initForReadingWithData:data];
		self.userMediaItemCollection = [unarchiver decodeObjectForKey:@"playlist"];
		//MPMediaItem *currentItem = [musicPlayer nowPlayingItem];
		[musicPlayer setQueueWithItemCollection: userMediaItemCollection];
		//[musicPlayer setNowPlayingItem:currentItem];
		[unarchiver release];
		[data release];
	}

	if (self.userMediaItemCollection) {
		UIImage	*imgPlaylist = [UIImage imageNamed:@"NowPlayingAlbumInfo.png"];
		[addMusicButton setImage:imgPlaylist forState:UIControlStateNormal];
	}
}


bool ScaleLyricsFont(float scale)
{
	int nHeight, nWeight;
	BYTE byItalic;
	tstring strFaceNameLatin9, strFaceNameOthers;
	//ProfileGetLyricsFont(SZ_SECT_LYR_DISPLAY, nHeight, nWeight, byItalic, strFaceNameLatin9, strFaceNameOthers);
	
	nHeight = (int)(nHeight * scale);
	if (nHeight < 12 || nHeight > 35)
		return false;
	
	//ProfileWriteLyricsFont(SZ_SECT_LYR_DISPLAY, nHeight, nWeight, byItalic, strFaceNameLatin9.c_str(), strFaceNameOthers.c_str());
	
	return true;
}

// scale the piece by the current scale
// reset the gesture recognizer's rotation to 0 after applying so the next callback is a delta from the current scale
- (void)scaleLyrics:(UIPinchGestureRecognizer *)gestureRecognizer
{
    // [self adjustAnchorPointForGestureRecognizer:gestureRecognizer];
    
    if ([gestureRecognizer state] == UIGestureRecognizerStateBegan || [gestureRecognizer state] == UIGestureRecognizerStateChanged) {
		
		if (!ScaleLyricsFont([gestureRecognizer scale]))
			return;

		[lyrVerticalView reloadFontSettings];
		[staticLyrView reloadFontSettings];

        [gestureRecognizer setScale:1];
    }
}

- (void)hideAdvancedCtrl:(UITapGestureRecognizer *)gestureRecognizer
{
/*	CGPoint ptTap = [gestureRecognizer locationInView: lyrVerticalView];
	
	if ([advancedCtrlView pointInside:ptTap withEvent:nil])
		return;*/

	BOOL isHidden = ![advancedCtrlView isHidden];
	g_Profile.WriteProfileInt(_T("HideAdvCtrl"), isHidden);
	[advancedCtrlView setHidden:isHidden];
}

// shift the piece's center by the pan amount
// reset the gesture recognizer's translation to {0, 0} after applying so the next callback is a delta from the current position
- (void)moveLyrics:(UIPanGestureRecognizer *)gestureRecognizer
{
	// [self adjustAnchorPointForGestureRecognizer:gestureRecognizer];

/*	CGPoint translation = [gestureRecognizer translationInView:lyrVerticalView];

	if ([gestureRecognizer state] == UIGestureRecognizerStateBegan) {
		[gestureRecognizer setTranslation:CGPointZero inView:lyrVerticalView];
	}
	else if ([gestureRecognizer state] == UIGestureRecognizerStateChanged) {
		[lyrVerticalView moveLyrics: translation.y];
		[gestureRecognizer setTranslation:CGPointZero inView:lyrVerticalView];
	}
	else if ([gestureRecognizer state] == UIGestureRecognizerStateEnded) {
	}*/
}

// ensure that the pinch, pan and rotate gesture recognizers on a particular view can all recognize simultaneously
// prevent other gesture recognizers from recognizing simultaneously
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    // if the gesture recognizers's view isn't one of our pieces, don't allow simultaneous recognition
    if (gestureRecognizer.view != [self lyrVerticalView] || gestureRecognizer.view != [self staticLyrView])
        return NO;
    
    // if the gesture recognizers are on different views, don't allow simultaneous recognition
    if (gestureRecognizer.view != otherGestureRecognizer.view)
        return NO;
    
    // if either of the gesture recognizers is the long press, don't allow simultaneous recognition
    if ([gestureRecognizer isKindOfClass:[UILongPressGestureRecognizer class]] || [otherGestureRecognizer isKindOfClass:[UILongPressGestureRecognizer class]])
        return NO;
    
    return YES;
}

// Invoked by the playingProgressTimer.
- (void) onUpdatePlayingProgress {
	if (!lyrVerticalView.inSeekingLyrics) {
		int currentPlaybackTime = (int)([musicPlayer currentPlaybackTime] + 0.5);

		[self updatePlaybackTime:currentPlaybackTime];

		[playingSlider setValue: currentPlaybackTime];
	}
}

#pragma mark MLApp event Delegate

UIImage *getAlbumArtWithSize(MPMusicPlayerController *musicPlayer, int width, int height);


CGContextRef MyCreateBitmapContext(int pixelsWide, int pixelsHigh)
{
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	
	// create the bitmap context
	CGContextRef bitmapContext = CGBitmapContextCreate (NULL, pixelsWide, pixelsHigh, 8,
														0, colorSpace,
														// this will give us an optimal BGRA format for the device:
														(kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst));
	CGColorSpaceRelease(colorSpace);
	
    return bitmapContext;
}

- (UIImage *)createDarkImage:(UIImage *)fromImage
{
	// create a bitmap graphics context the size of the image
	CGContextRef mainViewContentContext = MyCreateBitmapContext(fromImage.size.width, fromImage.size.height);
	
	CGRect rect = CGRectMake(0, 0, fromImage.size.width, fromImage.size.height);
	// draw the image into the bitmap context
	CGContextDrawImage(mainViewContentContext,
					   rect,
					   fromImage.CGImage);

	// Highlight the "first" image from the DrawTiledImage call.
	CGContextSetRGBFillColor(mainViewContentContext, 0.3, 0.3, 0.3, 0.5);
	CGContextFillRect(mainViewContentContext, rect);
	
	// create CGImageRef of the main view bitmap content, and then release that bitmap context
	CGImageRef darkImage = CGBitmapContextCreateImage(mainViewContentContext);
	CGContextRelease(mainViewContentContext);
	
	// convert the finished reflection image to a UIImage 
	UIImage *theImage = [UIImage imageWithCGImage:darkImage];
	
	// image is retained by the property setting above, so we can release the original
	CGImageRelease(darkImage);
	
	return theImage;
}

- (void)onLyricsChanged
{
	[searchingIndicator setHidden:YES];
	[messageLabel setHidden:YES];
	
	CMLData &lyrData = CMLApp::GetInstance()->GetLyricsData();
	// if (lyrData.HasLyricsOpened() && lyrData.GetLyrContentType() == LCT_TXT)
	{
		// Static lyrics
		[staticLyrView onLyricsChanged];

		UIImage *imageDark = nil;
		UIImage *image = getAlbumArtWithSize(self.musicPlayer, albumArtImage.frame.size.width, albumArtImage.frame.size.height);
		if (image) {
			NSLog(@"AlbumArt image is set.");
			if (lyrData.HasLyricsOpened())
				image = [self createDarkImage:image];
			imageDark = image;
		}
		[albumArtImage setImage:imageDark];
		[albumArtImage setHidden:NO];

		[lyrVerticalView setHidden:NO];
		[staticLyrView setHidden:YES];
	}
	[lyrVerticalView onLyricsChanged];
/*	else
	{
		[lyrVerticalView onLyricsChanged];
		[albumArtImage setHidden:YES];
		[staticLyrView setHidden:YES];
		[lyrVerticalView setHidden:NO];
	}*/
}

- (void)onMessage:(NSString*)msg showActivityIndicator:(bool)bShow infoMsg:(bool)bInfoMsg
{
	if (CMLApp::GetInstance()->GetLyricsData().HasLyricsOpened()) {
		if (!bInfoMsg) {
			UIAlertView *alert = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Warning", @"x")
															message:msg
														   delegate:nil
												  cancelButtonTitle:NSLocalizedString(@"OK", @"x")
												  otherButtonTitles:nil];
			[alert show];
			[alert release];
		}
	}
	else {
		[messageLabel setText:msg];
		[messageLabel setHidden:NO];
		[messageLabel setNeedsDisplay];
		if (bShow)
			[searchingIndicator setHidden:NO];
		else
			[searchingIndicator setHidden:YES];
		[searchingIndicator setNeedsDisplay];
		[lyrVerticalView setNeedsDisplay];
		[self.view setNeedsDisplay];
	}
}

- (void)onPopupLyrSearchView
{
}

#pragma mark Application state management_____________

- (void) didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}


- (void) viewDidUnload {
#ifdef _FREE
    banner.delegate = nil;
#endif // #ifdef _FREE

	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
	CMLApp::GetInstance()->Quit();
}


- (void)dealloc {

/*
	// This sample doesn't use libray change notifications; this code is here to show how
	//		it's done if you need it.
	[[NSNotificationCenter defaultCenter] removeObserver: self
													name: MPMediaLibraryDidChangeNotification
												  object: musicPlayer];

	[[MPMediaLibrary defaultMediaLibrary] endGeneratingLibraryChangeNotifications];
	
*/
	[playingProgressTimer		invalidate];
	[playingProgressTimer		release];

	[[NSNotificationCenter defaultCenter] removeObserver: self
													name: MPMusicPlayerControllerNowPlayingItemDidChangeNotification
												  object: musicPlayer];
	
	[[NSNotificationCenter defaultCenter] removeObserver: self
													name: MPMusicPlayerControllerPlaybackStateDidChangeNotification
												  object: musicPlayer];

	[musicPlayer endGeneratingPlaybackNotifications];
	[musicPlayer				release];
	
	[playPauseButton release];

	[addMusicButton release];
	[navigationBar				release];
	[artistLabel				release];
	[albumLabel					release];
	[titleLabel					release];
	[userMediaItemCollection	release];

	[advancedCtrlView release];
	[repeatButton release];
	[shuffleButton release];
	[playingSlider release];
	[playedTimeLable release];
	[leftTimeLable release];
	[playingProgressTimer release];
	
	[threadInit release];

#ifdef _FREE
    [banner release];
#endif // #ifdef _FREE
	
    [super dealloc];
}


#pragma mark ADBannerViewDelegate methods

#ifdef _FREE
-(void)bannerViewDidLoadAd:(ADBannerView *)banner
{
    [self layoutForCurrentOrientation:YES];
}

-(void)bannerView:(ADBannerView *)banner didFailToReceiveAdWithError:(NSError *)error
{
    [self layoutForCurrentOrientation:YES];
}

-(BOOL)bannerViewActionShouldBegin:(ADBannerView *)banner willLeaveApplication:(BOOL)willLeave
{
    return YES;
}

-(void)bannerViewActionDidFinish:(ADBannerView *)banner
{
}
#endif // #ifdef _FREE

@end
