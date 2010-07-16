
#import "MainViewController.h"
#import <Foundation/Foundation.h>
#import "LyricsSearchViewController.h"

#import "../../base/mlstdafx.h"
#import "../../base/base.h"
#import "../../Utils/Utils.h"
#import "../../OurMFCLib/OurMFClib.h"
#import "../../gfx/gfx.h"
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

@synthesize advancedCtrlView;
@synthesize repeatButton;
@synthesize shuffleButton;
@synthesize playingSlider;
@synthesize volumeSlider;
@synthesize playedTimeLable;
@synthesize leftTimeLable;
@synthesize playingProgressTimer;
@synthesize mediaLength;


@synthesize lyrView;
@synthesize scrollLyrView;
@synthesize lyrVerticalView;

@synthesize	mPlayCtrlToolbar;
@synthesize mPlayingButtons;
@synthesize mPausedButtons;

@synthesize mlApp;


#pragma mark Music control________________________________

// A toggle control for playing or pausing iPod library music playback, invoked
//		when the user taps the 'playBarButton' in the Navigation bar. 
- (IBAction) playOrPauseMusic: (id)sender {

	MPMusicPlaybackState playbackState = [musicPlayer playbackState];

	if (playbackState == MPMusicPlaybackStateStopped || playbackState == MPMusicPlaybackStatePaused) {
		[musicPlayer play];
	} else if (playbackState == MPMusicPlaybackStatePlaying) {
		[musicPlayer pause];
	}
}

- (IBAction)	rewindMusic:			(id) sender
{
	[musicPlayer skipToPreviousItem];
}

- (IBAction)	forwardMusic:			(id) sender
{
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
	CMLApp::GetInstance()->GetLyricsData().SetPlayElapsedTime(int([playingSlider value] * 1000));
	// [musicPlayer setCurrentPlaybackTime: [playingSlider value]];
}

- (IBAction)	volumeSeek:				(id) sender
{
	[musicPlayer setVolume: [volumeSlider value]];
}

- (IBAction)	forwardLyrics:			(id) sender
{
	CMLData &lyrData = CMLApp::GetInstance()->GetLyricsData();
	lyrData.SetOffsetTime(lyrData.GetOffsetTime() + 500);
}

- (IBAction)	backwardLyrics:			(id) sender
{
	CMLData &lyrData = CMLApp::GetInstance()->GetLyricsData();
	lyrData.SetOffsetTime(lyrData.GetOffsetTime() - 500);
}

// If there is no selected media item collection, display the media item picker. If there's 
// already a selected collection, display the list of selected songs.
- (IBAction) ShowMusic: (id) sender {
	if (self.userMediaItemCollection == nil ||
		[[self.userMediaItemCollection items] count] == 0)
	{
		MPMediaPickerController *picker =
		[[MPMediaPickerController alloc] initWithMediaTypes: MPMediaTypeMusic];
		
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
	if (mediaItemCollection) {

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
	}
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
		[mPlayCtrlToolbar setItems:mPausedButtons];
	} else if (playbackState == MPMusicPlaybackStatePlaying) {
		[mPlayCtrlToolbar setItems:mPlayingButtons];

	} else if (playbackState == MPMusicPlaybackStateStopped) {
	
		[mPlayCtrlToolbar setItems:mPausedButtons];

		// Even though stopped, invoking 'stop' ensures that the music player will play  
		//		its queue from the start.
		[musicPlayer stop];

	}

	[lyrVerticalView onPlaybackStateChanged:playbackState];
}

// When volume is changed
- (void) handle_VolumeChanged: (id) notification {
	
	[volumeSlider setValue: [musicPlayer volume]];
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

	[notificationCenter addObserver: self
						   selector: @selector (handle_VolumeChanged:)
							   name: MPMusicPlayerControllerVolumeDidChangeNotification
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


// To learn about the Settings bundle and user preferences, see User Defaults Programming Topics
//		for Cocoa and "The Settings Bundle" in iPhone Application Programming Guide 

// Returns whether or not to use the iPod music player instead of the application music player.
- (BOOL) useiPodPlayer {

	return YES;
	if ([[NSUserDefaults standardUserDefaults] boolForKey: PLAYER_TYPE_PREF_KEY]) {
		return YES;		
	} else {
		return NO;
	}
}

// Configure the application.
- (void) viewDidLoad {

    [super viewDidLoad];

	[self setPlayingProgressTimer: [NSTimer scheduledTimerWithTimeInterval: 0.5
															   target: self
															 selector: @selector (onUpdatePlayingProgress)
															 userInfo: nil
															  repeats: YES]];	
	
	[self setPlayedMusicOnce: NO];
	
	// Create play and pause buttons.
	[self setMPausedButtons: mPlayCtrlToolbar.items];
	[self setMPlayingButtons: [[NSMutableArray alloc] initWithArray:mPausedButtons]];

	// Create paused bar button
	UIImage *pausedButtonImage = [UIImage imageNamed: @"pause.png"];
	UIBarButtonItem *pauseBarButton = [[UIBarButtonItem alloc]	initWithImage:pausedButtonImage
																	  style:UIBarButtonItemStylePlain
																	  target:self
																	  action:@selector (playOrPauseMusic:)];
	[mPlayingButtons replaceObjectAtIndex:3 withObject:pauseBarButton];
	
	[mPlayCtrlToolbar setItems:mPlayingButtons];
	[mPlayCtrlToolbar setItems:mPausedButtons];

	// Instantiate the music player. If you specied the iPod music player in the Settings app,
	//		honor the current state of the built-in iPod app.
	if ([self useiPodPlayer]) {
	
		[self setMusicPlayer: [MPMusicPlayerController iPodMusicPlayer]];
		
		if ([musicPlayer nowPlayingItem]) {
		
			navigationBar.topItem.leftBarButtonItem.enabled = YES;
			
			// Update the UI to reflect the now-playing item. 
			[self handle_NowPlayingItemChanged: nil];
			
			if ([musicPlayer playbackState] == MPMusicPlaybackStatePaused) {
				[mPlayCtrlToolbar setItems:mPausedButtons];
			}
		}
		
	} else {
	
		[self setMusicPlayer: [MPMusicPlayerController applicationMusicPlayer]];
		
		// By default, an application music player takes on the shuffle and repeat modes
		//		of the built-in iPod app. Here they are both turned off.
		[musicPlayer setShuffleMode: MPMusicShuffleModeOff];
		[musicPlayer setRepeatMode: MPMusicRepeatModeNone];
	}

	[self registerForMediaPlayerNotifications];

	mlApp = [[MLApp alloc] init:musicPlayer];
	[mlApp setDelegate:self];


/*	CGRect rcLyrView = scrollLyrView.bounds;
	rcLyrView.size.height = 500;

	[self setLyrView:[[LyricsShowView alloc] initWithFrame:rcLyrView]];
	
	[lyrView setDelegate:self];
	[lyrView setMusicPlayer:self.musicPlayer];

	[scrollLyrView setBackgroundColor:[UIColor blackColor]];
	[scrollLyrView setCanCancelContentTouches:NO];
	scrollLyrView.clipsToBounds = YES;	// default is NO, we want to restrict drawing within our scrollview
	scrollLyrView.indicatorStyle = UIScrollViewIndicatorStyleWhite;
	// UIImageView *imageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"image0.jpg"]];
	[scrollLyrView addSubview:lyrView];
	// [scrollLyrView setContentSize:CGSizeMake(imageView.frame.size.width, imageView.frame.size.height)];
	[scrollLyrView setScrollEnabled:YES];
	// [imageView release];
	
	// Reset the scroll view to 1.0 zoom
	scrollLyrView.contentSize = CGSizeMake(rcLyrView.size.width, rcLyrView.size.height);
	

	[lyrView onLyricsChanged];*/


	[scrollLyrView setHidden:TRUE];

	[lyrVerticalView setMusicPlayer:self.musicPlayer];

	[lyrVerticalView InitLyrView];

	UIPinchGestureRecognizer *pinchGesture = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(scaleLyrics:)];
	[pinchGesture setDelegate:self];
	[lyrVerticalView addGestureRecognizer:pinchGesture];
	[pinchGesture release];

	UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(hideAdvancedCtrl:)];
	[tapGesture setDelegate:self];
	[lyrVerticalView addGestureRecognizer:tapGesture];
	[tapGesture release];

	UIPanGestureRecognizer *panGesture = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(moveLyrics:)];
	[panGesture setMaximumNumberOfTouches:2];
	[panGesture setDelegate:self];
	[lyrVerticalView addGestureRecognizer:panGesture];
	[panGesture release];

/*    UILongPressGestureRecognizer *longPressGesture = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(showResetMenu:)];
    [piece addGestureRecognizer:longPressGesture];
    [longPressGesture release];*/

	if (!g_Profile.GetProfileBool(_T("HideAdvCtrl"), TRUE))
		[advancedCtrlView setHidden:NO];
	
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

	// Set repeat button status
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
	// Set volume slider images
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

	[volumeSlider setThumbImage: sliderBall forState:UIControlStateNormal];
	[volumeSlider setThumbImage: sliderBallPressed forState:UIControlStateHighlighted];
	[volumeSlider setThumbImage: sliderBallPressed forState:UIControlStateSelected];
	[volumeSlider setMinimumTrackImage:stetchLeftTrack forState:UIControlStateNormal];
	[volumeSlider setMaximumTrackImage:stetchRightTrack forState:UIControlStateNormal];	
	
	[volumeSlider setMinimumValue: 0.0];
	[volumeSlider setMaximumValue: 1.0];
	[volumeSlider setValue: [musicPlayer volume]];
	
}

bool ScaleLyricsFont(float scale);

// scale the piece by the current scale
// reset the gesture recognizer's rotation to 0 after applying so the next callback is a delta from the current scale
- (void)scaleLyrics:(UIPinchGestureRecognizer *)gestureRecognizer
{
    // [self adjustAnchorPointForGestureRecognizer:gestureRecognizer];
    
    if ([gestureRecognizer state] == UIGestureRecognizerStateBegan || [gestureRecognizer state] == UIGestureRecognizerStateChanged) {
		
		if (!ScaleLyricsFont([gestureRecognizer scale]))
			return;

		if ([gestureRecognizer view] == self.lyrVerticalView)
			[lyrVerticalView reloadFontSettings];

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

	CGPoint translation = [gestureRecognizer translationInView:lyrVerticalView];

	if ([gestureRecognizer state] == UIGestureRecognizerStateBegan) {
		[gestureRecognizer setTranslation:CGPointZero inView:lyrVerticalView];
	}
	else if ([gestureRecognizer state] == UIGestureRecognizerStateChanged) {
		[lyrVerticalView moveLyrics: translation.y];
		[gestureRecognizer setTranslation:CGPointZero inView:lyrVerticalView];
	}
	else if ([gestureRecognizer state] == UIGestureRecognizerStateEnded) {
	}
}

// ensure that the pinch, pan and rotate gesture recognizers on a particular view can all recognize simultaneously
// prevent other gesture recognizers from recognizing simultaneously
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    // if the gesture recognizers's view isn't one of our pieces, don't allow simultaneous recognition
    if (gestureRecognizer.view != [self lyrVerticalView])
        return NO;
    
    // if the gesture recognizers are on different views, don't allow simultaneous recognition
    if (gestureRecognizer.view != otherGestureRecognizer.view)
        return NO;
    
    // if either of the gesture recognizers is the long press, don't allow simultaneous recognition
    if ([gestureRecognizer isKindOfClass:[UILongPressGestureRecognizer class]] || [otherGestureRecognizer isKindOfClass:[UILongPressGestureRecognizer class]])
        return NO;
    
    return YES;
}

extern "C" NSString *formatPlayingTime(int n);

// Invoked by the playingProgressTimer.
- (void) onUpdatePlayingProgress {
	if (!lyrVerticalView.inSeekingLyrics) {
		int currentPlaybackTime = (int)([musicPlayer currentPlaybackTime] + 0.5);

		[playedTimeLable setText:
		 [NSString stringWithFormat:@"%@", formatPlayingTime(currentPlaybackTime)]];
		[leftTimeLable setText:
		 [NSString stringWithFormat:@"-%@", 
		  formatPlayingTime(int(self.mediaLength - [musicPlayer currentPlaybackTime] + 0.5))]];

		[playingSlider setValue: currentPlaybackTime];
	}
}

#pragma mark MLApp event Delegate

- (void)onLyricsChanged
{
	[lyrVerticalView onLyricsChanged];
}

- (void)onMessage:(NSString*) msg
{
	[titleLabel setText:msg];
}

- (void)onPopupLyrSearchView
{
}

#pragma mark LyricsShowView Delegate

- (void)LyrContentHeightChanged:(LyricsShowView *)lyrShowView contentHeight:(int)height
{
	CGSize size = scrollLyrView.bounds.size;
	size.height = height;
	[scrollLyrView setContentSize:size];

	[scrollLyrView setNeedsDisplay];
}

#pragma mark Application state management_____________

- (void) didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}


- (void) viewDidUnload {
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
	
	[mPlayCtrlToolbar release];
	[mPausedButtons release];
	[mPlayCtrlToolbar release];

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
	[volumeSlider release];
	[playedTimeLable release];
	[leftTimeLable release];
	[playingProgressTimer release];

    [super dealloc];
}


@end
