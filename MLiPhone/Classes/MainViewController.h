
#define PLAYER_TYPE_PREF_KEY @"player_type_preference"
#define AUDIO_TYPE_PREF_KEY @"audio_technology_preference"

#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import "MusicTableViewController.h"
#import "MiniLyricsAppDelegate.h"
#import "LyrVecticalScrollView.h"
#import "MLApp.h"
#import "LyrStaticTextView.h"

#ifdef _FREE
#import <iAd/iAd.h>
#endif // #ifdef _FREE

@interface MainViewController : UIViewController 
	<MPMediaPickerControllerDelegate, MusicTableViewControllerDelegate, AVAudioPlayerDelegate,
	MLAppEventDelegate, UIGestureRecognizerDelegate
#ifdef _FREE
	, ADBannerViewDelegate
#endif // #ifdef _FREE
	> {

	MiniLyricsAppDelegate		*applicationDelegate;
	IBOutlet UIButton			*addMusicButton;
	IBOutlet UINavigationBar	*navigationBar;
	IBOutlet UILabel			*artistLabel;
	IBOutlet UILabel			*titleLabel;
	IBOutlet UILabel			*albumLabel;
	IBOutlet UILabel			*messageLabel;
	IBOutlet UIActivityIndicatorView *searchingIndicator;
	BOOL						playedMusicOnce;

	// Top play and lyrics display (font size ...)
	IBOutlet UIView				*advancedCtrlView;
	IBOutlet UIButton			*repeatButton;
	IBOutlet UIButton			*shuffleButton;
	IBOutlet UISlider			*playingSlider;
	IBOutlet UILabel			*playedTimeLable;
	IBOutlet UILabel			*leftTimeLable;
	IBOutlet UIImageView		*albumArtImage;
	NSTimer						*playingProgressTimer;
	double						mediaLength;

	IBOutlet LyrStaticTextView	*staticLyrView;
	
	IBOutlet LyrVecticalScrollView *lyrVerticalView;

	IBOutlet UIButton			*playPauseButton;
	
	MPMusicPlayerController		*musicPlayer;	
	MPMediaItemCollection		*userMediaItemCollection;

	MLApp						*mlApp;

#ifdef _FREE
	ADBannerView				*banner;
#endif // #ifdef _FREE

}

@property (nonatomic, retain)	UIButton				*addMusicButton;
@property (nonatomic, retain)	UINavigationBar			*navigationBar;
@property (readwrite)			BOOL					playedMusicOnce;
@property (nonatomic, retain)	UILabel					*artistLabel;
@property (nonatomic, retain)	UILabel					*titleLabel;
@property (nonatomic, retain)	UILabel					*albumLabel;
@property (nonatomic, retain)	UILabel					*messageLabel;
@property (nonatomic, retain)	UIActivityIndicatorView *searchingIndicator;

@property (nonatomic, retain)	UIView					*advancedCtrlView;
@property (nonatomic, retain)	UIButton				*repeatButton;
@property (nonatomic, retain)	UIButton				*shuffleButton;
@property (nonatomic, retain)	UISlider				*playingSlider;
@property (nonatomic, retain)	UILabel					*playedTimeLable;
@property (nonatomic, retain)	UILabel					*leftTimeLable;
@property (nonatomic, retain)	NSTimer					*playingProgressTimer;
@property (readwrite)			double					mediaLength;


@property (nonatomic, retain)	LyrStaticTextView		*staticLyrView;
@property (nonatomic, retain)	LyrVecticalScrollView	*lyrVerticalView;
@property (nonatomic, retain)	UIImageView				*albumArtImage;

@property (nonatomic, retain)	UIButton				*playPauseButton;

@property (nonatomic, retain)	MPMediaItemCollection	*userMediaItemCollection; 
@property (nonatomic, retain)	MPMusicPlayerController	*musicPlayer;

@property (nonatomic, retain)	MLApp					*mlApp;
@property (nonatomic, retain)	NSThread				*threadInit;

#ifdef _FREE
@property (nonatomic, retain)	ADBannerView			*banner;
#endif // #ifdef _FREE

- (IBAction)	playOrPauseMusic:		(id) sender;
- (IBAction)	rewindMusic:			(id) sender;
- (IBAction)	forwardMusic:			(id) sender;
- (IBAction)	ShowMusic:				(id) sender;
- (IBAction)	ShowLyrSearchView:		(id) sender;
- (IBAction)	shuffle:				(id) sender;
- (IBAction)	repeat:					(id) sender;
- (IBAction)	playingPosSeek:			(id) sender;
- (IBAction)	beginSeek:				(id) sender;
- (IBAction)	endSeek:				(id) sender;

- (IBAction)	forwardLyrics:			(id) sender;
- (IBAction)	backwardLyrics:			(id) sender;

- (void) initThreadRoute;
- (void) onInitThreadDone;
- (void) onUpdatePlayingProgress;

@end
