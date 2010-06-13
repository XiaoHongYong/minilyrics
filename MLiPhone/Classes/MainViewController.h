/*
    File: MainViewController.h
Abstract: View controller class for MiniLyrics. Sets up user interface, responds 
to and manages user interaction.
 Version: 1.1

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may
be used to endorse or promote products derived from the Apple Software
without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or
implied, are granted by Apple herein, including but not limited to any
patent rights that may be infringed by your derivative works or by other
works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2009 Apple Inc. All Rights Reserved.

*/

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
	<MPMediaPickerControllerDelegate, MusicTableViewControllerDelegate, AVAudioPlayerDelegate, LyrViewDelegate> {

	MiniLyricsAppDelegate			*applicationDelegate;
	IBOutlet UIBarButtonItem	*artworkItem;
	IBOutlet UINavigationBar	*navigationBar;
	IBOutlet UILabel			*nowPlayingLabel;
	BOOL						playedMusicOnce;

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

@property (nonatomic, retain)	UIBarButtonItem			*artworkItem;
@property (nonatomic, retain)	UINavigationBar			*navigationBar;
@property (nonatomic, retain)	UILabel					*nowPlayingLabel;
@property (readwrite)			BOOL					playedMusicOnce;

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
- (IBAction)	MiniLyrics:				(id) sender;
- (IBAction)	ShowMusic:				(id) sender;

- (BOOL) useiPodPlayer;

@end
