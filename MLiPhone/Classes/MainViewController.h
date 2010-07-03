
#define PLAYER_TYPE_PREF_KEY @"player_type_preference"
#define AUDIO_TYPE_PREF_KEY @"audio_technology_preference"

#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import "MusicTableViewController.h"
#import "MiniLyricsAppDelegate.h"
#import "LyricsShowView.h"
#import "LyrVecticalScrollView.h"
#import "MLApp.h"

@interface MainViewController : UIViewController 
	<MPMediaPickerControllerDelegate, MusicTableViewControllerDelegate, AVAudioPlayerDelegate, LyrViewDelegate, MLAppEventDelegate> {

	MiniLyricsAppDelegate			*applicationDelegate;
	IBOutlet UIButton			*addMusicButton;
	IBOutlet UINavigationBar	*navigationBar;
	IBOutlet UILabel			*nowPlayingLabel;
	BOOL						playedMusicOnce;
		
		
	// Top play and lyrics display (font size ...)
	IBOutlet UIView				*advancedCtrlView;
	IBOutlet UIButton			*repeatButton;
	IBOutlet UIButton			*shuffleButton;
	IBOutlet UISlider			*playingSlider;
	IBOutlet UISlider			*volumeSlider;
	IBOutlet UILabel			*playedTimeLable;
	IBOutlet UILabel			*leftTimeLable;
	NSTimer						*playingProgressTimer;
	double						mediaLength;

	IBOutlet UIScrollView		*scrollLyrView;
	LyricsShowView				*lyrView;
	
	IBOutlet LyrVecticalScrollView *lyrVerticalView;

	IBOutlet UIToolbar			*mPlayCtrlToolbar;
	NSMutableArray				*mPlayingButtons;
	NSArray						*mPausedButtons;
	
	MPMusicPlayerController		*musicPlayer;	
	MPMediaItemCollection		*userMediaItemCollection;

	MLApp						*mlApp;

}

@property (nonatomic, retain)	UIButton				*addMusicButton;
@property (nonatomic, retain)	UINavigationBar			*navigationBar;
@property (nonatomic, retain)	UILabel					*nowPlayingLabel;
@property (readwrite)			BOOL					playedMusicOnce;

@property (nonatomic, retain)	UIView					*advancedCtrlView;
@property (nonatomic, retain)	UIButton				*repeatButton;
@property (nonatomic, retain)	UIButton				*shuffleButton;
@property (nonatomic, retain)	UISlider				*playingSlider;
@property (nonatomic, retain)	UISlider				*volumeSlider;
@property (nonatomic, retain)	UILabel					*playedTimeLable;
@property (nonatomic, retain)	UILabel					*leftTimeLable;
@property (nonatomic, retain)	NSTimer					*playingProgressTimer;
@property (readwrite)			double					mediaLength;


@property (nonatomic, retain)	UIScrollView			*scrollLyrView;
@property (nonatomic, retain)	LyricsShowView			*lyrView;
@property (nonatomic, retain)	LyrVecticalScrollView	*lyrVerticalView;

@property (nonatomic, retain)	UIToolbar				*mPlayCtrlToolbar;
@property (nonatomic, retain)	NSArray					*mPausedButtons;
@property (nonatomic, retain)	NSMutableArray			*mPlayingButtons;

@property (nonatomic, retain)	MPMediaItemCollection	*userMediaItemCollection; 
@property (nonatomic, retain)	MPMusicPlayerController	*musicPlayer;

@property (nonatomic, retain)	MLApp					*mlApp;

- (IBAction)	playOrPauseMusic:		(id) sender;
- (IBAction)	rewindMusic:			(id) sender;
- (IBAction)	forwardMusic:			(id) sender;
- (IBAction)	ShowMusic:				(id) sender;
- (IBAction)	ShowLyrSearchView:		(id) sender;
- (IBAction)	shuffle:				(id) sender;
- (IBAction)	repeat:					(id) sender;
- (IBAction)	playingPosSeek:			(id) sender;
- (IBAction)	volumeSeek:				(id) sender;

- (BOOL) useiPodPlayer;

@end
